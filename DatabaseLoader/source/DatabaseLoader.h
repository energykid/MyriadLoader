#pragma once

#include "sol/sol.hpp"
#include "YYToolkit/Shared.hpp"
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

		virtual int GetInstanceID(int inst) = 0;

		virtual int GetInt(int inst, string varName) = 0;

		virtual bool GetBool(int inst, string varName) = 0;

		virtual int GetSound(string path) = 0;

		virtual int GetSprite(string path, int imgnum, int xorig, int yorig) = 0;

		virtual RValue SpawnBasicParticle(int x, int y, int sprite) = 0;

		virtual int SpawnParticle(int x, int y, int xvel, int yvel, int sprite) = 0;

		virtual void InvokeWithObjectIndex(
			string Object,
			sol::protected_function Method
		) = 0;
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

		int GetInstanceID(int inst) override final;

		int GetInt(int inst, string varName) override final;

		bool GetBool(int inst, string varName) override final;

		int GetSound(string path) override final;

		int GetSprite(string path, int imgnum, int xorig, int yorig) override final;

		RValue SpawnBasicParticle(int x, int y, int sprite) override final;

		int SpawnParticle(int x, int y, int xvel, int yvel, int sprite) override final;

		void InvokeWithObjectIndex(
			string Object,
			sol::protected_function Method
		) override final;
	};

	inline DLInterfaceImpl g_ModuleInterface;

	inline YYTKInterface* g_YYTKInterface;
}