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

void DatabaseLoader::DLInterfaceImpl::InitializeVariable(int inst, string varName, sol::object val)
{

}

void DatabaseLoader::DLInterfaceImpl::SetVariable(int inst, string varName, sol::object val)
{

}

double DatabaseLoader::DLInterfaceImpl::GetInstanceID(double inst)
{
	return g_YYTKInterface->CallBuiltin("variable_instance_get", {
	inst,
	"index" }).AsReal();
}

double DatabaseLoader::DLInterfaceImpl::GetNumber(double inst, string varName)
{
	return g_YYTKInterface->CallBuiltin("variable_instance_get", {
	inst,
	varName }).AsReal();
}

bool DatabaseLoader::DLInterfaceImpl::GetBool(double inst, string varName)
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
double DatabaseLoader::DLInterfaceImpl::GetSound(string path)
{
	return g_YYTKInterface->CallBuiltin("audio_create_stream", { "DatabaseLoader/" + path }).AsReal();
}

/// <summary>
/// Gets a sprite from the given path, with the given number of images ad the given origin, then returns the sprite asset.
/// </summary>
/// <param name="path"></param>
/// <param name="imgnum"></param>
/// <param name="xorig"></param>
/// <param name="yorig"></param>
/// <returns></returns>
double DatabaseLoader::DLInterfaceImpl::GetSprite(string path, int imgnum, int xorig, int yorig)
{
	return g_YYTKInterface->CallBuiltin("sprite_add", { "DatabaseLoader/" + path, imgnum, false, false, xorig, yorig }).AsReal();
}

RValue DatabaseLoader::DLInterfaceImpl::SpawnBasicParticle(int x, int y, double sprite)
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

double DatabaseLoader::DLInterfaceImpl::SpawnParticle(double x, double y, double xvel, double yvel, double sprite)
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