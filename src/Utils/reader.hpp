#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <format>

namespace reader
{

class BadFileException : std::exception
{

public:
    std::string file_path;

    BadFileException(std::string file_path)
        : file_path(file_path) {}

    const char* what() const throw()
    {
        return std::format("File '{}' does not exist", file_path).c_str();
    }
};

std::string read_file(std::string path)
{
    std::ifstream file(path);

    if (!file)
    {
        throw BadFileException(path);
    }

    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    file.close();

    return source;
}

} // namespace reader
