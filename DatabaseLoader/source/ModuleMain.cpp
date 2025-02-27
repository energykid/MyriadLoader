#include <YYToolkit/Shared.hpp>
#include "ModuleMain.h"
#include "DatabaseLoader.h"
#include "Keywords.h"

using namespace Aurie;
using namespace DatabaseLoader;

static DLInterface* dl_interface = nullptr;
static YYTK::YYTKInterface* yytk_interface = nullptr;

void ObjectBehaviorRun(FWFrame& Context)
{
	for (size_t var = 0; var < (g_ModuleInterface.objectBehaviors).size(); var++)
	{
		yytk_interface->InvokeWithObject(g_ModuleInterface.objectBehaviors[var].objectName, g_ModuleInterface.objectBehaviors[var].Step);
	}
};

void InitMods()
{

}

EXPORTED AurieStatus ModulePreInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(ModulePath);

	g_ModuleInterface.Create();

	AurieStatus last_status = AURIE_SUCCESS;

	last_status = ObCreateInterface(
		Module,
		&g_ModuleInterface,
		"Database"
	);

	if (!AurieSuccess(last_status))
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(ModulePath);

	g_ModuleInterface.Create();

	AurieStatus last_status = AURIE_SUCCESS;

	last_status = ObGetInterface(
		"YYTK_Main",
		reinterpret_cast<AurieInterfaceBase*&>(yytk_interface)
	);

	DatabaseLoader::g_YYTKInterface = yytk_interface;

	if (!AurieSuccess(last_status))
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	last_status = ObGetInterface(
		"Database",
		reinterpret_cast<AurieInterfaceBase*&>(dl_interface)
	);

	if (!AurieSuccess(last_status))
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	yytk_interface->PrintWarning("Database Loader has initialized!");

	yytk_interface->CreateCallback(
		Module,
		YYTK::EVENT_FRAME,
		ObjectBehaviorRun,
		0);

	InitMods();

	TRoutine original_function = nullptr;
	CScript* script_data = nullptr;
	int script_index = 0;

	// Get the script data
	g_YYTKInterface->GetNamedRoutinePointer(
		"gml_Script_resolve_weapon_base",
		reinterpret_cast<PVOID*>(&script_data)
	);

	// Create the hook
	MmCreateHook(
		g_ArSelfModule,
		"My Hook",
		script_data->m_Functions->m_ScriptFunction,
		Keywords::ResolveWeaponBase,
		reinterpret_cast<PVOID*>(&original_function)
	);


	return AURIE_SUCCESS;
}

//gml_Script___init_global