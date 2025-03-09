#pragma once

#include "DatabaseLoader.h"
#include "Aurie/shared.hpp"
#include "YYToolkit/Shared.hpp"

namespace DatabaseLoader
{
	class DBLua
	{
	public:
		static void InvokeWithObjectIndex(string Object, sol::protected_function func);
		static void InitVar(double inst, string varName, sol::object val);
		static void SetVar(double inst, string varName, sol::object val);
		static sol::object GetVar(double inst, string varName);
		static void InitDouble(double inst, string varName, double val);
		static void InitBool(double inst, string varName, bool val);
		static void InitString(double inst, string varName, string val);
		static void SetDouble(double inst, string varName, double val);
		static void SetBool(double inst, string varName, bool val);
		static void SetString(double inst, string varName, string val);
		static double GetDouble(double inst, string varName);
		static bool GetBool(double inst, string varName);
		static string GetString(double inst, string varName);

		static double GetCustomSound(string path);
		static double GetCustomSprite(string path, double imgnum, double xorig, double yorig);

		static double SpawnParticle(double x, double y, double xvel, double yvel, double sprite);

		static RValue CallBuiltinLua(
			IN const char* FunctionName,
			IN std::vector<RValue> Arguments
		);
		static AurieStatus CallBuiltinExLua(
			OUT RValue& Result,
			IN const char* FunctionName,
			IN CInstance* SelfInstance,
			IN CInstance* OtherInstance,
			IN std::vector<sol::object> Arguments
		);
	};
}