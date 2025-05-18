#pragma once
#include "SkipList.h"
#include <string>
namespace kvstore::engine {
  template <typename K = std::string, typename V = std::string>
  class Memtable{
    private:
      size_t max_size_;
      SkipList<K,V> skiplist_;
      size_t current_size_ = 0;
    public:
      explicit Memtable(size_t size): skiplist_(4){
        max_size_ = size;
      }

      void Add(const K& key, const V& value){
        skiplist_.Add(key, value);
        current_size_ += sizeof(key) + sizeof(value);
        skiplist_.Print();
      }
      
      bool Get(const K& key, V& out_value){
        const V *val = skiplist_.Get(key);
        if (val){
          out_value = *val;
          return true;
        }
        return false;
      }

      void Delete(const K& key){
        skiplist_.Add(key, "", true);
      }

      bool ShouldFlush(){
        return current_size_ < max_size_;
      }
  };
}
