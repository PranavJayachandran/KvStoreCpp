#include <filesystem>
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <vector>

#include "../include/kvstore/engine/MemtableManager.h"

using namespace kvstore::engine;

class MemtableManagerTest : public ::testing::Test {
protected:
  std::string test_wal_dir = kvstore::config::GetConfig().wal_dir;

  void SetUp() override {
    kvstore::config::LoadFromFile("../config.json");
    test_wal_dir = kvstore::config::GetConfig().wal_dir;
    std::filesystem::remove_all(test_wal_dir);
    std::filesystem::create_directory(test_wal_dir);
  }

  void TearDown() override { std::filesystem::remove_all(test_wal_dir); }
};

bool CallBack(MemtableIterator<>) { return false; }
TEST_F(MemtableManagerTest, BasicInsertAndGet) {
  MemtableManager<> manager(1024, CallBack);

  manager.Add("key1", "value1");

  std::string out;
  bool found = manager.Get("key1", out);

  EXPECT_TRUE(found);
  EXPECT_EQ(out, "value1");
}

TEST_F(MemtableManagerTest, DeleteRemovesKey) {
  MemtableManager<> manager(1024, CallBack);

  manager.Add("key1", "value1");
  manager.Delete("key1");

  std::string out;
  bool found = manager.Get("key1", out);

  EXPECT_FALSE(found);
}

TEST_F(MemtableManagerTest, SwapCreatesImmutableMemtable) {
  MemtableManager<> manager(1, CallBack); // small threshold forces swap

  manager.Add("key1", "value1");
  manager.Add("key2", "value2");

  auto imm = manager.GetImmuableMemtables();

  EXPECT_FALSE(imm.empty());
}

TEST_F(MemtableManagerTest, RecoveryFromWal) {
  {
    MemtableManager<> manager(1024, CallBack);
    manager.Add("key1", "value1");
  }

  // Simulate restart
  MemtableManager<> recovered(1024, CallBack);

  std::string out;
  bool found = recovered.Get("key1", out);

  EXPECT_TRUE(found);
  EXPECT_EQ(out, "value1");
}

TEST_F(MemtableManagerTest, RecoveryWithMultipleWalFiles) {
  {
    MemtableManager<> manager(1, CallBack);
    manager.Add("k1", "v1");
    manager.Add("k2", "v2"); // forces swap
    manager.Add("k3", "v3");
  }

  MemtableManager<> recovered(1024, CallBack);

  std::string out;

  EXPECT_TRUE(recovered.Get("k1", out));
  EXPECT_EQ(out, "v1");

  EXPECT_TRUE(recovered.Get("k2", out));
  EXPECT_EQ(out, "v2");

  EXPECT_TRUE(recovered.Get("k3", out));
  EXPECT_EQ(out, "v3");
}
