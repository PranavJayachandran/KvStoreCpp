#include <string>
#include <fstream>
#include <filesystem>
#include "../include/kvstore/engine/FileHandler.h"
namespace kvstore::engine {
      void FileHandler::AppendToFile(const std::string &file_name, std::string &data){ 
        std::ofstream out(file_name, std::ios::app | std::ios::binary);
        if(!out)
          throw std::runtime_error("Failed to open file: " + file_name);

        out << data;
      }

      std::string FileHandler::ReadFromFile(const std::string &file_name, int start_pos, int block_size){
        std::ifstream in(file_name, std::ios::binary);
        if(!in)
          throw std::runtime_error("Failed to open file: " + file_name);

        in.seekg(start_pos);

        std::string buffer(block_size, '\0');
        in.read(&buffer[0], block_size);
        return buffer;
      }

      void FileHandler::WriteToFile(const std::string &file_name, int start_pos, std::string &data){

        std::ifstream infile(file_name);
        if (!infile) {
          std::ofstream outfile(file_name); 
          outfile.close();
        }
        std::string temp_file_name = file_name + ".tmp";

        std::ifstream original(file_name, std::ios::binary); 
        if(!original)
          throw std::runtime_error("failed to open file: " + file_name);

        std::ofstream temp(temp_file_name, std::ios::binary | std::ios::out);
        if(!temp)
          throw std::runtime_error("failed to open file: " + temp_file_name);
        
        const size_t buffer_size = 4096;
        char buffer[buffer_size];
        std::streampos bytes_to_copy = start_pos;
        while(bytes_to_copy > 0){
          std::streamsize chunk = (bytes_to_copy > static_cast<std::streampos>(buffer_size)) ? buffer_size : static_cast<std::streamsize>(bytes_to_copy);
          original.read(buffer, chunk);
          temp.write(buffer,chunk);
          bytes_to_copy -= chunk;
        }

        temp.write(data.data(), data.size());

        while(original){
          original.read(buffer, buffer_size);
          std::streamsize s = original.gcount();
          if(s > 0){
            temp.write(buffer,s);
          }
        }

        original.close();
        temp.close();

        if (std::remove(file_name.c_str()) != 0) {
          throw std::runtime_error("Failed to remove original file");
        }
        if (std::rename(temp_file_name.c_str(), file_name.c_str()) != 0) {
          throw std::runtime_error("Failed to rename temp file");
        }      
      }

      int FileHandler::GetSize(const std::string &file_name){
        return std::filesystem::file_size(file_name);
      }
}
