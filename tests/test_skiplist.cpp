#include "../include/kvstore/engine/SkipList.h"
#include <gtest/gtest.h>
#include <tuple>

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
  std::vector<std::tuple<std::string,std::string, bool>> p;
  while(iterator.HasNext()){
    p.push_back(iterator.GetNext());
  }

  ASSERT_EQ(p.size(), 3);
  EXPECT_EQ(std::get<0>(p[0]), "key1");
  EXPECT_EQ(std::get<1>(p[0]), "value1");
  EXPECT_EQ(std::get<2>(p[0]), false);
  EXPECT_EQ(std::get<0>(p[1]), "key2");
  EXPECT_EQ(std::get<1>(p[1]), "value2");
  EXPECT_EQ(std::get<2>(p[1]), false);
  EXPECT_EQ(std::get<0>(p[2]), "key3");
  EXPECT_EQ(std::get<1>(p[2]), "value3");
  EXPECT_EQ(std::get<2>(p[2]), false);
}
