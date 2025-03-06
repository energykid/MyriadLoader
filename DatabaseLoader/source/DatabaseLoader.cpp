#include "DatabaseLoader.h"

using namespace DatabaseLoader;

vector<ObjectBehavior> DLInterfaceImpl::objectBehaviors = {};
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

void DatabaseLoader::DLInterfaceImpl::AddObjectBehavior(ObjectBehavior behavior)
{
	objectBehaviors.push_back(behavior);
}

/// <summary>
/// Adds a custom keyword.
/// </summary>
/// <param name="keyword"></param>
void DatabaseLoader::DLInterfaceImpl::AddCustomKeyword(Keyword keyword)
{
	customKeywords.push_back(keyword);
}

void DatabaseLoader::DLInterfaceImpl::InitializeVariable(int inst, string varName, RValue value)
{
	if (!g_YYTKInterface->CallBuiltin("variable_instance_exists", {
		inst,
		varName
		}).AsBool())
	{
		g_YYTKInterface->CallBuiltin("variable_instance_set", {
		inst,
		varName,
		value
			});
	}
}

void DatabaseLoader::DLInterfaceImpl::SetVariable(int inst, string varName, RValue value)
{
	g_YYTKInterface->CallBuiltin("variable_instance_set", {
	inst,
	varName,
	value
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
	return g_YYTKInterface->CallBuiltin("audio_create_stream", { "mods/Aurie/" + path }).AsReal();
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
	return g_YYTKInterface->CallBuiltin("sprite_add", { "mods/Aurie/" + path, imgnum, false, false, xorig, yorig }).AsReal();
}

/// <summary>
/// Spawns a particle with the given sprite at the given X and Y position and returns the instance index.
/// </summary>
/// <param name="x">X position to spawn the particle at.</param>
/// <param name="y">Y position to spawn the particle at.</param>
/// <param name="sprite">The sprite used by the particle, gathered from GetSprite().</param>
/// <returns></returns>
int DatabaseLoader::DLInterfaceImpl::SpawnParticle(int x, int y, int sprite)
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

	return part.AsReal();
}

int DatabaseLoader::DLInterfaceImpl::SpawnParticle(int x, int y, int xvel, int yvel, int sprite)
{
	RValue part = g_ModuleInterface.SpawnParticle(x, y, sprite);

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
