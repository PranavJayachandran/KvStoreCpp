#include "../include/kvstore/engine/FileHandler.h"
#include <fstream>
#include <iostream>
#include <gtest/gtest.h>

using kvstore::engine::FileHandler;

class FileHandlerTest : public ::testing::Test {
protected:
  std::string test_file;

  void SetUp() override {
    test_file = "gtest_temp_file.txt";
  }

  void TearDown() override {
    std::remove(test_file.c_str());
  }

  std::string ReadWholeFile() {
    std::ifstream in(test_file, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  }
};

TEST_F(FileHandlerTest, AppendToFile_AppendsCorrectly) {
  {
    std::ofstream out(test_file, std::ios::binary);
    out << "Hello ";
  }
  std::string val = "World";
  FileHandler::AppendToFile(test_file, val);

  std::string content = ReadWholeFile();
  EXPECT_EQ(content, "Hello World");
}

TEST_F(FileHandlerTest, ReadFromFile_ReadsCorrectSubstring) {
  {
    std::ofstream out(test_file, std::ios::binary);
    out << "Hello World";
  }

  std::string data = FileHandler::ReadFromFile(test_file, 2, 5);
  EXPECT_EQ(data, "llo W");
}

TEST_F(FileHandlerTest, WriteToFile_WritesToTheFile) {
  std::string val = "12 ";
  FileHandler::WriteToFile(test_file, val);

  std::string content = ReadWholeFile();
  EXPECT_EQ(content, "12");
}

TEST_F(FileHandlerTest, GetSize_ReturnsCorrectSize) {
  std::string content = "Hello, world!";
  {
    std::ofstream out(test_file, std::ios::binary);
    out << content;
  }

  int size = FileHandler::GetSize(test_file);
  EXPECT_EQ(size, content.size());
}
