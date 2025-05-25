#include <string>
namespace kvstore::engine {
  class FileHandler{
    public:
      void AppendToFile(std::string file_name, std::string data);

      std::string ReadFromFile(std::string file_name, int start_pos, int block_size);

      void WriteToFile(std::string file_name, int start_pos, std::string data);
  };
}
