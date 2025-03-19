#pragma once

#include "sol/sol.hpp"
#include "YYToolkit/YYTK_Shared.hpp"
#include "Aurie/shared.hpp"

using namespace std;
using namespace YYTK;
using namespace Aurie;

namespace DatabaseLoader
{
	// Interface

	class DLInterface : public AurieInterfaceBase
	{
	public:
		vector<sol::table> objectBehaviors;

		AurieStatus Create() override = 0;

		void Destroy() override = 0;

		void QueryVersion(
			OUT short& Major,
			OUT short& Minor,
			OUT short& Patch
		) override = 0;

		virtual void AddObjectBehavior(sol::table behavior) = 0;

		virtual void InitializeVariable(int inst, string varName, sol::object value) = 0;

		virtual void SetVariable(int inst, string varName, sol::object value) = 0;

		virtual double GetInstanceID(double inst) = 0;

		virtual double GetNumber(double inst, string varName) = 0;

		virtual bool GetBool(double inst, string varName) = 0;

		virtual double GetSound(string path) = 0;

		virtual double GetSprite(string path, int imgnum, int xorig, int yorig) = 0;

		virtual RValue SpawnBasicParticle(int x, int y, double sprite) = 0;

		virtual double SpawnParticle(double x, double y, double xvel, double yvel, double sprite) = 0;
	};

	class DLInterfaceImpl : public DLInterface
	{
	private:
	public:
		static vector<sol::table> objectBehaviors;

		AurieStatus Create() override final;

		void Destroy() override final;

		void QueryVersion(
			OUT short& Major,
			OUT short& Minor,
			OUT short& Patch
		) override final;

		void AddObjectBehavior(sol::table behavior) override final;

		void InitializeVariable(int inst, string varName, sol::object value) override final;

		void SetVariable(int inst, string varName, sol::object value) override final;

		double GetInstanceID(double inst) override final;

		double GetNumber(double inst, string varName) override final;

		bool GetBool(double inst, string varName) override final;

		double GetSound(string path) override final;

		double GetSprite(string path, int imgnum, int xorig, int yorig) override final;

		RValue SpawnBasicParticle(int x, int y, double sprite) override final;

		double SpawnParticle(double x, double y, double xvel, double yvel, double sprite) override final;
	};
	inline AurieModule* g_LocalModule;

	inline DLInterfaceImpl g_ModuleInterface;

	inline YYTKInterface* g_YYTKInterface;

	inline sol::state dl_lua;
	inline vector<sol::state> modState;

	inline vector<string> customEnemyNames;
	inline vector<string> customMinibossNames;
	inline vector<string> customBossNames;

	struct RoomFileReplacement {
		string sourceName;
		string destinationName;
		string backupName;

		RoomFileReplacement(string name, string name2, string name3) : sourceName(name), destinationName(name2), backupName(name3) {}
	};

	inline vector<RoomFileReplacement> roomFiles;

	class MusicType {
	public:
		MusicType(double num, const string name) : ID(num), Name(name) {}
		double ID;
		string Name;
	};
}