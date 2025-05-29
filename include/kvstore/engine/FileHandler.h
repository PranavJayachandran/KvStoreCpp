#pragma once
#include <string>
namespace kvstore::engine {
  class FileHandler{
    public:
      static void AppendToFile(const std::string &file_name, std::string &data);

      static std::string ReadFromFile(const std::string &file_name, int start_pos, int block_size);

      static void WriteToFile(const std::string &file_name, int start_pos, std::string &data);

      static int GetSize(const std::string &file_name);
  };
}
