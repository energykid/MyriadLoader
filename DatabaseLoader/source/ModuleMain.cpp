#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <YYToolkit/Shared.hpp>
#include "ModuleMain.h"
#include "DatabaseLoader.h"

using namespace Aurie;
using namespace DatabaseLoader;

static DLInterface* dl_interface = nullptr;
static YYTK::YYTKInterface* yytk_interface = nullptr;

static sol::state dl_lua;

void ObjectBehaviorRun(FWFrame& Context)
{
	static sol::table allBehaviors = dl_lua["object_behaviors"];

	for (int var = 0; var < allBehaviors.size() + 1; var++)
	{
		sol::lua_table tbl = allBehaviors[var];

		dl_interface->InvokeWithObjectIndex(tbl["objectName"], tbl["stepFunc"]);
	}
};

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

	yytk_interface->PrintInfo("Database Loader has initialized!");

	TRoutine original_function = nullptr;
	CScript* script_data = nullptr;
	int script_index = 0;

	dl_lua.open_libraries(sol::lib::base, sol::lib::package);

	dl_lua.script(R"(
		object_behaviors = {}

		function new_object_behavior(object, callback)
			local behavior = {}
			behavior.objectName = object
			behavior.stepFunc = callback
			object_behaviors[#object_behaviors] = behavior
		end
	)");

	dl_lua["debug_out"] = [](string text) {
		yytk_interface->PrintInfo(text);
		};

	dl_lua["init_variable"] = [](int inst, string varName, sol::object value) {
		dl_interface->InitializeVariable(inst, varName, value);
		};
	dl_lua["set_variable"] = [](int inst, string varName, sol::object value) {
		dl_interface->SetVariable(inst, varName, value);
		};
	dl_lua["spawn_particle"] = [](int x, int y, int xvel, int yvel, int sprite) {
		dl_interface->SpawnParticle(x, y, xvel, yvel, sprite);
		};
	dl_lua["get_int"] = [](int inst, string varName) {
		return dl_interface->GetInt(inst, varName);
		};
	dl_lua["get_bool"] = [](int inst, string varName) {
		return dl_interface->GetBool(inst, varName);
		};
	dl_lua["get_sprite"] = [](string path, int imgnum, int xorig, int yorig) {
		return dl_interface->GetSprite(path, imgnum, xorig, yorig);
		};
	dl_lua["get_sound"] = [](string path) {
		return dl_interface->GetSound(path);
		};

	dl_lua.script(R"(
		particleSprite = get_sprite('GhostParticle.png', 4, 4, 6)

		new_object_behavior('obj_ghost', function(obj)

			init_variable(obj, 'ghostTimer', 0)
			set_variable(obj, 'ghostTimer', get_int(obj, 'ghostTimer') + 1)
			
			debug_out(tostring(get_int(obj, 'ghostTimer')))

			if (math.fmod(get_int(obj, 'ghostTimer'), 5) == 0) then 
				spawn_particle(get_int(obj, 'x'), get_int(obj, 'y'), 0, -1, particleSprite) 
			end
		end)

		debug_out(object_behaviors[0].objectName)
	)");

	dl_lua["object_behaviors"][0]["stepFunc"].call(0);

	yytk_interface->CreateCallback(
		Module,
		YYTK::EVENT_FRAME,
		ObjectBehaviorRun,
		0);

	// Keyword initialization

	/*

	g_YYTKInterface->GetNamedRoutinePointer(
		"gml_Script_resolve_weapon_base",
		reinterpret_cast<PVOID*>(&script_data)
	);
	MmCreateHook(
		g_ArSelfModule,
		"Resolve Weapon Keywords",
		script_data->m_Functions->m_ScriptFunction,
		Keywords::ResolveWeaponBase,
		reinterpret_cast<PVOID*>(&original_function)
	);

	*/
	
	return AURIE_SUCCESS;
}