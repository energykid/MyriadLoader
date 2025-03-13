#pragma once

#include <nlohmann/json.hpp>
#include "sol/sol.hpp"
#include "Files.h"

using namespace std;
using namespace nlohmann;

namespace DatabaseLoader
{
	class SaverLoader
	{
	public:
		static void SaveVariable(string name, sol::object var);
		static sol::object LoadVariable(string name);
	};
}