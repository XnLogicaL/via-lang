#include "reader.h"

std::string FileReader::read_file(std::string path)
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