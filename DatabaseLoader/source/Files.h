#pragma once
#include "DatabaseLoader.h"
#include "Aurie/shared.hpp"
#include "YYToolkit/YYTK_Shared.hpp"
#include "DBLua.h"
#include <map>
#include "sol/sol.hpp"
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include "nlohmann/json.hpp"

using namespace std;
using namespace nlohmann;

namespace DatabaseLoader
{
	class Files
	{
	public:
		static string GetSteamDirectory();
		static string GetModsDirectory();
		static string GetModSavesDirectory();
		static std::vector<filesystem::path> GetImmediateSubfolders(const std::string& dir_path);
		static bool CopyFileTo(const std::string& sourcePath, const std::string& destinationPath);
		static std::vector<filesystem::path> GetFilesOfType(const std::string& dir_path, const std::string& extension);
		static bool MakeDirectory(string dir_name);
		static std::string GetFileContents(const std::string& filePath);
		static int HashString(string name);
	};
}