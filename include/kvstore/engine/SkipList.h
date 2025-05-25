#pragma once 
#include <random>
#include <vector>
#include <string>
#include <memory>
#include <iostream>

namespace kvstore::engine{
  template <typename K = std::string, typename V = std::string>
  class SkipList{
    private:
      struct Node{
        K key;
        V value;
        std::vector<std::shared_ptr<Node>> next;
        bool is_delete = false;
        Node(const std::string& k, const std::string& v, int level, bool is_delete)
        : key(k), value(v), next(level, nullptr), is_delete(is_delete){}
      };
      int max_level_;
      std::shared_ptr<Node> head_;

      int GetRandomLevel(){
        static std::mt19937 rng(std::random_device{}());  // Seed once
        std::uniform_int_distribution<int> dist(0, max_level_ - 1);
        return dist(rng);
      }

      void InsertValue(const K& key, const V& value, bool is_delete){
        std::vector<std::shared_ptr<Node>>update(max_level_, nullptr);
        int current_level = 0;
        auto iterator_node = head_;

        while(current_level < max_level_){
          while(iterator_node->next[current_level] != nullptr && iterator_node->next[current_level]->key < key){
            iterator_node = iterator_node->next[current_level];
          }
          update[current_level] = iterator_node;
          current_level++;
        }
        if(iterator_node->next[max_level_-1] != nullptr && iterator_node->next[max_level_-1]->key == key){
          iterator_node->next[max_level_-1]->value = value;
          iterator_node->next[max_level_-1]->is_delete = is_delete;
        }
        else{
          int random_level = GetRandomLevel();
          printf("%d\n",random_level);
          auto new_node = std::make_shared<Node>(key, value, max_level_, is_delete);
          while(random_level < max_level_){
            new_node->next[random_level] = update[random_level]->next[random_level];
            update[random_level]->next[random_level] = new_node;
            random_level++;
          }
        }
      }

    public:
      explicit SkipList(int max_level){
        this->max_level_ = max_level;
        this->head_ = std::make_shared<Node>(K(), V(), max_level_, false);
      }

      // No copying
      SkipList(const SkipList&) = delete;
      SkipList& operator=(const SkipList&) = delete;

      void Add(const K& key, const V& value){
        InsertValue(key, value, false);
      }

      void Delete(const K& key){
        InsertValue(key, "", true);
      }
      
      const V* Get(const K& key){
        int current_level = 0;
        auto iterator_node = head_;
        while(current_level < max_level_){
          while (iterator_node->next[current_level] != nullptr && iterator_node->next[current_level]->key < key){
            iterator_node = iterator_node->next[current_level];
          }
          current_level++;
        }
        iterator_node = iterator_node->next[max_level_-1];
        return (iterator_node != nullptr && iterator_node->key == key && !iterator_node->is_delete) ? &iterator_node->value : nullptr; 
      }

      void Print(){
        for(int i=0;i<max_level_;i++){
          auto iterator_node = head_;
          while(iterator_node != nullptr){
            printf("(Key: %s Value: %s)", iterator_node->key.c_str(), iterator_node->value.c_str());
            iterator_node = iterator_node->next[i];
          }
          printf("\n");
        }
      }
  };
}
