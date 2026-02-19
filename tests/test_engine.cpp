
#include <filesystem>
#include <gtest/gtest.h>
#include <string>

#include "../include/kvstore/engine/Engine.h"

using namespace kvstore::engine;

class EngineTest : public ::testing::Test {
protected:
  std::string test_wal_dir;
  std::string test_sst_dir;

  void SetUp() override {
    kvstore::config::LoadFromFile("../config.json");

    test_wal_dir = kvstore::config::GetConfig().wal_dir;
    test_sst_dir = kvstore::config::GetConfig().sst_dir;

    std::filesystem::remove_all(test_wal_dir);
    std::filesystem::remove_all(test_sst_dir);

    std::filesystem::create_directory(test_wal_dir);
    std::filesystem::create_directory(test_sst_dir);

    std::ofstream file("sst.txt");
    file.close();
  }

  void TearDown() override {
    std::filesystem::remove_all(test_wal_dir);
    std::filesystem::remove_all(test_sst_dir);
  }
};

TEST_F(EngineTest, BasicInsertAndGet) {
  Engine<> engine;

  engine.Add("key1", "value1");

  std::string out;
  bool found = engine.Get("key1", out);

  EXPECT_TRUE(found);
  EXPECT_EQ(out, "value1");
}

TEST_F(EngineTest, DeleteRemovesKey) {
  Engine<> engine;

  engine.Add("key1", "value1");
  engine.Delete("key1");

  std::string out;
  bool found = engine.Get("key1", out);

  EXPECT_FALSE(found);
}

TEST_F(EngineTest, MultipleKeys) {
  Engine<> engine;

  for (int i = 0; i < 10; i++) {
    engine.Add("key" + std::to_string(i), "value" + std::to_string(i));
  }

  std::string out;
  EXPECT_TRUE(engine.Get("key9", out));
  EXPECT_EQ(out, "value9");

  EXPECT_TRUE(engine.Get("key4", out));
  EXPECT_EQ(out, "value4");
}

TEST_F(EngineTest, OverwriteValue) {
  Engine<> engine;

  engine.Add("key1", "value1");
  engine.Add("key1", "value2");

  std::string out;
  bool found = engine.Get("key1", out);

  EXPECT_TRUE(found);
  EXPECT_EQ(out, "value2");
}

TEST_F(EngineTest, FlushToSST) {
  Engine<> engine;

  // small memtable triggers flush quickly
  for (int i = 0; i < 50; i++) {
    engine.Add("k" + std::to_string(i), "v" + std::to_string(i));
  }

  // give flush thread time
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  std::string out;
  EXPECT_TRUE(engine.Get("k10", out));
  EXPECT_EQ(out, "v10");
}

TEST_F(EngineTest, RecoveryAfterRestart) {
  {
    Engine<> engine;
    engine.Add("persist_ke", "persist_valu");
  }

  // simulate restart
  Engine<> recovered;

  std::string out;
  bool found = recovered.Get("persist_ke", out);

  EXPECT_TRUE(found);
  EXPECT_EQ(out, "persist_valu");
}

// TEST_F(EngineTest, ConcurrentWrites) {
//   Engine<> engine;
//
//   const int threads = 4;
//   const int ops = 200;
//
//   std::vector<std::thread> workers;
//
//   for (int t = 0; t < threads; t++) {
//     workers.emplace_back([&]() {
//       for (int i = 0; i < ops; i++) {
//         engine.Add("key" + std::to_string(i), "value" + std::to_string(i));
//       }
//     });
//   }
//
//   for (auto &t : workers)
//     t.join();
//
//   std::string out;
//   EXPECT_TRUE(engine.Get("key50", out));
// }
