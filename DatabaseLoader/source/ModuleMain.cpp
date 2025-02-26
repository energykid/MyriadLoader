#include <YYToolkit/Shared.hpp>
#include "ModuleMain.h"
#include "DatabaseLoader.h"

using namespace Aurie;
using namespace DatabaseLoader;


EXPORTED AurieStatus ModulePreInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(ModulePath);

	YYTK::YYTKInterface* yytk_interface = nullptr;
	DLInterface* dl_interface = nullptr;

	AurieStatus last_status = AURIE_SUCCESS;

	last_status = ObGetInterface(
		"YYTK_Main",
		reinterpret_cast<AurieInterfaceBase*&>(yytk_interface)
	);

	if (!AurieSuccess(last_status))
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	yytk_interface->PrintWarning("Database Loader has initialized!");

	yytk_interface->CreateCallback(
		Module,
		YYTK::EVENT_FRAME,
		DLBehaviorLoader::ObjectBehaviorRun,
		0
	);

	last_status = ObCreateInterface(
		Module,
		dl_interface,
		"Database");

	if (AurieSuccess(last_status))
		yytk_interface->PrintWarning("DLInterface initialized successfully!");

	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	return AURIE_SUCCESS;
}