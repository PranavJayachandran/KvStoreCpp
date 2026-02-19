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
    std::filesystem::remove_all(test_wal_dir);
    std::filesystem::create_directory(test_wal_dir);
  }

  void TearDown() override { std::filesystem::remove_all(test_wal_dir); }
};

TEST_F(MemtableManagerTest, BasicInsertAndGet) {
  MemtableManager<> manager(1024);

  manager.Add("key1", "value1");

  std::string out;
  bool found = manager.Get("key1", out);

  EXPECT_TRUE(found);
  EXPECT_EQ(out, "value1");
}

TEST_F(MemtableManagerTest, DeleteRemovesKey) {
  MemtableManager<> manager(1024);

  manager.Add("key1", "value1");
  manager.Delete("key1");

  std::string out;
  bool found = manager.Get("key1", out);

  EXPECT_FALSE(found);
}

TEST_F(MemtableManagerTest, SwapCreatesImmutableMemtable) {
  MemtableManager<> manager(1); // small threshold forces swap

  manager.Add("key1", "value1");
  manager.Add("key2", "value2");

  auto imm = manager.GetImmuableMemtables();

  EXPECT_FALSE(imm.empty());
}

TEST_F(MemtableManagerTest, ReadFromImmutableAfterSwap) {
  MemtableManager<> manager(1);

  manager.Add("key1", "value1");
  manager.Add("key2", "value2"); // triggers swap

  std::string out;
  bool found = manager.Get("key1", out);

  EXPECT_TRUE(found);
  EXPECT_EQ(out, "value1");
}

TEST_F(MemtableManagerTest, RecoveryFromWal) {
  {
    MemtableManager<> manager(1024);
    manager.Add("key1", "value1");
  }

  // Simulate restart
  MemtableManager<> recovered(1024);

  std::string out;
  bool found = recovered.Get("key1", out);

  EXPECT_TRUE(found);
  EXPECT_EQ(out, "value1");
}

TEST_F(MemtableManagerTest, RecoveryWithMultipleWalFiles) {
  {
    MemtableManager<> manager(1);
    manager.Add("k1", "v1");
    manager.Add("k2", "v2"); // forces swap
    manager.Add("k3", "v3");
  }

  MemtableManager<> recovered(1024);

  std::string out;

  EXPECT_TRUE(recovered.Get("k1", out));
  EXPECT_EQ(out, "v1");

  EXPECT_TRUE(recovered.Get("k2", out));
  EXPECT_EQ(out, "v2");

  EXPECT_TRUE(recovered.Get("k3", out));
  EXPECT_EQ(out, "v3");
}

TEST_F(MemtableManagerTest, ConcurrentWrites) {
  MemtableManager<> manager(1000);

  const int thread_count = 8;
  const int ops = 500;

  std::vector<std::thread> threads;

  for (int t = 0; t < thread_count; ++t) {
    threads.emplace_back([&]() {
      for (int i = 0; i < ops; ++i) {
        manager.Add("key" + std::to_string(i), "value" + std::to_string(i));
      }
    });
  }

  for (auto &th : threads)
    th.join();

  std::string out;
  EXPECT_TRUE(manager.Get("key10", out));
}
