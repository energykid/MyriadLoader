#include "Aurie/shared.hpp"
#include "ModuleMain.h"
#include "DatabaseLoader.h"
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
		g_YYTKInterface->CallBuiltin("array_push", { Result, g_YYTKInterface->CallBuiltin("array_get", { GMWrappers::GetGlobal("dbl_CustomMusicList"), i })});

		g_YYTKInterface->PrintInfo("music: " + to_string(g_YYTKInterface->CallBuiltin("array_get", { GMWrappers::GetGlobal("dbl_CustomMusicList"), i }).ToDouble()));
	}

	return Result;
}

RValue& DatabaseLoader::GMHooks::MusicDo(IN CInstance* Self, IN CInstance* Other, OUT RValue& Result, IN int ArgumentCount, IN RValue** Arguments)
{
	auto original_function = reinterpret_cast<decltype(&JukeboxInjection)>(MmGetHookTrampoline(g_ArSelfModule, "MusicDo"));
	RValue& return_value = original_function(Self, Other, Result, ArgumentCount, Arguments);

	RValue snd = *Arguments[0];
	RValue Sound = g_YYTKInterface->CallBuiltin("audio_play_sound", { snd, 0, false });
	GMWrappers::SetGlobal("current_music", Sound);
	g_YYTKInterface->CallBuiltin("audio_sound_pitch", { GMWrappers::GetGlobal("current_music"), 1 });
	GMWrappers::SetGlobal("song_length", g_YYTKInterface->CallBuiltin("audio_sound_length", { snd }));
	GMWrappers::SetGlobal("loop_length", 0); // todo: add custom loop length to music

	Result = Sound;

	return Result;
}

RValue& DatabaseLoader::GMHooks::MusicDoLoop(IN CInstance* Self, IN CInstance* Other, OUT RValue& Result, IN int ArgumentCount, IN RValue** Arguments)
{
	auto original_function = reinterpret_cast<decltype(&JukeboxInjection)>(MmGetHookTrampoline(g_ArSelfModule, "MusicDoLoop"));
	RValue& return_value = original_function(Self, Other, Result, ArgumentCount, Arguments);

	return Result;
}

RValue& DatabaseLoader::GMHooks::MusicDoLoopFromStart(IN CInstance* Self, IN CInstance* Other, OUT RValue& Result, IN int ArgumentCount, IN RValue** Arguments)
{
	auto original_function = reinterpret_cast<decltype(&JukeboxInjection)>(MmGetHookTrampoline(g_ArSelfModule, "MusicDoLoopFromStart"));
	RValue& return_value = original_function(Self, Other, Result, ArgumentCount, Arguments);

	RValue snd = *Arguments[0];
	
	if (snd.ToDouble() > 267)
	{
		RValue Sound = g_YYTKInterface->CallBuiltin("audio_play_sound", { snd, 0, true });
		GMWrappers::SetGlobal("current_music", Sound);
		g_YYTKInterface->CallBuiltin("audio_sound_pitch", { GMWrappers::GetGlobal("current_music"), 1 });
		GMWrappers::SetGlobal("song_length", g_YYTKInterface->CallBuiltin("audio_sound_length", { snd }));
		GMWrappers::SetGlobal("loop_length", 0); // todo: add custom loop length to music
		Result = Sound;
	}

	return Result;
}

RValue& DatabaseLoader::GMHooks::EnemyDamage(IN CInstance* Self, IN CInstance* Other, OUT RValue& Result, IN int ArgumentCount, IN RValue** Arguments)
{
	auto original_function = reinterpret_cast<decltype(&EnemyDamage)>(MmGetHookTrampoline(g_ArSelfModule, "EnemyDamage"));
	RValue& return_value = original_function(Self, Other, Result, ArgumentCount, Arguments);

	RValue Instance = Self->ToRValue();
	double InstanceID = g_YYTKInterface->CallBuiltin("variable_instance_get", { Instance, "id" }).ToDouble();

	RValue objectIndex = g_YYTKInterface->CallBuiltin("variable_instance_get", { Instance, "object_index" }).ToDouble();
	RValue CustomDataString = "";

	RValue ObjectIndexString = g_YYTKInterface->CallBuiltin("object_get_name", { objectIndex });

	bool is_custom = false;
	if (g_YYTKInterface->CallBuiltin("instance_variable_exists", { Instance, "dbl_CustomData" }).ToBoolean())
	{
		is_custom = true;
		CustomDataString = g_YYTKInterface->CallBuiltin("variable_instance_get", { Instance, "dbl_CustomData" });
	}

	double AttackDamage = 2;

	for (int stateNum = 0; stateNum < modState.size(); stateNum++)
	{
		if (is_custom)
		{
			if (modState.at(stateNum)["all_behaviors"])
			{
				sol::table count = modState.at(stateNum)["all_behaviors"];
				for (double var = 0; var < count.size() + 1; var++)
				{
					sol::table tbl = modState.at(stateNum)["all_behaviors"][var];
					if (modState.at(stateNum)["all_behaviors"][var])
					{
						if (tbl.get<string>("Name") == CustomDataString.ToString() || tbl.get<string>("Name") == "all")
						{
							sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["TakeDamage"].call(InstanceID, AttackDamage);
							if (!result.valid())
							{
								sol::error error = result;

								g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
							}
						}
					}
				}
			}
		}
		else
		{
			if (modState.at(stateNum)["all_behaviors"])
			{
				sol::table count = modState.at(stateNum)["all_behaviors"];
				for (double var = 0; var < count.size() + 1; var++)
				{
					sol::table tbl = modState.at(stateNum)["all_behaviors"][var];
					if (modState.at(stateNum)["all_behaviors"][var])
					{
						if (tbl.get<string>("Name") == ObjectIndexString.ToString() || tbl.get<string>("Name") == "all")
						{
							sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["TakeDamage"].call(InstanceID, AttackDamage);
							if (!result.valid())
							{
								sol::error error = result;

								g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
							}
						}
					}
				}
			}
		}
	}

	return Result;
}

void DatabaseLoader::GMHooks::EnemyData(FWCodeEvent& FunctionContext)
{
	CCode* Code = std::get<2>(FunctionContext.Arguments());

	CInstance* Self = std::get<0>(FunctionContext.Arguments());
	RValue Instance = Self->ToRValue();
	double InstanceID = g_YYTKInterface->CallBuiltin("variable_instance_get", { Instance, "id" }).ToDouble();

	RValue objectIndex = g_YYTKInterface->CallBuiltin("variable_instance_get", { Instance, "object_index" });

	if (!g_YYTKInterface->CallBuiltin("object_is_ancestor", { objectIndex, g_YYTKInterface->CallBuiltin("asset_get_index", {"obj_enemy"}) }).ToBoolean())
	{
		return;
	}

	string CustomDataString = "";

	bool is_custom = false;
	if (g_YYTKInterface->CallBuiltin("instance_variable_exists", { Instance, "dbl_CustomData" }).ToBoolean())
	{
		is_custom = true;
		CustomDataString = g_YYTKInterface->CallBuiltin("variable_instance_get", { Instance, "dbl_CustomData" }).ToString();
	}

	for (int stateNum = 0; stateNum < modState.size(); stateNum++)
	{
		if (is_custom)
		{
			if (dl_lua["all_behaviors"])
			{
				sol::table count = dl_lua["all_behaviors"];
				for (double var = 0; var < count.size() + 1; var++)
				{
					sol::table tbl = dl_lua["all_behaviors"][var];
					if (dl_lua["all_behaviors"][var])
					{
						if (tbl.get<string>("Name") == CustomDataString || tbl.get<string>("Name") == "all")
						{
							// Create script
							if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Create_0")
							{
								sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["Create"].call(InstanceID);
								if (!result.valid())
								{
									sol::error error = result;

									g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
								}
							}
							// Destroy script
							if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Destroy_0")
							{
								sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["Destroy"].call(InstanceID);
								if (!result.valid())
								{
									sol::error error = result;

									g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
								}
							}
						}
					}
				}
			}
		}
		else
		{
			if (dl_lua["all_behaviors"])
			{
				sol::table count = dl_lua["all_behaviors"];
				for (double var = 0; var < count.size() + 1; var++)
				{
					sol::table tbl = dl_lua["all_behaviors"][var];
					if (dl_lua["all_behaviors"][var])
					{
						if (tbl.get<string>("Name") == g_YYTKInterface->CallBuiltin("object_get_name", { objectIndex }).ToString()
							|| tbl.get<string>("Name") == "all")
						{
							// Create script
							if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Create_0")
							{
								sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["Create"].call(InstanceID);
								if (!result.valid())
								{
									sol::error error = result;

									g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
								}
							}
							// Destroy script
							if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Destroy_0")
							{
								sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["Destroy"].call(InstanceID);
								if (!result.valid())
								{
									sol::error error = result;

									g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
								}
							}
						}
					}
				}
			}
		}
	}
}