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

