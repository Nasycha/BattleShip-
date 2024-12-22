#include "FileHandler.h"
#include "FileExeption.h"

FileHandler::FileHandler(const std::string& file_name) : file_name(file_name) {}

void FileHandler::openForRead() {
    input_file.open(file_name);
    if (!input_file.is_open()) {
        throw FileExeption("Can't open file for reading");
    }
}

void FileHandler::openForWrite() {
    output_file.open(file_name);
    if (!output_file.is_open()) {
        throw FileExeption("Can't open file for writing.");
    }
}

void FileHandler::write(const nlohmann::json& info) {
    if (output_file.is_open()) {
        output_file << info.dump(4);
    } else {
        throw FileExeption("File not open for writing.");
    }
}

void FileHandler::read(nlohmann::json& info) {
    if (input_file.is_open()) {
        input_file >> info;
    } else {
        throw FileExeption("File not open for reading.");
    }
}

void FileHandler::closeRead() {
    if (input_file.is_open()) {
        input_file.close();
    }
}

void FileHandler::closeWrite() {
    if (output_file.is_open()) {
        output_file.close();
    }
}

FileHandler::~FileHandler() {
    closeRead();
    closeWrite();
}
