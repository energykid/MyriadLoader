#include "DatabaseLoader.h"
#include "Aurie/shared.hpp"
#include "YYToolkit/YYTK_Shared.hpp"
#include "DBLua.h"
#include "GMWrappers.h"
#include <map>
#include "sol/sol.hpp"

using namespace DatabaseLoader;

static std::map<std::string, TRoutine> m_BuiltinFunctionCache = {};

RValue ObjectToValue(sol::object obj)
{
	switch (obj.get_type())
	{
	case sol::lua_type_of_v<double>:
		return obj.as<double>();
	case sol::lua_type_of_v<bool>:
		return obj.as<bool>();
	case sol::lua_type_of_v<string>:
		return obj.as<string_view>();
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

double DatabaseLoader::DBLua::GetDouble(double inst, string varName)
{
	return g_YYTKInterface->CallBuiltin("variable_instance_get", {
		inst,
		(string_view)varName
		}).ToDouble();
}

bool DatabaseLoader::DBLua::GetBool(double inst, string varName)
{
	return g_YYTKInterface->CallBuiltin("variable_instance_get", {
		inst,
		(string_view)varName
		}).ToBoolean();
}

string DatabaseLoader::DBLua::GetString(double inst, string varName)
{
	return (string)g_YYTKInterface->CallBuiltin("variable_instance_get", {
		inst,
		(string_view)varName
		}).ToString();
}

double DatabaseLoader::DBLua::GetCustomSound(string path)
{
	RValue snd = g_YYTKInterface->CallBuiltin("audio_create_stream", { (string_view)("DatabaseLoader/Mods/" + path) });

	g_YYTKInterface->CallBuiltin("ds_list_add", { GMWrappers::GetGlobal("snd_list"), snd });

	return snd.ToDouble();
}

double DatabaseLoader::DBLua::GetCustomMusic(string path, string musicName)
{
	RValue snd = g_YYTKInterface->CallBuiltin("audio_create_stream", { (string_view)("DatabaseLoader/Mods/" + path) });

	if (!g_YYTKInterface->CallBuiltin("variable_global_exists", { "dbl_CustomMusicList" }))
	{
		g_YYTKInterface->CallBuiltin("variable_global_set", { "dbl_CustomMusicList", g_YYTKInterface->CallBuiltin("array_create", {}) });
	}
	g_YYTKInterface->CallBuiltin("array_push", { g_YYTKInterface->CallBuiltin("variable_global_get", { "dbl_CustomMusicList" }), snd});

	g_YYTKInterface->CallBuiltin("ds_list_add", { GMWrappers::GetGlobal("mus_list"), snd });
	g_YYTKInterface->CallBuiltin("ds_list_add", { GMWrappers::GetGlobal("song_name"), (string_view)musicName });

	return snd.ToDouble();
}

void DatabaseLoader::DBLua::UnlockSong(double songName)
{
	string name = g_YYTKInterface->CallBuiltin("audio_get_name", {songName}).ToString();

	GMWrappers::CallGameScript("gml_Script_music_unlock", { (string_view)name });
}

double DatabaseLoader::DBLua::GetCustomSprite(string path, double imgnum, double xorig, double yorig)
{
	return g_YYTKInterface->CallBuiltin("sprite_add", { (string_view)("DatabaseLoader/Mods/" + path), imgnum, false, false, xorig, yorig }).ToDouble();
}

void DatabaseLoader::DBLua::DoSound(double soundType, double x)
{
	GMWrappers::CallGameScript("gml_Script_sound_do", { soundType, x });
}

void DatabaseLoader::DBLua::DoSoundExt(double soundType, double pitch, double gain, double x)
{
	GMWrappers::CallGameScript("gml_Script_sound_do_ext", { soundType, pitch, gain, x });
}

double DatabaseLoader::DBLua::GetAsset(string name)
{
	return g_YYTKInterface->CallBuiltin("asset_get_index", { (string_view)name }).ToDouble();
}

double DatabaseLoader::DBLua::CallFunction(string name, sol::table args)
{
	vector<RValue> vals = {};

	for (size_t i = 0; i < args.size(); i++)
	{
		vals.push_back(args[i]);
	}

	return g_YYTKInterface->CallBuiltin(name.c_str(), vals).ToDouble();
}

void DatabaseLoader::DBLua::CallGameFunction(string name, sol::table args)
{
	vector<RValue> vals = {};

	for (size_t i = 0; i < args.size(); i++)
	{
		vals.push_back(args[i]);
	}

	GMWrappers::CallGameScript(name.c_str(), vals);
}

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
