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
	
	if (snd.ToDouble() > 500)
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
	if (g_YYTKInterface->CallBuiltin("instance_variable_exists", { Instance, "dbl_CustomEnemyName" }).ToBoolean())
	{
		is_custom = true;
		CustomDataString = g_YYTKInterface->CallBuiltin("variable_instance_get", { Instance, "dbl_CustomEnemyName" });
	}

	double AttackDamage = (Arguments[0])->ToDouble();

	if (is_custom)
	{
		for (double var = 0; var < AllData.size(); var++)
		{
			ContentData data = AllData[var];
			{
				if (data.DataType == "enemy")
				{
					if (data.Name == CustomDataString.ToString() || data.Name == "all")
					{
						data.TakeDamage(InstanceID, AttackDamage);
					}
				}
			}
		}
	}
	else
	{
		for (double var = 0; var < AllData.size(); var++)
		{
			ContentData data = AllData[var];
			{
				if (data.DataType == "enemy")
				{
					if (data.Name == ObjectIndexString.ToString() || data.Name == "all")
					{
						data.TakeDamage(InstanceID, AttackDamage);
					}
				}
			}
		}
	}
	return Result;
}

void DatabaseLoader::GMHooks::ContentDataEvents(FWCodeEvent& FunctionContext)
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
	string ObjectIndexString = g_YYTKInterface->CallBuiltin("object_get_name", { objectIndex }).ToString();

	for (size_t stateNum = 0; stateNum < modState.size(); stateNum++)
	{
		modState.at(stateNum)["view_x"] = g_YYTKInterface->CallBuiltin("camera_get_view_x", { viewCamera }).ToDouble();
		modState.at(stateNum)["screen_center_x"] = modState.at(stateNum).get<double>("view_x") + (290 / 2);
		modState.at(stateNum)["view_y"] = g_YYTKInterface->CallBuiltin("camera_get_view_y", { viewCamera }).ToDouble();
		modState.at(stateNum)["screen_center_y"] = modState.at(stateNum).get<double>("view_x") + (464 / 2);
	}

	bool is_custom = false;
	if (g_YYTKInterface->CallBuiltin("instance_variable_exists", { Instance, "dbl_CustomEnemyName" }).ToBoolean())
	{
		is_custom = true;
		CustomDataString = g_YYTKInterface->CallBuiltin("variable_instance_get", { Instance, "dbl_CustomEnemyName" }).ToString();
	}

	if (!is_custom)
	{
		for (double var = 0; var < AllData.size(); var++)
		{
			ContentData data = AllData[var];

			// Global scripts
			if (data.DataType == "global")
			{
				// Draw script
				if ((string)Code->GetName() == (string)"gml_Object_obj_view_Draw_73")
				{
					data.DrawUI();
					return;
				}
				// DrawInGame script
				if ((string)Code->GetName() == (string)"gml_Object_obj_player_Draw_0")
				{
					data.Draw(0);
					return;
				}
			}
			// Individual object scripts
			else if (data.Name == ObjectIndexString || data.Name == "all")
			{
				// Enemy scripts
				if (data.DataType == "enemy")
				{
					// Create script
					if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Create_0")
					{
						data.Create(InstanceID);
						return;
					}
					// Destroy script
					if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Destroy_0")
					{
						data.Destroy(InstanceID);
						return;
					}
					// Draw script
					if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Draw_0")
					{
						data.Draw(InstanceID);
						return;
					}
					// Step script
					if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Step_1")
					{
						if (ObjectIndexString != "obj_void_tentacle_segment") // Make an exception in on-step calls for trespasser tentacle segments
						{
							data.Step(InstanceID);
							return;
						}
						if (ObjectIndexString == "obj_void_tentacle_segment") // ...unless the eye is open
						{
							if (g_YYTKInterface->CallBuiltin("variable_instance_get", { Instance, "open" }).ToBoolean() == true)
							{
								data.Step(InstanceID);
								return;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		for (double var = 0; var < AllData.size(); var++)
		{
			ContentData data = AllData[var];
			if (data.Name == CustomDataString || data.Name == "all")
			{
				if (data.DataType == "enemy")
				{
					// Create script
					if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Create_0")
					{
						data.Create(InstanceID);
						return;
					}
					// Destroy script
					if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Destroy_0")
					{
						data.Destroy(InstanceID);
						return;
					}
					// Draw script
					if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Draw_0")
					{
						data.Draw(InstanceID);
						return;
					}
					// Step script
					if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Step_1")
					{
						data.Step(InstanceID);
						return;
					}
				}
			}
		}
	}
}