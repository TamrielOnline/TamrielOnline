#include "NativeFunctions.h"
#include "skse/PapyrusInput.h"
#include "skse/PapyrusShout.h"
#include "skse/PapyrusActorBase.h"
#include "skse/PapyrusActor.h"
#include "skse/GameBSExtraData.h"
#include "skse/GameReferences.h"
#include "skse/GameExtraData.h"
#include "skse/PapyrusWornObject.h"
#include "Utilities.h"
#include "Commands.h"

const _KeepOffsetFromActor KeepOffsetFromActor = (_KeepOffsetFromActor)0x8DA850;
const _GetCurrentStageID GetCurrentStageID = (_GetCurrentStageID)0x009152D0;
const _GetOpenState GetOpenState = (_GetOpenState)0x009027A0;
const _SetInvulnerable SetInvulnerable = (_SetInvulnerable)0x008FFC20;
const _Notification Notification = (_Notification)0x008EE550;
const _ToggleAI ToggleAI = (_ToggleAI)0x008EEF10;
const _MoveTo MoveTo = (_MoveTo)0x00908B60;
const _EnableAI EnableAI = (_EnableAI)0x008DAEE0;
const _SetAlpha SetAlpha = (_SetAlpha)0x008DBB80;
const _AllowPCDialogue AllowPCDialogue = (_AllowPCDialogue)0x008DB3C0;
const _IsDisabled IsDisabled = (_IsDisabled)0x00902DC0;
const _ClearKeepOffsetFromActor ClearKeepOffsetFromActor = (_ClearKeepOffsetFromActor)0x008DAE90;
const _IgnoreFriendlyHits IgnoreFriendlyHits = (_IgnoreFriendlyHits)0x00902930;
const _SetRelationshipRank SetRelationshipRank = (_SetRelationshipRank)0x008DC7F0;
const _SetHeadTracking SetHeadTracking = (_SetHeadTracking)0x008DACB0;
const _GetCurrentLocation GetCurrentLocation = (_GetCurrentLocation)0x00902D20;
const _GetWorldSpace GetWorldSpace = (_GetWorldSpace)0x00902D90;
const _GetDistance GetDistance = (_GetDistance)0x009026E0;
const _IsRunning IsRunning = (_IsRunning)0x008DB070;
const _IsSprinting IsSprinting = (_IsSprinting)0x008DB080;
const _ForceActorValue ForceActorValue = (_ForceActorValue)0x008DD5A0;
const _GetEquippedSpell GetEquippedSpell = (_GetEquippedSpell)0x008DB500;
const _Cast Cast = (_Cast)0x008F90B0;
const _GetEquippedItemType GetEquippedItemType = (_GetEquippedItemType)0x008DB550;
const _DisableNoWait DisableNoWait = (_DisableNoWait)0x009087B0;
const _SetValue SetValue = (_SetValue)0x008FE2B0;
const _SetINIBool SetINIBool = (_SetINIBool)0x00919460;
const _KillSilent KillSilent = (_KillSilent)0x008DA9E0;
const _Resurrect Resurrect = (_Resurrect)0x008DD990;
const _ForceActive ForceActive = (_ForceActive)0x00917FE0;
const _IsAllowedToFly IsAllowedToFly = (_IsAllowedToFly)0x008DA800;
const _IsDead IsDead = (_IsDead)0x008DA830;
const _IsAlarmed IsAlarmed = (_IsAlarmed)0x008DA7D0;
const _SendAssaultAlarm SendAssaultAlarm = (_SendAssaultAlarm)0x008DB9E0;
const _SetAlert SetAlert = (_SetAlert)0x008DB0D0;
const _IsAlerted IsAlerted = (_IsAlerted)0x008DAF90;
const _IsUnconscious IsUnconscious = (_IsUnconscious)0x008DC040;
const _SetUnconscious SetUnconscious = (_SetUnconscious)0x008DBD90;
const _IsSneaking IsSneaking = (_IsSneaking)0x008DDFD0;
const _GetActorValue_Native GetActorValue_Native = (_GetActorValue_Native)0x008DB430;
const _IsInCombat IsInCombat = (_IsInCombat)0x008DB020;
const _StartCombat StartCombat = (_StartCombat)0x008DDC20;
const _StopCombat StopCombat = (_StopCombat)0x008DBF40;
const _Is3DLoaded Is3DLoaded = (_Is3DLoaded)0x00902950;
const _SetMotionType SetMotionType = (_SetMotionType)0x00909C90;
const _EnableNoWait EnableNoWait = (_EnableNoWait)0x00908AE0;
const _GetValue GetValue = (_GetValue)0x008FE2A0;
const _GetCurrentWeather GetCurrentWeather = (_GetCurrentWeather)0x00918090;
const _IsWeaponDrawn IsWeaponDrawn = (_IsWeaponDrawn)0x008DC060;
const _FindClosestReferenceOfType FindClosestReferenceOfType = (_FindClosestReferenceOfType)0x008F4530;
const _IsBleedingOut IsBleedingOut = (_IsBleedingOut)0x008DA810;
const _IsBribed IsBribed = (_IsBribed)0x008DAFB0;
const _IsPlayerTeammate IsPlayerTeammate = (_IsPlayerTeammate)0x008DC030;
const _GetEquippedShout GetEquippedShout = (_GetEquippedShout)0x008DAF10;
const _GetVoiceRecoveryTime GetVoiceRecoveryTime = (_GetVoiceRecoveryTime)0x008DAF80;
const _IsInMenuMode IsInMenuMode = (_IsInMenuMode)0x00918D90;
const _GetActorValuePercentage GetActorValuePercentage = (_GetActorValuePercentage)0x008DA6C0;
const _GetCombatTarget GetCombatTarget = (_GetCombatTarget)0x008E1010;
const _GetArrestedState GetArrestedState = (_GetArrestedState)0x00452300;
const _SetAngle SetAngle = (_SetAngle)0x00909880;
const _SetPosition SetPosition = (_SetPosition)0x00909E40;
const _SendAnimationEvent SendAnimationEvent_Native = (_SendAnimationEvent)0x008EE630;
const _AddToMap AddToMap = (_AddToMap)0x009024E0;
const _SetLookAt SetLookAt = (_SetLookAt)0x008DBC10;
const _GetEquippedWeapon GetEquippedWeapon = (_GetEquippedWeapon)0x008DAF20;
const _EquipSpell EquipSpell = (_EquipSpell)0x008DD4E0;
const _Fire Fire = (_Fire)0x009149E0;
const _Lock Lock = (_Lock)0x00902A00;
const _SetOpen SetOpen = (_SetOpen)0x009039B0;
const _GetEquippedShield GetEquippedShield = (_GetEquippedShield)0x008DAF30;
const _GetAngleX GetAngleX = (_GetAngleX)0x00903330;
const _GetAngleY GetAngleY = (_GetAngleY)0x00903350;
const _GetAngleZ GetAngleZ = (_GetAngleZ)0x00903370;
const _IsLocked IsLocked_Native = (_IsLocked)0x009036F0;
const _Activate Activate = (_Activate)0x00902460;
const _IsBeingRidden IsBeingRidden = (_IsBeingRidden)0x008DCAF0;
const _GetSitState GetSitState = (_GetSitState)0x008DA8E0;
const _GetPositionX GetPositionX = (_GetPositionX)0x00903670;
const _GetPositionY GetPositionY = (_GetPositionY)0x00903680;
const _GetPositionZ GetPositionZ = (_GetPositionZ)0x00903690;
const _IsOnMount IsOnMount = (_IsOnMount)0x008DCB40;
const _SetAnimationVariableBool SetAnimationVariableBool = (_SetAnimationVariableBool)0x00902AE0;
const _SetAnimationVariableInt SetAnimationVariableInt = (_SetAnimationVariableInt)0x00902B70;
const _SetAnimationVariableFloat SetAnimationVariableFloat = (_SetAnimationVariableFloat)0x00902C00;
const _GetAnimationVariableFloat GetAnimationVariableFloat_Native = (_GetAnimationVariableFloat)0x009034D0;
const _GetAnimationVariableBool GetAnimationVariableBool_Native = (_GetAnimationVariableBool)0x00903390;
const _SetDontMove SetDontMove = (_SetDontMove)0x008DAD20;
const _IsChildLocation IsChildLocation = (_IsChildLocation)0x008E9FC0;
const _IsInterior IsInterior = (_IsInterior)0x00900F90;
const _RemoveItem RemoveItem = (_RemoveItem)0x00909650;
const _UnequipSpell UnequipSpell = (_UnequipSpell)0x008DD540;
const _UnequipItemSlot UnequipItemSlot = (_UnequipItemSlot)0x008DAD70;
const _TranslateTo TranslateTo = (_TranslateTo)0x00911F10;
const _RemoveAllItems RemoveAllItems = (_RemoveAllItems)0x00903900;
const _StartQuest StartQuest = (_StartQuest)0x00915E00;
const _SetCurrentStageID SetCurrentStageID = (_SetCurrentStageID)0x00915CC0;
const _SetObjectiveDisplayed SetObjectiveDisplayed = (_SetObjectiveDisplayed)0x009156A0;
const _SetObjectiveCompleted SetObjectiveCompleted = (_SetObjectiveCompleted)0x00915590;

std::vector<TESObjectREFR*> lockedObjectList;

// 08 - DNAM: general data
struct Data
{
	UInt32	unk00;			// 00 - unknown
	struct Flags {
		bool running : 1;					// 00 -   1
		bool completed : 1;					// 01 -   2
		bool : 1;							// 02 -   4
		bool allowRepeatStages : 1;			// 03 -   8
		bool startGameEnabled : 1;			// 04 -  10
		bool : 1;							// 05 -  20
		bool : 1;							// 06 -  40
		bool stopping : 1;					// 07 -  80
		bool runOnce : 1;					// 08 - 100
		bool excludeFromDialogueExport : 1;	// 09 - 200
		bool warnOnAliasFillFailure : 1;	// 10 - 400
		bool active : 1;					// 11 - 800
		bool : 1;							// 12
		bool : 1;							// 13
		bool : 1;							// 14
		bool : 1;							// 15
	} flags;				// 04
	UInt8	priority;		// 06
	UInt8	type;			// 07
};

bool NativeFunctions::QIsActive(TESQuest* quest) { return ((Data*)&quest->unk07C)->flags.active; }
bool NativeFunctions::QIsCompleted(TESQuest* quest) { return ((Data*)&quest->unk07C)->flags.completed; }
bool NativeFunctions::QIsRunning(TESQuest* quest) { return ((Data*)&quest->unk07C)->flags.running && !((Data*)&quest->unk07C)->flags.stopping && quest->unk148 == 0; }
bool NativeFunctions::QIsStarting(TESQuest* quest) { return ((Data*)&quest->unk07C)->flags.running && !((Data*)&quest->unk07C)->flags.stopping && quest->unk148 != 0; }
bool NativeFunctions::QIsStopping(TESQuest* quest) { return !((Data*)&quest->unk07C)->flags.running && ((Data*)&quest->unk07C)->flags.stopping; }
bool NativeFunctions::QIsStopped(TESQuest* quest) { return !((Data*)&quest->unk07C)->flags.running && !((Data*)&quest->unk07C)->flags.stopping; }
UInt16 NativeFunctions::QGetCurrentStageID(TESQuest* quest) { return quest->unk138; }

bool NativeFunctions::GetAnimationVariableBool(Actor* target, std::string asVariableNam)
{
	if (!target || !(*g_skyrimVM)->GetClassRegistry())
		return false;

	return GetAnimationVariableBool_Native((*g_skyrimVM)->GetClassRegistry(), 0, target, &BSFixedString(asVariableNam.c_str()));
}

float NativeFunctions::GetAnimationVariableFloat(Actor* target, std::string asVariableNam)
{
	if (!target || !(*g_skyrimVM)->GetClassRegistry())
		return false;

	return GetAnimationVariableFloat_Native((*g_skyrimVM)->GetClassRegistry(), 0, target, &BSFixedString(asVariableNam.c_str()));
}

void NativeFunctions::ExecuteCommand(TESObjectREFR* target, const char* aEvent)
{
	i_ExecuteCommand(aEvent, target);
}

void NativeFunctions::ExecuteCommand(const char* aEvent, TESObjectREFR* target)
{
	i_ExecuteCommand(aEvent, target);
}

void NativeFunctions::ExecuteMoveTo(TESObjectREFR* self, TESObjectREFR* target, float offsetX, float offsetY, float offsetZ)
{
	if (!self || !target)
		return;

	ExecuteCommand(("MoveTo " + Utilities::GetFormIDString(target->formID) +
		" " + std::to_string(offsetX) + " " + std::to_string(offsetY) + " " + std::to_string(offsetZ)).c_str(), self);
}

void NativeFunctions::SendAnimationEvent(TESObjectREFR* target, const char* aEvent)
{
	if (!target)
		return;

	ExecuteCommand(((std::string)"sae " + aEvent).c_str(), target);
}

float NativeFunctions::GetAV(Actor* target, std::string avName)
{
	if (!target)
		return NULL;

	return GetActorValue_Native((*g_skyrimVM)->GetClassRegistry(), 0, target, &BSFixedString(avName.c_str()));
}

bool NativeFunctions::IsLocked(TESObjectREFR* target)
{
	if (!target)
		return NULL;

	return IsLocked_Native((*g_skyrimVM)->GetClassRegistry(), 0, target);
}

bool NativeFunctions::IsAIEnabled(TESObjectREFR* sourceObject)
{
	Actor* thisActor = DYNAMIC_CAST(sourceObject, TESObjectREFR, Actor);

	if (!thisActor)
		return false;

	return (thisActor->flags1 & Actor::kFlags_AIEnabled) == Actor::kFlags_AIEnabled;
}

UInt32 NativeFunctions::GetNthSpellId(TESShout* shout, int i)
{
	if (!shout)
		return NULL;

	return papyrusShout::GetNthSpell(shout, i)->formID;
}

void NativeFunctions::NativeFunctions::SetFaceTextureSet(TESObjectREFR* theActor, UInt32 textureId)
{
	if (!theActor)
		return;

	TESNPC* thisNPC = DYNAMIC_CAST(((Actor*)DYNAMIC_CAST(theActor, TESObjectREFR, Actor))->baseForm, TESForm, TESNPC);
	BGSTextureSet* textureSet = DYNAMIC_CAST(LookupFormByID(textureId), TESForm, BGSTextureSet);

	if (!thisNPC || !textureSet)
		return;

	papyrusActorBase::SetFaceTextureSet(thisNPC, textureSet);
}

void NativeFunctions::ChangeHeadPart(TESObjectREFR* player, UInt32 partId)
{
	if (!player)
		return;

	BGSHeadPart* newPart = DYNAMIC_CAST(LookupFormByID(partId), TESForm, BGSHeadPart);
	Actor* thePlayer = DYNAMIC_CAST(player, TESObjectREFR, Actor);

	if (!newPart || !thePlayer)
		return;

	papyrusActor::ChangeHeadPart(thePlayer, newPart);
}

void NativeFunctions::SetHeight(TESObjectREFR* theActor, float height)
{
	if (!theActor)
		return;

	TESNPC* thisNPC = DYNAMIC_CAST(((Actor*)DYNAMIC_CAST(theActor, TESObjectREFR, Actor))->baseForm, TESForm, TESNPC);

	if (!thisNPC)
		return;

	papyrusActorBase::SetHeight(thisNPC, height);
}

void NativeFunctions::SetDisplayName(TESObjectREFR* object, const char* value, bool force)
{
	if (!object)
		return;

	BSFixedString nameString(value);
	referenceUtils::SetDisplayName(&object->extraData, nameString, force);
}

TESNPC* NativeFunctions::GetPlayerBase()
{
	return (TESNPC*)(*g_thePlayer)->baseForm;
}

TESNPC* NativeFunctions::GetTemplate(TESNPC* npcTemplate)
{
	if (!npcTemplate)
		return NULL;

	if (npcTemplate->GetRootTemplate() == NULL)
		return npcTemplate;
	else
		return npcTemplate->GetRootTemplate();
}

TESObjectREFR* NativeFunctions::GetSitTarget(TESObjectREFR* sourceObject)
{
	if (!sourceObject)
		return NULL;

	Actor* thisActor = DYNAMIC_CAST(sourceObject, TESObjectREFR, Actor);

	if (!thisActor)
		return NULL;
	ActorProcessManager * processManager = thisActor->processManager;
	if (!processManager)
		return NULL;
	MiddleProcess * middleProcess = processManager->middleProcess;
	if (!middleProcess)
		return NULL;

	TESObjectREFR * refr = NULL;
	UInt32 furnitureHandle = middleProcess->furnitureHandle;
	if (furnitureHandle == (*g_invalidRefHandle) || furnitureHandle == 0)
		return NULL;

	LookupREFRByHandle(&furnitureHandle, &refr);
	return refr;
}

TESObjectREFR * NativeFunctions::GetEnableParent(TESObjectREFR* object)
{
	if (!object)
		return NULL;

	ExtraEnableStateParent* xEnableParent = static_cast<ExtraEnableStateParent*>(object->extraData.GetByType(kExtraData_EnableStateParent));
	if (!xEnableParent)
		return NULL;

	return xEnableParent->GetReference();
}

void NativeFunctions::RemoveCollision(TESObjectREFR* object)
{
	if (!object)
		return;

	ExtraCollisionData* xCollisionDat = static_cast<ExtraCollisionData*>(object->extraData.GetByType(kExtraData_CollisionData));

	if (!xCollisionDat)
		return;

	xCollisionDat->data->collisionLayer = 0x0010AA40;
}

char* NativeFunctions::GetSitAnimation(TESObjectREFR* sourceObject)
{
	if (!sourceObject)
		return NULL;

	Actor* thisActor = DYNAMIC_CAST(sourceObject, TESObjectREFR, Actor);

	if (!thisActor)
		return NULL;
	ActorProcessManager * processManager = thisActor->processManager;
	if (!processManager)
		return NULL;
	MiddleProcess * middleProcess = processManager->middleProcess;
	if (!middleProcess)
		return NULL;

	TESIdleForm* furnitureHandle = middleProcess->currentIdle;
	if (!furnitureHandle)
		return NULL;

	if (furnitureHandle->relatedIdle2)
		return (char*)furnitureHandle->relatedIdle2->animationEvent.data;
	else
		return (char*)furnitureHandle->animationEvent.data;
}

void NativeFunctions::SetHairColor(TESObjectREFR* theActor, UInt32 colorId)
{
	if (!theActor)
		return;

	TESNPC* thisNPC = DYNAMIC_CAST(((Actor*)DYNAMIC_CAST(theActor, TESObjectREFR, Actor))->baseForm, TESForm, TESNPC);
	BGSColorForm* colorForm = DYNAMIC_CAST(LookupFormByID(colorId), TESForm, BGSColorForm);

	if (thisNPC && colorForm && thisNPC->headData) {
		thisNPC->headData->hairColor = colorForm;
	}
}

void NativeFunctions::QueueNiNodeUpdate(TESObjectREFR* player)
{
	if (!player)
		return;

	Actor* thePlayer = DYNAMIC_CAST(player, TESObjectREFR, Actor);

	if (!thePlayer)
		return;

	papyrusActor::QueueNiNodeUpdate(thePlayer);
}

UInt32 NativeFunctions::GetPackageId(TESPackage* package)
{
	if (package)
		return package->formID;
	else
		return 0;
}

void NativeFunctions::SetVoice(TESObjectREFR* player, UInt32 voiceId)
{
	if (!player)
		return;

	if (!player->baseForm) {
		TESNPC* basePlayer = DYNAMIC_CAST(player->baseForm, TESForm, TESNPC);
		basePlayer->actorData.voiceType = DYNAMIC_CAST(LookupFormByID(voiceId), TESForm, BGSVoiceType);
	}
}

bool NativeFunctions::IsWindowOpen(char* menuName)
{
	BSFixedString tMenuName(menuName);

	if (!tMenuName.data)
		return 0;

	MenuManager * mm = MenuManager::GetSingleton();
	if (!mm)
		return false;

	return mm->IsMenuOpen(&tMenuName);
}

class GridCellArray
{
public:
	virtual ~GridCellArray();

	UInt32			unk04;
	UInt32			unk08;
	UInt32			size;
	TESObjectCELL**	cells;
};

std::vector<TESObjectREFR*> NativeFunctions::GetCloseReferences(TESObjectREFR* player, float fRadius, bool sameLocation)
{
	GridCellArray* arr = (GridCellArray*)(*g_TES)->gridCellArray;
	double fRadiusSquare = fRadius * fRadius;

	std::vector<TESObjectREFR*> list;
	int x, y;
	for (x = 0; x < arr->size; x++)
	{
		for (y = 0; y < arr->size; y++)
		{
			TESObjectCELL* cell = arr->cells[x + y*arr->size];
			if (cell->unk30 != 7)
				continue;

			TESObjectREFR* ref = nullptr;
			int i = 0;
			while (cell->objectList.GetNthItem(i++, ref))
			{
				if (!ref || ref == player || ref == *g_thePlayer)
					continue;
				if (ref->GetNiNode() && (ref->GetFormType() == Actor::kTypeID || ref->GetFormType() == kFormType_Reference))
				{
					if (!sameLocation)
					{
						double dx = ref->pos.x - player->pos.x;
						double dy = ref->pos.y - player->pos.y;
						double dz = ref->pos.z - player->pos.z;
						double d2 = dx*dx + dy*dy + dz*dz;

						if (d2 < fRadiusSquare)
							list.push_back(ref);
					}
					else
					{
						//If they are in the same location as us, add them.
						if (GetCurrentLocation((*g_skyrimVM)->GetClassRegistry(), 0, *g_thePlayer) == GetCurrentLocation((*g_skyrimVM)->GetClassRegistry(), 0, ref))
							list.push_back(ref);
					}
				}
			}
		}
	}

	return list;
}

std::vector<std::string> NativeFunctions::actorFormList;
char* NativeFunctions::GetActorForm(int pos)
{
	return (char*)actorFormList[pos].c_str();
}

int NativeFunctions::SetupNpcs(TESObjectREFR* target, bool isInInterior)
{
	if (!target)
		return NULL;

	actorFormList.clear();

	std::vector<TESObjectREFR*> closeTargets = GetCloseReferences(target, 4096, true);

	for (int i = 0; i < closeTargets.size(); i++)
		actorFormList.push_back(std::to_string(closeTargets[i]->formID));

	return closeTargets.size();
}

UInt32 NativeFunctions::GetNearestXMarker()
{
	std::vector<TESObjectREFR*> obList = GetCloseReferences((*g_thePlayer), 4096, false);

	for (int i = 0; i < obList.size(); i++)
	{
		if (obList[i]->GetFormType() != Actor::kTypeID)
			return obList[i]->formID;
	}

	return 0;
}

std::vector<TESObjectREFR*> NativeFunctions::GetAllLockedObjects(TESObjectREFR* target)
{
	return GetCloseReferences(target, 4096, false);
}

std::vector<TESObjectREFR*> NativeFunctions::GetCloseLockedRefs(TESObjectREFR* player, float fRadius)
{
	GridCellArray* arr = (GridCellArray*)(*g_TES)->gridCellArray;
	double fRadiusSquare = fRadius * fRadius;

	std::vector<TESObjectREFR*> list;

	if (player == NULL)
		return list;

	int x, y;
	for (x = 0; x < arr->size; x++)
	{
		for (y = 0; y < arr->size; y++)
		{
			TESObjectCELL* cell = arr->cells[x + y*arr->size];
			if (cell->unk30 != 7)
				continue;

			TESObjectREFR* ref = NULL;
			int i = 0;
			while (cell->objectList.GetNthItem(i++, ref))
			{
				if (!ref || ref == player || ref == *g_thePlayer)
					continue;
				if (IsLocked(ref))
				{
					double dx = ref->pos.x - player->pos.x;
					double dy = ref->pos.y - player->pos.y;
					double dz = ref->pos.z - player->pos.z;
					double d2 = dx*dx + dy*dy + dz*dz;

					if (d2 < fRadiusSquare)
						list.push_back(ref);
				}
			}
		}
	}

	return list;
}

std::vector<TESObjectREFR*> NativeFunctions::GetLocationReferences(BGSLocation* PlayerCurrentLocation)
{
	GridCellArray* arr = (GridCellArray*)(*g_TES)->gridCellArray;

	std::vector<TESObjectREFR*> list;
	int x, y;
	for (x = 0; x < arr->size; x++)
	{
		for (y = 0; y < arr->size; y++)
		{
			TESObjectCELL* cell = arr->cells[x + y*arr->size];
			if (cell->unk30 != 7)
				continue;

			TESObjectREFR* ref = nullptr;
			int i = 0;
			while (cell->objectList.GetNthItem(i++, ref))
			{
				if (!ref || ref->formID == (*g_thePlayer)->formID)
					continue;

				bool disabled = IsDisabled((*g_skyrimVM)->GetClassRegistry(), 23000, ref);

				if (ref->GetFormType() == Actor::kTypeID && !disabled)
				{
					bool sameLocation = PlayerCurrentLocation == GetCurrentLocation((*g_skyrimVM)->GetClassRegistry(), 23000, ref);
					//If they are in the same location as us, add them.
					if (sameLocation)
						list.push_back(ref);
				}
			}
		}
	}

	return list;
}

std::vector<TESObjectREFR*> NativeFunctions::GetCellMembers(TESObjectCELL* cell)
{
	std::vector<TESObjectREFR*> list;

	TESObjectREFR* ref = nullptr;

	int i = 0;
	while (cell->objectList.GetNthItem(i++, ref))
	{
		if (!ref || ref->formID == (*g_thePlayer)->formID)
			continue;

		bool disabled = IsDisabled((*g_skyrimVM)->GetClassRegistry(), 23000, ref);

		if (ref->GetFormType() == Actor::kTypeID && !disabled)
			list.push_back(ref);
	}

	return list;
}

_IsKeyPressed NativeFunctions::IsKeyPressed = &(papyrusInput::IsKeyPressed);
_GetMappedKey NativeFunctions::GetMappedKey = &(papyrusInput::GetMappedKey);
_GetName NativeFunctions::GetName = &(papyrusForm::GetName);
_GetFaceTextureSet NativeFunctions::GetFaceTextureSet = &(papyrusActorBase::GetFaceTextureSet);
_GetNthHeadPart NativeFunctions::GetNthHeadPart = &(papyrusActorBase::GetNthHeadPart);
_GetWornForm NativeFunctions::GetWornForm = &(papyrusActor::GetWornForm);
_GetDisplayName NativeFunctions::GetDisplayName = &(papyrusObjectReference::GetDisplayName);