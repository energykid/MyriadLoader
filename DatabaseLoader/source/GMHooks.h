#pragma once

#include "Aurie/shared.hpp"
#include "GMWrappers.h"
#include "DatabaseLoader.h"
#include "YYToolkit/YYTK_Shared.hpp"

namespace DatabaseLoader
{
	class GMHooks
	{
	public:
		static RValue& JukeboxInjection(
			IN CInstance* Self,
			IN CInstance* Other,
			OUT RValue& Result,
			IN int ArgumentCount,
			IN RValue** Arguments
		);
	};
}