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

	inline sol::protected_function dummyFunction;

	struct ContentData {
	public:
		std::string DataType;
		std::string Name;

		std::function<void(double)> Create = [](double) {};
		std::function<void(double)> Step = [](double) {};
		std::function<void(double)> Destroy = [](double) {};
		std::function<void(double, double)> TakeDamage = [](double, double) {};
		std::function<void(double)> Draw = [](double) {};
		std::function<void()> DrawUI = []() {};

		ContentData(const sol::table& data_table) :
			DataType(data_table.get<string>("DataType")),
			Name(data_table.get<string>("Name")),
			Create(data_table.get_or("Create", dummyFunction)),
			Step(data_table.get_or("Step", dummyFunction)),
			Destroy(data_table.get_or("Destroy", dummyFunction)),
			TakeDamage(data_table.get_or("TakeDamage", dummyFunction)),
			Draw(data_table.get_or("Draw", dummyFunction)),
			DrawUI(data_table.get_or("DrawUI", dummyFunction))
		{}
	};

	static inline vector<ContentData> AllData;
}