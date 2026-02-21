#include "../include/kvstore/engine/FileHandler.h"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
namespace kvstore::engine {
void FileHandler::AppendToFile(const std::string &file_name,
                               std::string &data) {
  std::ofstream out(file_name, std::ios::app | std::ios::binary);
  if (!out)
    throw std::runtime_error("Failed to open file: " + file_name);

  out << data;
}

std::string FileHandler::ReadFromFile(const std::string &file_name,
                                      int start_pos, int block_size) {
  std::ifstream in(file_name, std::ios::binary);
  if (!in)
    throw std::runtime_error("Failed to open file: " + file_name);

  in.seekg(start_pos);

  std::string buffer(block_size, '\0');
  in.read(&buffer[0], block_size);
  return buffer;
}

void FileHandler::CreateFile(const std::string &folder_name,
                             const std::string &file_name) {
  std::ofstream out(folder_name + "/" + file_name);
}

void FileHandler::WriteToFile(const std::string &file_name, std::string &data) {
  std::filesystem::path file_path(file_name);
  auto parent = file_path.parent_path();
  if (!parent.empty()) {
    std::filesystem::create_directories(file_path.parent_path());
  }
  std::ofstream out(file_name);
  if (!out.is_open()) {
    throw std::runtime_error("Unable to open file: " + file_name);
  }
  out << data;
  out.close();
}

void FileHandler::DeleteFile(const std::string &file_name) {
  if (std::remove(&file_name[0]) != 0) {
    std::cerr << "Failed to delete" + file_name;
  }
}

int FileHandler::GetSize(const std::string &file_name) {
  return std::filesystem::file_size(file_name);
}

int FileHandler::GetNumberofFiles(const std::string &folder_name) {
  int count = 0;
  for (const auto &entry : std::filesystem::directory_iterator(folder_name)) {
    if (std::filesystem::is_regular_file(entry.status())) {
      ++count;
    }
  }
  return count;
}

std::vector<std::string>
FileHandler::GetAllFileNames(const std::string &folder_name) {
  std::vector<std::string> file_names;
  for (const auto &entry : std::filesystem::directory_iterator(folder_name)) {
    if (entry.is_regular_file()) {
      file_names.push_back(entry.path().filename().string());
    }
  }
  return file_names;
}

FileHandler::FileStream
FileHandler::GetPointerToFile(const std::string &file_name) {
  auto file_stream =
      std::make_unique<std::ifstream>(file_name, std::ios::binary);
  if (file_stream->is_open()) {
    FileHandler::FileStream fs;
    fs.file_name = file_name, fs.stream = std::move(file_stream);
    return fs;
  } else {
    std::cerr << "Failed to open file: " << file_name << "\n";
    FileHandler::FileStream fs;
    return fs;
  }
}

std::vector<FileHandler::FileStream>
FileHandler::GetPointersToAllFiles(const std::string &folder_name) {
  std::vector<FileStream> streams;
  if (!std::filesystem::exists(folder_name) ||
      !std::filesystem::is_directory(folder_name)) {
    std::cerr << "Path is not a valid directory: " << folder_name << '\n';
    return streams;
  }
  for (const auto &entry : std::filesystem::directory_iterator(folder_name)) {
    if (entry.is_regular_file()) {
      auto file_stream =
          std::make_unique<std::ifstream>(entry.path(), std::ios::binary);
      if (file_stream->is_open()) {
        FileHandler::FileStream fs;
        fs.file_name = entry.path().string();
        fs.stream = std::move(file_stream);
        streams.push_back(std::move(fs));
      } else {
        std::cerr << "Failed to open file: " << entry.path() << '\n';
      }
    }
  }
  return streams;
}
} // namespace kvstore::engine
