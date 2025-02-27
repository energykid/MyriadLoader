#include "Keywords.h"
#include "DatabaseLoader.h"

void DatabaseLoader::Keywords::ResolveWeaponBase(
	OUT RValue* Result, 
	IN CInstance* Self, 
	IN CInstance* Other, 
	IN int ArgumentCount, 
	IN RValue* Arguments)
{
	g_YYTKInterface->CallBuiltin(
		"ds_list_add",
		{
			g_YYTKInterface->CallBuiltin(
			"variable_instance_get",
			{
				Self, "keywords"
				}),
			
		}
		);
}
