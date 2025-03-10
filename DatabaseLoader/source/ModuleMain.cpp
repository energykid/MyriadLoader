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

static sol::table allBehaviors;

string GetUserDirectory() {
	char path[MAX_PATH];
	if (SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, path) == S_OK) {
		return string(path);
	}
	else {
		return "";
	}
}

void ObjectBehaviorRun(FWFrame& context)
{
	UNREFERENCED_PARAMETER(context);

	if (allBehaviors)
	{
		for (double var = 1; var < allBehaviors.size() + 1; var++)
		{
			sol::lua_table tbl = allBehaviors[var];
			DBLua::InvokeWithObjectIndex(tbl["objectName"], tbl["stepFunc"]);
		}
	}
};

sol::table NewObjectBehavior(string objectName, sol::protected_function func)
{
	static sol::table allBehaviors = DatabaseLoader::dl_lua["object_behaviors"];

	if (allBehaviors)
	{
		sol::table behav = DatabaseLoader::dl_lua.create_table();
		behav["objectName"] = objectName;
		behav["stepFunc"] = func;

		allBehaviors.add(behav);

		return behav;
	}

	return sol::nil;
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

	DatabaseLoader::dl_lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math);

	DatabaseLoader::dl_lua["debug_out"] = [](string text) {
		yytk_interface->PrintInfo(text);
		};

	DatabaseLoader::dl_lua["object_behaviors"] = DatabaseLoader::dl_lua.create_table();

	DatabaseLoader::dl_lua["object_behaviors"][0] = sol::nil;

	allBehaviors = DatabaseLoader::dl_lua["object_behaviors"];

	DatabaseLoader::dl_lua["new_object_behavior"] = NewObjectBehavior;

	DatabaseLoader::dl_lua["spawn_particle"] = DBLua::SpawnParticle;

	DatabaseLoader::dl_lua["init_var"] = DBLua::InitVar;

	DatabaseLoader::dl_lua["set_var"] = DBLua::SetVar;

	DatabaseLoader::dl_lua["init_number"] = DBLua::InitVar;

	DatabaseLoader::dl_lua["init_bool"] = DBLua::InitVar;

	DatabaseLoader::dl_lua["init_string"] = DBLua::InitVar;

	DatabaseLoader::dl_lua["set_number"] = DBLua::SetVar;

	DatabaseLoader::dl_lua["set_bool"] = DBLua::SetVar;

	DatabaseLoader::dl_lua["set_string"] = DBLua::SetVar;

	DatabaseLoader::dl_lua["get_number"] = DBLua::GetDouble;

	DatabaseLoader::dl_lua["get_bool"] = DBLua::GetBool;

	DatabaseLoader::dl_lua["get_string"] = DBLua::GetString;

	DatabaseLoader::dl_lua["custom_sprite"] = DBLua::GetCustomSprite;

	DatabaseLoader::dl_lua["custom_sound"] = DBLua::GetCustomSound;

	DatabaseLoader::dl_lua["custom_music"] = DBLua::GetCustomMusic;

	DatabaseLoader::dl_lua["unlock_song"] = DBLua::UnlockSong;

	DatabaseLoader::dl_lua["get_asset"] = DBLua::GetAsset;

	DatabaseLoader::dl_lua["call_gm_function"] = DBLua::CallFunction;

	DatabaseLoader::dl_lua["call_game_function"] = DBLua::CallGameFunction;

	DatabaseLoader::dl_lua["play_sound"] = DBLua::DoSound;

	DatabaseLoader::dl_lua["play_sound_ext"] = DBLua::DoSoundExt;

	g_YYTKInterface->PrintInfo("[Database Loader] Built-in functions loaded!");

	string dir = "C:/Program Files (x86)/Steam/steamapps/common/Star of Providence/DatabaseLoader/Mods";

	Files::MakeDirectory(dir);

	vector<filesystem::path> filesystem = Files::GetFilesOfType(dir, ".lua");

	for (size_t i = 0; i < filesystem.size(); i++)
	{
		g_YYTKInterface->PrintInfo("[Database Loader] File '" + filesystem[i].filename().string() + "' loaded...");
		dl_lua.script_file(filesystem[i].string());
	}

	yytk_interface->CreateCallback(
		Module,
		YYTK::EVENT_FRAME,
		ObjectBehaviorRun,
		0);

	CScript* script_data = nullptr;
	int script_index = 0;
	PVOID original_function = nullptr;

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

	return AURIE_SUCCESS;
}