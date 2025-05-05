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
		static void InvokeWithCustomData(string Name, sol::protected_function func);
		static void InitVar(double inst, string varName, sol::object val);
		static void SetVar(double inst, string varName, sol::object val);
		static void InitGlobal(string varName, sol::object val);
		static void SetGlobal(string varName, sol::object val);
		static sol::lua_value GetVar(double inst, string varName);
		static sol::lua_value GetGlobal(string varName);
		static double GetDouble(double inst, string varName);
		static bool GetBool(double inst, string varName);
		static string GetString(double inst, string varName);

		static double GetCustomSound(string path);
		static double GetCustomMusic(string path, string musicName);
		static void UnlockSong(double songName);
		static double GetCustomSprite(string path, double imgnum, double xorig, double yorig, double frames);

		static void DoSound(double soundType, double x);
		static void DoSoundExt(double soundType, double pitch, double gain, double x);

		static void DoMusic(double soundType);
		static void ShowBossMessage(double x, double y, string str);

		static double GetAsset(string name);
		static sol::lua_value CallFunction(string name, sol::table args);
		static void CallGameFunction(string name, sol::table args);

		static double SpawnParticle(double x, double y, double xvel, double yvel, double sprite);
		static double SpawnEnemy(double x, double y, string name);
		static double SpawnBossIntro(double x, double y, string name);
		static void KillBoss();
		static void ClearBullets(double x, double y);
		static void AddScreenshake(double amount);
		static double SpawnProjectile(double x, double y, double xvel, double yvel, sol::object bullet);
		static double SpawnLaser(double x, double y, double angle, double lifetime, sol::object laser, sol::object laserstart, sol::object lasereend);

		static void AddCallbackTo(double id, sol::protected_function function);

		static void DrawSetDepth(double dep);
		static void DrawSetColor(double col);

		static double CreateColor(double r, double g, double b);

		static void DrawRect(double x1, double y1, double x2, double y2, bool outline);
		static void DrawSprite(double x, double y, double spriteID, double frameNumber);
		static void DrawString(double x, double y, string text);
		static void DrawStringColor(double x, double y, string text, double color);
		static void DrawSpriteExt(double x, double y, double spriteID, double frameNumber, double rotation, double xScale, double yScale, double color, double alpha);

		static void DrawPrimitiveBeginTexture(double spriteID, double frame);
		static void DrawPrimitiveBeginSolid();
		static void DrawVertexColor(double x, double y, double color, double alpha);
		static void DrawVertexTexture(double x, double y, double texcoordx, double texcoordy);
		static void DrawVertexEnd();

		static sol::table DirectionTo(double x1, double y1, double x2, double y2);

		static sol::table EnemyData(string name);
		static sol::table ProjectileData(string name);
		static sol::table GlobalData();
		static sol::table PlayerData();

		static void AddBestiaryEntry(string name, double race, double mugshot, double sprite, double hp, double score);

		static void AddRoomsTo(string sourceName, string destinationName);

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