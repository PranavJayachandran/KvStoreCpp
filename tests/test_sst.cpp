#include "../include/kvstore/engine/SST.h"
#include "../include/kvstore/config/Config.h"
#include <fstream>
#include <gtest/gtest.h>

using kvstore::engine::SST;

class SSTTest : public ::testing::Test {
protected:
  void SetUp() override {
    kvstore::config::LoadFromFile("../config.json");
    std::ofstream file("sst.txt");
    file.close();
    sst = std::make_unique<SST<std::string, std::string>>();
  }

  void TearDown() override {
    std::remove("sst.txt");
  }

  std::string Pad(const std::string& input, int size) {
    return input + std::string(size - input.size(), '\0');
  }

  const int key_size = 10;
  const int value_size = 20;
  std::unique_ptr<SST<std::string, std::string>> sst;
};


TEST_F(SSTTest, WhenNoKeyPresent_ShouldReturnFalse) {
  const std::string key = "key1";

  std::string val;
  bool found = sst->Get(key, val);

  EXPECT_FALSE(found);
  EXPECT_EQ(val, "");
}

std::string Pad(const std::string& input, int size) {
  return input + std::string(size - input.size(), '\0');
}
TEST(SSTTestFlush, Flush_ShouldWriteToFile){
  kvstore::engine::Memtable<>memtable(4);
  memtable.Add("key2", "value2");
  memtable.Add("key1", "value1");
  memtable.Add("key4", "value4");
  memtable.Add("key3", "value3");
  memtable.Delete("key3");

  kvstore::engine::SST<> sst;
  std::string file_name = "some_file_name.txt";
  sst.Flush(memtable.GetMemtableITerator(), file_name);
  std::ifstream file(file_name);
  std::string data;
  file >> data;
  std::string expected_data;
  expected_data += Pad("key1", 10) + Pad("value1", 20) + "0";
  expected_data += Pad("key2", 10) + Pad("value2", 20) + "0";
  expected_data += Pad("key3", 10) + Pad("", 20) + "1";
  expected_data += Pad("key4", 10) + Pad("value4", 20) + "0";
  EXPECT_EQ(data,expected_data);
  std::remove(file_name.c_str());
}


