#include "../include/kvstore/engine/BloomFilter.h"
#include <gtest/gtest.h>

using kvstore::engine::BloomFilter;

TEST(BloomFilterTest, AddAndExists){
  BloomFilter bf(10);
  bf.Add("key1");  
  bool isPresent = bf.Exists("key1");
  EXPECT_EQ(isPresent, true);
}

TEST(BloomFilterTest, DoesnotExists){
  BloomFilter bf(10);
  bool isPresent = bf.Exists("key1");
  EXPECT_EQ(isPresent, false);
}
TEST(BloomFilterTest, Clear){
  BloomFilter bf(10);
  bf.Add("key1");  
  bool isPresent = bf.Exists("key1");
  EXPECT_EQ(isPresent, true);

  bf.Clear();
  isPresent = bf.Exists("key1");
  EXPECT_EQ(isPresent, false);

}
