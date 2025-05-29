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

TEST_F(SSTTest, WhenNewKeyIsAdded_ItCanBeRetrieved) {
  const std:: string key = "key1", value = "value1";
  sst->Add(key, value);

  std::string val = "Hello";
  bool is_present = sst->Get("key1",val);
  ASSERT_EQ(is_present, true);
  EXPECT_EQ(val, value); 
}

TEST_F(SSTTest, WhenExistingKeyIsAdded_ValueIsUpdated) {
  const std::string key = "key1", value = "value1";
  sst->Add(key, value);

  std::string val;
  ASSERT_TRUE(sst->Get(key, val));
  EXPECT_EQ(val, value);

  const std::string updated_value = "new value 1";
  sst->Add(key, updated_value);

  ASSERT_TRUE(sst->Get(key, val));
  EXPECT_EQ(val, updated_value);
}

TEST_F(SSTTest, WhenEntriesAreAdded_ShouldBeSavedInOrder) {
  const std::string key1 = "key1", value1 = "value1";
  const std::string key2 = "key2", value2 = "value2";
  const std::string key3 = "key3", value3 = "value3";

  sst->Add(key1, value1);
  sst->Add(key3, value3);
  sst->Add(key2, value2);

  std::ifstream filein("sst.txt", std::ios::binary);
  std::stringstream buffer;
  buffer << filein.rdbuf();
  std::string data = buffer.str();

  std::string expected_data;
  expected_data += Pad(key1, key_size) + Pad(value1, value_size) + "0";
  expected_data += Pad(key2, key_size) + Pad(value2, value_size) + "0";
  expected_data += Pad(key3, key_size) + Pad(value3, value_size) + "0";

  EXPECT_EQ(data, expected_data);
}

TEST_F(SSTTest, WhenNoKeyPresent_ShouldReturnFalse) {
  const std::string key = "key1";

  std::string val;
  bool found = sst->Get(key, val);

  EXPECT_FALSE(found);
  EXPECT_EQ(val, "");
}

TEST_F(SSTTest, WhenEntryDeleted_ItShouldNotBeReturnedWhenGet) {
  const std::string key = "key1", value = "value1";
  sst->Add(key, value);

  std::string val;
  ASSERT_TRUE(sst->Get(key, val));
  EXPECT_EQ(val, value);

  sst->Delete(key);
  bool found = sst->Get(key, val);

  EXPECT_FALSE(found);
  EXPECT_EQ(val, "");
}

TEST_F(SSTTest, DeleteForNewEntry_ShouldAddTombstone) {
  const std::string key = "key1";
  std::string value;

  sst->Delete(key);
  bool found = sst->Get(key, value);
  EXPECT_FALSE(found);

  std::ifstream filein("sst.txt", std::ios::binary);
  std::stringstream buffer;
  buffer << filein.rdbuf();
  std::string data = buffer.str();

  std::string expected_data;
  expected_data += Pad(key, key_size);
  expected_data += std::string(value_size, '\0');
  expected_data += "1";

  EXPECT_EQ(data, expected_data);
}
