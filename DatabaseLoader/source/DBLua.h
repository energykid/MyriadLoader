#pragma once

#include "DatabaseLoader.h"
#include "Aurie/shared.hpp"
#include "YYToolkit/YYTK_Shared.hpp"

namespace DatabaseLoader
{
	class DBLua
	{
		static void InitDouble(double inst, string varName, double val);
		static void InitBool(double inst, string varName, bool val);
		static void InitString(double inst, string varName, string val);
		static void InitArray(double inst, string varName, sol::table vals);
		static void SetDouble(double inst, string varName, double val);
		static void SetBool(double inst, string varName, bool val);
		static void SetString(double inst, string varName, string val);
		static void SetArray(double inst, string varName, sol::table vals);
	public:

		static void InvokeWithObjectIndex(string Object, sol::protected_function func);
		static void InitVar(double inst, string varName, sol::object val);
		static void SetVar(double inst, string varName, sol::object val);
		static double GetDouble(double inst, string varName);
		static bool GetBool(double inst, string varName);
		static string GetString(double inst, string varName);

		static double GetCustomSound(string path);
		static double GetCustomMusic(string path, string musicName);
		static void UnlockSong(double songName);
		static double GetCustomSprite(string path, double imgnum, double xorig, double yorig);

		static void DoSound(double soundType, double x);
		static void DoSoundExt(double soundType, double pitch, double gain, double x);

		static double GetAsset(string name);
		static sol::lua_value CallFunction(string name, sol::table args);
		static void CallGameFunction(string name, sol::table args);

		static double SpawnParticle(double x, double y, double xvel, double yvel, double sprite);

		static void DrawSprite(double x, double y, double spriteID, double frameNumber);
		static void DrawSpriteExt(double x, double y, double spriteID, double frameNumber, double rotation, double xScale, double yScale, double color, double alpha);

		static sol::table EnemyData(string name);
		static sol::table ProjectileData(string name);

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