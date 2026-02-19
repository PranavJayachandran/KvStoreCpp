#include "../include/kvstore/config/Config.h"
#include "../include/kvstore/engine/SST.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

using kvstore::engine::SST;
namespace fs = std::filesystem;
class SSTTest : public ::testing::Test {
protected:
  void SetUp() override {
    kvstore::config::LoadFromFile("../config.json");
    std::ofstream file("sst.txt");
    file.close();
    sst = std::make_unique<SST<std::string, std::string>>();
  }

  void TearDown() override {
    std::remove("sst.txt");
    clearDirectory(kvstore::config::GetConfig().sst_dir);
  }

  std::string Pad(const std::string &input, int size) {
    return input + std::string(size - input.size(), '\0');
  }
  void clearDirectory(const std::string &dir_path) {
    fs::path path_to_clear{dir_path};

    if (!fs::exists(path_to_clear) || !fs::is_directory(path_to_clear)) {
      std::cerr << "Directory not found or is not a directory: " << dir_path
                << std::endl;
      return;
    }

    try {
      // Iterate over all entries in the directory (non-recursive)
      for (const auto &entry : fs::directory_iterator(path_to_clear)) {
        // Use remove_all to handle both files and subdirectories recursively
        fs::remove_all(entry.path());
      }
      std::cout << "All files and subdirectories in " << dir_path
                << " have been deleted." << std::endl;
    } catch (const fs::filesystem_error &e) {
      std::cerr << "Filesystem error: " << e.what() << std::endl;
      std::cerr << "Path: " << e.path1().string() << std::endl;
    }
  }

  const int key_size = 10;
  const int value_size = 20;
  std::unique_ptr<SST<std::string, std::string>> sst;
};

TEST_F(SSTTest, WhenNoKeyPresent_ShouldReturnFalse) {
  const std::string key = "key1";

  std::string val;
  bool found = sst->Get(key, val);

  EXPECT_FALSE(found);
  EXPECT_EQ(val, "");
}

std::string Pad(const std::string &input, int size) {
  return input + std::string(size - input.size(), '\0');
}

TEST_F(SSTTest, Flush_ShouldWriteToFile) {
  kvstore::engine::Memtable<> memtable(4, "wal");
  memtable.Add("key2", "value2");
  memtable.Add("key1", "value1");
  memtable.Add("key4", "value4");
  memtable.Add("key3", "value3");
  memtable.Delete("key3");

  kvstore::engine::SST<> sst;
  std::string file_name = sst.Flush(memtable.GetMemtableITerator());

  std::ifstream file(file_name);
  std::string data;
  file >> data;
  std::string expected_data;
  expected_data += Pad("key1", 10) + Pad("value1", 20) + "0";
  expected_data += Pad("key2", 10) + Pad("value2", 20) + "0";
  expected_data += Pad("key3", 10) + Pad("", 20) + "1";
  expected_data += Pad("key4", 10) + Pad("value4", 20) + "0";
  EXPECT_EQ(data, expected_data);

  std::remove(file_name.c_str());
}

TEST_F(SSTTest, FlushCreatesMetadataForLevel0) {
  kvstore::engine::Memtable<> memtable(4, "wal");
  memtable.Add("a", "1");
  memtable.Add("b", "2");

  sst->Flush(memtable.GetMemtableITerator());

  auto metas = sst->GetMetadataForLevel(0);

  ASSERT_EQ(metas.size(), 1);
  EXPECT_EQ(metas[0].min_key, "a");
  EXPECT_EQ(metas[0].max_key, "b");
  EXPECT_GT(metas[0].file_size, 0);
}

TEST_F(SSTTest, MetadataTracksCorrectKeyRange) {
  kvstore::engine::Memtable<> memtable(4, "wal");
  memtable.Add("key1", "v1");
  memtable.Add("key3", "v3");
  memtable.Add("key2", "v2");

  sst->Flush(memtable.GetMemtableITerator());

  auto metas = sst->GetMetadataForLevel(0);
  ASSERT_EQ(metas.size(), 1);

  EXPECT_EQ(metas[0].min_key, "key1");
  EXPECT_EQ(metas[0].max_key, "key3");
}

TEST_F(SSTTest, MetadataRemovedAfterCompaction) {
  kvstore::engine::Memtable<> mem1(4, "wal");
  mem1.Add("a", "1");
  sst->Flush(mem1.GetMemtableITerator());

  kvstore::engine::Memtable<> mem2(4, "wal");
  mem2.Add("b", "2");
  sst->Flush(mem2.GetMemtableITerator());

  // triggers compaction
  auto level0 = sst->GetMetadataForLevel(0);
  EXPECT_LE(level0.size(), 1);
}
