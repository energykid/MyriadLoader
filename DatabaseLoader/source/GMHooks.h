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
		static RValue& MusicDo(
			IN CInstance* Self,
			IN CInstance* Other,
			OUT RValue& Result,
			IN int ArgumentCount,
			IN RValue** Arguments
		);
		static RValue& MusicDoLoop(
			IN CInstance* Self,
			IN CInstance* Other,
			OUT RValue& Result,
			IN int ArgumentCount,
			IN RValue** Arguments
		);
		static RValue& MusicDoLoopFromStart(
			IN CInstance* Self,
			IN CInstance* Other,
			OUT RValue& Result,
			IN int ArgumentCount,
			IN RValue** Arguments
		);
		static RValue& EnemyDamage(
			IN CInstance* Self,
			IN CInstance* Other,
			OUT RValue& Result,
			IN int ArgumentCount,
			IN RValue** Arguments
		);
		static void EnemyData(
			FWCodeEvent& FunctionContext
		);
	};
}