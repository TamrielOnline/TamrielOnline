#pragma once
#include <vector>
#include <string>
#include "skse\GameReferences.h"

using namespace std;

class GameState
{
public:
	GameState() = default;

	/* Most of the important actor values. However there are still the
	mods, mults, and some others that I have not added yet. Don't do
	a lazy job when you get to them.*/
	struct ActorValueInfo
	{
		int skOneHanded, skTwoHanded, skMarksman, skBlock, skSmithing, skHeavyArmor, skLightArmor, skPickpocket,
			skSneak, skAlchemy, skSpeechcraft, skAlteration, skConjuration, skDestruction, skIllusion, skRestoration,
			skEnchanting, atHealth, atMagicka, atStamina, stInventoryWeight, stCarryWeight;

		float mtSpeed, stCritChance, stMeleeDamage, stUnarmedDamage, stMass, ssParalysis, ssInvisibility, ssWaterBreathing, ssWaterWalking, stBlindness,
			mtWeaponSpeed, stBowStaggerBonus, mtMovementNoise, mtLeftWeaponSpeed, stDragonSouls, mtAttackDamage, stReflectDamage, rtDamage,
			rtMagic;
	};

	struct Vector3
	{
		int x, y, z;
	};

	struct Transform
	{
		Vector3 pos, rot;
	};

	struct ActorState
	{
		ActorValueInfo av;
		Transform transform;
		NiNode* root;
		int weight, sex;
		string raceName, displayName;
		BGSLocation* currentLocation;
		TESWorldSpace* currentWorldspace;
		float height;
		bool bIsBlocking, bIsRidingHorse, bIsAnimationPlaying, bLeftCasting, bRightCasting;
		UInt32 fRightSpellName, fLeftSpellName, fHeadArmorId, fHairTypeId, fHairLongId, fVoiceId, fAmmoId, fBodyArmorId, fHandsArmorId, fForeArmArmorId,
			fAmuletArmorId, fRingArmorId, fFeetArmorId, fCalvesArmorId, fShieldArmorId, fCircletArmorId, fFaceset, fMouthId, fHeadId, fEyesId, fHairId,
			fHairColor, fBeardId, fScarId, fBrowId, fShoutId, fRaceId, fHorseHandleId, fRightWeaponId, fLeftWeaponId;
	};

	static TESQuest* mainQuest;
	static bool IsLoading, IsMenuOpen, IsRefreshing;
	static ActorState plState;
	static string loadOrder;
	static TESFaction* npcFaction;
	static VMClassRegistry* skyrimVMRegistry;
};