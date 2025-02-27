#pragma once

#include "YYToolkit/Shared.hpp"
#include "Aurie/shared.hpp"

using namespace std;
using namespace YYTK;
using namespace Aurie;

namespace DatabaseLoader
{
	class Keyword
	{
		vector<string> WeaponNames;
		function<void(CInstance* self, CInstance* other)> Fire{};
		function<void(CInstance* self, CInstance* other)> Step{};

		Keyword(vector<string> names, function<void(CInstance* self, CInstance* other)> fire, function<void(CInstance* self, CInstance* other)> step) :
			WeaponNames(names),
			Fire(fire),
			Step(step) {};
	};

	class ObjectBehavior
	{
	public:
		string objectName;
		function<void(CInstance* self, CInstance* other)> Step{};
		function<void(CInstance* self, CInstance* other)> Create{};

		ObjectBehavior(string name, function<void(CInstance* self, CInstance* other)> step) :
			objectName(name),
			Step(step) {};
	};

	// Interface

	class DLInterface : public AurieInterfaceBase
	{
	public:
		vector<ObjectBehavior> objectBehaviors;
		vector<Keyword> customKeywords;

		AurieStatus Create() override = 0;

		void Destroy() override = 0;

		void QueryVersion(
			OUT short& Major,
			OUT short& Minor,
			OUT short& Patch
		) override = 0;

		virtual void AddObjectBehavior(ObjectBehavior behavior) = 0;

		virtual void AddCustomKeyword(Keyword keyword) = 0;

		virtual RValue GetSound(string path) = 0;

		virtual RValue GetSprite(string path, int imgnum, int xorig, int yorig) = 0;
	};

	class DLInterfaceImpl : public DLInterface
	{
	private:
	public:
		static vector<ObjectBehavior> objectBehaviors;

		AurieStatus Create() override final;

		void Destroy() override final;

		void QueryVersion(
			OUT short& Major,
			OUT short& Minor,
			OUT short& Patch
		) override final;

		void AddObjectBehavior(ObjectBehavior behavior) override final;

		void AddCustomKeyword(Keyword keyword) override final;

		RValue GetSound(string path) override final;

		RValue GetSprite(string path, int imgnum, int xorig, int yorig) override final;
	};

	inline DLInterfaceImpl g_ModuleInterface;

	inline YYTKInterface* g_YYTKInterface;
}