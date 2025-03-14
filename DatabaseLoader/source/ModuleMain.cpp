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

#pragma comment(lib, "lua51.lib")

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
	ContentData dt = ContentData(data);

	AllData.push_back(dt);

	g_YYTKInterface->PrintInfo(dt.DataType + " data created for object " + dt.Name + ".");
}

void UnloadMods()
{
	string dir = Files::GetModsDirectory();

	vector<filesystem::path> mods = Files::GetImmediateSubfolders(dir);

	for (size_t i = 0; i < mods.size(); i++)
	{
		currentState = i;

		modState[currentState]["dbl_unload"].call();

		g_YYTKInterface->Print(CM_LIGHTBLUE, "[Myriad Loader] Unloaded mod " + mods[i].filename().string());
	}

	modState.clear();
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
		modState.at(stateNum)["view_x"] = g_YYTKInterface->CallBuiltin("camera_get_view_x", { viewCamera }).ToDouble();
		modState.at(stateNum)["screen_center_x"] = modState.at(stateNum).get<double>("view_x") + (290 / 2);
		modState.at(stateNum)["view_y"] = g_YYTKInterface->CallBuiltin("camera_get_view_y", { viewCamera }).ToDouble();
		modState.at(stateNum)["screen_center_y"] = modState.at(stateNum).get<double>("view_x") + (464 / 2);
	}

	context.Call();

	for (double var = 0; var < AllData.size(); var++)
	{
		ContentData data = AllData[var];

		if (data.DataType == "global")
		{
			data.Step(0);
		}
	}
};


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

sol::state GetModState()
{
	sol::state inState;

	inState.open_libraries(sol::lib::base, sol::lib::package, sol::lib::table, sol::lib::math, sol::lib::string);

	inState["modName"] = "";

	inState["debug_out"] = [](string text) {
		yytk_interface->PrintInfo(text);
		};

	inState["object_behaviors"] = inState.create_table();

	inState["enemy_data"] = DBLua::EnemyData;

	inState["projectile_data"] = DBLua::ProjectileData;

	inState["global_data"] = DBLua::GlobalData;

	inState["view_x"] = 0;
	inState["view_y"] = 0;
	inState["screen_center_x"] = 0;
	inState["screen_center_y"] = 0;

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

	inState["unlock_song"] = DBLua::UnlockSong;

	inState["get_asset"] = DBLua::GetAsset;

	inState["call_function"] = DBLua::CallFunction;

	inState["call_game_function"] = DBLua::CallGameFunction;

	inState["play_sound"] = DBLua::DoSound;

	inState["play_sound_ext"] = DBLua::DoSoundExt;

	inState["draw_sprite"] = DBLua::DrawSprite;

	inState["draw_sprite_ext"] = DBLua::DrawSpriteExt;

	inState["draw_text"] = DBLua::DrawString;

	inState["draw_rectangle"] = DBLua::DrawRect;

	inState["draw_set_depth"] = DBLua::DrawSetDepth;

	inState["save_data"] = SaverLoader::SaveVariable;

	inState["load_data"] = SaverLoader::LoadVariable;

	inState["create_color"] = DBLua::CreateColor;
	inState["create_colour"] = DBLua::CreateColor;

	inState["draw_set_color"] = DBLua::DrawSetColor;
	inState["draw_set_colour"] = DBLua::DrawSetColor;

	return inState;
}

int LoadFileRequire(lua_State* L)
{
	std::string path = sol::stack::get<std::string>(L);

	std::string script = Files::GetFileContents(Files::GetModsDirectory() + "/" + path + ".lua");

	if (script != "")
	{
		g_YYTKInterface->Print(CM_LIGHTGREEN, "[Myriad Loader] Loaded module " + path);
		luaL_loadbuffer(L, script.data(), script.size(), path.c_str());
	}
	else
	{
		g_YYTKInterface->Print(CM_LIGHTRED, "[Myriad Loader] Could not load module: " + path);
	}

	return 1;
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

	if (!AurieSuccess(last_status))
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	dl_lua = GetModState();
	
	dl_lua["all_behaviors"] = dl_lua.create_table();

	string dir = Files::GetModsDirectory();
	string savedir = Files::GetModSavesDirectory();

	Files::MakeDirectory(dir);
	Files::MakeDirectory(savedir);

	vector<filesystem::path> mods = Files::GetImmediateSubfolders(dir);

	for (size_t i = 0; i < mods.size(); i++)
	{
		modState.push_back(GetModState());

		currentState = i;

		modState[currentState].clear_package_loaders();
		modState[currentState].add_package_loader(LoadFileRequire);

		modState[currentState]["all_behaviors"] = modState[currentState].create_table();

		modState[currentState].script_file(mods[i].string() + "/main.lua");

		modState[currentState]["dbl_load"].call();

		g_YYTKInterface->Print(CM_LIGHTBLUE, "[Myriad Loader] Loaded mod " + mods[i].filename().string());
	}

	yytk_interface->CreateCallback(
		Module,
		YYTK::EVENT_OBJECT_CALL,
		GMHooks::ContentDataEvents,
		0);

	yytk_interface->CreateCallback(
		Module,
		YYTK::EVENT_FRAME,
		ObjectBehaviorRun,
		0);

	CScript* script_data = nullptr;
	int script_index = 0;
	PVOID original_function = nullptr;

	TRoutine game_function = nullptr;
	TRoutine original_builtin_function = nullptr;

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

	//gml_GlobalScript_player_takeHit - player data is next

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