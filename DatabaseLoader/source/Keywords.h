#pragma once

#include "YYToolkit/Shared.hpp"

using namespace std;
using namespace Aurie;
using namespace YYTK;

namespace DatabaseLoader
{
	class Keywords
	{
	public:
		static void ResolveWeaponBase(
			OUT RValue* Result,
			IN CInstance* Self,
			IN CInstance* Other,
			IN int ArgumentCount,
			IN RValue* Arguments
		);
	};
}