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

void DatabaseLoader::DLInterfaceImpl::AddCustomKeyword(Keyword keyword)
{
	customKeywords.push_back(keyword);
}

RValue DatabaseLoader::DLInterfaceImpl::GetSound(string path)
{
	return g_YYTKInterface->CallBuiltin("audio_create_stream", { "mods/Aurie/" + path});
}

RValue DatabaseLoader::DLInterfaceImpl::GetSprite(string path, int imgnum, int xorig, int yorig)
{
	return g_YYTKInterface->CallBuiltin("audio_create_stream", { "mods/Aurie/" + path, imgnum, false, false, xorig, yorig });
}
