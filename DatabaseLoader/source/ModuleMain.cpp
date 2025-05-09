#define SOL_ALL_SAFETIES_ON 1
#include <YYToolkit/YYTK_Shared.hpp>
#include "ModuleMain.h"
#include "DatabaseLoader.h"
#include <sol/sol.hpp>
#include "DBLua.h"
#include "Files.h"
#include <string>
#include <windows.h>
#include <shlobj_core.h>
#include "GMHooks.h"
#include <filesystem>
#include <iostream>
#include "SaverLoader.h"
#include <fstream>
#include <thread>

#pragma comment(lib, "lua54.lib")

using namespace Aurie;
using namespace DatabaseLoader;

static DLInterface* dl_interface = nullptr;
static YYTK::YYTKInterface* yytk_interface = nullptr;

static sol::table CopyTableFromStateTo(sol::state& source, sol::state& target, sol::table table_to_copy) {

	sol::table new_table = target.create_table();
	for (auto it = table_to_copy.begin(); it != table_to_copy.end(); ++it) {

		auto [key, value] = *it;

		new_table.set(key, table_to_copy.copy(target)); // Recursively copy nested values if needed [6, 8]
	}
	return new_table;
}


string GetUserDirectory() {
	char path[MAX_PATH];
	if (SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, path) == S_OK) {
		return string(path);
	}
	else {
		return "";
	}
}

static void RegisterData(sol::table data)
{
	sol::table tbl = modState[currentState]["all_behaviors"];

	modState[currentState]["all_behaviors"][tbl.size() + 1] = data;

	RValue enemytype = g_YYTKInterface->CallBuiltin("asset_get_index", { (string_view)data.get<string>("Name") });

	string getName = data.get<string>("Name");
	string getType = data.get<string>("DataType");

	if (getType == "enemy" && getName != "all")
	{
		if (!g_YYTKInterface->CallBuiltin("object_exists", { enemytype }))
		{
			int id = Files::HashString(getName);

			if (data.get<bool>("Boss") == true)
			{
				if (std::find(customBossNames.begin(), customBossNames.end(), getName) == customBossNames.end())
				{
					g_YYTKInterface->CallBuiltin("array_set", { GMWrappers::GetGlobal("gen_list"), id, id });
					customBossNames.push_back(getName);

					string bosslist = "bosslist_" + to_string(data.get<int>("BossFloor"));

					if (g_YYTKInterface->CallBuiltin("variable_global_exists", { (string_view)bosslist }))
					{
						g_YYTKInterface->CallBuiltin("ds_list_add", { GMWrappers::GetGlobal(bosslist), id });
						g_YYTKInterface->Print(CM_LIGHTPURPLE, "[Myriad Loader] Boss '" + getName + "' (numeric ID " + to_string(id) + ") implemented for floor " + to_string(data.get<int>("BossFloor")));
					}
					else
					{
						g_YYTKInterface->Print(CM_LIGHTPURPLE, "[Myriad Loader] Created boss '" + getName + "' with numeric ID: " + to_string(id) + " using custom spawning logic");
					}
				}
			}
			else if (data.get<bool>("Miniboss") == true)
			{
				if (std::find(customMinibossNames.begin(), customMinibossNames.end(), getName) == customMinibossNames.end())
				{
					g_YYTKInterface->CallBuiltin("array_set", { GMWrappers::GetGlobal("gen_list"), id, id });
					customMinibossNames.push_back(getName);
					g_YYTKInterface->Print(CM_LIGHTPURPLE, "[Myriad Loader] Created miniboss '" + getName + "' with numeric ID: " + to_string(id));
				}
			}
			else if (std::find(customEnemyNames.begin(), customEnemyNames.end(), getName) == customEnemyNames.end())
			{
				g_YYTKInterface->CallBuiltin("array_set", { GMWrappers::GetGlobal("gen_list"), id, id });
				customEnemyNames.push_back(getName);
				g_YYTKInterface->Print(CM_LIGHTPURPLE, "[Myriad Loader] Created enemy '" + getName + "' with numeric ID: " + to_string(id));
			}
		}
	}

	if (getType == "cartridge" && getName != "all")
	{
		if (!g_YYTKInterface->CallBuiltin("object_exists", { enemytype }))
		{
			string getShownName = data.get<string>("ShownName");
			string getDescription = data.get<string>("Description");
			int id = Files::HashString(getName);

			if (std::find(customCartridgeNames.begin(), customCartridgeNames.end(), getName) == customCartridgeNames.end())
			{

				g_YYTKInterface->CallBuiltin("array_set", { GMWrappers::GetGlobal("gen_list"), id, id });
				g_YYTKInterface->CallBuiltin("array_set", { GMWrappers::GetGlobal("cart_name"), id, g_YYTKInterface->CallBuiltin("array_create", { 2, (string_view)getShownName })});
				g_YYTKInterface->CallBuiltin("array_set", { GMWrappers::GetGlobal("cart_desc"), id, g_YYTKInterface->CallBuiltin("array_create", { 2, (string_view)getDescription })});

				customCartridgeNames.push_back(getName);


				g_YYTKInterface->Print(CM_LIGHTPURPLE, "[Myriad Loader] Created cartridge '" + getName + "' with numeric ID: " + to_string(id));
			}
		}
	}

	if (getType == "floormap" && getName != "all")
	{
		if (!g_YYTKInterface->CallBuiltin("object_exists", { enemytype }))
		{
			bool forceFloor = data.get<bool>("ShouldForceFloor");
			string floorRooms = data.get<string>("Rooms");
			int id = Files::HashString(getName);

			if (std::find(customFloorNames.begin(), customFloorNames.end(), getName) == customFloorNames.end())
			{

				RValue floordsmap = g_YYTKInterface->CallBuiltin("ds_map_create", {});
				g_YYTKInterface->CallBuiltin("ds_map_copy", { floordsmap, GMWrappers::GetGlobal("floormap_1") });
				string floormapnum = "floormap_" + to_string(data.get<int>("Floor") - 1);

				if (g_YYTKInterface->CallBuiltin("variable_global_exists", { (string_view)floormapnum }))
				{

					g_YYTKInterface->CallBuiltin("ds_map_set", { GMWrappers::GetGlobal(floormapnum), "next", id});
					g_YYTKInterface->CallBuiltin("ds_map_set", { floordsmap, "layout", (string_view)floorRooms });
					g_YYTKInterface->Print(CM_LIGHTPURPLE, "[Myriad Loader] Floor '" + getName + "' (numeric ID " + to_string(id) + ") implemented for floor " + to_string(data.get<int>("Floor")));
				}
				else
				{
					g_YYTKInterface->Print(CM_LIGHTPURPLE, "[Myriad Loader] Created floor '" + getName + "' with numeric ID: " + to_string(id) + " using custom spawning logic");
				}

				if (forceFloor)
				{
					g_YYTKInterface->CallBuiltin("ds_map_set", { floordsmap, "index", id });

					g_YYTKInterface->CallBuiltin("ds_map_set", { GMWrappers::GetGlobal("current_floormap"), "next", g_YYTKInterface->CallBuiltin("ds_map_find_value", { floordsmap, "index" }) });

					g_YYTKInterface->CallBuiltin("ds_map_set", { GMWrappers::GetGlobal(floormapnum), "next", g_YYTKInterface->CallBuiltin("ds_map_find_value", {floordsmap, "index"}) });

					g_YYTKInterface->CallBuiltin("ds_map_set", { GMWrappers::GetGlobal(floormapnum), "layout", (string_view)floorRooms});

					g_YYTKInterface->PrintWarning(to_string(id));


					/*
					RValue thingsize = g_YYTKInterface->CallBuiltin("ds_map_size", { GMWrappers::GetGlobal("current_floormap") });
					RValue thing = g_YYTKInterface->CallBuiltin("ds_map_find_first", { GMWrappers::GetGlobal("current_floormap") });

					for (int i = 0; i < thingsize.ToDouble() - 1; i++)
					{
						if (!thing.ToString().empty())
						{
							thing = g_YYTKInterface->CallBuiltin("ds_map_find_next", { GMWrappers::GetGlobal("current_floormap"), thing });
							g_YYTKInterface->PrintWarning(thing.ToString());
						}
					}
					*/
				}

				customFloorNames.push_back(getName);
			}
		}
	}

}

void DatabaseLoader::UnloadMods()
{
	string dir = Files::GetModsDirectory();

	vector<filesystem::path> mods = Files::GetImmediateSubfolders(dir);

	for (size_t i = 0; i < mods.size(); i++) 
	{
		currentState = i;

		modState[currentState]["mod_unload"].call();

		//lua_close(modState[currentState]);

		g_YYTKInterface->Print(CM_LIGHTBLUE, "[Myriad Loader] Unloaded mod " + mods[i].filename().string());
	}

	customEnemyNames.clear();
	customMinibossNames.clear();
	customBossNames.clear();
	customCartridgeNames.clear();
	customFloorNames.clear();
	modState.clear();
}

sol::state DatabaseLoader::GetModState()
{
	sol::state inState;

	inState.open_libraries(sol::lib::base, sol::lib::package, sol::lib::table, sol::lib::math, sol::lib::string);

	inState["modName"] = "";

	inState["debug_out"] = [](string text) {
		yytk_interface->PrintInfo(text);
		};

	inState["object_behaviors"] = inState.create_table();

	inState["hard_mode"] = false;
	inState["paused"] = false;
	inState["loop"] = 0;
	inState["view_x"] = 0;
	inState["view_y"] = 0;
	inState["player"] = 0;
	inState["player_x"] = 0;
	inState["player_y"] = 0;
	inState["player_dead"] = false;
	inState["screen_center_x"] = 0;
	inState["screen_center_y"] = 0;

	inState["enemy_data"] = DBLua::EnemyData;
	inState["cartridge_data"] = DBLua::CartridgeData;
	inState["projectile_data"] = DBLua::ProjectileData;
	inState["global_data"] = DBLua::GlobalData;
	inState["player_data"] = DBLua::PlayerData;

	inState["register_data"] = RegisterData;

	inState["spawn_particle"] = DBLua::SpawnParticle;

	inState["init_var"] = DBLua::InitVar;

	inState["set_var"] = DBLua::SetVar;

	inState["get_var"] = DBLua::GetVar;

	inState["init_global"] = DBLua::InitGlobal;

	inState["set_global"] = DBLua::SetGlobal;

	inState["get_global"] = DBLua::GetGlobal;

	inState["init_number"] = DBLua::InitVar;

	inState["init_bool"] = DBLua::InitVar;

	inState["init_string"] = DBLua::InitVar;

	inState["set_number"] = DBLua::SetVar;

	inState["set_bool"] = DBLua::SetVar;

	inState["set_string"] = DBLua::SetVar;

	inState["get_number"] = DBLua::GetVar;

	inState["get_bool"] = DBLua::GetVar;

	inState["get_string"] = DBLua::GetVar;

	inState["custom_sprite"] = DBLua::GetCustomSprite;

	inState["custom_sound"] = DBLua::GetCustomSound;

	inState["custom_music"] = DBLua::GetCustomMusic;

	//inState["unlock_song"] = DBLua::UnlockSong;

	inState["get_asset"] = DBLua::GetAsset;

	inState["call_function"] = DBLua::CallFunction;

	inState["call_game_function"] = DBLua::CallGameFunction;

	inState["play_sound"] = DBLua::DoSound;

	inState["play_sound_ext"] = DBLua::DoSoundExt;

	inState["play_music"] = DBLua::DoMusic;

	inState["boss_message"] = DBLua::ShowBossMessage;

	inState["draw_sprite"] = DBLua::DrawSprite;

	inState["draw_sprite_ext"] = DBLua::DrawSpriteExt;

	inState["draw_primitive_begin_texture"] = DBLua::DrawPrimitiveBeginTexture;
	inState["draw_vertex_texture"] = DBLua::DrawVertexTexture;
	inState["draw_primitive_begin"] = DBLua::DrawPrimitiveBeginSolid;
	inState["draw_vertex_color"] = DBLua::DrawVertexColor;
	inState["draw_primitive_end"] = DBLua::DrawVertexEnd;

	inState["draw_sprite"] = DBLua::DrawSprite;

	inState["draw_text"] = DBLua::DrawString;

	inState["draw_rectangle"] = DBLua::DrawRect;

	inState["draw_set_depth"] = DBLua::DrawSetDepth;

	inState["save_data"] = SaverLoader::SaveVariable;

	inState["load_data"] = SaverLoader::LoadVariable;

	inState["create_color"] = DBLua::CreateColor;
	inState["create_colour"] = DBLua::CreateColor;

	inState["draw_set_color"] = DBLua::DrawSetColor;
	inState["draw_set_colour"] = DBLua::DrawSetColor;

	inState["spawn_enemy"] = DBLua::SpawnEnemy;

	inState["spawn_boss_intro"] = DBLua::SpawnBossIntro;

	inState["kill_boss_effect"] = DBLua::KillBoss;

	inState["add_screenshake"] = DBLua::AddScreenshake;

	inState["clear_bullets"] = DBLua::ClearBullets;

	inState["spawn_projectile"] = DBLua::SpawnProjectile;

	inState["spawn_laser"] = DBLua::SpawnLaser;

	//inState["get_direction"] = DBLua::DirectionTo;

	inState["add_rooms_to"] = DBLua::AddRoomsTo;

	inState["custom_floor"] = DBLua::FloorData;

	//inState["add_bestiary_entry"] = DBLua::AddBestiaryEntry;

	inState["check_cart"] = DBLua::CheckCart;

	return inState;
}

void UnloadRoomFiles()
{
	for (size_t i = 0; i < roomFiles.size(); i++)
	{
		Files::CopyFileTo(roomFiles.at(i).backupName, roomFiles.at(i).destinationName);
	}
}

void ObjectBehaviorRun(FWFrame& context)
{
	UNREFERENCED_PARAMETER(context);

	CInstance* GlobalInstance;

	RValue view;
	g_YYTKInterface->GetGlobalInstance(&GlobalInstance);

	g_YYTKInterface->GetBuiltin("view_current", GlobalInstance, 0, view);

	RValue viewCamera = g_YYTKInterface->CallBuiltin("view_get_camera", { view });
	
	for (size_t stateNum = 0; stateNum < modState.size(); stateNum++)
	{
		g_YYTKInterface->CallBuiltin("audio_sound_gain", { GMWrappers::GetGlobal("current_music"), GMWrappers::GetGlobal("volume_music"), 0 });

		modState.at(stateNum)["hard_mode"] = GMWrappers::GetGlobal("hardmode").ToBoolean();
		modState.at(stateNum)["paused"] = g_YYTKInterface->CallBuiltin("instance_exists", { g_YYTKInterface->CallBuiltin("asset_get_index", {"obj_pause"})}).ToBoolean();
		modState.at(stateNum)["loop"] = GMWrappers::GetGlobal("game_loop");
		modState.at(stateNum)["view_x"] = g_YYTKInterface->CallBuiltin("camera_get_view_x", { viewCamera }).ToDouble();
		modState.at(stateNum)["screen_center_x"] = modState.at(stateNum).get<double>("view_x") + 120;
		modState.at(stateNum)["view_y"] = g_YYTKInterface->CallBuiltin("camera_get_view_y", { viewCamera }).ToDouble();
		modState.at(stateNum)["screen_center_y"] = modState.at(stateNum).get<double>("view_y") + 160;

		RValue playerAsset = g_YYTKInterface->CallBuiltin("asset_get_index", { "obj_player" });
		RValue player = g_YYTKInterface->CallBuiltin("instance_find", { playerAsset, 0 });

		modState.at(stateNum)["player_dead"] = false;

		if (g_YYTKInterface->CallBuiltin("instance_exists", {player.ToDouble()}))
		{
			modState.at(stateNum)["player"] = player.ToDouble();
			modState.at(stateNum)["player_x"] = g_YYTKInterface->CallBuiltin("variable_instance_get", { player, "x" }).ToDouble();
			modState.at(stateNum)["player_y"] = g_YYTKInterface->CallBuiltin("variable_instance_get", { player, "y" }).ToDouble();
		}
		else
		{
			modState.at(stateNum)["player_dead"] = true;
		}

		if (modState.at(stateNum)["all_behaviors"])
		{
			sol::table count = modState.at(stateNum)["all_behaviors"];
			for (double var = 0; var < count.size() + 1; var++)
			{
				sol::table tbl = modState.at(stateNum)["all_behaviors"][var];
				if (modState.at(stateNum)["all_behaviors"][var])
				{
					if (g_YYTKInterface->CallBuiltin("object_exists", {g_YYTKInterface->CallBuiltin("asset_get_index", {(string_view)tbl.get<string>("Name")})}))
					{
						if (tbl.get<string>("DataType") == "enemy" || tbl.get<string>("DataType") == "projectile")
						{
							DBLua::InvokeWithObjectIndex(tbl.get<string>("Name"), tbl["Step"]);
						}
					}

					if (tbl.get<string>("Name") == "all")
					{
						if (tbl.get<string>("DataType") == "enemy")
						{
							DBLua::InvokeWithObjectIndex("obj_enemy", tbl["Step"]);
						}
					}

					if (tbl.get<string>("DataType") == "global")
					{
						modState.at(stateNum)["all_behaviors"][var]["Step"].call();
					}
				}
			}
		}
	}
};

double degreesToRadians(double degrees) {
	return degrees * 3.14159265358979323846 / 180.0;
}

void DrawLoadingScreen(FWCodeEvent& context)
{
	CCode* Code = std::get<2>(context.Arguments());

	if ((string)Code->GetName() == "gml_Object_obj_screen_Draw_64" && loadingMods)
	{
		static double rotation = 0;
		static double dist = 0;
		static double frame = 0;
		CInstance* Self = std::get<0>(context.Arguments());
		RValue Instance = Self->ToRValue();

		frame += 0.2;
		rotation += sin(frame / 10) * 0.06;
		dist = lerp(dist, 15, 0.3);
		for (size_t i = 0; i < 6; i++)
		{
			double x = 120;
			g_YYTKInterface->CallBuiltin("draw_sprite", { g_YYTKInterface->CallBuiltin("asset_get_index", {"spr_myriad"}), frame + i, x + (cos(rotation + degreesToRadians(i * 60)) * dist), 40 + (-sin(rotation + degreesToRadians(i * 60)) * dist)});
			g_YYTKInterface->CallBuiltin("draw_set_font", { GMWrappers::GetGlobal("font_nes") });
			g_YYTKInterface->CallBuiltin("draw_set_halign", { 1 });
			g_YYTKInterface->CallBuiltin("draw_text", { x, 60, "loading mods..." });
		}
	}
}

RValue ObjectToRValue(sol::object obj)
{
	RValue val = 0;

	switch (obj.get_type())
	{
	case sol::lua_type_of_v<double>:
		return obj.as<double>();
	case sol::lua_type_of_v<bool>:
		return obj.as<bool>();
	case sol::lua_type_of_v<string>:
		return obj.as<string_view>();
	default:
		break;
	}

	return 0;
}

int DatabaseLoader::LoadFileRequire(lua_State* L)
{
	std::string path = sol::stack::get<std::string>(L);

	std::string script = Files::GetFileContents(Files::GetModsDirectory() + path + ".lua");
	std::string script2 = Files::GetFileContents(Files::GetModsDirectory() + path);

	if (script != "")
	{
		g_YYTKInterface->Print(CM_LIGHTGREEN, "[Myriad Loader] Loaded module " + path);
		luaL_loadbuffer(L, script.data(), script.size(), path.c_str());
	}
	else if (script2 != "")
	{
		g_YYTKInterface->Print(CM_LIGHTGREEN, "[Myriad Loader] Loaded module " + path);
		luaL_loadbuffer(L, script2.data(), script2.size(), path.c_str());
	}
	else
	{
		g_YYTKInterface->Print(CM_LIGHTRED, "[Myriad Loader] Could not load module: " + path);
	}

	return 1;
}

HWINEVENTHOOK closeWindowHook;

void HandleWindowEvent(DWORD event, HWND hwnd) {
	if (event == EVENT_OBJECT_DESTROY) {
		UnloadMods();
	}
}

void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
	if (idObject == OBJID_WINDOW) {
		HandleWindowEvent(event, hwnd);
	}
}

BOOL(*g_WriteFileTrampoline)(
	IN HANDLE,
	IN LPCVOID,
	IN DWORD,
	OPTIONAL OUT LPDWORD,
	IN OUT OPTIONAL LPOVERLAPPED
	) = nullptr;

BOOL WINAPI WriteFileHook(
	IN HANDLE File,
	IN LPCVOID Buffer,
	IN DWORD NumberOfBytesToWrite,
	OPTIONAL OUT LPDWORD NumberOfBytesWritten,
	IN OUT OPTIONAL LPOVERLAPPED OverlapInformation
)
{
	if (File == GetStdHandle(STD_OUTPUT_HANDLE) ||
		File == GetStdHandle(STD_ERROR_HANDLE))
	{
		SetLastError(ERROR_FILE_NOT_FOUND);
		return false;
	}

	return g_WriteFileTrampoline(File, Buffer, NumberOfBytesToWrite, NumberOfBytesWritten, OverlapInformation);
}

void LoadMods(AurieModule* Module)
{
	dl_lua = GetModState();

	dl_lua["all_behaviors"] = dl_lua.create_table();

	string dir = Files::GetModsDirectory();
	string savedir = Files::GetModSavesDirectory();
	string rooms = Files::GetSteamDirectory() + "rooms/";
	string roomsBackup = Files::GetSteamDirectory() + "rooms/backup/";

	if (firstLoad)
	{
		Files::MakeDirectory(dir);
		Files::MakeDirectory(savedir);
		Files::MakeDirectory(roomsBackup);
	}

	vector<filesystem::path> mods = Files::GetImmediateSubfolders(dir);

	for (size_t i = 0; i < mods.size(); i++)
	{
		modState.push_back(GetModState());

		currentState = i;

		modState[currentState].clear_package_loaders();
		modState[currentState].add_package_loader(LoadFileRequire);

		modState[currentState]["all_behaviors"] = modState[currentState].create_table();

		if (std::filesystem::exists(mods[i].string() + "/main.lua"))
		{
			modState[currentState].script_file(mods[i].string() + "/main.lua");

			modState[currentState]["mod_load"].call();
		}

		g_YYTKInterface->Print(CM_LIGHTBLUE, "[Myriad Loader] Loaded mod " + mods[i].filename().string());
	}

	for (size_t i = 0; i < roomFiles.size(); i++)
	{
		Files::CopyFileTo(roomFiles.at(i).destinationName, roomFiles.at(i).backupName);
		Files::AddRoomsToFile(roomFiles.at(i).sourceName, roomFiles.at(i).destinationName);
	}

	if (firstLoad)
	{
		yytk_interface->CreateCallback(
			Module,
			YYTK::EVENT_OBJECT_CALL,
			GMHooks::EnemyData,
			0);

		yytk_interface->CreateCallback(
			Module,
			YYTK::EVENT_FRAME,
			ObjectBehaviorRun,
			0);
	}

	CScript* script_data = nullptr;
	int script_index = 0;
	PVOID original_function = nullptr;

	TRoutine game_function = nullptr;
	TRoutine original_builtin_function = nullptr;

	if (firstLoad)
	{
		g_YYTKInterface->GetNamedRoutinePointer(
			"gml_Script_music_jukebox_get_songs",
			reinterpret_cast<PVOID*>(&script_data)
		);
		MmCreateHook(
			g_ArSelfModule,
			"Jukebox Injection",
			script_data->m_Functions->m_ScriptFunction,
			GMHooks::JukeboxInjection,
			&original_function
		);

		g_YYTKInterface->GetNamedRoutinePointer(
			"gml_Script_music_do",
			reinterpret_cast<PVOID*>(&script_data)
		);
		MmCreateHook(
			g_ArSelfModule,
			"MusicDo",
			script_data->m_Functions->m_ScriptFunction,
			GMHooks::MusicDo,
			&original_function
		);

		g_YYTKInterface->GetNamedRoutinePointer(
			"gml_Script_music_do_loop",
			reinterpret_cast<PVOID*>(&script_data)
		);
		MmCreateHook(
			g_ArSelfModule,
			"MusicDoLoop",
			script_data->m_Functions->m_ScriptFunction,
			GMHooks::MusicDoLoop,
			&original_function
		);

		g_YYTKInterface->GetNamedRoutinePointer(
			"gml_Script_anon_gml_Object_obj_boss_setter_Create_0_29_gml_Object_obj_boss_setter_Create_0",
			reinterpret_cast<PVOID*>(&script_data)
		);
		MmCreateHook(
			g_ArSelfModule,
			"MusicDoLoop",
			script_data->m_Functions->m_ScriptFunction,
			GMHooks::ChooseBossIntro,
			&original_function
		);

		g_YYTKInterface->GetNamedRoutinePointer(
			"gml_Script_music_do_loop_from_start",
			reinterpret_cast<PVOID*>(&script_data)
		);
		MmCreateHook(
			g_ArSelfModule,
			"MusicDoLoopFromStart",
			script_data->m_Functions->m_ScriptFunction,
			GMHooks::MusicDoLoopFromStart,
			&original_function
		);

		g_YYTKInterface->GetNamedRoutinePointer(
			"gml_Script_enemy_damage",
			reinterpret_cast<PVOID*>(&script_data)
		);
		MmCreateHook(
			g_ArSelfModule,
			"EnemyDamage",
			script_data->m_Functions->m_ScriptFunction,
			GMHooks::EnemyDamage,
			&original_function
		);

		g_YYTKInterface->GetNamedRoutinePointer(
			"gml_Script_player_takeHit",
			reinterpret_cast<PVOID*>(&script_data)
		);
		MmCreateHook(
			g_ArSelfModule,
			"PlayerTakeHit",
			script_data->m_Functions->m_ScriptFunction,
			GMHooks::PlayerTakeHit,
			&original_function
		);

		g_YYTKInterface->GetNamedRoutinePointer(
			"gml_Script_button_exit_to_menu",
			reinterpret_cast<PVOID*>(&script_data)
		);
		MmCreateHook(
			g_ArSelfModule,
			"ReloadAllMods",
			script_data->m_Functions->m_ScriptFunction,
			GMHooks::ReloadAllMods,
			&original_function
		);
	}

	int size = g_YYTKInterface->CallBuiltin("array_length", { GMWrappers::GetGlobal("gen_list") }).ToInt64();

	for (size_t i = size; i <= 15000; i++)
	{
		g_YYTKInterface->CallBuiltin("array_set", { GMWrappers::GetGlobal("gen_list"), i, i });
	}

	GMWrappers::CallGameScript("gml_Script_load_room_files", {});

	if (firstLoad)
	{
		g_YYTKInterface->GetNamedRoutinePointer(
			"gml_Script_instance_create",
			reinterpret_cast<PVOID*>(&script_data)
		);
		MmCreateHook(
			g_ArSelfModule,
			"SpawnRoomObject",
			script_data->m_Functions->m_ScriptFunction,
			GMHooks::SpawnRoomObject,
			&original_function
		);

		g_YYTKInterface->GetNamedRoutinePointer(
			"gml_Script_write_savedata",
			reinterpret_cast<PVOID*>(&script_data)
		);
		MmCreateHook(
			g_ArSelfModule,
			"WriteSaveData",
			script_data->m_Functions->m_ScriptFunction,
			GMHooks::WriteSaveData,
			&original_function
		);

		g_YYTKInterface->GetNamedRoutinePointer(
			"gml_Script_write_midsave",
			reinterpret_cast<PVOID*>(&script_data)
		);
		MmCreateHook(
			g_ArSelfModule,
			"WriteMidSave",
			script_data->m_Functions->m_ScriptFunction,
			GMHooks::WriteMidSave,
			&original_function
		);

		g_YYTKInterface->GetNamedRoutinePointer(
			"gml_GlobalScript_button_exit_out",
			reinterpret_cast<PVOID*>(&script_data)
		);
		MmCreateHook(
			g_ArSelfModule,
			"ExitGame",
			script_data->m_Functions->m_ScriptFunction,
			GMHooks::ExitGame,
			&original_function
		);

		g_YYTKInterface->GetNamedRoutinePointer(
			"gml_Script_button_start",
			reinterpret_cast<PVOID*>(&script_data)
		);
		MmCreateHook(
			g_ArSelfModule,
			"EnterRun",
			script_data->m_Functions->m_ScriptFunction,
			GMHooks::EnterRun,
			&original_function
		);

		g_YYTKInterface->GetNamedRoutinePointer(
			"gml_Object_obj_beacon_Other_25",
			reinterpret_cast<PVOID*>(&script_data)
		);
		MmCreateHook(
			g_ArSelfModule,
			"ChooseBossIntro",
			script_data->m_Functions->m_ScriptFunction,
			GMHooks::ChooseBossIntro,
			&original_function
		);

		MmCreateHook(
			Module,
			"QL_WriteFile",
			WriteFile,
			WriteFileHook,
			reinterpret_cast<PVOID*>(&g_WriteFileTrampoline)
		);
	}

	UnloadRoomFiles();

	g_YYTKInterface->CallBuiltin("instance_activate_all", {});
	loadingMods = false;
	firstLoad = false;
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	g_LocalModule = Module;

	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus last_status = AURIE_SUCCESS;

	last_status = ObGetInterface(
		"YYTK_Main",
		reinterpret_cast<AurieInterfaceBase*&>(yytk_interface)
	);

	DatabaseLoader::g_YYTKInterface = yytk_interface;

	yytk_interface->CreateCallback(
		Module,
		YYTK::EVENT_OBJECT_CALL,
		DrawLoadingScreen,
		0);

	if (!AurieSuccess(last_status))
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	loadingMods = true;

	g_YYTKInterface->CallBuiltin("instance_deactivate_object", { g_YYTKInterface->CallBuiltin("asset_get_index", {"obj_intro"}) });

	std::thread LoadModsThread(LoadMods, Module);

	LoadModsThread.join();

	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleUnload(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	UnloadMods();

	return AURIE_SUCCESS;
}