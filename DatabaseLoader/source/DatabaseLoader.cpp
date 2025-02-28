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

/// <summary>
/// Gets a sound from the given path, then returns the sound asset. .OGG ONLY.
/// </summary>
/// <param name="path">The file path to get the sound from - starting with the mod folder.
/// Ex. Getting the file at "Monolith/mods/Aurie/ModName/Textures/tex.png" you would input "ModName/Textures/tex.png".
/// </param>
/// <returns></returns>
RValue DatabaseLoader::DLInterfaceImpl::GetSound(string path)
{
	return g_YYTKInterface->CallBuiltin("audio_create_stream", { "mods/Aurie/" + path });
}

/// <summary>
/// Gets a sprite from the given path, with the given number of images ad the given origin, then returns the sprite asset.
/// </summary>
/// <param name="path"></param>
/// <param name="imgnum"></param>
/// <param name="xorig"></param>
/// <param name="yorig"></param>
/// <returns></returns>
RValue DatabaseLoader::DLInterfaceImpl::GetSprite(string path, int imgnum, int xorig, int yorig)
{
	g_YYTKInterface->PrintInfo("mods/Aurie/" + path + ": Sprite added!");

	return g_YYTKInterface->CallBuiltin("sprite_add", { "mods/Aurie/" + path, imgnum, false, false, xorig, yorig });
}

/// <summary>
/// Spawns a particle with the given sprite at the given X and Y position and returns the instance index.
/// </summary>
/// <param name="x">X position to spawn the particle at.</param>
/// <param name="y">Y position to spawn the particle at.</param>
/// <param name="sprite">The sprite used by the particle, gathered from GetSprite().</param>
/// <returns></returns>
RValue DatabaseLoader::DLInterfaceImpl::SpawnParticle(int x, int y, RValue sprite)
{
	g_YYTKInterface->PrintInfo("Particle spawned 1");

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

	g_YYTKInterface->PrintInfo("Particle spawned 2");

	g_YYTKInterface->CallBuiltin(
		"variable_instance_set",
		{
			part,
			"sprite_index",
			sprite
		}
	);

	g_YYTKInterface->PrintInfo("Particle spawned 3");

	return part;
}

RValue DatabaseLoader::DLInterfaceImpl::SpawnParticle(int x, int y, int xvel, int yvel, RValue sprite)
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

	return part;
}
