#pragma once

#include "YYToolkit/Shared.hpp"
#include "Aurie/shared.hpp"

using namespace std;
using namespace YYTK;
using namespace Aurie;

namespace DatabaseLoader
{
	class ObjectBehavior
	{
	public:
		string objectName;
		function<void(CInstance* self, CInstance* other)> Step{};

		ObjectBehavior(string name, function<void(CInstance* self, CInstance* other)> step) :
			objectName(name),
			Step(step) {};
	};

	class DLBehaviorLoader
	{
	public:
		static YYTK::YYTKInterface* yytk_interface;
		static vector<ObjectBehavior> objectBehaviors;
		static void ObjectBehaviorRun(FWFrame& Context)
		{
			for (size_t var = 0; var < objectBehaviors.size(); var++)
			{
				yytk_interface->InvokeWithObject(objectBehaviors[var].objectName, objectBehaviors[var].Step);
			}
		};
	};

	class DLInterface : public AurieInterfaceBase
	{
		AurieStatus Create() override final;

		void Destroy() override final;

		void QueryVersion(
			OUT short& Major,
			OUT short& Minor,
			OUT short& Patch
		) override final;

	public:
		static void AddObjectBehavior(ObjectBehavior behavior)
		{
			DLBehaviorLoader::objectBehaviors.push_back(behavior);
		};
	};
}