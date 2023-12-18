#include "fileloader.h"
#include <fstream>

char* loadText(const char* fileName)
{
    std::ifstream file(fileName, std::ios::ate);
    unsigned int fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    char* text = new char[fileSize];
    file.get(text, fileSize, '\0');
    return text;
}
