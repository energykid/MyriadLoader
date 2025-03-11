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
	sol::table tbl = dl_lua["all_behaviors"];

	g_YYTKInterface->PrintWarning(data.get<string>("Name"));

	dl_lua["all_behaviors"][tbl.size() + 1] = data;
}

void ObjectBehaviorRun(FWFrame& context)
{
	UNREFERENCED_PARAMETER(context);

	if (dl_lua["all_behaviors"])
	{
		sol::table count = dl_lua["all_behaviors"];
		for (double var = 0; var < count.size() + 1; var++)
		{
			sol::table tbl = dl_lua["all_behaviors"][var];
			if (dl_lua["all_behaviors"][var])
			{
				if (g_YYTKInterface->CallBuiltin("object_exists", { g_YYTKInterface->CallBuiltin("asset_get_index", { (string_view)tbl.get<string>("Name") } ) }))
				{
					DBLua::InvokeWithObjectIndex(tbl.get<string>("Name"), tbl["Step"]);
				}
			}
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

	inState.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math);\

	inState["debug_out"] = [](string text) {
		yytk_interface->PrintInfo(text);
		};

	inState["object_behaviors"] = inState.create_table();

	inState["enemy_data"] = DBLua::EnemyData;

	inState["projectile_data"] = DBLua::ProjectileData;

	inState["register_data"] = RegisterData;

	inState["spawn_particle"] = DBLua::SpawnParticle;

	inState["init_var"] = DBLua::InitVar;

	inState["set_var"] = DBLua::SetVar;

	inState["init_number"] = DBLua::InitVar;

	inState["init_bool"] = DBLua::InitVar;

	inState["init_string"] = DBLua::InitVar;

	inState["set_number"] = DBLua::SetVar;

	inState["set_bool"] = DBLua::SetVar;

	inState["set_string"] = DBLua::SetVar;

	inState["get_number"] = DBLua::GetDouble;

	inState["get_bool"] = DBLua::GetBool;

	inState["get_string"] = DBLua::GetString;

	inState["custom_sprite"] = DBLua::GetCustomSprite;

	inState["custom_sound"] = DBLua::GetCustomSound;

	inState["custom_music"] = DBLua::GetCustomMusic;

	inState["unlock_song"] = DBLua::UnlockSong;

	inState["get_asset"] = DBLua::GetAsset;

	inState["call_gm_function"] = DBLua::CallFunction;

	inState["call_game_function"] = DBLua::CallGameFunction;

	inState["play_sound"] = DBLua::DoSound;

	inState["play_sound_ext"] = DBLua::DoSoundExt;

	inState["draw_sprite"] = DBLua::DrawSprite;

	inState["draw_sprite_ext"] = DBLua::DrawSpriteExt;

	return inState;
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

	yytk_interface->PrintInfo("Database Loader has initialized!");

	dl_lua = GetModState();
	
	dl_lua["all_behaviors"] = dl_lua.create_table();

	g_YYTKInterface->PrintInfo("[Database Loader] Built-in functions loaded!");

	string dir = Files::GetModsDirectory();

	Files::MakeDirectory(dir);

	vector<filesystem::path> mods = Files::GetImmediateSubfolders(dir);

	for (size_t i = 0; i < mods.size(); i++)
	{
		vector<filesystem::path> filesystem = Files::GetFilesOfType(mods[i].string(), ".lua");

		modState = GetModState();

		for (size_t i = 0; i < filesystem.size(); i++)
		{
			g_YYTKInterface->PrintInfo("[Database Loader] File '" + filesystem[i].filename().string() + "' loaded...");
			dl_lua.script_file(filesystem[i].string());
		}
	}

	yytk_interface->CreateCallback(
		Module,
		YYTK::EVENT_OBJECT_CALL,
		GMHooks::EnemyData,
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

	return AURIE_SUCCESS;
}