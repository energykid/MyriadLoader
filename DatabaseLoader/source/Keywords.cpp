#include "Keywords.h"
#include "DatabaseLoader.h"

/*
RValue& DatabaseLoader::Keywords::ResolveWeaponBase(
	OUT RValue* Result, 
	IN CInstance* Self, 
	IN CInstance* Other, 
	IN int ArgumentCount, 
	IN RValue* Arguments)
{
	for (size_t i = 0; i < g_ModuleInterface.customKeywords.size(); i++)
	{
		bool shouldAddKeyword = false;

		Keyword keyword = g_ModuleInterface.customKeywords[i];

		for (size_t j = 0; j < keyword.WeaponTypes.size(); j++)
		{
			if (g_YYTKInterface->CallBuiltin(
				"array_contains",
				{
					g_YYTKInterface->CallBuiltin(
						"variable_instance_get",
						{
							Self, "keywords"
						}),
					keyword.WeaponTypes[j]
				}
			).AsBool())
			{
				g_YYTKInterface->CallBuiltin(
					"ds_list_add",
					{
						g_YYTKInterface->CallBuiltin(
						"variable_instance_get",
						{
							Self, "keywords"
							}),
						keyword.Name
					}
				);

				g_YYTKInterface->PrintInfo("Keyword " + keyword.Name + " initialized in weapon type " + to_string(g_YYTKInterface->CallBuiltin(
					"variable_instance_get",
					{
						Self, "name"
					}).AsReal()));
			}
		}
	}

	return *Result;
}
*/