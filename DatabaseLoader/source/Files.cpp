#include <iostream>
#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include "Files.h"

using namespace std;
using namespace std::filesystem;

string DatabaseLoader::Files::GetSteamDirectory()
{
    return "C:/Program Files (x86)/Steam/steamapps/common/Star of Providence/";
}

string DatabaseLoader::Files::GetModsDirectory()
{
    return DatabaseLoader::Files::GetSteamDirectory() + "DatabaseLoader/Mods";
}

std::vector<filesystem::path> DatabaseLoader::Files::GetImmediateSubfolders(const std::string& dir_path)
{
    vector<path> files;
    for (directory_iterator i(dir_path), end; i != end; ++i)
    {
        if (is_directory(i->path()))
        {
            files.push_back(i->path());
        }
    }
    return files;
}

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

bool DatabaseLoader::Files::CopyFileTo(const std::string& sourcePath, const std::string& destinationPath) {
    std::ifstream sourceFile(sourcePath, std::ios::binary);
    if (!sourceFile.is_open()) {
        std::cerr << "Error opening source file: " << sourcePath << std::endl;
        return false;
    }

    std::ofstream destinationFile(destinationPath, std::ios::binary);
    if (!destinationFile.is_open()) {
        std::cerr << "Error opening destination file: " << destinationPath << std::endl;
        return false;
    }

    destinationFile << sourceFile.rdbuf();

    if (sourceFile.fail() || destinationFile.fail()) {
        std::cerr << "Error during file copy." << std::endl;
        sourceFile.close();
        destinationFile.close();
        return false;
    }

    sourceFile.close();
    destinationFile.close();
    return true;
}