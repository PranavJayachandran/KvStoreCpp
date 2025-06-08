#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <memory>
namespace kvstore::engine {
  class FileHandler{
    public:
      struct FileStream {
        std::string file_name;
        std::unique_ptr<std::ifstream> stream;
      
        FileStream() = default;
        FileStream(const FileStream&) = delete;
        FileStream& operator=(const FileStream&) = delete;
      
        FileStream(FileStream&&) = default;
        FileStream& operator=(FileStream&&) = default;
      };

      static void AppendToFile(const std::string &file_name, std::string &data);

      static std::string ReadFromFile(const std::string &file_name, int start_pos, int block_size);

      static void WriteToFile(const std::string &file_name, std::string &data);

      static void DeleteFile(const std::string &file_name);

      static void CreateFile(const std::string &folder_name, const std::string &file_name);

      static int GetSize(const std::string &file_name);

      static int GetNumberofFiles(const std::string &folder_name);
      
      static std::vector<std::string> GetAllFileNames(const std::string &folder_name);

      static FileStream GetPointerToFile(const std::string &file_name);
      
      static std::vector<FileStream> GetPointersToAllFiles(const std::string &folder_name);
  };
}
