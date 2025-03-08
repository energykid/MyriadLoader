#pragma once

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

namespace DatabaseLoader
{
	class Files
	{
	public:
		static std::vector<std::string> GetFilesOfType(const std::string& dir_path, const std::string& extension);
	};
}