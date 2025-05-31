#pragma once
#include "SkipList.h"
#include <string>
namespace kvstore::engine {

  template <typename K = std::string, typename V = std::string>
  class MemtableIterator;

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
        skiplist_.Delete(key);
      }

      bool ShouldFlush(){
        return current_size_ < max_size_;
      }

      MemtableIterator<K,V> GetMemtableITerator(){
        return MemtableIterator<K,V>(skiplist_.GetSkipListIterator());
      }
  };

  template <typename K, typename V>
  class MemtableIterator{
    private:
      SkipListIterator<K,V> skip_list_iterator_;
    public:
      explicit MemtableIterator(SkipListIterator<K,V> skip_list_iterator) : skip_list_iterator_(skip_list_iterator){} 
      bool HasNext(){
        return skip_list_iterator_.HasNext();
      }

      std::pair<K,V> GetNext(){
        return skip_list_iterator_.GetNext();
      }
  };
}
