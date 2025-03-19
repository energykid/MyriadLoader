#include "DatabaseLoader.h"
#include "Aurie/shared.hpp"
#include "YYToolkit/YYTK_Shared.hpp"
#include "DBLua.h"
#include "GMWrappers.h"
#include "ModuleMain.h"
#include "Files.h"
#include <map>
#include "sol/sol.hpp"
#include <iostream>
#include <string>

using namespace DatabaseLoader;

static std::map<std::string, TRoutine> m_BuiltinFunctionCache = {};

RValue ObjectToValue(sol::object obj)
{
	switch (obj.get_type())
	{
	case sol::lua_type_of_v<double>:
		return RValue(obj.as<double>());
	case sol::lua_type_of_v<bool>:
		return RValue(obj.as<bool>());
	case sol::lua_type_of_v<string>:
		return RValue(obj.as<string_view>());
	}
}
sol::lua_value ValueToObject(RValue obj)
{
	switch (obj.m_Kind)
	{
	case YYTK::VALUE_REAL:
		return sol::lua_value(modState[currentState], obj.ToDouble());
	case YYTK::VALUE_BOOL:
		return sol::lua_value(modState[currentState], obj.ToBoolean());
	case YYTK::VALUE_STRING:
		return sol::lua_value(modState[currentState], obj.ToString());
	}
}
RValue DatabaseLoader::DBLua::CallBuiltinLua(
	IN const char* FunctionName,
	IN std::vector<RValue> Arguments
)
{
	CInstance* global_instance = nullptr;

	if (!AurieSuccess(g_YYTKInterface->GetGlobalInstance(&global_instance)))
		return {};

	// Previous check should've tripped
	assert(global_instance != nullptr);

	// Make sure to return an unset RValue if the lookup fails
	RValue result;
	if (!AurieSuccess(g_YYTKInterface->CallBuiltinEx(
		result,
		FunctionName,
		global_instance,
		global_instance,
		Arguments
	)))
	{
		return {};
	}

	return result;
}

AurieStatus DatabaseLoader::DBLua::CallBuiltinExLua(
	OUT RValue& Result,
	IN const char* FunctionName,
	IN CInstance* SelfInstance,
	IN CInstance* OtherInstance,
	IN std::vector<sol::object> Arguments
)
{
	std::vector<RValue> Args;

	for (size_t i = 0; i < Arguments.size(); i++)
	{
		Args.push_back(ObjectToValue(Arguments[i]));
	}

	if (m_BuiltinFunctionCache.contains(FunctionName))
	{
		m_BuiltinFunctionCache.at(FunctionName)(
			Result,
			SelfInstance,
			OtherInstance,
			static_cast<int>(Arguments.size()),
			Args.data()
			);

		return AURIE_SUCCESS;
	}

	// We don't have a cached value, so we try to fetch
	TRoutine function = nullptr;
	AurieStatus last_status = AURIE_SUCCESS;

	// Query for the function pointer
	last_status = g_YYTKInterface->GetNamedRoutinePointer(
		FunctionName,
		reinterpret_cast<PVOID*>(&function)
	);

	// Make sure we found a function
	if (!AurieSuccess(last_status))
		return last_status;

	// Previous check should've fired
	assert(function != nullptr);

	// Cache the result
	m_BuiltinFunctionCache.insert(
		std::make_pair(FunctionName, function)
	);

	function(
		Result,
		SelfInstance,
		OtherInstance,
		static_cast<int>(Arguments.size()),
		Args.data()
	);

	return AURIE_SUCCESS;
}

void DatabaseLoader::DBLua::InvokeWithObjectIndex(string Object, sol::protected_function func)
{
	RValue object_index = g_YYTKInterface->CallBuiltin(
		"asset_get_index",
		{ (string_view)Object }
	);

	int64_t object_count = static_cast<int64_t>(g_YYTKInterface->CallBuiltin(
		"instance_number",
		{ object_index }
	).ToDouble());

	for (int64_t i = 0; i < object_count; i++)
	{
		RValue instance = g_YYTKInterface->CallBuiltin(
			"instance_find",
			{
				object_index,
				i
			}
		);

		sol::protected_function_result result = func.call(g_YYTKInterface->CallBuiltin("variable_instance_get", { instance, "id" }).ToDouble());
		if (!result.valid())
		{
			sol::error error = result;

			g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
		}
	}
}
void DatabaseLoader::DBLua::InvokeWithCustomData(string Name, sol::protected_function func)
{
	RValue object_index = g_YYTKInterface->CallBuiltin(
		"asset_get_index",
		{ "obj_enemy" }
	);

	int64_t object_count = static_cast<int64_t>(g_YYTKInterface->CallBuiltin(
		"instance_number",
		{ object_index }
	).ToDouble());

	for (int64_t i = 0; i < object_count; i++)
	{
		RValue instance = g_YYTKInterface->CallBuiltin(
			"instance_find",
			{
				object_index,
				i
			}
		);

		if (g_YYTKInterface->CallBuiltin("variable_instance_exists", { instance, "myr_CustomName" }))
		{
			if (g_YYTKInterface->CallBuiltin("variable_instance_get", { instance, "myr_CustomName" }).ToString() == Name)
			{
				sol::protected_function_result result = func.call(g_YYTKInterface->CallBuiltin("variable_instance_get", { instance, "id" }).ToDouble());
				if (!result.valid())
				{
					sol::error error = result;
					g_YYTKInterface->PrintWarning("LUA ERROR: " + (string)error.what());
				}
			}
		}
	}
}

/***
Initialize a number, boolean, or string, local to the instance ID provided. Doesn't run any code if the variable exists already.
@function init_var
@param inst the instance ID to set the variable to
@param varName the name of the variable to set
@param val the value to set the variable to
@return void
*/
void DatabaseLoader::DBLua::InitVar(double inst, string varName, sol::object val)
{
	switch (val.get_type())
	{
	case sol::lua_type_of_v<double>:
		DatabaseLoader::DBLua::InitDouble(inst, varName, val.as<double>());
		break;
	case sol::lua_type_of_v<bool>:
		DatabaseLoader::DBLua::InitBool(inst, varName, val.as<bool>());
		break;
	case sol::lua_type_of_v<string>:
		DatabaseLoader::DBLua::InitString(inst, varName, val.as<string>());
		break;
	}
}

/***
Set a number, boolean, or string, local to the instance ID provided. Will work with GameMaker built-in variables too.
@function set_var
@param inst the instance ID to set the variable to
@param varName the name of the variable to set
@param val the value to set the variable to
@return void
*/
void DatabaseLoader::DBLua::SetVar(double inst, string varName, sol::object val)
{
	switch (val.get_type())
	{
	case sol::lua_type_of_v<double>:
		DatabaseLoader::DBLua::SetDouble(inst, varName, val.as<double>());
		break;
	case sol::lua_type_of_v<bool>:
		DatabaseLoader::DBLua::SetBool(inst, varName, val.as<bool>());
		break;
	case sol::lua_type_of_v<string>:
		DatabaseLoader::DBLua::SetString(inst, varName, val.as<string>());
		break;
	case sol::lua_type_of_v<sol::table>:
		DatabaseLoader::DBLua::SetArray(inst, varName, val.as<sol::table>());
		break;
	}
}

void DatabaseLoader::DBLua::InitGlobal(string varName, sol::object val)
{
	if (!g_YYTKInterface->CallBuiltin("variable_global_exists", {
		(string_view)varName
		}).ToBoolean())
	{
		GMWrappers::SetGlobal(varName, ObjectToValue(val));
	}
}

void DatabaseLoader::DBLua::SetGlobal(string varName, sol::object val)
{
	GMWrappers::SetGlobal(varName, ObjectToValue(val));
}

sol::lua_value DatabaseLoader::DBLua::GetVar(double inst, string varName)
{
	RValue val = g_YYTKInterface->CallBuiltin("variable_instance_get", {
		inst,
		(string_view)varName });

	return ValueToObject(val);
}

sol::lua_value DatabaseLoader::DBLua::GetGlobal(string varName)
{
	return ValueToObject(GMWrappers::GetGlobal(varName));
}

void DatabaseLoader::DBLua::InitDouble(double inst, string varName, double val)
{
	if (!g_YYTKInterface->CallBuiltin("variable_instance_exists", {
		inst,
		(string_view)varName
		}).ToBoolean())
	{
		g_YYTKInterface->CallBuiltin("variable_instance_set", {
		inst,
		(string_view)varName,
		val
			});
	}
}

void DatabaseLoader::DBLua::InitBool(double inst, string varName, bool val)
{
	if (!g_YYTKInterface->CallBuiltin("variable_instance_exists", {
		inst,
		(string_view)varName
		}).ToBoolean())
	{
		g_YYTKInterface->CallBuiltin("variable_instance_set", {
		inst,
		(string_view)varName,
		val
			});
	}
}

void DatabaseLoader::DBLua::InitString(double inst, string varName, string val)
{
	if (!g_YYTKInterface->CallBuiltin("variable_instance_exists", {
		inst,
		(string_view)varName
		}).ToBoolean())
	{
		g_YYTKInterface->CallBuiltin("variable_instance_set", {
		inst,
		(string_view)varName,
		(string_view)val
			});
	}
}

void DatabaseLoader::DBLua::InitArray(double inst, string varName, sol::table vals)
{
	if (!g_YYTKInterface->CallBuiltin("variable_instance_exists", {
		inst,
		(string_view)varName
		}).ToBoolean())
	{
		SetArray(inst, varName, vals);
	}
}

void DatabaseLoader::DBLua::SetDouble(double inst, string varName, double val)
{
	g_YYTKInterface->CallBuiltin("variable_instance_set", {
	inst,
	(string_view)varName,
	val
		});
}

void DatabaseLoader::DBLua::SetBool(double inst, string varName, bool val)
{
	g_YYTKInterface->CallBuiltin("variable_instance_set", {
	inst,
	(string_view)varName,
	val
		});
}

void DatabaseLoader::DBLua::SetString(double inst, string varName, string val)
{
	g_YYTKInterface->CallBuiltin("variable_instance_set", {
	inst,
	(string_view)varName,
	(string_view)val
		});
}

void DatabaseLoader::DBLua::SetArray(double inst, string varName, sol::table vals)
{
	RValue arr = g_YYTKInterface->CallBuiltin("array_create", { vals.size() });

	for (size_t i = 1; i < vals.size() + 1; i++)
	{
		g_YYTKInterface->CallBuiltin("array_push", { arr, ObjectToValue(vals[i]) });
	}

	g_YYTKInterface->CallBuiltin("variable_instance_set", {
	inst,
	(string_view)varName,
	arr
		});
}


/***
Retrieve a number local to the instance provided. Can only retrieve numbers.
@function get_double
@param inst the instance ID to get the variable from
@param varName the name of the variable to get
@return the value retrieved
*/
double DatabaseLoader::DBLua::GetDouble(double inst, string varName)
{
	return g_YYTKInterface->CallBuiltin("variable_instance_get", {
		inst,
		(string_view)varName
		}).ToDouble();
}

/***
Retrieve a boolean local to the instance provided. Can only retrieve booleans.
@function get_bool
@param inst the instance ID to get the variable from
@param varName the name of the variable to get
@return the value retrieved
*/
bool DatabaseLoader::DBLua::GetBool(double inst, string varName)
{
	return g_YYTKInterface->CallBuiltin("variable_instance_get", {
		inst,
		(string_view)varName
		}).ToBoolean();
}

/***
Retrieve a string local to the instance provided. Can only retrieve string.
@function get_string
@param inst the instance ID to get the variable from
@param varName the name of the variable to get
@return the value retrieved
*/
string DatabaseLoader::DBLua::GetString(double inst, string varName)
{
	return (string)g_YYTKInterface->CallBuiltin("variable_instance_get", {
		inst,
		(string_view)varName
		}).ToString();
}

/***
Register a custom sound effect from the provided file path.
@function init_var
@param path the path (starting at DatabaseLoader/Mods/) to get the sound from
@return the sound ID assigned to the provided file
*/
double DatabaseLoader::DBLua::GetCustomSound(string path)
{
	RValue snd = g_YYTKInterface->CallBuiltin("audio_create_stream", { (string_view)("MyriadLoader/Mods/" + path) });

	g_YYTKInterface->CallBuiltin("ds_list_add", { GMWrappers::GetGlobal("snd_list"), snd });

	return snd.ToDouble();
}

/***
Register a custom music piece from the provided file path, then add it to the jukebox.
@function custom_music
@param path the path (starting at DatabaseLoader/Mods/) to get the sound from
@param musicName the name this music piece will have at the jukebox
@return the sound ID assigned to the provided file
*/
double DatabaseLoader::DBLua::GetCustomMusic(string path, string musicName)
{
	RValue snd = g_YYTKInterface->CallBuiltin("audio_create_stream", { (string_view)("MyriadLoader/Mods/" + path) });

	if (!g_YYTKInterface->CallBuiltin("variable_global_exists", { "myr_CustomMusicList" }))
	{
		g_YYTKInterface->CallBuiltin("variable_global_set", { "myr_CustomMusicList", g_YYTKInterface->CallBuiltin("array_create", {}) });
	}
	g_YYTKInterface->CallBuiltin("array_push", { g_YYTKInterface->CallBuiltin("variable_global_get", { "myr_CustomMusicList" }), snd });

	g_YYTKInterface->CallBuiltin("ds_list_add", { GMWrappers::GetGlobal("mus_list"), snd });
	g_YYTKInterface->CallBuiltin("ds_list_add", { GMWrappers::GetGlobal("song_name"), (string_view)musicName });

	//Files::CopyFileTo(Files::GetModsDirectory() + path, Files::GetSteamDirectory());

	return snd.ToDouble();
}

/***
Unlock the provided music piece at the jukebox.
@function unlock_song
@param songName the sound ID assigned to the song to unlock
@return void
*/
void DatabaseLoader::DBLua::UnlockSong(double songName)
{
	string name = g_YYTKInterface->CallBuiltin("audio_get_name", { songName }).ToString();

	GMWrappers::CallGameScript("gml_Script_music_unlock", { (string_view)name });
}

/***
Unlock the provided music piece at the jukebox.
@function custom_sprite
@param path the path (starting at DatabaseLoader/Mods/) to get the sprite from 
@param imgnum number of frames in the sprite (1 if still image)
@param xorig the x position in pixels of the sprite's origin 
@param yorig the y position in pixels of the sprite's origin  
@return the sprite ID assigned to the texture
*/
double DatabaseLoader::DBLua::GetCustomSprite(string path, double imgnum, double xorig, double yorig, double frames)
{
	RValue a = g_YYTKInterface->CallBuiltin("sprite_add", { (string_view)("MyriadLoader/Mods/" + path), imgnum, false, false, xorig, yorig }).ToDouble();

	g_YYTKInterface->CallBuiltin("sprite_set_speed", {a, frames, 0});

	return a.ToDouble();
}

/***
Plays a sound at the given X position.
@function do_sound
@param soundType the assigned ID of the sound
@param x the x position of the sound (for directional audio)
@return void
*/
void DatabaseLoader::DBLua::DoSound(double soundType, double x)
{
	GMWrappers::CallGameScript("gml_Script_sound_do", { soundType, x });
}

/***
Plays a sound at the given X position.
@function do_sound_ext
@param soundType the assigned ID of the sound
@param pitch the pitch to play the sound with (1 being default)
@param gain the gain of the sound (from 0 to 1)
@param x the x position of the sound (for directional audio)
@return void
*/
void DatabaseLoader::DBLua::DoSoundExt(double soundType, double pitch, double gain, double x)
{
	GMWrappers::CallGameScript("gml_Script_sound_do_ext", { soundType, pitch, gain, x });
}

/***
Retrieves the asset ID with the given name.
@function get_asset
@param name the name of the asset to get
@return the asset ID retrieved
*/
double DatabaseLoader::DBLua::GetAsset(string name)
{
	return g_YYTKInterface->CallBuiltin("asset_get_index", { (string_view)name }).ToDouble();
}

/***
Calls a built-in GameMaker function. Returns void.
@function call_gm_function
@param name the name of the function to call as a string
@param args a table containing the arguments to pass in
@return void
*/
sol::lua_value DatabaseLoader::DBLua::CallFunction(string name, sol::table args)
{
	vector<RValue> vals = {};

	for (size_t i = 1; i < args.size() + 1; i++)
	{
		vals.push_back(ObjectToValue(args[i]));
	}

	RValue val = g_YYTKInterface->CallBuiltin(name.c_str(), vals);

	return ValueToObject(val);
}

void DatabaseLoader::DBLua::DrawRect(double x1, double y1, double x2, double y2, bool outline)
{
	RValue val = g_YYTKInterface->CallBuiltin("draw_rectangle", {x1, y1, x2, y2, outline});
}

/***
Calls a Star of Providence function. Returns void.
@function call_game_function
@param name the name of the function to call as a string
@param args a table containing the arguments to pass in
@return void
*/
void DatabaseLoader::DBLua::CallGameFunction(string name, sol::table args)
{
	vector<RValue> vals = {};

	for (size_t i = 0; i < args.size(); i++)
	{
		vals.push_back(args[i]);
	}

	GMWrappers::CallGameScript(name.c_str(), vals);
}

/***
Spawns a customizable particle.
@function spawn_particle
@param x the x position to spawn the particle at
@param y the y position to spawn the particle at
@param xvel the x velocity to spawn the particle with
@param yvel the y velocity to spawn the particle with
@param sprite the ID of the sprite to spawn the particle with (can be gained with custom_sprite)
@return the instance ID of the particle spawned
*/
double DatabaseLoader::DBLua::SpawnParticle(double x, double y, double xvel, double yvel, double sprite)
{
	RValue part = g_YYTKInterface->CallBuiltin(
		"instance_create_depth",
		{
			x,
			y,
			0,
			g_YYTKInterface->CallBuiltin(
			"asset_get_index",
				{"obj_generic_particle"}
			),
		}
	);

	g_YYTKInterface->CallBuiltin(
		"variable_instance_set",
		{
			part,
			"sprite_index",
			sprite
		}
	);

	g_YYTKInterface->CallBuiltin(
		"variable_instance_set",
		{
			part,
			"hspeed",
			xvel
		}
	);
	g_YYTKInterface->CallBuiltin(
		"variable_instance_set",
		{
			part,
			"vspeed",
			yvel
		}
	);

	return part.ToDouble();
}

double DatabaseLoader::DBLua::SpawnEnemy(double x, double y, string name)
{
	RValue enemyAsset = g_YYTKInterface->CallBuiltin("asset_get_index", { (string_view)name });
	RValue enemyExists = g_YYTKInterface->CallBuiltin("object_exists", { enemyAsset });

	if (enemyExists)
	{
		RValue enemy = g_YYTKInterface->CallBuiltin("instance_create_depth", { x, y, 0,
			g_YYTKInterface->CallBuiltin("asset_get_index", {(string_view)name}) });
		return enemy.ToDouble();
	}
	else
	{
		// The object name (between swarmers for regular enemies, and the templates for the other two types of enemies)
		string ObjectToSpawn = "obj_swarmer";
		if (std::find(customMinibossNames.begin(), customMinibossNames.end(), name) != customMinibossNames.end()) ObjectToSpawn = "obj_miniboss_template";

		RValue enemy = g_YYTKInterface->CallBuiltin("instance_create_depth", { x, y, 0,
			g_YYTKInterface->CallBuiltin("asset_get_index", { (string_view)ObjectToSpawn }) });
		g_YYTKInterface->CallBuiltin("variable_instance_set", { enemy, "myr_CustomName", (string_view)name });
		g_YYTKInterface->CallBuiltin("variable_instance_set", { enemy, "behavior", "myr_custom" });
		if (ObjectToSpawn == "obj_miniboss_template")
			g_YYTKInterface->CallBuiltin("variable_instance_set", { enemy, "damage_source_id", 61 });

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
						if (tbl.get<string>("DataType") == "enemy")
						{
							if (tbl.get<string>("Name") == name)
							{
								sol::protected_function_result result = modState.at(stateNum)["all_behaviors"][var]["Create"].call(g_YYTKInterface->CallBuiltin("variable_instance_get", { enemy, "id" }).ToDouble());
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

		return enemy.ToDouble();
	}

	return 0.0;
}

double DatabaseLoader::DBLua::SpawnProjectile(double x, double y, double xvel, double yvel, double sprite)
{
	RValue proj = g_YYTKInterface->CallBuiltin("instance_create_depth", { x, y, 1, g_YYTKInterface->CallBuiltin("asset_get_index", {"obj_bullet_type"})});

	double id = g_YYTKInterface->CallBuiltin("variable_instance_get", { proj, "id" }).ToDouble();

	SetDouble(id, "hspeed", xvel);
	SetDouble(id, "vspeed", yvel);
	SetDouble(id, "sprite_index", sprite);

	return proj.ToDouble();
}

void DatabaseLoader::DBLua::AddCallbackTo(double id, sol::protected_function function)
{

}

/***
Sets the depth for draw functions.
@function draw_set_depth
@param dep the depth to set
@return void
*/
void DatabaseLoader::DBLua::DrawSetDepth(double dep)
{
	g_YYTKInterface->CallBuiltin("gpu_set_depth", { dep });
}

void DatabaseLoader::DBLua::DrawSetColor(double col)
{
	g_YYTKInterface->CallBuiltin("draw_set_color", { col });
}

double DatabaseLoader::DBLua::CreateColor(double r, double g, double b)
{
	return r + (g * 16 * 16) + (b * 16 * 16 * 16 * 16);
}

void DatabaseLoader::DBLua::DrawSprite(double x, double y, double spriteID, double frameNumber)
{
	g_YYTKInterface->CallBuiltin("draw_sprite", { spriteID, frameNumber, x, y });
}

void DatabaseLoader::DBLua::DrawString(double x, double y, string text)
{
	vector<RValue> vals;

	vals.push_back(x);
	vals.push_back(y);
	vals.push_back((string_view)(text + " "));

	g_YYTKInterface->CallBuiltin("draw_set_halign", { 0 });
	g_YYTKInterface->CallBuiltin("draw_set_valign", { 0 });

	g_YYTKInterface->CallBuiltin("draw_text", vals);
}

void DatabaseLoader::DBLua::DrawStringColor(double x, double y, string text, double color)
{
	vector<RValue> vals;

	vals.push_back(x);
	vals.push_back(y);
	vals.push_back((string_view)(text + " "));

	g_YYTKInterface->CallBuiltin("draw_set_halign", { 0 });
	g_YYTKInterface->CallBuiltin("draw_set_valign", { 0 });

	g_YYTKInterface->CallBuiltin("draw_set_color", { color });

	g_YYTKInterface->CallBuiltin("draw_text", vals);
}

void DatabaseLoader::DBLua::DrawSpriteExt(double x, double y, double spriteID, double frameNumber, double rotation, double xScale, double yScale, double color, double alpha)
{
	g_YYTKInterface->CallBuiltin("draw_sprite_ext", { spriteID, frameNumber, x, y, xScale, yScale, rotation, color, alpha });
}

sol::table DatabaseLoader::DBLua::DirectionTo(double x1, double y1, double x2, double y2)
{
	RValue dir = g_YYTKInterface->CallBuiltin("point_direction", {x1, y1, x2, y2});

	RValue x = g_YYTKInterface->CallBuiltin("lengthdir_x", { 1, dir });
	RValue y = g_YYTKInterface->CallBuiltin("lengthdir_y", { 1, dir });

	return modState[currentState].create_table_with("x", x.ToDouble(), "y", y.ToDouble());
}

sol::table DatabaseLoader::DBLua::EnemyData(string name)
{
	return modState[currentState].create_table_with(
		"DataType", "enemy",
		"Name", name,
		"Miniboss", false,
		"Boss", false,
		"Create", [](double) {},
		"Step", [](double) {},
		"Destroy", [](double) {},
		"Draw", [](double) {},
		"TakeDamage", [](double, double) {});
}

sol::table DatabaseLoader::DBLua::ProjectileData(string name)
{
	return modState[currentState].create_table_with(
		"DataType", "projectile",
		"Name", name,
		"Create", [](double) {},
		"Step", [](double) {},
		"CollideWith", [](double, double) {},
		"Draw", [](double) {});
}

sol::table DatabaseLoader::DBLua::GlobalData()
{
	return modState[currentState].create_table_with(
		"DataType", "global",
		"Step", []() {},
		"Draw", []() {},
		"DrawUI", []() {});
}

sol::table DatabaseLoader::DBLua::PlayerData()
{
	return modState[currentState].create_table_with(
		"DataType", "player",
		"Step", [](double) {},
		"TakeDamage", [](double, double) {},
		"Draw", [](double) {});
}

void DatabaseLoader::DBLua::AddRoomsTo(string sourceName, string destinationName)
{
	roomFiles.push_back(RoomFileReplacement(
		Files::GetModsDirectory() + sourceName,
		Files::GetSteamDirectory() + "rooms/" + destinationName,
		Files::GetSteamDirectory() + "rooms/backup/" + destinationName
		));
}
