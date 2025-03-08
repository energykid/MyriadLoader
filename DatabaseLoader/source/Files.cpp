#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include "Files.h"

using namespace std;
using namespace std::filesystem;

namespace fs = std::filesystem;

std::vector<std::string> DatabaseLoader::Files::GetFilesOfType(const std::string& dir_path, const std::string& extension)
{
    std::vector<std::string> files; 
    for (recursive_directory_iterator i(dir_path), end; i != end; ++i)
        if (!is_directory(i->path()) && i->path().filename().extension() == extension)
            files.push_back(i->path().filename().string());
    return files;
}
