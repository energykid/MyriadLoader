#include "DatabaseLoader.h"
#include "Aurie/shared.hpp"
#include "YYToolkit/YYTK_Shared.hpp"
#include "DBLua.h"
#include "GMWrappers.h"
#include "DatabaseLoader.h"
#include "ModuleMain.h"
#include "Files.h"
#include <map>
#include "sol/sol.hpp"

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
		return obj.ToDouble();
	case YYTK::VALUE_BOOL:
		return obj.ToBoolean();
	case YYTK::VALUE_STRING:
		return obj.ToString();
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
	RValue snd = g_YYTKInterface->CallBuiltin("audio_create_stream", { (string_view)("DatabaseLoader/Mods/" + path) });

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
	RValue snd = g_YYTKInterface->CallBuiltin("audio_create_stream", { (string_view)("DatabaseLoader/Mods/" + path) });

	if (!g_YYTKInterface->CallBuiltin("variable_global_exists", { "dbl_CustomMusicList" }))
	{
		g_YYTKInterface->CallBuiltin("variable_global_set", { "dbl_CustomMusicList", g_YYTKInterface->CallBuiltin("array_create", {}) });
	}
	g_YYTKInterface->CallBuiltin("array_push", { g_YYTKInterface->CallBuiltin("variable_global_get", { "dbl_CustomMusicList" }), snd });

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
	string name = g_YYTKInterface->CallBuiltin("audio_get_name", {songName}).ToString();

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
double DatabaseLoader::DBLua::GetCustomSprite(string path, double imgnum, double xorig, double yorig)
{
	return g_YYTKInterface->CallBuiltin("sprite_add", { (string_view)("DatabaseLoader/Mods/" + path), imgnum, false, false, xorig, yorig }).ToDouble();
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

void DatabaseLoader::DBLua::DrawSprite(double x, double y, double spriteID, double frameNumber)
{
	g_YYTKInterface->CallBuiltin("draw_sprite", { spriteID, frameNumber, x, y });
}

void DatabaseLoader::DBLua::DrawSpriteExt(double x, double y, double spriteID, double frameNumber, double rotation, double xScale, double yScale, double color, double alpha)
{
	g_YYTKInterface->CallBuiltin("draw_sprite_ext", { spriteID, frameNumber, x, y, xScale, yScale, rotation, color, alpha });
}

sol::table DatabaseLoader::DBLua::EnemyData(string name)
{
	sol::table data = dl_lua.create_table_with(
		"DataType", "enemy",
		"Name", name,
		"Create", [](double) {},
		"Step", [](double) {},
		"Destroy", [](double) {},
		"TakeDamage", [](double, double) {});

	return data;
}

sol::table DatabaseLoader::DBLua::ProjectileData(string name)
{
	return dl_lua.create_table_with(
		"DataType", "projectile",
		"Name", name);
}