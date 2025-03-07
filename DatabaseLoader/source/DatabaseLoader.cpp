#include "DatabaseLoader.h"
#include "Aurie/shared.hpp"

using namespace DatabaseLoader;

vector<sol::table> DLInterfaceImpl::objectBehaviors = {};
YYTKInterface* g_YYTKInterface = nullptr;

AurieStatus DLInterfaceImpl::Create()
{
	objectBehaviors = {};

	return AURIE_SUCCESS;
}

void DatabaseLoader::DLInterfaceImpl::Destroy()
{
}

void DatabaseLoader::DLInterfaceImpl::QueryVersion(OUT short& Major, OUT short& Minor, OUT short& Patch)
{
}

void DatabaseLoader::DLInterfaceImpl::AddObjectBehavior(sol::table behavior)
{
	objectBehaviors.push_back(behavior);
}

void DatabaseLoader::DLInterfaceImpl::InitializeVariable(int inst, string varName, sol::object value)
{
	RValue val = 0;
	switch (value.get_type())
	{
	case sol::lua_type_of_v<int>:
		val = value.as<int>();
		break;
	case sol::lua_type_of_v<bool>:
		val = value.as<bool>();
		break;
	case sol::lua_type_of_v<string>:
		val = value.as<string>();
		break;
	default:
		break;
	}

	if (!g_YYTKInterface->CallBuiltin("variable_instance_exists", {
		inst,
		varName
		}).AsBool())
	{
		g_YYTKInterface->CallBuiltin("variable_instance_set", {
		inst,
		varName,
		val
			});
	}
}

void DatabaseLoader::DLInterfaceImpl::SetVariable(int inst, string varName, sol::object value)
{
	RValue val = 0;
	switch (value.get_type())
	{
	case sol::lua_type_of_v<int>:
		val = value.as<int>();
		break;
	case sol::lua_type_of_v<bool>:
		val = value.as<bool>();
		break;
	case sol::lua_type_of_v<string>:
		val = value.as<string>();
		break;
	default:
		break;
	}
	g_YYTKInterface->CallBuiltin("variable_instance_set", {
	inst,
	varName,
	val
		});
}

int DatabaseLoader::DLInterfaceImpl::GetInstanceID(int inst)
{
	return g_YYTKInterface->CallBuiltin("variable_instance_get", {
	inst,
	"index" }).AsReal();
}

int DatabaseLoader::DLInterfaceImpl::GetInt(int inst, string varName)
{
	return g_YYTKInterface->CallBuiltin("variable_instance_get", {
	inst,
	varName }).AsReal();
}

bool DatabaseLoader::DLInterfaceImpl::GetBool(int inst, string varName)
{
	return g_YYTKInterface->CallBuiltin("variable_instance_get", {
	inst,
	varName }).AsBool();
}

/// <summary>
/// Gets a sound from the given path, then returns the sound asset. .OGG ONLY.
/// </summary>
/// <param name="path">The file path to get the sound from - starting with the mod folder.
/// Ex. Getting the file at "Monolith/mods/Aurie/ModName/Textures/tex.png" you would input "ModName/Textures/tex.png".
/// </param>
/// <returns></returns>
int DatabaseLoader::DLInterfaceImpl::GetSound(string path)
{
	return (int)g_YYTKInterface->CallBuiltin("audio_create_stream", { "DatabaseLoader/" + path }).AsReal();
}

/// <summary>
/// Gets a sprite from the given path, with the given number of images ad the given origin, then returns the sprite asset.
/// </summary>
/// <param name="path"></param>
/// <param name="imgnum"></param>
/// <param name="xorig"></param>
/// <param name="yorig"></param>
/// <returns></returns>
int DatabaseLoader::DLInterfaceImpl::GetSprite(string path, int imgnum, int xorig, int yorig)
{
	return (int)g_YYTKInterface->CallBuiltin("sprite_add", { "DatabaseLoader/" + path, imgnum, false, false, xorig, yorig }).AsReal();
}

RValue DatabaseLoader::DLInterfaceImpl::SpawnBasicParticle(int x, int y, int sprite)
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

	return part;
}

int DatabaseLoader::DLInterfaceImpl::SpawnParticle(int x, int y, int xvel, int yvel, int sprite)
{
	RValue part = g_ModuleInterface.SpawnBasicParticle(x, y, sprite);

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

	return part.AsReal();
}

void DLInterfaceImpl::InvokeWithObjectIndex(string Object, sol::protected_function func)
{
	RValue object_index = g_YYTKInterface->CallBuiltin(
		"asset_get_index",
		{ Object }
	);

	int64_t object_count = static_cast<int64_t>(g_YYTKInterface->CallBuiltin(
		"instance_number",
		{ object_index }
	).AsReal());

	for (int64_t i = 0; i < object_count; i++)
	{
		RValue instance = g_YYTKInterface->CallBuiltin(
			"instance_find",
			{
				object_index,
				i
			}
		);
		int id = g_YYTKInterface->CallBuiltin("instance_id_get", { instance }).AsReal();

		func.call(id);
	}
}
