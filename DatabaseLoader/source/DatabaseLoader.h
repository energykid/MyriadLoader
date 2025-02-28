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
	public:
		vector<int> WeaponTypes;
		string Name;
		function<void(CInstance* self, CInstance* other)> Fire{};
		function<void(CInstance* self, CInstance* other)> Step{};

		/// <summary>
		/// A custom keyword for use by DBLoader.
		/// </summary>
		/// <param name="name">Internal name of the keyword.</param>
		/// <param name="weaponTypes">List of weapon IDs this keyword applies to.</param>
		/// <param name="fire">Script that runs when firing a projectile from a weapon using this keyword.</param>
		/// <param name="step">Script that runs every frame while holding a weapon using this keyword.</param>
		Keyword(string name, vector<int> weaponTypes, function<void(CInstance* self, CInstance* other)> fire, function<void(CInstance* self, CInstance* other)> step) :
			Name(name),
			WeaponTypes(weaponTypes),
			Fire(fire),
			Step(step) {};
	};

	class ObjectBehavior
	{
	public:
		string objectName;
		function<void(CInstance* self, CInstance* other)> Step{};
		function<void(CInstance* self, CInstance* other)> Create{};

		/// <summary>
		/// A custom per-object behavior.
		/// </summary>
		/// <param name="name">The name of the object that uses this behavior.</param>
		/// <param name="step">The function to run every step from the named object.</param>
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

		virtual RValue SpawnParticle(int x, int y, RValue sprite) = 0;

		virtual RValue SpawnParticle(int x, int y, int xvel, int yvel, RValue sprite) = 0;
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

		RValue SpawnParticle(int x, int y, RValue sprite) override final;

		RValue SpawnParticle(int x, int y, int xvel, int yvel, RValue sprite) override final;
	};

	inline DLInterfaceImpl g_ModuleInterface;

	inline YYTKInterface* g_YYTKInterface;
}