#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <memory>
namespace kvstore::engine {
  class FileHandler{
    public:
      struct FileStream{
        std::string file_name;
        std::unique_ptr<std::ifstream>stream;
      };

      static void AppendToFile(const std::string &file_name, std::string &data);

      static std::string ReadFromFile(const std::string &file_name, int start_pos, int block_size);

      static void WriteToFile(const std::string &file_name, std::string &data);

      static int GetSize(const std::string &file_name);

      static int GetNumberofFiles(std::string &folder_name);
      
      static std::vector<FileStream> GetPointersToAllFiles(const std::string &folder_name);
  };
}
