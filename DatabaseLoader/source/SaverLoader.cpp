#include <nlohmann/json.hpp>
#include "Files.h"
#include "SaverLoader.h"
#include "ModuleMain.h"
#include "fstream"
#include "DatabaseLoader.h"

using namespace DatabaseLoader;
using namespace nlohmann;


void DatabaseLoader::SaverLoader::SaveVariable(string name, sol::object var)
{
	/*switch (var.get_type())
	{
	case sol::lua_type_of_v<double>:
		data_from_file[name] = var.as<double>();
		break;
	case sol::lua_type_of_v<bool>:
		data_from_file[name] = var.as<bool>();
		break;
	case sol::lua_type_of_v<string>:
		data_from_file[name] = var.as<string>();
		break;
	}*/
	string str = Files::GetModSavesDirectory() + "/" + modState[currentState].get<string>("modName") + ".json";
	
	g_YYTKInterface->PrintInfo(str);
	/*
	std::ofstream file_out(Files::GetModSavesDirectory() + "/" + modState[currentState].get<string>("modName") + ".json");
	file_out << std::setw(4) << data_from_file << std::endl;*/
}

sol::object DatabaseLoader::SaverLoader::LoadVariable(string name)
{
	std::ifstream file_in(Files::GetModSavesDirectory() + "/" + modState[currentState].get<string>("modName") + ".json");
	json data_from_file;
	file_in >> data_from_file;
	json var = data_from_file[name];
	return sol::make_object(modState[currentState], var);
}