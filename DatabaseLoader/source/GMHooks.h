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
		static RValue& PlayerTakeHit(
			IN CInstance* Self,
			IN CInstance* Other,
			OUT RValue& Result,
			IN int ArgumentCount,
			IN RValue** Arguments
		);
		static RValue& ReloadAllMods(
			IN CInstance* Self,
			IN CInstance* Other,
			OUT RValue& Result,
			IN int ArgumentCount,
			IN RValue** Arguments
		);
		static RValue& SpawnRoomObject(
			IN CInstance* Self,
			IN CInstance* Other,
			OUT RValue& Result,
			IN int ArgumentCount,
			IN RValue** Arguments
		);
		static RValue& WriteSaveData(
			IN CInstance* Self,
			IN CInstance* Other,
			OUT RValue& Result,
			IN int ArgumentCount,
			IN RValue** Arguments
		);
		static RValue& WriteMidSave(
			IN CInstance* Self,
			IN CInstance* Other,
			OUT RValue& Result,
			IN int ArgumentCount,
			IN RValue** Arguments
		);
		static RValue& ExitGame(
			IN CInstance* Self,
			IN CInstance* Other,
			OUT RValue& Result,
			IN int ArgumentCount,
			IN RValue** Arguments
		);
		static RValue& EnterRun(
			IN CInstance* Self,
			IN CInstance* Other,
			OUT RValue& Result,
			IN int ArgumentCount,
			IN RValue** Arguments
		);
		static RValue& ChooseBossIntro(
			IN CInstance* Self,
			IN CInstance* Other,
			OUT RValue& Result,
			IN int ArgumentCount,
			IN RValue** Arguments
		);

		static void FloorData(
			FWCodeEvent& FunctionContext
		);

		static void EnemyData(
			FWCodeEvent& FunctionContext
		);

		static void CartridgeData(
			FWCodeEvent& FunctionContext
		);

	};
}