#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include "Files.h"

using namespace std;
using namespace std::filesystem;

vector<filesystem::path> DatabaseLoader::Files::GetFilesOfType(const string& dir_path, const string& extension)
{
    vector<path> files;
    for (recursive_directory_iterator i(dir_path), end; i != end; ++i)
    {
        if (!is_directory(i->path()) && i->path().filename().extension() == extension)
        {
            files.push_back(i->path());
        }
    }
    return files;
}

bool DatabaseLoader::Files::MakeDirectory(string dir_name)
{
    if (std::filesystem::create_directories(dir_name))
    {
        g_YYTKInterface->PrintInfo("[DatabaseLoader] Directory created: " + dir_name);
    }
}