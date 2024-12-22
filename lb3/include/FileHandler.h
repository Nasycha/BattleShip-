#pragma once

#include <string>
#include <fstream>
#include "json.hpp"

class FileHandler {
private:
    std::string file_name;
    std::ifstream input_file;
    std::ofstream output_file;

public:
    FileHandler(const std::string& file_name);
    void openForRead();
    void openForWrite();
    void write(const nlohmann::json& info);
    void read(nlohmann::json& info);
    void closeRead();
    void closeWrite();
    ~FileHandler();
};