#ifndef __NATIVEFUNCTIONS_H
#define __NATIVEFUNCTIONS_H

#include <string>
#include "skse\NiNodes.h"
#include "skse\PapyrusObjectReference.h"
#include "skse\PapyrusQuest.h"
#include "skse/PapyrusGame.h"
#include "skse/PapyrusEvents.h"
#include "skse/GameData.h"
#include "skse/GameRTTI.h"
#include "skse/PapyrusArgs.h"
#include "skse/PapyrusNativeFunctions.h"
#include "skse/PapyrusForm.h"

//SAMPLE - KeepOffsetFromActor((*g_skyrimVM)->GetClassRegistry(), 0, *g_thePlayer, *g_thePlayer, 0, 0, 0, 0, 0, 0, 500, 20);
typedef void(*_KeepOffsetFromActor)(const VMClassRegistry* registry, const UInt32 stackId, Actor* target, const Actor* arTarget,
	const float afOffsetX, const float afOffsetY, const float afOffsetZ, const float afOffsetAngleX, const float afOffsetAngleY, const float afOffsetAngleZ, const float afCatchUpRadius, const float afFollowRadius);
extern const _KeepOffsetFromActor KeepOffsetFromActor;

typedef int(*_GetCurrentStageID)(const VMClassRegistry* registry, const UInt32 stackId, const TESQuest* quest);
extern const _GetCurrentStageID GetCurrentStageID;

typedef int(*_GetOpenState)(const VMClassRegistry* registry, const UInt32 stackId, TESObjectREFR* target);
extern const _GetOpenState GetOpenState;

typedef int(*_Notification)(VMClassRegistry* registry, UInt32 stackId, UInt32* handle, BSFixedString* message);
extern const _Notification Notification;

typedef int(*_ToggleAI)(const VMClassRegistry* registry, const UInt32 stackId, TESObjectREFR* target);
extern const _ToggleAI ToggleAI;

typedef void(*_MoveTo)(const VMClassRegistry* registry, const UInt32 stackId, TESObjectREFR* target, const TESObjectREFR* arTarget,
	const float afXOffset, const float afYOffset, const float afZOffset, const bool abMatchRotation);
extern const _MoveTo MoveTo;

typedef void(*_EnableAI)(const VMClassRegistry* registry, UInt32 stackId, Actor* target, bool abEnabled);
extern const _EnableAI EnableAI;

typedef void(*_SetDontMove)(const VMClassRegistry* registry, UInt32 stackId, Actor* target, bool abEnabled);
extern const _SetDontMove SetDontMove;

typedef void(*_SetAlpha)(const VMClassRegistry* registry, UInt32 stackId, Actor* target, float afTargetAlpha, bool abFade);
extern const _SetAlpha SetAlpha;

typedef void(*_AllowPCDialogue)(const VMClassRegistry* registry, UInt32 stackId, Actor* target, bool abTalk);
extern const _AllowPCDialogue AllowPCDialogue;

typedef bool(*_IsDisabled)(const VMClassRegistry* registry, UInt32 stackId, TESObjectREFR* target);
extern const _IsDisabled IsDisabled;

typedef void(*_ClearKeepOffsetFromActor)(const VMClassRegistry* registry, UInt32 stackId, Actor* target);
extern const _ClearKeepOffsetFromActor ClearKeepOffsetFromActor;

typedef void(*_SetInvulnerable)(const VMClassRegistry* registry, UInt32 stackId, TESNPC* target, bool abInvulnerable);
extern const _SetInvulnerable SetInvulnerable;

typedef void(*_IgnoreFriendlyHits)(const VMClassRegistry* registry, UInt32 stackId, TESObjectREFR* target, bool abIgnore);
extern const _IgnoreFriendlyHits IgnoreFriendlyHits;

typedef void(*_SetRelationshipRank)(const VMClassRegistry* registry, UInt32 stackId, Actor* target, Actor* akOther, int aiRank);
extern const _SetRelationshipRank SetRelationshipRank;

typedef void(*_SetHeadTracking)(const VMClassRegistry* registry, UInt32 stackId, Actor* target, bool abEnable);
extern const _SetHeadTracking SetHeadTracking;

typedef BGSLocation*(*_GetCurrentLocation)(const VMClassRegistry* registry, UInt32 stackId, TESObjectREFR* target);
extern const _GetCurrentLocation GetCurrentLocation;

typedef TESWorldSpace*(*_GetWorldSpace)(const VMClassRegistry* registry, UInt32 stackid, TESObjectREFR* target);
extern const _GetWorldSpace GetWorldSpace;

typedef float(*_GetDistance)(const VMClassRegistry* registry, UInt32 stackId, TESObjectREFR* target, TESObjectREFR* akOther);
extern const _GetDistance GetDistance;

typedef bool(*_IsRunning)(const VMClassRegistry* registry, UInt32 stackId, Actor* target);
extern const _IsRunning IsRunning;

typedef bool(*_IsSprinting)(const VMClassRegistry* registry, UInt32 stackId, Actor* target);
extern const _IsSprinting IsSprinting;

typedef void(*_ForceActorValue)(const VMClassRegistry* registry, UInt32 stackId, Actor* target, BSFixedString* asValueName, float afNewValue);
extern const _ForceActorValue ForceActorValue;

typedef SpellItem*(*_GetEquippedSpell)(const VMClassRegistry* registry, UInt32 stackId, Actor* target, int aiSource);
extern const _GetEquippedSpell GetEquippedSpell;

typedef void(*_Cast)(const VMClassRegistry* registry, UInt32 stackId, SpellItem* target, TESObjectREFR* akSource, TESObjectREFR* akTarget);
extern const _Cast Cast;

typedef int(*_GetEquippedItemType)(const VMClassRegistry* registry, UInt32 stackId, Actor* target, int aiHand);
extern const _GetEquippedItemType GetEquippedItemType;

typedef void(*_DisableNoWait)(const VMClassRegistry* registry, UInt32 stackId, TESObjectREFR* target, bool abFadeOut);
extern const _DisableNoWait DisableNoWait;

typedef void(*_SetValue)(const VMClassRegistry* registry, UInt32 stackId, TESGlobal* target, float afValue);
extern const _SetValue SetValue;

typedef void(*_SetINIBool)(const VMClassRegistry* registry, UInt32 stackId, UInt32* handle, BSFixedString* ini, bool value);
extern const _SetINIBool SetINIBool;

typedef void(*_KillSilent)(const VMClassRegistry* registry, UInt32 stackId, Actor* target, Actor* abKiller);
extern const _KillSilent KillSilent;

typedef void(*_Resurrect)(const VMClassRegistry* registry, UInt32 stackId, Actor* target);
extern const _Resurrect Resurrect;

typedef void(*_ForceActive)(const VMClassRegistry* registry, UInt32 stackId, TESWeather* target, bool abOverride);
extern const _ForceActive ForceActive;

typedef bool(*_IsAllowedToFly)(const VMClassRegistry* registry, UInt32 stackId, Actor* target);
extern const _IsAllowedToFly IsAllowedToFly;

typedef bool(*_IsDead)(const VMClassRegistry* registry, UInt32 stackId, Actor* target);
extern const _IsDead IsDead;

typedef bool(*_IsAlarmed)(const VMClassRegistry* registry, UInt32 stackId, Actor* target);
extern const _IsAlarmed IsAlarmed;

typedef void(*_SendAssaultAlarm)(const VMClassRegistry* registry, UInt32 stackId, Actor* target);
extern const _SendAssaultAlarm SendAssaultAlarm;

typedef void(*_SetAlert)(const VMClassRegistry* registry, UInt32 stackId, Actor* target, bool abAlerted);
extern const _SetAlert SetAlert;

typedef bool(*_IsAlerted)(const VMClassRegistry* registry, const UInt32 stackId, const Actor* target);
extern const _IsAlerted IsAlerted;

typedef bool(*_IsUnconscious)(const VMClassRegistry* registry, const UInt32 stackId, const Actor* target);
extern const _IsUnconscious IsUnconscious;

typedef void(*_SetUnconscious)(const VMClassRegistry* registry, UInt32 stackId, Actor* target, bool abUnconscious);
extern const _SetUnconscious SetUnconscious;

typedef bool(*_IsSneaking)(const VMClassRegistry* registry, const UInt32 stackId, const Actor* target);
extern const _IsSneaking IsSneaking;

typedef float(*_GetActorValue_Native)(const VMClassRegistry* registry, const UInt32 stackId, const Actor* target, const BSFixedString* asValueName);
extern const _GetActorValue_Native GetActorValue_Native;

typedef bool(*_IsInCombat)(const VMClassRegistry* registry, const UInt32 stackId, const Actor* target);
extern const _IsInCombat IsInCombat;

typedef void(*_StartCombat)(const VMClassRegistry* registry, UInt32 stackId, Actor* target, Actor* akTarget);
extern const _StartCombat StartCombat;

typedef void(*_StopCombat)(const VMClassRegistry* registry, UInt32 stackId, Actor* target);
extern const _StopCombat StopCombat;

typedef bool(*_Is3DLoaded)(const VMClassRegistry* registry, const UInt32 stackId, const TESObjectREFR* target);
extern const _Is3DLoaded Is3DLoaded;

typedef void(*_SetMotionType)(const VMClassRegistry* registry, UInt32 stackId, TESObjectREFR* target, int aiMotionType, bool abAllowActivate);
extern const _SetMotionType SetMotionType;

typedef void(*_EnableNoWait)(const VMClassRegistry* registry, UInt32 stackId, TESObjectREFR* target, bool abFadeIn);
extern const _EnableNoWait EnableNoWait;

typedef float(*_GetValue)(const VMClassRegistry* registry, const UInt32 stackId, const TESGlobal* target);
extern const _GetValue GetValue;

typedef TESWeather*(*_GetCurrentWeather)(const VMClassRegistry* registry, const UInt32 stackId, const UInt32* handleId);
extern const _GetCurrentWeather GetCurrentWeather;

typedef bool(*_IsWeaponDrawn)(const VMClassRegistry* registry, const UInt32 stackId, const Actor* target);
extern const _IsWeaponDrawn IsWeaponDrawn;

typedef TESObjectREFR*(*_FindClosestReferenceOfType)(const VMClassRegistry* registry, UInt32 stackId, UInt32* handleId, TESForm* arBaseObject,
	float afX, float afY, float afZ, float afRadius);
extern const _FindClosestReferenceOfType FindClosestReferenceOfType;

typedef bool(*_IsBleedingOut)(const VMClassRegistry* registry, const UInt32 stackId, const Actor* target);
extern const _IsBleedingOut IsBleedingOut;

typedef bool(*_IsBribed)(const VMClassRegistry* registry, const UInt32 stackId, const Actor* target);
extern const _IsBribed IsBribed;

typedef bool(*_IsPlayerTeammate)(const VMClassRegistry* registry, const UInt32 stackId, const Actor* target);
extern const _IsPlayerTeammate IsPlayerTeammate;

typedef TESShout*(*_GetEquippedShout)(const VMClassRegistry* registry, const UInt32 stackId, const Actor* target);
extern const _GetEquippedShout GetEquippedShout;

typedef float(*_GetVoiceRecoveryTime)(const VMClassRegistry* registry, const UInt32 stackId, const Actor* target);
extern const _GetVoiceRecoveryTime GetVoiceRecoveryTime;

typedef bool(*_IsInMenuMode)(const VMClassRegistry* registry, const UInt32 stackId, const UInt32* target);
extern const _IsInMenuMode IsInMenuMode;

typedef float(*_GetActorValuePercentage)(const VMClassRegistry* registry, const UInt32 stackId, const Actor* target, const BSFixedString* asValueName);
extern const _GetActorValuePercentage GetActorValuePercentage;

typedef Actor*(*_GetCombatTarget)(const VMClassRegistry* registry, const UInt32 stackId, const Actor* target);
extern const _GetCombatTarget GetCombatTarget;

typedef double(*_GetArrestedState)(const TESObjectREFR* target);
extern const _GetArrestedState GetArrestedState;

typedef void(*_SetAngle)(const VMClassRegistry* registry, const UInt32 stackId, const TESObjectREFR* target, const float x, const float y, const float z);
extern const _SetAngle SetAngle;

typedef void(*_SetPosition)(const VMClassRegistry* registry, const UInt32 stackId, TESObjectREFR* target, const float x, const float y, const float z);
extern const _SetPosition SetPosition;

typedef void(*_SendAnimationEvent)(VMClassRegistry* registry, UInt32 stackId, UInt32* handle, TESObjectREFR* target, const BSFixedString* asValueName);
extern const _SendAnimationEvent SendAnimationEvent_Native;

typedef void(*_AddToMap)(const VMClassRegistry* registry, UInt32 stackId, TESObjectREFR* target, bool abFastTravel);
extern const _AddToMap AddToMap;

typedef void(*_SetLookAt)(const VMClassRegistry* registry, UInt32 stackId, Actor* source, TESObjectREFR* akTarget, bool abPathingLookAt);
extern const _SetLookAt SetLookAt;

typedef TESObjectWEAP*(*_GetEquippedWeapon)(const VMClassRegistry* registry, const UInt32 stackId, const Actor* source, const bool abLeftHand);
extern const _GetEquippedWeapon GetEquippedWeapon;

typedef void(*_EquipSpell)(const VMClassRegistry* registry, const UInt32 stackId, Actor* source, const SpellItem* akSpell, const int aiSource);
extern const _EquipSpell EquipSpell;

typedef void(*_Fire)(const VMClassRegistry* registry, UInt32 stackId, TESObjectWEAP* target, TESObjectREFR* source, TESAmmo* akAmmo);
extern const _Fire Fire;

typedef void(*_Lock)(const VMClassRegistry* registry, UInt32 stackId, TESObjectREFR* target, bool abLock, bool abAsOwner);
extern const _Lock Lock;

typedef void(*_SetOpen)(const VMClassRegistry* registry, UInt32 stackId, TESObjectREFR* target, bool abOpen);
extern const _SetOpen SetOpen;

typedef TESObjectARMO*(*_GetEquippedShield)(const VMClassRegistry* registry, const UInt32 stackId, const Actor* target);
extern const _GetEquippedShield GetEquippedShield;

typedef float(*_GetAngleX)(const VMClassRegistry* registry, const UInt32 stackId, const TESObjectREFR* target);
extern const _GetAngleX GetAngleX;

typedef float(*_GetAngleY)(const VMClassRegistry* registry, const UInt32 stackId, const TESObjectREFR* target);
extern const _GetAngleY GetAngleY;

typedef float(*_GetAngleZ)(const VMClassRegistry* registry, const UInt32 stackId, const TESObjectREFR* target);
extern const _GetAngleZ GetAngleZ;

typedef bool(*_IsLocked)(const VMClassRegistry* registry, const UInt32 stackId, const TESObjectREFR* target);
extern const _IsLocked IsLocked_Native;

typedef bool(*_Activate)(const VMClassRegistry* registry, const UInt32 stackId, TESObjectREFR* target, const TESObjectREFR* akActivator, const bool abDefaultProcessingOnly);
extern const _Activate Activate;

typedef bool(*_IsBeingRidden)(const VMClassRegistry* registry, const UInt32 stackId, const Actor* target);
extern const _IsBeingRidden IsBeingRidden;

typedef int(*_GetSitState)(const VMClassRegistry* registry, const UInt32 stackId, const Actor* target);
extern const _GetSitState GetSitState;

typedef float(*_GetPositionX)(const VMClassRegistry* registry, const UInt32 stackId, const TESObjectREFR* target);
extern const _GetPositionX GetPositionX;

typedef float(*_GetPositionY)(const VMClassRegistry* registry, const UInt32 stackId, const TESObjectREFR* target);
extern const _GetPositionY GetPositionY;

typedef float(*_GetPositionZ)(const VMClassRegistry* registry, const UInt32 stackId, const TESObjectREFR* target);
extern const _GetPositionZ GetPositionZ;

typedef bool(*_IsOnMount)(const VMClassRegistry* registry, const UInt32 stackId, const Actor* target);
extern const _IsOnMount IsOnMount;

typedef int(*_SetAnimationVariableBool)(const VMClassRegistry* registry, UInt32 stackId, TESObjectREFR* target, BSFixedString* asVariableName, bool abNewValue);
extern const _SetAnimationVariableBool SetAnimationVariableBool;

typedef int(*_SetAnimationVariableInt)(const VMClassRegistry* registry, UInt32 stackId, TESObjectREFR* target, BSFixedString* asVariableName, int abNewValue);
extern const _SetAnimationVariableInt SetAnimationVariableInt;

typedef int(*_SetAnimationVariableFloat)(const VMClassRegistry* registry, UInt32 stackId, TESObjectREFR* target, BSFixedString* asVariableName, float abNewValue);
extern const _SetAnimationVariableFloat SetAnimationVariableFloat;

typedef float(*_GetAnimationVariableFloat)(const VMClassRegistry* registry, UInt32 stackId, TESObjectREFR* target, BSFixedString* asVariableNam);
extern const _GetAnimationVariableFloat GetAnimationVariableFloat;

typedef bool(*_GetAnimationVariableBool)(const VMClassRegistry* registry, UInt32 stackId, TESObjectREFR* target, BSFixedString* asVariableNam);
extern const _GetAnimationVariableBool GetAnimationVariableBool_Native;

typedef bool(*_IsChildLocation)(const VMClassRegistry* registry, const UInt32 stackId, const BGSLocation* parent, const BGSLocation* child);
extern const _IsChildLocation IsChildLocation;

typedef bool(*_IsInterior)(const VMClassRegistry* registry, const UInt32 stackId, const TESObjectCELL* cell);
extern const _IsInterior IsInterior;

typedef void(*_RemoveItem)(const VMClassRegistry* registry, const UInt32 stackId, TESObjectREFR* source, TESForm* akItemToRemove, int aiCount, bool abSilent, TESObjectREFR* akOtherContainer);
extern const _RemoveItem RemoveItem;

typedef void(*_UnequipSpell)(const VMClassRegistry* registry, const UInt32 stackId, Actor* source, SpellItem* akSpellToRemove, int aiSource);
extern const _UnequipSpell UnequipSpell;

typedef void(*_UnequipItemSlot)(const VMClassRegistry* registry, const UInt32 stackId, Actor* source, int equipSlot);
extern const _UnequipItemSlot UnequipItemSlot;

typedef void(*_TranslateTo)(const VMClassRegistry* registry, const UInt32 stackId, TESObjectREFR* source, float afX, float afY, float afZ, float afAngleX, float afAngleY, float afAngleZ, float afSpeed, float afMaxRotationSpeed);
extern const _TranslateTo TranslateTo;

typedef void(*_RemoveAllItems)(const VMClassRegistry* registry, const UInt32 stackId, TESObjectREFR* akTransferTo, bool abKeepOwnership, bool abRemoveQuestItems);
extern const _RemoveAllItems RemoveAllItems;

typedef bool(*_StartQuest)(const VMClassRegistry* registry, const UInt32 stackId, TESQuest* tQuest);
extern const _StartQuest StartQuest;

typedef bool(*_SetCurrentStageID)(const VMClassRegistry* registry, const UInt32 stackId, TESQuest* tQuest, int stage);
extern const _SetCurrentStageID SetCurrentStageID;

typedef bool(*_SetObjectiveDisplayed)(const VMClassRegistry* registry, const UInt32 stackId, TESQuest* tQuest, int aiObjective, bool abDisplayed, bool abForce);
extern const _SetObjectiveDisplayed SetObjectiveDisplayed;

typedef bool(*_SetObjectiveCompleted)(const VMClassRegistry* registry, const UInt32 stackId, TESQuest* tQuest, int aiObjective, bool abCompleted);
extern const _SetObjectiveCompleted SetObjectiveCompleted;

typedef bool (*_IsKeyPressed)(StaticFunctionTag* thisInput, UInt32 dxKeycode);
typedef SInt32 (*_GetMappedKey)(StaticFunctionTag* thisInput, BSFixedString name, UInt32 deviceType);
typedef BSFixedString(*_GetName)(TESForm* thisForm);
typedef BGSTextureSet * (*_GetFaceTextureSet)(TESNPC* thisNPC);
typedef BGSHeadPart* (*_GetNthHeadPart)(TESNPC* thisNPC, UInt32 n);
typedef TESForm* (*_GetWornForm)(Actor* thisActor, UInt32 slot);
typedef BSFixedString (*_GetDisplayName)(TESObjectREFR* object);

struct commandStruct
{
public:
	UInt32 targetId;
	const char* command;

	commandStruct() {}
	commandStruct(UInt32 tId, const char* cmd)
	{
		targetId = tId;
		command = cmd;
	}
};

class NativeFunctions
{
public:
	NativeFunctions() = default;

	static std::vector<TESObjectREFR*> lockedObjectList;

	static bool QIsActive(TESQuest* quest);
	static bool QIsCompleted(TESQuest* quest);
	static bool QIsRunning(TESQuest* quest);
	static bool QIsStarting(TESQuest* quest);
	static bool QIsStopping(TESQuest* quest);
	static bool QIsStopped(TESQuest* quest);
	static UInt16 QGetCurrentStageID(TESQuest* quest);

	static void SendAnimationEvent(TESObjectREFR* target, const char* aEvent);
	static void ExecuteMoveTo(TESObjectREFR* self, TESObjectREFR* target, float offsetX, float offsetY, float offsetZ);
	static void ExecuteCommand(TESObjectREFR* target, const char* aEvent);
	static void ExecuteCommand(const char* aEvent, TESObjectREFR* target);
	static void ExecuteStartCombat(Actor* self, Actor* target);
	static float GetAV(Actor* target, std::string avName);
	static bool IsLocked(TESObjectREFR* target);
	static void n_ExecuteCommand(BSFixedString text, TESObjectREFR* target);

	static std::vector<TESObjectREFR*> GetAllLockedObjects(TESObjectREFR* target);
	static bool IsAIEnabled(TESObjectREFR* sourceObject);
	static UInt32 GetNthSpellId(TESShout* shout, int i);
	static void SetFaceTextureSet(TESObjectREFR* theActor, UInt32 textureId);
	static void ChangeHeadPart(TESObjectREFR* player, UInt32 partId);
	static void SetHeight(TESObjectREFR* theActor, float height);
	static void SetDisplayName(TESObjectREFR* object, const char* value, bool force);
	static TESNPC* GetPlayerBase();
	static TESNPC* GetTemplate(TESNPC* npcTemplate);
	static TESObjectREFR* GetSitTarget(TESObjectREFR* sourceObject);
	static TESObjectREFR * GetEnableParent(TESObjectREFR* object);
	static void RemoveCollision(TESObjectREFR* object);
	static char* GetSitAnimation(TESObjectREFR* sourceObject);
	static void SetHairColor(TESObjectREFR* theActor, UInt32 colorId);
	static void QueueNiNodeUpdate(TESObjectREFR* player);
	static UInt32 GetPackageId(TESPackage* package);
	static int SetupNpcs(TESObjectREFR* target, bool isInInterior);
	static UInt32 GetNearestXMarker();
	static void SetVoice(TESObjectREFR* player, UInt32 voiceId);
	static bool IsWindowOpen(char* menuName);
	static char* GetActorForm(int pos);
	static bool GetAnimationVariableBool(Actor* target, std::string animEvent);
	static float GetAnimationVariableFloat(Actor* target, std::string asVariableNam);

	static std::vector<TESObjectREFR*> GetCloseReferences(TESObjectREFR* player, float fRadius, bool sameLocation);
	static std::vector<TESObjectREFR*> GetCloseLockedRefs(TESObjectREFR* player, float fRadius);
	static std::vector<TESObjectREFR*> GetLocationReferences(BGSLocation* PlayerCurrentLocation);
	static std::vector<TESObjectREFR*> GetCellMembers(TESObjectCELL* cell);

	static _IsKeyPressed IsKeyPressed;
	static _GetMappedKey GetMappedKey;
	static _GetName GetName;
	static _GetFaceTextureSet GetFaceTextureSet;
	static _GetNthHeadPart GetNthHeadPart;
	static _GetWornForm GetWornForm;
	static _GetDisplayName GetDisplayName;
	static std::vector<std::string> actorFormList;
};
#endif