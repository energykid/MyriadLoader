#include "DatabaseLoader.h"

using namespace DatabaseLoader;

YYTK::YYTKInterface* DLBehaviorLoader::yytk_interface = nullptr;
vector<ObjectBehavior> DLBehaviorLoader::objectBehaviors = {};

AurieStatus DLInterface::Create()
{
	DLBehaviorLoader::objectBehaviors = {};

	AurieStatus status = ObGetInterface(
		"YYTK_Main",
		reinterpret_cast<AurieInterfaceBase*&>(DLBehaviorLoader::yytk_interface)
	);

	if (!AurieSuccess(status))
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	return AURIE_SUCCESS;
}

void DatabaseLoader::DLInterface::Destroy()
{
}

void DatabaseLoader::DLInterface::QueryVersion(OUT short& Major, OUT short& Minor, OUT short& Patch)
{
}
