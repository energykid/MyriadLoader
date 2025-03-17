#include "Aurie/shared.hpp"
#include "ModuleMain.h"
#include "DatabaseLoader.h"
#include "GMWrappers.h"
#include "GMHooks.h"
#include "DBLua.h"
#include "Files.h"
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

	for (int i = 0; i < g_YYTKInterface->CallBuiltin("array_length", { g_YYTKInterface->CallBuiltin("variable_global_get", { "myr_CustomMusicList" }) }).ToDouble(); i++)
	{
		g_YYTKInterface->CallBuiltin("array_push", { Result, g_YYTKInterface->CallBuiltin("array_get", { GMWrappers::GetGlobal("myr_CustomMusicList"), i })});
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
	if (g_YYTKInterface->CallBuiltin("instance_variable_exists", { Instance, "myr_CustomName" }).ToBoolean())
	{
		is_custom = true;
		CustomDataString = g_YYTKInterface->CallBuiltin("variable_instance_get", { Instance, "myr_CustomName" });
	}

	double AttackDamage = (Arguments[0])->ToDouble();

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
						if (tbl.get<string>("DataType") == "enemy")
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
						if (tbl.get<string>("DataType") == "enemy")
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
	}
	return Result;
}

RValue& DatabaseLoader::GMHooks::PlayerTakeHit(IN CInstance* Self, IN CInstance* Other, OUT RValue& Result, IN int ArgumentCount, IN RValue** Arguments)
{
	auto original_function = reinterpret_cast<decltype(&PlayerTakeHit)>(MmGetHookTrampoline(g_ArSelfModule, "PlayerTakeHit"));
	RValue& return_value = original_function(Self, Other, Result, ArgumentCount, Arguments);

	for (int stateNum = 0; stateNum < modState.size(); stateNum++)
	{
		if (modState.at(stateNum)["all_behaviors"])
		{
			sol::table count = modState.at(stateNum)["all_behaviors"];
			for (double var = 0; var < count.size() + 1; var++)
			{
				sol::table tbl = modState.at(stateNum)["all_behaviors"][var];
				if (modState.at(stateNum)["all_behaviors"][var])
				{
					if (tbl.get<string>("DataType") == "player")
					{
						RValue Instance = Self->ToRValue();
						RValue ImmuneFrames;
						g_YYTKInterface->CallBuiltinEx(ImmuneFrames, "alarm_get", Self, Other, { 1 });
						double InstanceID = g_YYTKInterface->CallBuiltin("variable_instance_get", { Instance, "id" }).ToDouble();

						double AttackDamage = (Arguments[0])->ToDouble();

						if (ImmuneFrames.ToDouble() <= 0)
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

RValue& DatabaseLoader::GMHooks::SpawnRoomObject(IN CInstance* Self, IN CInstance* Other, OUT RValue& Result, IN int ArgumentCount, IN RValue** Arguments)
{
	auto original_function = reinterpret_cast<decltype(&SpawnRoomObject)>(MmGetHookTrampoline(g_ArSelfModule, "SpawnRoomObject"));
	bool enemyFound = false;
	for (size_t i = 0; i < customEnemyNames.size(); i++)
	{
		if (Arguments[2]->ToDouble() == Files::HashString(customEnemyNames[i]))
		{
			Result = DBLua::SpawnEnemy(Arguments[0]->ToDouble(), Arguments[1]->ToDouble(), customEnemyNames[i]);
			enemyFound = true;
		}
	}
	for (size_t i = 0; i < customMinibossNames.size(); i++)
	{
		if (Arguments[2]->ToDouble() == Files::HashString(customMinibossNames[i]))
		{
			Result = DBLua::SpawnEnemy(Arguments[0]->ToDouble(), Arguments[1]->ToDouble(), customMinibossNames[i]);
			enemyFound = true;
		}
	}
	if (!enemyFound)
	{
		RValue& return_value = original_function(Self, Other, Result, ArgumentCount, Arguments);
	}
	
	return Result;
}

void DatabaseLoader::GMHooks::EnemyData(FWCodeEvent& FunctionContext)
{
	vector<string> AllNames;
	//Enemy data
	AllNames.push_back("gml_Object_obj_enemy_Create_0");
	AllNames.push_back("gml_Object_obj_swarmer_Step_0");
	AllNames.push_back("gml_Object_obj_miniboss_template_Step_0");
	AllNames.push_back("gml_Object_obj_enemy_Draw_0");
	AllNames.push_back("gml_Object_obj_enemy_Destroy_0");
	//Projectile data
	AllNames.push_back("gml_Object_obj_bullet_type_Create_0");
	AllNames.push_back("gml_Object_obj_bullet_type_Step_0");
	AllNames.push_back("gml_Object_obj_bullet_type_Collision_obj_floor"); // CollideTile(proj, tile)
	//Global data
	AllNames.push_back("gml_Object_obj_view_Draw_73");
	AllNames.push_back("gml_Object_obj_player_Draw_0");

	CCode* Code = std::get<2>(FunctionContext.Arguments());

	if (std::find(AllNames.begin(), AllNames.end(), Code->GetName()) != AllNames.end())
	{
		CInstance* GlobalInstance;

		RValue view;
		g_YYTKInterface->GetGlobalInstance(&GlobalInstance);

		g_YYTKInterface->GetBuiltin("view_current", GlobalInstance, 0, view);

		RValue viewCamera = g_YYTKInterface->CallBuiltin("view_get_camera", { view });

		CInstance* Self = std::get<0>(FunctionContext.Arguments());
		CInstance* Other = std::get<1>(FunctionContext.Arguments());
		RValue Instance = Self->ToRValue();
		RValue OtherInstance = Other->ToRValue();
		double InstanceID = g_YYTKInterface->CallBuiltin("variable_instance_get", { Instance, "id" }).ToDouble();
		double OtherInstanceID = g_YYTKInterface->CallBuiltin("variable_instance_get", { OtherInstance, "id" }).ToDouble();

		RValue objectIndex = g_YYTKInterface->CallBuiltin("variable_instance_get", { Instance, "object_index" });

		string CustomDataString = "";

		for (size_t stateNum = 0; stateNum < modState.size(); stateNum++)
		{
			modState.at(stateNum)["view_x"] = g_YYTKInterface->CallBuiltin("camera_get_view_x", { viewCamera }).ToDouble();
			modState.at(stateNum)["screen_center_x"] = modState.at(stateNum).get<double>("view_x") + (290 / 2);
			modState.at(stateNum)["view_y"] = g_YYTKInterface->CallBuiltin("camera_get_view_y", { viewCamera }).ToDouble();
			modState.at(stateNum)["screen_center_y"] = modState.at(stateNum).get<double>("view_x") + (464 / 2);

			RValue playerAsset = g_YYTKInterface->CallBuiltin("asset_get_index", { "obj_player" });
			RValue player = g_YYTKInterface->CallBuiltin("instance_find", { playerAsset, 0 });

			if (player.ToDouble() != -4)
			{
				modState.at(stateNum)["player_x"] = g_YYTKInterface->CallBuiltin("variable_instance_get", { player, "x" }).ToDouble();
				modState.at(stateNum)["player_y"] = g_YYTKInterface->CallBuiltin("variable_instance_get", { player, "y" }).ToDouble();
			}
		}

		bool is_custom = false;
		if (g_YYTKInterface->CallBuiltin("variable_instance_exists", { Instance, "myr_CustomName" }).ToBoolean())
		{
			is_custom = true;
			CustomDataString = g_YYTKInterface->CallBuiltin("variable_instance_get", { Instance, "myr_CustomName" }).ToString();
		}

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
							if (tbl.get<string>("Name") == CustomDataString || tbl.get<string>("Name") == "all")
							{
								// Enemy scripts
								if (tbl.get<string>("DataType") == "enemy")
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
									// Draw script
									if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Draw_0")
									{
										sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["Draw"].call(InstanceID);
										if (!result.valid())
										{
											sol::error error = result;

											g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
										}
									}
									// Step script
									if ((string)Code->GetName() == (string)"gml_Object_obj_swarmer_Step_0")
									{
										sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["Step"].call(InstanceID);
										if (!result.valid())
										{
											sol::error error = result;

											g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
										}
									}
									// Miniboss step script
									if ((string)Code->GetName() == (string)"gml_Object_obj_miniboss_template_Step_0")
									{
										if (tbl.get<bool>("Miniboss") == true)
										{
											sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["Step"].call(InstanceID);
											if (!result.valid())
											{
												sol::error error = result;

												g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
											}
										}
									}
								}
								// Projectile scripts
								if (tbl.get<string>("DataType") == "projectile")
								{
									// Create script
									if ((string)Code->GetName() == (string)"gml_Object_obj_bullet_type_Create_0")
									{
										sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["Create"].call(InstanceID);
										if (!result.valid())
										{
											sol::error error = result;

											g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
										}
									}
									// Step script
									if ((string)Code->GetName() == (string)"gml_Object_obj_bullet_type_Step_0")
									{
										sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["Step"].call(InstanceID);
										if (!result.valid())
										{
											sol::error error = result;

											g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
										}
									}
									// Collide script
									if ((string)Code->GetName() == (string)"gml_Object_obj_bullet_type_Collision_obj_floor")
									{
										sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["CollideWith"].call(InstanceID, OtherInstanceID);
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
				if (modState.at(stateNum)["all_behaviors"])
				{
					sol::table count = modState.at(stateNum)["all_behaviors"];
					for (double var = 0; var < count.size() + 1; var++)
					{
						sol::table tbl = modState.at(stateNum)["all_behaviors"][var];
						if (modState.at(stateNum)["all_behaviors"][var])
						{
							// Enemy scripts
							if (tbl.get<string>("DataType") == "enemy")
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
									// Draw script
									if ((string)Code->GetName() == (string)"gml_Object_obj_enemy_Draw_0")
									{
										sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["Draw"].call(InstanceID);
										if (!result.valid())
										{
											sol::error error = result;

											g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
										}
									}
								}
							}
							// Projectile scripts
							if (tbl.get<string>("DataType") == "projectile")
							{
								// Create script
								if ((string)Code->GetName() == (string)"gml_Object_obj_bullet_type_Create_0")
								{
									sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["Create"].call(InstanceID);
									if (!result.valid())
									{
										sol::error error = result;

										g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
									}
								}
								// Step script
								if ((string)Code->GetName() == (string)"gml_Object_obj_bullet_type_Step_0")
								{
									sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["Step"].call(InstanceID);
									if (!result.valid())
									{
										sol::error error = result;

										g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
									}
								}
								// Collide script
								if ((string)Code->GetName() == (string)"gml_Object_obj_bullet_type_Collision_obj_floor")
								{
									sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["Collide"].call(InstanceID, OtherInstanceID);
									if (!result.valid())
									{
										sol::error error = result;

										g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
									}
								}
							}
							// Global scripts
							if (tbl.get<string>("DataType") == "global")
							{
								// DrawUI script
								if ((string)Code->GetName() == (string)"gml_Object_obj_view_Draw_73")
								{
									sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["DrawUI"].call();
									if (!result.valid())
									{
										sol::error error = result;

										g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
									}
								}
								// Draw script
								if ((string)Code->GetName() == (string)"gml_Object_obj_player_Draw_0")
								{
									sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["Draw"].call();
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
}