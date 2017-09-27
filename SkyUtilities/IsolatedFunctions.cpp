#include "IsolatedFunctions.h"
#include "skse/GameForms.h"
#include "skse/GameRTTI.h"

TESForm* IsolatedFunctions::Retrieve(StaticFunctionTag* base, BSFixedString refId)
{
	return LookupFormByID(strtoul(refId.data, nullptr, 0));
}

Actor* IsolatedFunctions::RetrieveActor(StaticFunctionTag* base, BSFixedString refId)
{
	Actor* placeHolder = DYNAMIC_CAST(LookupFormByID(strtoul(refId.data, nullptr, 0)), TESForm, Actor);
	return placeHolder;
}

TESObjectREFR* IsolatedFunctions::RetrieveObject(StaticFunctionTag* base, BSFixedString refId)
{
	TESObjectREFR* placeHolder = DYNAMIC_CAST(LookupFormByID(strtoul(refId.data, nullptr, 0)), TESForm, TESObjectREFR);
	return placeHolder;
}

bool IsolatedFunctions::RegisterFuncs(VMClassRegistry* registry)
{
	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, TESForm*, BSFixedString>("Retrieve", "SkyUtilitiesScript", Retrieve, registry));
	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, Actor*, BSFixedString>("RetrieveActor", "SkyUtilitiesScript", RetrieveActor, registry));
	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, TESObjectREFR*, BSFixedString>("RetrieveObject", "SkyUtilitiesScript", RetrieveObject, registry));

	registry->SetFunctionFlags("SkyUtilitiesScript", "Retrieve", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "RetrieveActor", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "RetrieveObject", VMClassRegistry::kFunctionFlag_NoWait);
	return true;
}