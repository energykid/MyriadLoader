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

using namespace std;

namespace DatabaseLoader
{
	class Files
	{
	public:
		static std::vector<filesystem::path> GetFilesOfType(const std::string& dir_path, const std::string& extension);
		static bool MakeDirectory(string dir_name);
	};
}