#include <string>
#include "../config/Config.h"
#include "FileHandler.h"

namespace kvstore::engine {
  template <typename K = std::string, typename V = std::string>
  class SST{
    private:
      const std::string directory_name = config::GetConfig().sst_dir; 
      const int sst_key_block_size = config::GetConfig().sst_key_block_size;
      const int sst_value_block_size = config::GetConfig().sst_value_block_size;
      const std::string sst = "sst.txt";

      bool Search(const K&key, int& out_index){
        int start = 0, file_size = FileHandler::GetSize(sst); 
        // the size of each entry would be size of key + size of value + 1 ( 1 for tombstone)
        int number_of_entries = file_size/(sst_value_block_size + sst_key_block_size + 1);
        int end = number_of_entries - 1;
        while(start <= end){
          int mid = start + (end - start)/2;
          std::string value = FileHandler::ReadFromFile(sst, mid, sst_key_block_size);
          if(value == key){
            out_index = mid;
            return true;
          }
          if(value < key){
            start = mid + 1;
          }
          else{
            end = mid - 1;
          }
        }
        return false;
      }
      
      int GetInsertPosition(const K&key){
        int start = 0, file_size = FileHandler::GetSize(sst); 
        // the size of each entry would be size of key + size of value + 1 ( 1 for tombstone)
        int number_of_entries = file_size/(sst_value_block_size + sst_key_block_size + 1);
        int end = number_of_entries - 1;
        while(start <= end){
          int mid = start + (end - start)/2;
          std::string value = FileHandler::ReadFromFile(sst, mid * (sst_value_block_size + sst_key_block_size + 1), sst_key_block_size);
          if(value < key){
            start = mid + 1;
          }
          else{
            end = mid - 1;
          }
        }
        return start;
      }

      std::string FixSize(const K&key, int size){
        std::string value;
        if(key.size() > size){
          value = key.substr(0, size);
        }
        else {
          value = key + std::string(size - key.size(), '\0');
        }

        return value;
      }

    public:
      void Add(const K&key, const V& value, bool isDelete = false){
        std::string fixed_key = FixSize(key, sst_key_block_size);
        std::string fixed_value = FixSize(value, sst_value_block_size);
        int index = -1;
        bool isPresent = Search(fixed_key, index);
        if(!isPresent){
          index = GetInsertPosition(fixed_key);
        }
        std::string data_to_write =  fixed_key + fixed_value + (isDelete ? '1' : '0');
        FileHandler::WriteToFile(sst, index * (sst_value_block_size + sst_key_block_size + 1), data_to_write);
      }

      bool Get(const K& key, V& out_value){
        std::string fixed_key = FixSize(key, sst_key_block_size);
        int index = -1;
        bool isPresent = Search(fixed_key, index);
        if(!isPresent){
          out_value = "";
          return false;
        }
        std::string data = FileHandler::ReadFromFile(sst, index * (sst_key_block_size + sst_value_block_size + 1) + sst_key_block_size, sst_value_block_size + 1);
        if(data.size() == 0 || data.back() == '1'){
          out_value = "";
          return false;
        }
        out_value = data.substr(0, data.size()-1);
        out_value.erase(out_value.find_last_not_of('\0') + 1);
        return true;
      }

      void Delete(const K& key){
        Add(key, "", true);
      }
  };
}
