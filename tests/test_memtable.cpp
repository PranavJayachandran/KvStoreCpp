#include "../include/kvstore/engine/Memtable.h"
#include <gtest/gtest.h>
#include <tuple>

using kvstore::engine::Memtable;

void clean_wal() {
  std::filesystem::path path{"wal/wal.txt"};
  if (std::filesystem::exists(path)) {
    std::filesystem::remove(path);
  }
}

TEST(MemtableTest, Insert) {
  clean_wal();
  kvstore::config::LoadFromFile("../config.json");
  Memtable<std::string, std::string> memtable(10);
  memtable.Add("key1", "value1");
  memtable.Add("key2", "value2");

  std::string val1;
  bool isPresent1 = memtable.Get("key1", val1);
  EXPECT_EQ(isPresent1, true);
  EXPECT_EQ(val1, "value1");

  std::string val2;
  bool isPresent2 = memtable.Get("key2", val2);
  EXPECT_EQ(isPresent2, true);
  EXPECT_EQ(val2, "value2");

  std::string val3;
  bool isPresent3 = memtable.Get("key3", val3);
  EXPECT_EQ(isPresent3, false);
}

TEST(Memtable, Update) {
  clean_wal();
  kvstore::config::LoadFromFile("../config.json");
  Memtable<> memtable(4);
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
  clean_wal();
  kvstore::config::LoadFromFile("../config.json");
  Memtable<> memtable(4);

  memtable.Add("key1", "value1");

  std::string val;
  bool isPresent = memtable.Get("key1", val);
  EXPECT_EQ(isPresent, true);
  EXPECT_EQ(val, "value1");
  memtable.Delete("key1");

  isPresent = memtable.Get("key1", val);
  EXPECT_EQ(isPresent, false);
}

TEST(MemtableIterator, Iterator_ShouldGetAllTheValues) {
  clean_wal();
  kvstore::config::LoadFromFile("../config.json");
  Memtable<> memtable(4);

  memtable.Add("key2", "value2");
  memtable.Add("key1", "value1");
  memtable.Add("key4", "value4");
  memtable.Add("key3", "value3");

  memtable.Delete("key3");

  kvstore::engine::MemtableIterator<> iterator = memtable.GetMemtableITerator();
  std::vector<std::tuple<std::string, std::string, bool>> p;
  while (iterator.HasNext()) {
    p.push_back(iterator.GetNext());
  }
  ASSERT_EQ(p.size(), 4);
  EXPECT_EQ(std::get<0>(p[0]), "key1");
  EXPECT_EQ(std::get<1>(p[0]), "value1");
  EXPECT_EQ(std::get<2>(p[0]), false);
  EXPECT_EQ(std::get<0>(p[1]), "key2");
  EXPECT_EQ(std::get<1>(p[1]), "value2");
  EXPECT_EQ(std::get<2>(p[1]), false);
  EXPECT_EQ(std::get<0>(p[2]), "key3");
  EXPECT_EQ(std::get<1>(p[2]), "");
  EXPECT_EQ(std::get<2>(p[2]), true);
  EXPECT_EQ(std::get<0>(p[3]), "key4");
  EXPECT_EQ(std::get<1>(p[3]), "value4");
  EXPECT_EQ(std::get<2>(p[3]), false);
}
