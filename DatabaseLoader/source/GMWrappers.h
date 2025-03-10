#pragma once

#include "YYToolkit/YYTK_Shared.hpp"
#include "Aurie/shared.hpp"

using namespace Aurie; 
using namespace YYTK;
using namespace std;

namespace DatabaseLoader
{
	class GMWrappers
	{
	public:
		static RValue GetGlobal(string name);
		static void SetGlobal(string name, RValue val);
		static RValue CallGameScript(IN std::string_view ScriptName, IN const std::vector<RValue>& Arguments);
		static AurieStatus CallGameScriptEx(OUT RValue& Result, IN std::string_view ScriptName, IN CInstance* SelfInstance, IN CInstance* OtherInstance, IN const std::vector<RValue>& Arguments);
	};
}