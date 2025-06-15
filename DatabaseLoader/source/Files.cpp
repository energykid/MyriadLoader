#include <iostream>
#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include "Files.h"
#include "nlohmann/json.hpp"

using namespace std;
using namespace std::filesystem;
using namespace nlohmann;

string DatabaseLoader::Files::GetSteamDirectory()
{
    CInstance* GlobalInstance;
    RValue directory;
    g_YYTKInterface->GetGlobalInstance(&GlobalInstance);

    g_YYTKInterface->GetBuiltin("program_directory", GlobalInstance, 0, directory);

    return directory.ToString();
}

string DatabaseLoader::Files::GetModsDirectory()
{
    return DatabaseLoader::Files::GetSteamDirectory() + "MyriadLoader/Mods/";
}

string DatabaseLoader::Files::GetModSavesDirectory()
{
    return DatabaseLoader::Files::GetSteamDirectory() + "MyriadLoader/Saves/";
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
    bool dir = std::filesystem::create_directories(dir_name);
    if (dir)
    {
        g_YYTKInterface->PrintInfo("[Myriad Loader] Directory created: " + dir_name);
    }
    return dir;
}

std::string DatabaseLoader::Files::GetFileContents(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return ""; // Returns an empty string if the file fails to open
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int DatabaseLoader::Files::HashString(string name)
{
    std::hash<std::string> hash_fn;
    return 5000 + (hash_fn(name) % 10000);
}

bool DatabaseLoader::Files::CopyFileTo(const std::string& sourcePath, const std::string& destinationPath) {
    string sf = GetFileContents(sourcePath);

    std::ofstream destinationFile(destinationPath, std::ios::trunc);
    if (!destinationFile.is_open()) {
        std::cerr << "Error opening destination file: " << destinationPath << std::endl;
        return false;
    }

    destinationFile << sf;

    if (destinationFile.fail()) {
        std::cerr << "Error during file copy." << std::endl;
        destinationFile.close();
        return false;
    }

    destinationFile.close();
    return true;
}

bool DatabaseLoader::Files::AddRoomsToFile(const std::string& sourcePath, const std::string& destinationPath)
{

    std::ifstream sourceFile(sourcePath, std::ios::in);
    if (!sourceFile.is_open()) {
        std::cerr << "Error opening source file: " << sourcePath << std::endl;
        return false;
    }

    std::ifstream destinationFileIn(destinationPath, std::ios::in);
    if (!destinationFileIn.is_open()) {
        std::cerr << "Error opening destination file: " << destinationPath << std::endl;
        return false;
    }

    std::string line1;
    std::string line2;

    std::string content1 = GetFileContents(sourcePath);
    std::string content2 = GetFileContents(destinationPath);

    std::stringstream ss1(content1);
    std::stringstream ss2(content2);

    std::getline(ss1, line1);
    std::getline(ss2, line2);

    content1.erase(content1.length() - 2, content1.length() - 1);
    content1.erase(0, line1.length());
    content2.erase(0, line2.length());

    try 
    {

        int glag = std::stoi(line1) + std::stoi(line2);
        content2.insert(0, to_string(glag));

    }

    catch (const std::invalid_argument&)
    {
        g_YYTKInterface->PrintWarning(line1);
        g_YYTKInterface->PrintWarning(line2);

    }

    catch (const std::out_of_range&)
    {
        g_YYTKInterface->PrintWarning("out of range");
    }
    
    destinationFileIn.close();

    std::ofstream destinationFile(destinationPath, std::ios::trunc);
    if (!destinationFile.is_open()) {
        std::cerr << "Error opening destination file: " << destinationPath << std::endl;
        return false;
    }

    destinationFile << content2;
    destinationFile << content1;

    if (destinationFile.fail()) {
        std::cerr << "Error during room addition." << std::endl;
        sourceFile.close();
        destinationFile.close();
        return false;
    }

    sourceFile.close();
    destinationFile.close();
    return true;

}
