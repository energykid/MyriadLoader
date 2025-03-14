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

	double AttackDamage = (Arguments[0])->ToDouble();

	{
		if (is_custom)
		{
			{
				for (double var = 0; var < AllData.size(); var++)
				{
					ContentData data = AllData[var];
					{
						if (data.DataType == "enemy")
						{
							if (data.Name == CustomDataString.ToString() || data.Name == "all")
							{
								sol::protected_function_result result = data.TakeDamage.call(InstanceID, AttackDamage);
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
			//if (modState.at(stateNum)["all_behaviors"])
			{
				for (double var = 0; var < AllData.size(); var++)
				{
					ContentData data = AllData[var];
					{
						if (data.DataType == "enemy")
						{
							if (data.Name == ObjectIndexString.ToString() || data.Name == "all")
							{
								sol::protected_function_result result = data.TakeDamage.call(InstanceID, AttackDamage);
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
	return Result;
}

void DatabaseLoader::GMHooks::EnemyData(FWCodeEvent& FunctionContext)
{
	CCode* Code = std::get<2>(FunctionContext.Arguments());

	CInstance* GlobalInstance;

	RValue view;
	g_YYTKInterface->GetGlobalInstance(&GlobalInstance);

	g_YYTKInterface->GetBuiltin("view_current", GlobalInstance, 0, view);

	RValue viewCamera = g_YYTKInterface->CallBuiltin("view_get_camera", { view });

	CInstance* Self = std::get<0>(FunctionContext.Arguments());
	RValue Instance = Self->ToRValue();
	double InstanceID = g_YYTKInterface->CallBuiltin("variable_instance_get", { Instance, "id" }).ToDouble();

	RValue objectIndex = g_YYTKInterface->CallBuiltin("variable_instance_get", { Instance, "object_index" });

	string CustomDataString = "";

	for (size_t stateNum = 0; stateNum < modState.size(); stateNum++)
	{
		modState.at(stateNum)["view_x"] = g_YYTKInterface->CallBuiltin("camera_get_view_x", { viewCamera }).ToDouble();
		modState.at(stateNum)["screen_center_x"] = modState.at(stateNum).get<double>("view_x") + (290 / 2);
		modState.at(stateNum)["view_y"] = g_YYTKInterface->CallBuiltin("camera_get_view_y", { viewCamera }).ToDouble();
		modState.at(stateNum)["screen_center_y"] = modState.at(stateNum).get<double>("view_x") + (464 / 2);
	}

	bool is_custom = false;
	if (g_YYTKInterface->CallBuiltin("instance_variable_exists", { Instance, "dbl_CustomData" }).ToBoolean())
	{
		is_custom = true;
		CustomDataString = g_YYTKInterface->CallBuiltin("variable_instance_get", { Instance, "dbl_CustomData" }).ToString();
	}

	{
		if (is_custom)
		{
			{
				for (double var = 0; var < AllData.size(); var++)
				{
					ContentData data = AllData[var];
					//if (modState.at(stateNum)["all_behaviors"][var])
					{
						if (data.Name == CustomDataString || data.Name == "all")
						{
							if (data.DataType == "enemy")
							{
								// Create script
								if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Create_0")
								{
									sol::protected_function_result result = data.Create.call(InstanceID);
									if (!result.valid())
									{
										sol::error error = result;

										g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
									}
								}
								// Destroy script
								if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Destroy_0")
								{
									sol::protected_function_result result = data.Destroy.call(InstanceID);
									if (!result.valid())
									{
										sol::error error = result;

										g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
									}
								}
								// Draw script
								if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Draw_0")
								{
									sol::protected_function_result result = data.Draw.call(InstanceID);
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
		else
		{
			{
				for (double var = 0; var < AllData.size(); var++)
				{
					ContentData data = AllData[var];
					//if (modState.at(stateNum)["all_behaviors"][var])
					{
						// Enemy scripts
						if (data.DataType == "enemy")
						{
							if (data.Name == g_YYTKInterface->CallBuiltin("object_get_name", { objectIndex }).ToString()
								|| data.Name == "all")
							{
								// Create script
								if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Create_0")
								{
									sol::protected_function_result result = data.Create.call(InstanceID);
									if (!result.valid())
									{
										sol::error error = result;

										g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
									}
								}
								// Destroy script
								if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Destroy_0")
								{
									sol::protected_function_result result = data.Destroy.call(InstanceID);
									if (!result.valid())
									{
										sol::error error = result;

										g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
									}
								}
								// Draw script
								if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Draw_0")
								{
									sol::protected_function_result result = data.Draw.call(InstanceID);
									if (!result.valid())
									{
										sol::error error = result;

										g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
									}
								}
							}
						}
						// Global scripts
						if (data.DataType == "global")
						{
							// Draw script
							if ((string)Code->GetName() == (string)"gml_Object_obj_view_Draw_73")
							{
								sol::protected_function_result result = data.DrawUI.call();
								if (!result.valid())
								{
									sol::error error = result;

									g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
								}
							}
							// DrawInGame script
							if ((string)Code->GetName() == (string)"gml_Object_obj_player_Draw_0")
							{
								sol::protected_function_result result = data.Draw.call();
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