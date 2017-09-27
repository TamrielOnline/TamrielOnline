#pragma once

#include "skse/PapyrusVM.h"
#include "skse/PapyrusNativeFunctions.h"
#include "skse/GameReferences.h"

class IsolatedFunctions
{
public:
	IsolatedFunctions() = default;

	/* This function takes a string reference id, and returns the associated form.
	This is used to make it easier to pass ids between papyrus and c++. As papyrus will routinely improperly cast strings to
	integers if the id is too large for a normal integer. This occurs even when we specify the use of an unisgned integer */
	static TESForm* Retrieve(StaticFunctionTag* base, BSFixedString refId);
	static Actor* RetrieveActor(StaticFunctionTag* base, BSFixedString refId);
	static TESObjectREFR* RetrieveObject(StaticFunctionTag* base, BSFixedString refId);
	static bool RegisterFuncs(VMClassRegistry* registry);
};

