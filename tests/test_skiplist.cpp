#include "../include/kvstore/engine/SkipList.h"
#include <gtest/gtest.h>

using kvstore::engine::SkipList;

TEST(SkipListTest, InsertAndGet) {
    SkipList<std::string,std::string> list(4); // std::string, std::string by default

    list.Add("key1", "value1");
    list.Add("key2", "value2");

    const std::string* val1 = list.Get("key1");
    ASSERT_NE(val1, nullptr);
    EXPECT_EQ(*val1, "value1");

    const std::string* val2 = list.Get("key2");
    ASSERT_NE(val2, nullptr);
    EXPECT_EQ(*val2, "value2");

    const std::string* val3 = list.Get("key3");
    EXPECT_EQ(val3, nullptr); // not inserted
}

TEST(SkipListTest, Update) {
  SkipList<>list(4);

  list.Add("key1", "value1");

  const std::string* val1 = list.Get("key1");
  EXPECT_EQ(*val1, "value1");
  list.Add("key1", "value2");

  val1 = list.Get("key1");
  EXPECT_EQ(*val1, "value2");
}

TEST(SkipListTest, Delete) {
  SkipList<>list(4);

  list.Add("key1", "value1");

  const std::string* val1 = list.Get("key1");
  EXPECT_EQ(*val1, "value1");
  list.Delete("key1");

  val1 = list.Get("key1");
  EXPECT_EQ(val1, nullptr); 
}


TEST(SkipListTest, Iterator_ShouldGetAllTheValues){
  SkipList<>list(4);
  list.Add("key1","value1");
  list.Add("key2","value2");
  list.Add("key3","value3");

  kvstore::engine::SkipListIterator<> iterator = list.GetSkipListIterator();
  std::vector<std::pair<std::string,std::string>> p;
  while(iterator.HasNext()){
    p.push_back(iterator.GetNext());
  }

  ASSERT_EQ(p.size(), 3);
  EXPECT_EQ(p[0].first, "key1");
  EXPECT_EQ(p[0].second, "value1");
  EXPECT_EQ(p[1].first, "key2");
  EXPECT_EQ(p[1].second, "value2");
  EXPECT_EQ(p[2].first, "key3");
  EXPECT_EQ(p[2].second, "value3");
}

