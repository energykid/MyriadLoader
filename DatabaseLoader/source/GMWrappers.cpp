#include "Aurie/shared.hpp"
#include "GMWrappers.h"
#include "DatabaseLoader.h"
#include "YYToolkit/YYTK_Shared.hpp"

using namespace DatabaseLoader;
using namespace Aurie;
using namespace YYTK;

// Gathered from YYTK's experimental branch.

RValue DatabaseLoader::GMWrappers::GetGlobal(string name)
{
	return (g_YYTKInterface->CallBuiltin("variable_global_get", { (string_view)name }));
}

void DatabaseLoader::GMWrappers::SetGlobal(string name, RValue val)
{
	g_YYTKInterface->CallBuiltin("variable_global_set", { (string_view)name, val });
}

RValue DatabaseLoader::GMWrappers::CallGameScript(
	IN std::string_view ScriptName,
	IN const std::vector<RValue>& Arguments
)
{
	AurieStatus last_status = AURIE_SUCCESS;
	CInstance* global_instance = nullptr;

	// Get the global instance, which is the context we'll use
	// for the call to the script.
	last_status = g_YYTKInterface->GetGlobalInstance(
		&global_instance
	);

	if (!AurieSuccess(last_status))
		return RValue();

	// Call the script.
	RValue result;
	last_status = CallGameScriptEx(
		result,
		ScriptName,
		global_instance,
		global_instance,
		Arguments
	);

	return result;
}

AurieStatus DatabaseLoader::GMWrappers::CallGameScriptEx(
	OUT RValue& Result,
	IN std::string_view ScriptName,
	IN CInstance* SelfInstance,
	IN CInstance* OtherInstance,
	IN const std::vector<RValue>& Arguments
)
{
	AurieStatus last_status = AURIE_SUCCESS;

	// We need to be safe here, as the caller might pass
	// in a name of a built-in function.
	//
	// Since there is no OBJECT_TYPE-like mechanism in YYToolkit, 
	// we need to check it via the ID. Scripts have an ID > 100'000.
	int function_index = -1;
	last_status = g_YYTKInterface->GetNamedRoutineIndex(
		ScriptName.data(),
		&function_index
	);

	if (!AurieSuccess(last_status))
		return AURIE_OBJECT_NOT_FOUND;

	// Reject IDs under 100'000 (built-in functions)
	// and IDs that are above or equal to 500'000 (extension functions).
	if (function_index < 100'000 || function_index >= 500'000)
		return AURIE_INVALID_PARAMETER;

	// Get the actual script object
	CScript* script_object = nullptr;
	last_status = g_YYTKInterface->GetScriptData(
		function_index - 100'000,
		script_object
	);

	// If we failed getting it, we return the status code
	// returned by GetScriptData.
	if (!AurieSuccess(last_status))
		return last_status;

	// Is there a function linked to this script?
	if (!script_object->m_Functions)
		return AURIE_ACCESS_DENIED;

	// Is the function linked to this script valid?
	if (!script_object->m_Functions->m_ScriptFunction)
		return AURIE_ACCESS_DENIED;

	// Create a vector with a pre-allocated array
	std::vector<const RValue*> rvalue_pointers;
	rvalue_pointers.reserve(Arguments.size());

	// Push all the arguments back in there
	for (const auto& arg : Arguments)
		rvalue_pointers.push_back(&arg);

	// Call the actual script
	script_object->m_Functions->m_ScriptFunction(
		SelfInstance,
		OtherInstance,
		Result,
		rvalue_pointers.size(),
		const_cast<RValue**>(rvalue_pointers.data())
	);

	return AURIE_SUCCESS;
}