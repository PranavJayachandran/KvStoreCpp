// Should be able to write to file
// shoud be able to read from a file
//
//
// Get 
// Add
// Delete
// Every entry should be of size X ie key value and a bit to decide tombstone or not -> a for key, b for value and 1 for tombstone => a + b + 1 = X.
#include <string>
#include "../config/Config.h"
#include "FileHandler.h"

namespace kvstore::engine {
  template <typename K = std::string, typename V = std::string>
  class SST{
    private:
      const std::string directory_name = config::GetConfig().sst_dir; 
    public:
      void Add(const K&key, const V& value){
        printf("%s", directory_name);
      }
      bool Get(const K& key, V& out_value){
        return true;
      }
      void Delete(const K& key){

      }
  };
}
