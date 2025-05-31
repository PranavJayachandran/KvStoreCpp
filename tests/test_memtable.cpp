#include "../include/kvstore/engine/Memtable.h"
#include <gtest/gtest.h>

using kvstore::engine::Memtable;

TEST(MemtableTest, Insert){
    Memtable<std::string,std::string> memtable(10);
    memtable.Add("key1", "value1");
    memtable.Add("key2", "value2");

    std::string val1;
    bool isPresent1 = memtable.Get("key1",val1);
    EXPECT_EQ(isPresent1, true);
    EXPECT_EQ(val1, "value1");

    std::string val2;
    bool isPresent2 = memtable.Get("key2",val2);
    EXPECT_EQ(isPresent2, true);
    EXPECT_EQ(val2, "value2");

    std::string val3;
    bool isPresent3 = memtable.Get("key3", val3);
    EXPECT_EQ(isPresent3, false);
}

TEST(Memtable, Update) {
  Memtable<>memtable(4);
  memtable.Add("key1", "value1");
   
  std::string val;
  bool isPresent = memtable.Get("key1", val);
  EXPECT_EQ(isPresent, true);
  EXPECT_EQ(val, "value1");
  memtable.Add("key1", "value2");

  isPresent = memtable.Get("key1", val);
  EXPECT_EQ(isPresent, true);
  EXPECT_EQ(val, "value2");
}

TEST(Memtable, Delete) {
  Memtable<>memtable(4);

  memtable.Add("key1", "value1");

  std::string val;
  bool isPresent = memtable.Get("key1", val);
  EXPECT_EQ(isPresent, true);
  EXPECT_EQ(val, "value1");
  memtable.Delete("key1");

  isPresent = memtable.Get("key1", val);
  EXPECT_EQ(isPresent, false);
}

TEST(MemtableIterator, Iterator_ShouldGetAllTheValues){
  Memtable<>memtable(4);

  memtable.Add("key2", "value2");
  memtable.Add("key1", "value1");
  memtable.Add("key4", "value4");
  memtable.Add("key3", "value3");

  kvstore::engine::MemtableIterator<> iterator = memtable.GetMemtableITerator();
  std::vector<std::pair<std::string,std::string>> p;
  while(iterator.HasNext()){
    p.push_back(iterator.GetNext()); 
  }  
  ASSERT_EQ(p.size(), 4);
  EXPECT_EQ(p[0].first, "key1");
  EXPECT_EQ(p[0].second, "value1");
  EXPECT_EQ(p[1].first, "key2");
  EXPECT_EQ(p[1].second, "value2");
  EXPECT_EQ(p[2].first, "key3");
  EXPECT_EQ(p[2].second, "value3");
  EXPECT_EQ(p[3].first, "key4");
  EXPECT_EQ(p[3].second, "value4");
}
