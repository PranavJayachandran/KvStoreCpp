#include "../config/Config.h"
#include "FileHandler.h"
#include "Memtable.h"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <ctime>
#include <filesystem>
#include <functional>
#include <iterator>
#include <optional>
#include <queue>
#include <string>
#include <unordered_set>

namespace kvstore::engine {
template <typename K = std::string, typename V = std::string> class SST {
private:
  class Data {
  public:
    K key;
    V value;
    bool is_deleted = false;
    std::string file_name;

    bool operator<(const Data &other) const {
      // In case the keys are same the, one which was added later is the more
      // correct one. File nane is utc now.
      if (other.key == key) {
        return file_name > other.file_name;
      }
      return key < other.key;
    }
  };

  struct Metadata {
    std::string file_name;
    std::string min_key;
    std::string max_key;
    std::size_t file_size;
  };

  const std::string directory_name = config::GetConfig().sst_dir;
  std::mutex compaction_mx;
  const int sst_key_block_size = config::GetConfig().sst_key_block_size;
  const int sst_value_block_size = config::GetConfig().sst_value_block_size;
  const std::vector<int> sst_number_of_files_per_level =
      config::GetConfig().sst_number_of_files_per_level;
  const std::vector<int> sst_size_of_file_per_level =
      config::GetConfig().sst_size_of_file_per_level;
  const int sst_number_of_levels = config::GetConfig().sst_level_count;
  const std::vector<std::string> sst_level_directories =
      config::GetConfig().sst_level_folders;

  std::vector<std::vector<Metadata>> sst_metadata;

  bool Search(const K &item, std::string &out_value) {
    std::lock_guard<std::mutex> lock(compaction_mx);
    for (int i = 0; i < sst_metadata.size(); i++) {
      int low = 0, high = sst_metadata[i].size() - 1;
      int entry_index = -1;
      while (low <= high) {
        int mid = (low + high) / 2;
        if (sst_metadata[i][mid].min_key > item) {
          high = mid - 1;
        } else if (sst_metadata[i][mid].max_key < item) {
          low = mid + 1;
        } else {
          entry_index = mid;
          break;
        }
      }

      if (entry_index == -1)
        continue;

      const Metadata entry = sst_metadata[i][entry_index];

      int start = 0, file_size = entry.file_size;
      // the size of each entry would be size of key + size of
      // value + 1 ( 1 for tombstone)
      int number_of_entries =
          file_size / (sst_value_block_size + sst_key_block_size + 1);
      int end = number_of_entries - 1;
      while (start <= end) {
        int mid = start + (end - start) / 2;
        std::string data = FileHandler::ReadFromFile(
            entry.file_name,
            mid * (sst_key_block_size + sst_value_block_size + 1),
            sst_key_block_size + sst_value_block_size + 1);
        std::string key = data.substr(0, sst_key_block_size);
        std::string value =
            data.substr(sst_key_block_size, sst_value_block_size);
        if (key == item) {
          if (data.back() == '1')
            return false;
          out_value = value;
          return true;
        }
        if (key < item) {
          start = mid + 1;
        } else {
          end = mid - 1;
        }
      }
    }
    return false;
  }

  std::string FixSize(const K &key, int size) {
    std::string value;
    if (key.size() > size) {
      std::cout << "key was larger than expected size of " << size
                << ". Truncating it" << "\n";
      value = key.substr(0, size);
    } else {
      value = key + std::string(size - key.size(), '\0');
    }

    return value;
  }

  std::string GetSstFileName() {
    auto now = std::chrono::system_clock::now();
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(
                      now.time_since_epoch())
                      .count();
    return std::to_string(micros) + ".txt";
  }

  std::string GetKeyFromString(const std::string &buffer) {
    int i = 0;
    std::string key = "";
    while (i < sst_key_block_size) {
      if (buffer[i] == '\0')
        break;
      key += buffer[i];
      i++;
    }
    return key;
  }

  std::string GetValueFromString(const std::string &buffer) {
    int i = sst_key_block_size;
    std::string value = "";
    while (i < sst_key_block_size + sst_value_block_size) {
      if (buffer[i] == '\0')
        break;
      value += buffer[i];
      i++;
    }
    return value;
  }

  void delete_sst(int level, const std::string &file_path) {
    std::string file_name =
        std::filesystem::path(file_path).filename().string();
    UnRegisterMetadata(level, file_name);
    FileHandler::DeleteFile(file_path);
  }

  bool IsTombstoneEntry(const std::string &buffer) {
    return buffer[sst_key_block_size + sst_value_block_size] == '1';
  }

  Data GetDataFromString(const std::string &buffer,
                         const std::string &file_name) {
    Data d;
    d.key = GetKeyFromString(buffer);
    d.value = GetValueFromString(buffer);
    d.is_deleted = IsTombstoneEntry(buffer);
    d.file_name = file_name;
    return d;
  }

  std::string GetFirstEntry(std::ifstream &stream) {
    stream.clear();
    stream.seekg(0);
    std::string key(sst_key_block_size, '\0');
    stream.read(&key[0], sst_key_block_size);
    return key;
  }

  std::string GetLastEntry(std::ifstream &stream) {
    stream.clear();
    stream.seekg(-(sst_key_block_size + sst_value_block_size + 1),
                 std::ios::end);
    std::string key(sst_key_block_size, '\0');
    stream.read(&key[0], sst_key_block_size);
    return key;
  }

  std::pair<std::string, std::string>
  GetStartAndEndKeys(std::vector<FileHandler::FileStream> &files) {
    std::string start = "", end = "";
    for (int i = 0; i < files.size(); i++) {
      std::ifstream &stream = *(files[i].stream);
      std::string key = GetFirstEntry(stream);
      if (start.size() == 0)
        start = key;
      start = min(start, key);
      end = max(end, key);
      key = GetLastEntry(stream);
      start = min(start, key);
      end = max(end, key);
    }
    return {start, end};
  }

  // Returns the overlapping_files in increasing order of file_name(timestamp).
  std::vector<FileHandler::FileStream>
  GetOverLappingFilesFromLevel(int level, std::string start, std::string end) {
    std::vector<FileHandler::FileStream> files_in_level =
        FileHandler::GetPointersToAllFiles(directory_name + "/" +
                                           sst_level_directories[level]);
    std::vector<FileHandler::FileStream> overlapping_files;
    for (int i = 0; i < files_in_level.size(); i++) {
      std::ifstream &stream = *(files_in_level[i].stream);
      std::string start_value = GetFirstEntry(stream);
      std::string end_value = GetLastEntry(stream);

      if ((start_value <= start && start <= end_value) ||
          (start <= start_value && start_value <= end)) {
        files_in_level[i].stream->seekg(0);
        overlapping_files.emplace_back(std::move(files_in_level[i]));
      }
    }

    std::sort(
        overlapping_files.begin(), overlapping_files.end(),
        [](const FileHandler::FileStream &a, const FileHandler::FileStream &b) {
          return a.file_name < b.file_name;
        });
    return overlapping_files;
  }

  std::string PadData(Data level_value) {
    std::string temp = FixSize(level_value.key, sst_key_block_size) +
                       FixSize(level_value.value, sst_value_block_size) +
                       (level_value.is_deleted ? "1" : "0");
    return temp;
  }

  void MergeAndWriteForLevelAndLevelPlus1(
      int level, std::function<std::optional<Data>()> GetNextValueFromLevel,
      std::function<std::optional<Data>()> GetNextValueFromLevelPlus1) {
    std::string data_to_write = "";
    auto level0_opt = GetNextValueFromLevel();
    auto level1_opt = GetNextValueFromLevelPlus1();

    std::string first_key_written{""}, last_key_written{""};
    while (level1_opt && level0_opt) {
      Data level0_value = *level0_opt;
      Data level1_value = *level1_opt;

      std::optional<Data> selected_level;
      if (level0_value.key <= level1_value.key) {
        if (level + 1 == sst_number_of_levels - 1 && level0_value.is_deleted) {
          ;
        } else {
          selected_level = level0_value;
        }
        if (level0_value.key == level1_value.key) {
          level1_opt = GetNextValueFromLevelPlus1();
        }
        level0_opt = GetNextValueFromLevel();
      } else {
        if (level + 1 == sst_number_of_levels - 1 && level1_value.is_deleted) {
          ;
        } else {
          selected_level = level1_value;
        }
        level1_opt = GetNextValueFromLevelPlus1();
      }

      if (selected_level.has_value()) {
        std::string temp = PadData(selected_level.value());
        if (data_to_write.size() + temp.size() >=
            sst_size_of_file_per_level[level + 1]) {
          std::string sst_level1_new_file_name = GetSstFileName();
          std::string file_name = directory_name + "/" +
                                  sst_level_directories[level + 1] + "/" +
                                  sst_level1_new_file_name;
          FileHandler::WriteToFile(file_name, data_to_write);
          Metadata metadata{sst_level1_new_file_name, first_key_written,
                            last_key_written, data_to_write.size()};
          RegisterMetadata(level + 1, metadata);
          data_to_write = "";

          first_key_written = "";
          last_key_written = "";
        }
        data_to_write += temp;
        if (first_key_written == "") {
          first_key_written = selected_level.value().key;
        }
        last_key_written = selected_level.value().key;
      }
    }

    while (level0_opt) {
      Data level0_value = *level0_opt;
      if (!(level + 1 == sst_number_of_levels - 1 && level0_value.is_deleted)) {
        std::string temp = FixSize(level0_value.key, sst_key_block_size) +
                           FixSize(level0_value.value, sst_value_block_size) +
                           (level0_value.is_deleted ? "1" : "0");
        if (data_to_write.size() + temp.size() >=
            sst_size_of_file_per_level[level + 1]) {
          std::string sst_level0_new_file_name = GetSstFileName();
          std::string file_name = directory_name + "/" +
                                  sst_level_directories[level + 1] + "/" +
                                  sst_level0_new_file_name;
          FileHandler::WriteToFile(file_name, data_to_write);
          Metadata metadata{file_name, first_key_written, last_key_written,
                            data_to_write.size()};
          RegisterMetadata(level + 1, metadata);
          data_to_write = "";
          first_key_written = "";
          last_key_written = "";
        }
        data_to_write += temp;
        if (first_key_written == "") {
          first_key_written = level0_value.key;
        }
        last_key_written = level0_value.key;
      }
      level0_opt = GetNextValueFromLevel();
    }

    while (level1_opt) {
      Data level1_value = *level1_opt;
      if (!(level + 1 == sst_number_of_levels - 1 && level1_value.is_deleted)) {
        std::string temp = FixSize(level1_value.key, sst_key_block_size) +
                           FixSize(level1_value.value, sst_value_block_size) +
                           (level1_value.is_deleted ? "1" : "0");
        if (data_to_write.size() + temp.size() >=
            sst_size_of_file_per_level[level + 1]) {
          std::string sst_level1_new_file_name = GetSstFileName();
          std::string file_name = directory_name + "/" +
                                  sst_level_directories[level + 1] + "/" +
                                  sst_level1_new_file_name;
          FileHandler::WriteToFile(file_name, data_to_write);
          Metadata metadata{sst_level1_new_file_name, first_key_written,
                            last_key_written, data_to_write.size()};
          RegisterMetadata(level + 1, metadata);
          data_to_write = "";
          first_key_written = "";
          last_key_written = "";
        }
        data_to_write += temp;
        if (first_key_written == "") {
          first_key_written = level1_value.key;
        }
        last_key_written = level1_value.key;
      }
      level1_opt = GetNextValueFromLevelPlus1();
    }
    std::string sst_level1_new_file_name = GetSstFileName();
    std::string file_name = directory_name + "/" +
                            sst_level_directories[level + 1] + "/" +
                            sst_level1_new_file_name;
    FileHandler::WriteToFile(file_name, data_to_write);
    Metadata metadata{sst_level1_new_file_name, first_key_written,
                      last_key_written, data_to_write.size()};
    RegisterMetadata(level + 1, metadata);
  }

  void Level0Compaction() {

    // Get the pointers to all the files in level0
    std::vector<FileHandler::FileStream> files_in_level0 =
        FileHandler::GetPointersToAllFiles(directory_name + "/" +
                                           sst_level_directories[0]);
    std::priority_queue<std::pair<Data, int>, std::vector<std::pair<Data, int>>,
                        std::greater<std::pair<Data, int>>>
        level0_data;
    // Find the lowest and highest keys from level0 to select files from level1
    // that they would be merged into.
    std::pair<std::string, std::string> start_end =
        GetStartAndEndKeys(files_in_level0);

    for (int i = 0; i < files_in_level0.size(); i++) {
      if (files_in_level0[i].stream->is_open()) {
        files_in_level0[i].stream->seekg(0);
        std::string buffer(sst_key_block_size + sst_value_block_size + 1, '\0');
        files_in_level0[i].stream->read(&buffer[0], buffer.size());
        if (files_in_level0[i].stream->gcount() ==
            sst_key_block_size + sst_value_block_size + 1) {
          level0_data.push(
              {GetDataFromString(buffer, files_in_level0[i].file_name), i});
        }
      }
    }

    std::unordered_set<std::string> taken_values;

    auto GetNextValueFromLevel0 = [&]() -> std::optional<Data> {
      while (!level0_data.empty()) {
        std::pair<Data, int> data = level0_data.top();
        level0_data.pop();

        std::string buffer(sst_key_block_size + sst_value_block_size + 1, '\0');
        files_in_level0[data.second].stream->read(&buffer[0], buffer.size());
        if (files_in_level0[data.second].stream->gcount() ==
            (sst_key_block_size + sst_value_block_size + 1)) {
          level0_data.push({GetDataFromString(
                                buffer, files_in_level0[data.second].file_name),
                            data.second});
        }

        if (taken_values.find(data.first.key) == taken_values.end()) {
          taken_values.insert(data.first.key);
          return data.first;
        }
      }
      return std::nullopt;
    };

    std::vector<FileHandler::FileStream> files_in_level1 =
        GetOverLappingFilesFromLevel(1, start_end.first, start_end.second);
    std::string data_to_write;
    int file_index = 0; // Keeps track of the file being read.

    auto GetNextValueFromLevel1 = [&]() -> std::optional<Data> {
      while (file_index < files_in_level1.size()) {
        std::string buffer(sst_key_block_size + sst_value_block_size + 1, '\0');
        files_in_level1[file_index].stream->read(&buffer[0], buffer.size());
        if (files_in_level1[file_index].stream->gcount() ==
            sst_key_block_size + sst_value_block_size + 1) {
          const std::string file_name = "";
          return GetDataFromString(buffer, file_name);
        } else {
          file_index++;
        }
      }
      return std::nullopt;
    };
    MergeAndWriteForLevelAndLevelPlus1(0, GetNextValueFromLevel0,
                                       GetNextValueFromLevel1);
    for (int i = 0; i < files_in_level0.size(); i++) {
      delete_sst(0, files_in_level0[i].file_name);
    }
    for (int i = 0; i < files_in_level1.size(); i++) {
      delete_sst(1, files_in_level1[i].file_name);
    }

    std::string folder_name = directory_name + "/" + sst_level_directories[1];
    if (FileHandler::GetNumberofFiles(folder_name) >=
        sst_number_of_files_per_level[1]) {
      LevelCompaction(1);
    }
  }

  void LevelCompaction(int level) {
    if (level == sst_number_of_levels) {
      return;
    }

    // Write some better logic of find a file from level to merge to level+1.
    std::vector<std::string> file_names = FileHandler::GetAllFileNames(
        directory_name + "/" + sst_level_directories[level]);
    srand(time(0));
    int index = rand() % file_names.size();

    std::string level_file_name = file_names[index];
    FileHandler::FileStream file = FileHandler::GetPointerToFile(
        directory_name + "/" + sst_level_directories[level] + "/" +
        level_file_name);
    std::vector<FileHandler::FileStream> files;
    files.push_back(std::move(file));
    std::pair<std::string, std::string> start_end = GetStartAndEndKeys(files);
    files[0].stream->seekg(0);
    std::vector<FileHandler::FileStream> files_in_level_plus_1 =
        GetOverLappingFilesFromLevel(level + 1, start_end.first,
                                     start_end.second);

    int file_index = 0;
    auto GetNextValueFromLevel = [&]() -> std::optional<Data> {
      std::string buffer(sst_key_block_size + sst_value_block_size + 1, '\0');
      files[0].stream->read(&buffer[0], buffer.size());
      if (files[0].stream->gcount() ==
          sst_key_block_size + sst_value_block_size + 1) {
        const std::string file_name = "";
        return GetDataFromString(buffer, file_name);
      }
      return std::nullopt;
    };

    auto GetNextValueFromLevelPlus1 = [&]() -> std::optional<Data> {
      while (file_index < files_in_level_plus_1.size()) {
        std::string buffer(sst_key_block_size + sst_value_block_size + 1, '\0');
        files_in_level_plus_1[file_index].stream->read(&buffer[0],
                                                       buffer.size());
        if (files_in_level_plus_1[file_index].stream->gcount() ==
            sst_key_block_size + sst_value_block_size + 1) {
          const std::string file_name = "";
          return GetDataFromString(buffer, file_name);
        } else {
          file_index++;
        }
      }
      return std::nullopt;
    };
    MergeAndWriteForLevelAndLevelPlus1(level, GetNextValueFromLevel,
                                       GetNextValueFromLevelPlus1);
    delete_sst(level, files[0].file_name);
    for (int i = 0; i < files_in_level_plus_1.size(); i++) {
      delete_sst(level + 1, files_in_level_plus_1[i].file_name);
    }

    std::string folder_name =
        directory_name + "/" + sst_level_directories[level];
    if (FileHandler::GetNumberofFiles(folder_name) >=
        sst_number_of_files_per_level[level]) {
      LevelCompaction(level + 1);
    }
  }

  void RegisterMetadata(int level, const Metadata &metadata) {
    auto &vec = sst_metadata[level];
    vec.insert(std::upper_bound(vec.begin(), vec.end(), metadata,
                                [](const Metadata &a, const Metadata &b) {
                                  return a.min_key < b.min_key;
                                }),
               metadata);
  }
  void UnRegisterMetadata(int level, const std::string key) {
    auto it =
        std::lower_bound(sst_metadata[level].begin(), sst_metadata[level].end(),
                         key, [](const Metadata &a, const std::string &b) {
                           return a.file_name < b;
                         });
    if (it != sst_metadata[level].end() && it->file_name == key) {
      sst_metadata[level].erase(it);
    }
  }

public:
  explicit SST() {
    sst_metadata.resize(sst_number_of_levels + 1);
    for (int i = 0; i <= sst_number_of_levels; i++) {
      std::filesystem::create_directories(directory_name + "/level" +
                                          std::to_string(i));
    }
  }
  std::string Flush(MemtableIterator<K, V> memtableIterator) {
    std::lock_guard<std::mutex> lock(compaction_mx);
    std::string data_to_write = "";

    std::string first_key = "", last_key = "";
    while (memtableIterator.HasNext()) {
      std::tuple<K, V, bool> data = memtableIterator.GetNext();
      std::string fixed_key = FixSize(std::get<0>(data), sst_key_block_size);
      std::string fixed_value =
          FixSize(std::get<1>(data), sst_value_block_size);
      if (first_key.size() == 0) {
        first_key = std::get<0>(data);
      }
      last_key = std::get<0>(data);
      data_to_write +=
          fixed_key + fixed_value + (std::get<2>(data) ? '1' : '0');
    }

    std::string file_name = GetSstFileName();
    std::string file_path =
        directory_name + "/" + sst_level_directories[0] + "/" + file_name;
    FileHandler::WriteToFile(file_path, data_to_write);

    Metadata metadata{file_name, first_key, last_key, data_to_write.size()};
    RegisterMetadata(0, metadata);

    std::string folder_name = directory_name + "/" + sst_level_directories[0];
    int file_count = FileHandler::GetNumberofFiles(folder_name);
    if (file_count > 1) {
      Compaction();
    }
    return file_path;
  }

  // Level 0 compaction should be a single merge and then write.
  void Compaction() { Level0Compaction(); }

  bool Get(const K &key, V &out_value) {
    std::string fixed_key = FixSize(key, sst_key_block_size);
    std::string data;
    bool isPresent = Search(fixed_key, out_value);
    if (!isPresent) {
      out_value = "";
      return false;
    }
    out_value.erase(out_value.find_last_not_of('\0') + 1);
    return true;
  }

  // Exposing for testing.
  const std::vector<Metadata> &GetMetadataForLevel(int level) const {
    return sst_metadata.at(level);
  }
};
} // namespace kvstore::engine
