#pragma once
#include <string>
#include <vector>
#include<functional>
namespace kvstore ::engine {
  template <typename T = std::string>
  class BloomFilter{
    private:
      size_t size;
      long long bits;
      std::vector<std::function<int(const T&s)>> hashFunctions;
      void Mark(int pos){
        bits |= (1 << pos);
      }
      bool IsMarked(int pos) const {
        return ( bits >> pos) & 1 ; 
      }
      int Hash(int seed, const T& val) const {
        int hash = 0;
        for (char ch : val) {
            hash = hash * seed + static_cast<unsigned char>(ch);
        }
        return std::abs(hash) % size;
    }
      
    public:
      explicit BloomFilter(int size) : size(size){
        hashFunctions = {
          [this](const T &s) {return Hash(37,s);},
          [this](const T &s) {return Hash(31,s);},
          [this](const T &s) {return Hash(29,s);},
        };
      }

      void Add(const T &message){
        for (const auto& func : hashFunctions){
          Mark(func(message));
        }
      }

      void Clear(){
        bits = 0;
      }

      bool Exists(const T &message) const{
        for (const auto &func: hashFunctions){
          if(!IsMarked(func(message))){
              return false;
          }
        }
        return true;
      }
  };
}
