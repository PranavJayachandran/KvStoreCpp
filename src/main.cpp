#include <iostream>
#include "../include/kvstore/engine/Memtable.h"
int main(){
  std::cout<<"Hello";
  auto p = new kvstore::engine::Memtable<std::string, std::string>(4);
  p->Add("one","one1");
  p->Add("two", "two1");
  p->Add("one","one2");
  std::string value;
  if(p->Get("one", value)) {
    std::cout << "Value: " << value << std::endl;
} else {
    std::cout << "Key not found." << std::endl;
}
}
