#include "Aurie/shared.hpp"
#include "ModuleMain.h"
#include "GMWrappers.h"
#include "GMHooks.h"
#include "YYToolkit/YYTK_Shared.hpp"

using namespace DatabaseLoader;
using namespace Aurie;
using namespace YYTK;

RValue& DatabaseLoader::GMHooks::JukeboxInjection(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	auto original_function = reinterpret_cast<decltype(&JukeboxInjection)>(MmGetHookTrampoline(g_ArSelfModule, "Jukebox Injection"));
	RValue& return_value = original_function(Self, Other, Result, ArgumentCount, Arguments);

	for (int i = 0; i < g_YYTKInterface->CallBuiltin("array_length", { g_YYTKInterface->CallBuiltin("variable_global_get", { "dbl_CustomMusicList" }) }).ToDouble(); i++)
	{
		g_YYTKInterface->CallBuiltin("array_push", {Result, g_YYTKInterface->CallBuiltin("array_get", { g_YYTKInterface->CallBuiltin("variable_global_get", { "dbl_CustomMusicList" }), i }) });
	}

	g_YYTKInterface->PrintInfo("the return type is " + Result.GetKindName());

	return Result;
}

