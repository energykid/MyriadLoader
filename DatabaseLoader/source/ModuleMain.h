#pragma once
#include "YYToolkit/YYTK_Shared.hpp"
#include "sol/sol.hpp"

using namespace std;
using namespace Aurie;
using namespace YYTK;

namespace DatabaseLoader
{
	inline int behaviorCount = 0;
	inline int currentState = 0;
	inline bool loadingMods = true;
	class ModuleMain
	{
	};
}