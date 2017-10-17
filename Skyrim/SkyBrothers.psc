Scriptname SkyBrothers extends ObjectReference

;-- Properties --------------------------------------
Actor property player auto
Actor property playerRef auto
actorbase property pActorBase auto
Float Property PulseRate = 0.033 Auto ; How often do we update?
int Property refId Auto ;Used to temporarily store the reference id for the actor being manipulated
int property tGuid Auto ;Used to temporarily store the guid of the user being whose data is being manipulated.
int Property lastActivated Auto ;Stores the last activated button
float Property lastActivatedTime Auto ;Stores the time of the last activated button
float Property gameDaysPassed Auto
float property gameDay Auto
float property gameHour Auto
float property gameMonth Auto
float property gameYear Auto

;-- Variables ---------------------------------------

;-- Functions ---------------------------------------

function LoadPlayer()
	player = (self as ObjectReference) as Actor
	playerRef = game.GetPlayer()
	pActorBase = game.GetPlayer().GetActorBase()
	while skse.GetVersionRelease() < 47
		debug.Notification("Please upgrade SKSE")
		utility.Wait(5 as Float)
	endWhile
	RegisterForSingleUpdate(PulseRate) ; Call this last to be safe, we don't want to create any unnecessary threading issues. 
endFunction

function OnInit()
	LoadPlayer()
endFunction
 
Event OnPlayerLoadGame()
    LoadPlayer()
EndEvent

function OnUpdate()
	if (player == game.GetPlayer())
	    ReadMessages()
	    self.RegisterForSingleUpdate(PulseRate) ; Call this last to be safe, we don't want to create any unnecessary threading issues. 
	else
		if utility.GetCurrentRealTime() < 10 as Float
			self.UnregisterForUpdate()
			self.DisableNoWait(false)
		endif
	endif
endFunction

Function SneakStart(Actor ref)
    Debug.SendAnimationEvent(ref, "SneakStart")
EndFunction

Function SneakStop(Actor ref)
    Debug.SendAnimationEvent(ref, "SneakStop")
EndFunction

; Credit goes to chriz2fer for his npc jumping implementation
Function StartJumping(Actor ref) 
    if SkyUtilitiesScript.GetRemotePlayerDataInt(tGuid, "sitType") == 0
        if SkyUtilitiesScript.GetRemotePlayerDataBool(tGuid, "isMounted") == false
            ;GetAnimationVariableFloat("Speed")
            if ref.IsRunning() == true || ref.IsSprinting() == true
                ref.PlayIdle(Game.GetForm(0x000884A3) as Idle)
            else
                ref.PlayIdle(Game.GetForm(0x000884A2) as Idle)
            endif
        else
            Actor horse = Game.GetForm(SkyUtilitiesScript.GetRemotePlayerDataString(tGuid, "horse") as int) as Actor
            Debug.SendAnimationEvent(horse, "forwardJumpStart")
        endif
    endif

    ref.SetMotionType(4, false)
    ref.TranslateTo(ref.x, ref.y, ref.z + (ref.GetHeight()/1.4), ref.GetAngleX(), ref.GetAngleY(), ref.GetAngleZ(), 500, 0)
    ;ref.TranslateTo(ref.x + 0.6 * math.Cos(ref.GetAngleZ()), ref.x + 0.6 * math.Sin(ref.GetAngleZ()), ref.z + (ref.GetHeight()/1.4), ref.GetAngleX(), ref.GetAngleY(), ref.GetAngleZ(), 500, 0)
EndFunction

Function StopJumping(Actor ref)
    Debug.SendAnimationEvent(ref, "JumpLand")
EndFunction

Function StartSheath(Actor ref)
    ref.InterruptCast()
EndFunction

Function AttackRight(Actor ref)
   if SkyUtilitiesScript.GetRemotePlayerDataInt(tGuid, "sitType") == 0
        Debug.SendAnimationEvent(ref, "attackStart")
    endif
EndFunction

Function AttackLeft(Actor ref)
   if SkyUtilitiesScript.GetRemotePlayerDataInt(tGuid, "sitType") == 0
        Debug.SendAnimationEvent(ref, "attackStartLeftHand")
    endif
EndFunction

Function SetActorValues(Actor ref, string[] values)
    ref.ForceActorValue("OneHanded", values[0] as int)
    ref.ForceActorValue("TwoHanded", values[1] as int)
    ref.ForceActorValue("Marksman", values[2] as int)
    ref.ForceActorValue("Block", values[3] as int)
    ref.ForceActorValue("Smithing", values[4] as int)
    ref.ForceActorValue("HeavyArmor", values[5] as int)
    ref.ForceActorValue("LightArmor", values[6] as int)
    ref.ForceActorValue("Pickpocket", values[7] as int)
    ref.ForceActorValue("Sneak", values[8] as int)
    ref.ForceActorValue("Alchemy", values[9] as int)
    ref.ForceActorValue("Speechcraft", values[10] as int)
    ref.ForceActorValue("Alteration", values[11] as int)
    ref.ForceActorValue("Conjuration", values[12] as int)
    ref.ForceActorValue("Destruction", values[13] as int)
    ref.ForceActorValue("Illusion", values[14] as int)
    ref.ForceActorValue("Restoration", values[15] as int)
    ref.ForceActorValue("Enchanting", values[16] as int)
    ref.ForceActorValue("Health", 1000000000)
    ref.ForceActorValue("Magicka", 10000)
    ref.ForceActorValue("Stamina", 10000)
    ref.ForceActorValue("SpeedMult", values[20] as int)
    ref.ForceActorValue("InventoryWeight", values[21] as int)
    ref.ForceActorValue("CarryWeight", values[22] as int)
    ref.ForceActorValue("CritChance", values[23] as int)
    ref.ForceActorValue("MeleeDamage", values[24] as int)
    ref.ForceActorValue("UnarmedDamage", values[25] as int)
    ref.ForceActorValue("Mass", values[26] as int)
    ref.ForceActorValue("Paralysis", values[27] as int)
    ref.ForceActorValue("Invisibility", values[28] as int)
    ref.ForceActorValue("WaterBreathing", values[29] as int)
    ref.ForceActorValue("WaterWalking", values[30] as int)
    ref.ForceActorValue("Blindness", values[31] as int)
    ref.ForceActorValue("WeaponSpeedMult", values[32] as int)
    ref.ForceActorValue("BowStaggerBonus", values[33] as int)
    ref.ForceActorValue("MovementNoiseMult", values[34] as int)
    ref.ForceActorValue("LeftWeaponSpeedMult", values[35] as int)
    ref.ForceActorValue("DragonSouls", values[36] as int)
    ref.ForceActorValue("AttackDamageMult", values[37] as int)
    ref.ForceActorValue("ReflectDamage", values[38] as int)
    ref.ForceActorValue("DamageResist", 10000)
    ref.ForceActorValue("MagicResist", 10000)
EndFunction

Function AttackDual(Actor ref)
   if SkyUtilitiesScript.GetRemotePlayerDataInt(tGuid, "sitType") == 0
        Debug.SendAnimationEvent(ref, "attackStartDualWield")
    endif
EndFunction

Function InterruptCastAll(Actor ref)
    if SkyUtilitiesScript.GetRemotePlayerDataInt(tGuid, "sitType") == 0 && SkyUtilitiesScript.GetRemotePlayerDataBool(tGuid, "alert") == true
        ref.InterruptCast()
    endif
EndFunction

Function InterruptCastLeft(Actor ref)
    if SkyUtilitiesScript.GetRemotePlayerDataInt(tGuid, "sitType") == 0 && SkyUtilitiesScript.GetRemotePlayerDataBool(tGuid, "alert") == true
        Spell tSpell = ref.GetEquippedSpell(0)
        Spell interruptSpell = Game.GetForm(0x00021143) as Spell ;Clairvoyance

        if tSpell != None
            int spellId = tSpell.GetFormId()
            
            interruptSpell.SetEquipType(Game.GetForm(0x00013F43) as EquipSlot) ;Set the spell to be casted from the left hand
            interruptSpell.Cast(ref, ref)

            ref.UnequipSpell(tSpell, 0)
            ref.EquipSpell(Game.GetForm(spellId) as Spell, 0)
        endif
    endif
EndFunction

Function InterruptCastRight(Actor ref)
    if SkyUtilitiesScript.GetRemotePlayerDataInt(tGuid, "sitType") == 0 && SkyUtilitiesScript.GetRemotePlayerDataBool(tGuid, "alert") == true
        Spell tSpell = ref.GetEquippedSpell(1)
        Spell interruptSpell = Game.GetForm(0x00021143) as Spell;Clairvoyance

        if tSpell != None
            int spellId = tSpell.GetFormId()
            
            interruptSpell.SetEquipType(Game.GetForm(0x00013F42) as EquipSlot) ;Set the spell to be casted from the left hand
            interruptSpell.Cast(ref, ref)

            ref.UnequipSpell(tSpell, 1)
            ref.EquipSpell(Game.GetForm(spellId) as Spell, 1)
        endif
    endif
EndFunction

Function CastLeft(Actor ref, string[] values)
    if SkyUtilitiesScript.GetRemotePlayerDataInt(tGuid, "sitType") == 0 && SkyUtilitiesScript.GetRemotePlayerDataBool(tGuid, "alert") == true
        Spell tSpell = ref.GetEquippedSpell(0)

        if tSpell != None
            if values[0] == ""
                tSpell.Cast(ref) ;Auto cast
            elseif values[0] == "0"
                tSpell.Cast(ref, ref) ;Self cast
            endif
        endif
    endif
EndFunction

Function CastRight(Actor ref, string[] values)
    if SkyUtilitiesScript.GetRemotePlayerDataInt(tGuid, "sitType") == 0 && SkyUtilitiesScript.GetRemotePlayerDataBool(tGuid, "alert") == true
        Spell tSpell = ref.GetEquippedSpell(1)

        if tSpell != None
            if values[0] == ""
                tSpell.Cast(ref) ;Auto cast
            elseif values[0] == "0"
                tSpell.Cast(ref, ref) ;Self cast
            endif
        endif
    endif
EndFunction

Function DualCast(Actor ref, string[] values)
    if SkyUtilitiesScript.GetRemotePlayerDataInt(tGuid, "sitType") == 0 && SkyUtilitiesScript.GetRemotePlayerDataInt(tGuid, "alert") == true
        Spell spellLeft = ref.GetEquippedSpell(0)
        Spell spellRight = ref.GetEquippedSpell(1)

        if spellLeft != None && spellRight != None
            if values[0] == ""
                spellLeft.Cast(ref) ;Auto cast
                spellRight.Cast(ref) ;Auto cast
            elseif values[0] == "0"
                spellLeft.Cast(ref, ref) ;Self cast
                spellRight.Cast(ref, ref) ;Self cast
            endif
        endif
    endif
EndFunction

;We can at some point merge all of these "SendAnimationEvent" functions together into something generic
Function BlockStart(Actor ref)
    if SkyUtilitiesScript.GetRemotePlayerDataInt(tGuid, "sitType") == 0
        Debug.SendAnimationEvent(ref, "blockStart")
    endif
EndFunction

Function BlockStop(Actor ref)
    if SkyUtilitiesScript.GetRemotePlayerDataInt(tGuid, "sitType") == 0
        Debug.SendAnimationEvent(ref, "BlockStop")
    endif
EndFunction

Function BowAttack(Actor ref)
    if SkyUtilitiesScript.GetRemotePlayerDataInt(tGuid, "sitType") == 0
        Debug.SendAnimationEvent(ref, "bowAttackStart")
    endif
EndFunction

Function FireBow(Actor ref)
    if SkyUtilitiesScript.GetRemotePlayerDataInt(tGuid, "sitType") == 0
        Debug.SendAnimationEvent(ref, "blockStart")
        Debug.SendAnimationEvent(ref, "bowAttackStart")

        ObjectReference targetController = Game.GetForm(SkyUtilitiesScript.GetRemotePlayerDataString(tGuid, "targetController") as int) as ObjectReference
        ref.SetLookAt(targetController, false)

        Weapon bow = Game.GetForm(0x000139AD) as Weapon
        Ammo tAmmo = Game.GetForm(0x000139BF) as Ammo

        bow.Fire(ref, tAmmo)
        Debug.SendAnimationEvent(ref, "attackRelease")
    endif
EndFunction

Function BashAttack(Actor ref)
    if SkyUtilitiesScript.GetRemotePlayerDataInt(tGuid, "sitType") == 0
        Debug.SendAnimationEvent(ref, "bashStart")
    endif
EndFunction

Function ShoutRelease(Actor ref, string[] values)
    if SkyUtilitiesScript.GetRemotePlayerDataInt(tGuid, "sitType") == 0
        Debug.SendAnimationEvent(ref, "shoutStart")
        Debug.SendAnimationEvent(ref, "MT_BreathExhaleShort")

        Spell shoutSpell = Game.GetForm(values[0] as int) as Spell

        if shoutSpell == None
            return
        endif

        ObjectReference targetController = Game.GetForm(SkyUtilitiesScript.GetRemotePlayerDataString(tGuid, "targetController") as int) as ObjectReference
        ref.SetLookAt(targetController, false)

        shoutSpell.Cast(ref, targetController)
    endif
EndFunction

Function ForceAV(Actor ref, string avName, string[] values)
    ref.ForceActorValue(avName, values[0] as int)
EndFunction

Function Equip(Actor ref, string[] values)
    if values[1] == "right"
        ref.UnequipItemEx(ref.GetEquippedObject(1), 1, true) ;Unequip current right item

        if values[0] != "0"
            Form newRightEquip = Game.GetForm(values[0] as int)
            SkyUtilitiesScript.EquipItem(ref, newRightEquip, 1)
            SkyUtilitiesScript.SetRemotePlayerDataBool(tGuid, "equippedSpell", false)
        endif
    elseif values[1] == "left"
        ref.UnequipItemEx(ref.GetEquippedObject(0), 0, true) ;Unequip current right item

        if values[0] != "0"
            Form newLeftEquip = Game.GetForm(values[0] as int)
            SkyUtilitiesScript.EquipItem(ref, newLeftEquip, 0)
            SkyUtilitiesScript.SetRemotePlayerDataBool(tGuid, "equippedSpell", false)
        endif
    elseif values[1] == "head"
        ref.UnequipItemSlot(30)

        if values[0] != "0"
            Form newHeadEquip = Game.GetForm(values[0] as int)
            ref.EquipItem(newHeadEquip, true, true)
        endif
    elseif values[1] == "body"
        ref.UnequipItemSlot(32)

        if values[0] != "0"
            Form newBodyEquip = Game.GetForm(values[0] as int)
            ref.EquipItem(newBodyEquip, true, true)
        endif
    elseif values[1] == "hands"
        ref.UnequipItemSlot(33)

        if values[0] != "0"
            Form newHandsEquip = Game.GetForm(values[0] as int)
            ref.EquipItem(newHandsEquip, true, true)
        endif
    elseif values[1] == "forearm"
        ref.UnequipItemSlot(34)

        if values[0] != "0"
            Form newForearmEquip = Game.GetForm(values[0] as int)
            ref.EquipItem(newForearmEquip, true, true)
        endif
    elseif values[1] == "amulet"
        ref.UnequipItemSlot(35)

        if values[0] != "0"
            Form newAmuletEquip = Game.GetForm(values[0] as int)
            ref.EquipItem(newAmuletEquip, true, true)
        endif
    elseif values[1] == "ring"
        ref.UnequipItemSlot(36)

        if values[0] != "0"
            Form newRingEquip = Game.GetForm(values[0] as int)
            ref.EquipItem(newRingEquip, true, true)
        endif
    elseif values[1] == "feet"
        ref.UnequipItemSlot(37)

        if values[0] != "0"
            Form newFeetEquip = Game.GetForm(values[0] as int)
            ref.EquipItem(newFeetEquip, true, true)
        endif
    elseif values[1] == "calves"
        ref.UnequipItemSlot(38)

        if values[0] != "0"
            Form newCalvesEquip = Game.GetForm(values[0] as int)
            ref.EquipItem(newCalvesEquip, true, true)
        endif
    elseif values[1] == "circlet"
        ref.UnequipItemSlot(42)

        if values[0] != "0"
            Form newCircletEquip = Game.GetForm(values[0] as int)
            ref.EquipItem(newCircletEquip, true, true)
        endif
    else
        if values[0] != "0"
            Form newOtherEquip = Game.GetForm(values[0] as int)
            ref.EquipItem(newOtherEquip, true, true)
        endif
    endif

    int EquipTypeLeft = ref.GetEquippedItemType(0)
    int EquipTypeRight = ref.GetEquippedItemType(1)

    if EquipTypeLeft == 7 || EquipTypeRight == 7
        ref.GetActorBase().SetClass(Game.GetForm(0x00013181) as Class)
    elseif EquipTypeLeft == 0 && EquipTypeRight == 0
        ref.GetActorBase().SetClass(Game.GetForm(0x00013182) as Class)
    else
        if EquipTypeLeft == 5 || EquipTypeLeft == 6 || EquipTypeRight == 5 || EquipTypeRight == 6
            ref.GetActorBase().SetCombatStyle(Game.GetForm(0x0001CE15) as CombatStyle)
        elseif EquipTypeLeft != 9 && EquipTypeRight != 9 && (EquipTypeLeft == 0 || EquipTypeRight == 0)
            ref.GetActorBase().SetClass(Game.GetForm(0x00013176) as Class)
        elseif EquipTypeLeft != 9 && EquipTypeRight != 9 && EquipTypeLeft != 5 && EquipTypeLeft != 6 && EquipTypeRight != 5 && EquipTypeRight != 6
            ref.GetActorBase().SetClass(Game.GetForm(0x00013176) as Class)
        endif
    endif
EndFunction

Function Unlock(Actor ref, string[] values)
    ObjectReference tLock = Game.GetForm(values[0] as int) as ObjectReference

    if tLock != None
        tLock.Lock(false)
        tLock.SetOpen(true)
    endif
EndFunction

Function StartLock(Actor ref, string[] values)
    ObjectReference tLock = Game.GetForm(values[0] as int) as ObjectReference

    if tLock != None
        tLock.Lock(true)
        tLock.SetOpen(false)
    endif
EndFunction

Function EquipSpell(Actor ref, string[] values)
    Spell equipSpell = Game.GetForm(values[0] as int) as Spell

    if values[1] == "right"
        Weapon right = ref.GetEquippedWeapon(false)
        Spell rightSpell = ref.GetEquippedSpell(1)

        if right != None
            ref.RemoveItem(right, 1, true)
        endif

        if rightSpell != None
            ref.UnequipSpell(rightSpell, 1)
        endif

        if equipSpell != None
            ref.EquipSpell(equipSpell, 1)
            SkyUtilitiesScript.SetRemotePlayerDataBool(tGuid, "equippedSpell", true)
        endif
    else
        Weapon left = ref.GetEquippedWeapon(true)
        Spell leftSpell = ref.GetEquippedSpell(0)

        if left != None
            ref.RemoveItem(left, 1, true)
        endif

        if leftSpell != None
            ref.UnequipSpell(leftSpell, 0)
        endif

        if equipSpell != None
            ref.EquipSpell(equipSpell, 0)
            SkyUtilitiesScript.SetRemotePlayerDataBool(tGuid, "equippedSpell", true)
        endif
    endif
EndFunction

Function RemoveSpellLeft(Actor ref)
    Spell leftSpell = ref.GetEquippedSpell(0)

    if leftSpell != None
        ref.UnequipSpell(leftSpell, 0)
        SkyUtilitiesScript.SetRemotePlayerDataBool(tGuid, "equippedSpell", false)
    endif
EndFunction

Function RemoveSpellRight(Actor ref)
    Spell rightSpell = ref.GetEquippedSpell(1)

    if rightSpell != None
        ref.UnequipSpell(rightSpell, 1)
        SkyUtilitiesScript.SetRemotePlayerDataBool(tGuid, "equippedSpell", false)
    endif
EndFunction

Function EquipShout(Actor ref, string[] values)
    ref.EquipShout(Game.GetForm(values[0] as int) as Shout)
EndFunction

Function SetHeadPart(Actor ref, string[] values)
    ref.ChangeHeadPart(Game.GetForm(values[0] as int) as HeadPart)
EndFunction

Function ActivateButton(Actor ref, string[] values)
    ObjectReference activationTarget = Game.GetForm(values[0] as int) as ObjectReference

    if activationTarget != None
        if activationTarget.GetFormId() != lastActivated || (activationTarget.GetFormId() == lastActivated && (Utility.GetCurrentRealTime() - lastActivatedTime) >= 1)
            activationTarget.Activate(ref, true)
            lastActivatedTime = Utility.GetCurrentRealTime()
        endif
    endif
EndFunction

Function ActivateAnimation(Actor ref, string[] values)
    if SkyUtilitiesScript.GetRemotePlayerDataInt(tGuid, "sitType") == 0 && SkyUtilitiesScript.GetRemotePlayerDataString(tGuid, "lastLocation") == SkyUtilitiesScript.GetRemotePlayerDataString(-1, "locationid")
        SkyUtilitiesScript.SetRemotePlayerDataInt(tGuid, "sitType", 1000)
        ref.ClearKeepOffsetFromActor()
        ref.SetUnconscious(true)
        Debug.SendAnimationEvent(ref, values[0])
    endif
EndFunction

Function SetStage(Actor ref, string[] values)
    Quest tQuest
    int count = 0

    While count < values.Length
        tQuest = Game.GetForm(values[count] as int) as Quest

        if tQuest != None
            if tQuest.IsActive() == false && tQuest.IsRunning() == false
                tQuest.Start()
            endif

            if values[count + 1] != "" && tQuest.GetCurrentStageId() < (values[count + 1] as int)
                tQuest.SetCurrentStageId(values[count + 1] as int)
                tQuest.SetObjectiveDisplayed(values[count + 1] as int, true, false)
            endif
        endif

        count += 2
    EndWhile
EndFunction

Function RideHorse(Actor ref, string[] values)
    if SkyUtilitiesScript.HasSecondsPassed("dropTimer", 0.1) == true
        Form mount = Game.GetForm(values[0] as int)
        ObjectReference mountRef = mount as ObjectReference

        if mountRef != None
            SkyUtilitiesScript.SetRemotePlayerDataInt(tGuid, "horse", mount.GetFormId())
            mountRef.MoveTo(ref, 0, 0, 0, true)
        else
            mountRef = ref.PlaceAtMe(mount, 1, true, false)
            SkyUtilitiesScript.SetRemotePlayerDataInt(tGuid, "horse", mount.GetFormId())
        endif

        SkyUtilitiesScript.SetRemotePlayerDataBool(tGuid, "isMounted", true)
        ref.ClearKeepOffsetFromActor()
        SkyUtilitiesScript.MountActor(ref, mountRef) ;This should call the "mountactor" console command in C++, MountActor(rider, mountTarget)
        SkyUtilitiesScript.StartTimer("dropTimer")
    endif
EndFunction

Function DismountHorse(Actor ref)
    if ref.IsOnMount() == true
        Debug.SendAnimationEvent(ref, "HorseExit")
    endif

    SkyUtilitiesScript.SetRemotePlayerDataBool(tGuid, "isMounted", false)
EndFunction

Function Getup(Actor ref)
    Debug.SendAnimationEvent(ref, "IdleForceDefaultState")
    ref.SetUnconscious(false)
    SkyUtilitiesScript.SetRemotePlayerDataInt(tGuid, "sitType", 0)
EndFunction

Function DropItem(Actor ref, string[] values)
    Form tItem = Game.GetForm(values[0] as int)

    if tItem != None
        ObjectReference nItem = ref.PlaceAtMe(tItem, 1, false)
        nItem.MoveTo(ref, 0, 0, 50)
    endif
EndFunction

Function Death(Actor ref, string[] values)
    Actor dTarget = Game.GetForm(values[0] as int) as Actor

    if dTarget != None
        if values[1] == "1"
            dTarget.KillEssential(ref)
        else
            ;This should call the "Resurrect 1" console command in C++, this command resurrects the actor with their inventory intact unlike the papyrus command. 
            ;Additionally we may want to try the recommendation on the papyrus discussion page, with the actor casting reanimation on themself to further reduce the C++ dependency.
            SkyUtilitiesScript.ResurrectExtended(dTarget)
        endif
    endif
EndFunction

Function KOFA(Actor ref, string[] values)
    if ref == None
        return
    endif

    SkyUtilitiesScript.PlayerKOFA(ref, values[0] as int)
EndFunction

Function UpdatePlayerPosition(Actor ref, string[] values)
    int playerNr = tguid
    ObjectReference positionController = SkyUtilitiesScript.RetrieveObject(values[6])
    ObjectReference horse = SkyUtilitiesScript.RetrieveObject(SkyUtilitiesScript.GetRemotePlayerDataString(playerNr, "horse"))
    ObjectReference targetController = SkyUtilitiesScript.RetrieveObject(SkyUtilitiesScript.GetRemotePlayerDataString(playerNr, "targetController"))

    if SkyUtilitiesScript.GetRemotePlayerDataBool(playerNr, "forceTeleport") == true
        positionController.MoveTo(player, 0, -2000, 0, true)
        SkyUtilitiesScript.SetRemotePlayerDataInt(playerNr, "sitType", 0)
        SkyUtilitiesScript.SetRemotePlayerDataBool(playerNr, "forceTeleport", false)
    endif

    positionController.SetPosition(values[0] as int, values[1] as int, values[2] as int)

    if positionController.GetDistance(ref) > 1024
        ref.MoveTo(positionController)
        SkyUtilitiesScript.PlayerKOFA(ref, playerNr)
    endif

    if SkyUtilitiesScript.GetRemotePlayerDataInt(playerNr, "sitType") != 0 || SkyUtilitiesScript.GetRemotePlayerDataBool(playerNr, "firstUpdate") == true
        ref.MoveTo(positionController)
        SkyUtilitiesScript.SetRemotePlayerDataBool(playerNr, "firstUpdate", false)
        SkyUtilitiesScript.PlayerKOFA(ref, playerNr)
    endif

    if ref.IsOnMount()
        if horse != None
            horse.SetAngle(values[3] as int, values[4] as int, values[5] as int)
        endif
    else
        ref.SetAngle(values[3] as int, values[4] as int, values[5] as int)
    endif

    if SkyUtilitiesScript.GetRemotePlayerDataString(playerNr, "sitAnim") == ""
        if SkyUtilitiesScript.GetRemotePlayerDataInt(playerNr, "sitType") == 1000
            ref.SetDontMove(false)
            Debug.SendAnimationEvent(ref, "IdleForceDefaultState")
            ref.SetUnconscious(false)
            SkyUtilitiesScript.SetRemotePlayerDataInt(playerNr, "sitType", 0)
        elseif SkyUtilitiesScript.GetRemotePlayerDataInt(playerNr, "sitType") == 0
            if SkyUtilitiesScript.GetSitAnimation(ref) != ""
                Debug.SendAnimationEvent(ref, "IdleForceDefaultState")
            endif
        endif
    else
        if SkyUtilitiesScript.GetRemotePlayerDataInt(playerNr, "sitType") == 0
            SkyUtilitiesScript.SetRemotePlayerDataInt(playerNr, "sitType", 1000)
            ref.SetDontMove(true)
            ref.ClearKeepOffsetFromActor()
            ref.SetUnconscious(true)
            Debug.SendAnimationEvent(ref, SkyUtilitiesScript.GetRemotePlayerDataString(playerNr, "sitAnim"))
        endif
    endif

    if targetController != None
        float xOffset = 1000 * Math.sin(values[4] as int)
        float yOffset = 1000 * Math.cos(values[4] as int)
        float zOffset = (values[4] as int) + (SkyUtilitiesScript.GetRemotePlayerDataFloat(playerNr, "height") * 0.75)

        ; Move target controller
        targetController.MoveTo(player, xOffset, yOffset, zOffset, true)
        ;SetLookAt(skyrimVMRegistry, 0, RemotePlayerMap[playerNr].actor, RemotePlayerMap[playerNr].targetController, false);
        if SkyUtilitiesScript.GetRemotePlayerDataBool(playerNr, "jump") == true && ref.GetPositionZ() <= SkyUtilitiesScript.GetRemotePlayerDataFloat(playerNr, "jumpZOrigin")
            SkyUtilitiesScript.SetRemotePlayerDataBool(playerNr, "jump", false)
        endif

        if ref.IsWeaponDrawn() == true
            if SkyUtilitiesScript.GetRemotePlayerDataBool(playerNr, "alert") == false
                ref.SetAlert(false)
                Debug.SendAnimationEvent(ref, "Unequip")
            endif
        else
            if SkyUtilitiesScript.GetRemotePlayerDataBool(playerNr, "alert") == true
                ref.SetAlert(true)

                if SkyUtilitiesScript.GetRemotePlayerDataBool(playerNr, "equippedSpell") == true
                    Debug.SendAnimationEvent(ref, "Magic_Equip")
                else
                    Debug.SendAnimationEvent(ref, "weapEquip")
                endif
            endif
        endif
    endif
EndFunction

Function SetWeather(Actor ref, string[] values)
    Weather fWeather = Game.GetForm(values[0] as int) as Weather

    if fWeather != None
        fWeather.ForceActive(false)
    endif
EndFunction

Actor Function SpawnPlayer(Actor ref, string[] values) global
    Form emptyBase = Game.GetForm(0x0008A91A) ; A generic item marker form
    ActorBase playerBase = None
    ObjectReference targetController = ref.PlaceAtMe(emptyBase, 1, true, false)
    ObjectReference movementController = ref.PlaceAtMe(emptyBase, 1, true, false)

    int raceId = values[0] as int
    int sex = values[1] as int
    string raceName = values[2]
    ; values[3] - values[16] { 3 - rightWeaponId, 4 - leftWeaponId, 5 - headArmorId, 6 - hairTypeId, 7 - hairLongId, 8 - bodyArmorId, 9 - handsArmorId, 10 - forearmArmorId, 11 - amuletArmorId,
    ; 12 - ringArmorId, 13 - feetArmorId, 14 - calvesArmorId, 15 - shieldArmorId, 16 - circletArmorId }
    ; values[17] - values[29] { 17 - mouthId, 18 - headId, 19 - eyesId, 20 - hairId, 21 - beardId, 22 - scarId, 23 - browId, 24 - height, 25 - faceset, 26 - hairColor, 27 - voiceId, 28 - weight, 29 - displayName}
    string guid = values[30]

    bool setRaceDirect = false
    if raceId == 79683 || raceId == 559168 ; High Elves
        if sex == 1
            playerBase = Game.GetForm(0x00079BED) as ActorBase
        else
            playerBase = Game.GetForm(0x0005EF9C) as ActorBase
        endif
    elseif raceId == 79680 || raceId == 559162 ; Argonians
        if sex == 1
            playerBase = Game.GetForm(0x000B2E11) as ActorBase
        else
            playerBase = Game.GetForm(0x00043E57) as ActorBase
        endif
    elseif raceId == 79689 || raceId == 559236 ; Wood Elves
        if sex == 1
            playerBase = Game.GetForm(0x00079CD3) as ActorBase
        else
            playerBase = Game.GetForm(0x0005EF9A) as ActorBase
        endif
    elseif raceId == 79681 || raceId == 559164 ; Bretons
        if sex == 1
            playerBase = Game.GetForm(0x00079F65) as ActorBase
        else
            playerBase = Game.GetForm(0x00079F6A) as ActorBase
        endif
    elseif raceId == 79682 || raceId == 559165 ; Dark Elves
        if sex == 1
            playerBase = Game.GetForm(0x00079F5B) as ActorBase
        else
            playerBase = Game.GetForm(0x0005EFA7) as ActorBase
        endif
    elseif raceId == 79684 || raceId == 559172 ; Imperials
        if sex == 1
            playerBase = Game.GetForm(0x00079F66) as ActorBase
        else
            playerBase = Game.GetForm(0x00026921) as ActorBase
        endif
    elseif raceId == 79685 || raceId == 559173 ; Khajiit
        if sex == 1
            playerBase = Game.GetForm(0x000EE856) as ActorBase
        else
            playerBase = Game.GetForm(0x00043E59) as ActorBase
        endif
    elseif raceId == 79686 || raceId == 558996 ; Nords
        if sex == 1
            playerBase = Game.GetForm(0x00079F68) as ActorBase
        else
            playerBase = Game.GetForm(0x0001750C) as ActorBase
        endif
    elseif raceId == 79687 || raceId == 688825 ; Orcs
        if sex == 1
            playerBase = Game.GetForm(0x00079F4E) as ActorBase
        else
            playerBase = Game.GetForm(0x00079F69) as ActorBase
        endif
    elseif raceId == 79688 || raceId == 559174 ; Redguard
        if sex == 1
            playerBase = Game.GetForm(0x00079F67) as ActorBase
        else
            playerBase = Game.GetForm(0x0005B4F8) as ActorBase
        endif
    else ; Custom races, animals, monsters, etc...
        playerBase = Game.GetForm(0x00079F67) as ActorBase ; The race will be changed after the character is spawned
        setRaceDirect = true
    endif

    playerBase.SetInvulnerable(true)
    playerBase.SetHeight(values[24] as float)
    playerBase.SetFaceTextureSet(Game.GetForm(values[25] as int) as TextureSet)
    playerBase.SetHairColor(Game.GetForm(values[26] as int) as ColorForm)
    playerBase.SetVoiceType(Game.GetForm(values[27] as int) as VoiceType)
    movementController.SetPosition(ref.GetPositionX(), ref.GetPositionY() - 2000, ref.GetPositionZ())

    Actor newPlayer = movementController.PlaceAtMe(playerBase as Form, 1, true, false) as Actor
    SkyUtilitiesScript.EnableNet(newPlayer)

    if setRaceDirect == true
        SkyUtilitiesScript.SetRace(newPlayer, raceName) ; This should call the console version. Probably can be replaced with the papyrus version if we send the race's formId.
    endif

    newPlayer.IgnoreFriendlyHits(true)
    newPlayer.SetRelationshipRank(ref, 1)
    SkyUtilitiesScript.APS(newPlayer, "SkyTools InitiateTracking")
    newPlayer.RemoveAllItems()
    SkyUtilitiesScript.SetDisplayName(newPlayer, values[29])
    SkyUtilitiesScript.SetRemotePlayerDataString(guid as int, "name", values[29])
    newPlayer.ChangeHeadPart(Game.GetForm(values[17] as int) as HeadPart)
    newPlayer.ChangeHeadPart(Game.GetForm(values[18] as int) as HeadPart)
    newPlayer.ChangeHeadPart(Game.GetForm(values[19] as int) as HeadPart)
    newPlayer.ChangeHeadPart(Game.GetForm(values[20] as int) as HeadPart)
    newPlayer.ChangeHeadPart(Game.GetForm(values[21] as int) as HeadPart)
    newPlayer.ChangeHeadPart(Game.GetForm(values[22] as int) as HeadPart)
    newPlayer.ChangeHeadPart(Game.GetForm(values[23] as int) as HeadPart)
    newPlayer.QueueNiNodeUpdate()

    Form rawEquip = Game.GetForm(values[3] as int)

    if rawEquip != None
        if rawEquip.GetType() == 41
            SkyUtilitiesScript.EquipItem(newPlayer, rawEquip, 1)
        elseif rawEquip.GetType() == 22
            newPlayer.EquipSpell(rawEquip as Spell, 1)
        endif
    endif

    rawEquip = Game.GetForm(values[4] as int)

    if rawEquip != None
        if rawEquip.GetType() == 41
            SkyUtilitiesScript.EquipItem(newPlayer, rawEquip, 0)
        elseif rawEquip.GetType() == 22
            newPlayer.EquipSpell(rawEquip as Spell, 0)
        endif
    endif

    int count = 5

    while count < 17
        rawEquip = Game.GetForm(values[count] as int)
        if rawEquip != None
            newPlayer.EquipItem(rawEquip, true, false)
        endif
        count += 1
    endwhile

    newPlayer.SetRestrained(true)
    newPlayer.MoveTo(newPlayer, 0, 0, 0, true) ; Releases the player's movement
    newPlayer.ForceAV("Blindness", 10000000000000)
    newPlayer.ForceAV("ShoutRecoveryMult", 0)
    newPlayer.ForceAV("VoiceRate", 10000000000000)
    newPlayer.ForceAV("VoicePoints", 10000000000000)
    newPlayer.ForceAV("Health", 10000000000000)
    newPlayer.ForceAV("Magicka", 10000000000000)
    newPlayer.ForceAV("Stamina", 10000000000000)

    SkyUtilitiesScript.InitializeNewPlayer(newPlayer.GetFormID(), movementController.GetFormID(), targetController.GetFormID(), guid as int)

    newPlayer.SetAnimationVariableBool("bAllowRotation", false)
    ; Ensures that the actor always responds to movement requests.If this is on, the actor will sometimes refuse to walk.
    newPlayer.SetAnimationVariableBool("bMotionDriven", false)
    newPlayer.SetAnimationVariableBool("bAnimationDriven", false)
    ; Keeps actor from turning their body to face someone, does not actually affect head tracking.
    newPlayer.SetAnimationVariableBool("bHeadTracking", false)
    newPlayer.SetAnimationVariableBool("bHeadTrackSpine", false)

    Debug.SendAnimationEvent(newPlayer, "IdleForceDefaultState")

    return newPlayer
EndFunction

Function SetTOD(string[] values)
    if (values[0] as float) != gameDaysPassed
        gameDaysPassed = (values[0] as float)
        (Game.GetForm(0x00000039) as GlobalVariable).SetValue(gameDaysPassed)
    endif

    if (values[1] as float) != gameDay
        gameDay = (values[1] as float)
        (Game.GetForm(0x00000037) as GlobalVariable).SetValue(gameDay)
    endif

    if (values[2] as float) != gameHour
        gameHour = (values[2] as float)
        (Game.GetForm(0x00000038) as GlobalVariable).SetValue(gameHour)
    endif

    if (values[3] as float) != gameMonth
        gameMonth = (values[3] as float)
        (Game.GetForm(0x00000036) as GlobalVariable).SetValue(gameMonth)
    endif

    if (values[4] as float) != gameYear
        gameYear = (values[4] as float)
        (Game.GetForm(0x00000035) as GlobalVariable).SetValue(gameYear)
    endif
EndFunction

Function ReadMessages()
    if SkyUtilitiesScript.GetMessageCount() <= 0
        return
    endif

    string msg = SkyUtilitiesScript.ReadMessageName()
    Actor ref = SkyUtilitiesScript.ReadMessageRef() as Actor
    tGuid = SkyUtilitiesScript.ReadMessageGuid()
    string[] values = SkyUtilitiesScript.ReadMessageValues() ;This call pops the message off the stack, and moves on to the next message

    ProcessMessage(msg, ref, values)
EndFunction

;Values needs to be altered, as we have offset everything by one in the papyrus code
Function ProcessMessage(string msg, Actor ref, string[] values)
    if ref == None
        Debug.Trace("Actor is None for: " + msg)

        int count = 0

        while count < values.Length
            Debug.Trace("Values " + count + ": " + values[count])
            count += 1
        endwhile

        return
    endif

    refId = ref.GetFormId()

    if msg == "StartSneaking"
        SneakStart(ref)
    elseif msg == "StopSneaking"
        SneakStop(ref)
    elseif msg == "StartJumping"
        StartJumping(ref)
    elseif msg == "StopJumping"
        StopJumping(ref)
    elseif msg == "StartSheath"
        StartSheath(ref)
    elseif msg == "AttackRight"
       AttackRight(ref)
    elseif msg == "AttackLeft"
       AttackLeft(ref)
    elseif msg == "actorValues"
        SetActorValues(ref, values)
    elseif msg == "AttackDual"
        AttackDual(ref)
    elseif msg == "InterruptCast"
        InterruptCastAll(ref)
    elseif msg == "InterruptCastLeft"
        InterruptCastLeft(ref)
    elseif msg == "InterruptCastRight"
        InterruptCastRight(ref)
    elseif msg == "CastLeft"
        CastLeft(ref, values)
    elseif msg == "CastRight"
        CastRight(ref, values)
    elseif msg == "DualCast"
        DualCast(ref, values)
    elseif msg == "BlockStart"
        BlockStart(ref)
    elseif msg == "BlockStop"
        BlockStop(ref)
    elseif msg == "BowAttack"
        BowAttack(ref)
    elseif msg == "FireBow"
        FireBow(ref)
    elseif msg == "BashAttack"
        BashAttack(ref)
    elseif msg == "ShoutRelease"
        ShoutRelease(ref, values)
    elseif msg == "MeleeDamage"
        ForceAV(ref, "MeleeDamage", values)
    elseif msg == "UnarmedDamage"
        ForceAV(ref, "UnarmedDamage", values)
    elseif msg == "Paralysis"
        ForceAV(ref, "Paralysis", values)
    elseif msg == "Invisibility"
        ForceAV(ref, "Invisibility", values)
    elseif msg == "WaterBreathing"
        ForceAV(ref, "WaterBreathing", values)
    elseif msg == "WaterWalking"
        ForceAV(ref, "WaterWalking", values)
    elseif msg == "AttackDamageMult"
        ForceAV(ref, "AttackDamageMult", values)
    elseif msg == "SpeedMult"
        ForceAV(ref, "SpeedMult", values)
    elseif msg == "DamageResist"
        ForceAV(ref, "DamageResist", values)
    elseif msg == "MagicResist"
        ForceAV(ref, "MagicResist", values)
    elseif msg == "Equip"
        Equip(ref, values)
    elseif msg == "Unlock"
        Unlock(ref, values)
    elseif msg == "Lock"
        StartLock(ref, values)
    elseif msg == "EquipSpell"
        EquipSpell(ref, values)
    elseif msg == "RemoveSpellLeft"
        RemoveSpellLeft(ref)
    elseif msg == "RemoveSpellRight"
        RemoveSpellRight(ref)
    elseif msg == "EquipShout"
        EquipShout(ref, values)
    elseif msg == "SetBrow" || msg == "SetScar" || msg == "SetBeard" || msg == "SetHair" || msg == "SetEyes" || msg == "SetHead" || msg == "SetMouth"
        SetHeadPart(ref, values)
    elseif msg == "ActivateButton"
        ActivateButton(ref, values)
    elseif msg == "ActivateAnimation"
        ActivateAnimation(ref, values)
    elseif msg == "SetStage"
        SetStage(ref, values)
    elseif msg == "RideHorse"
        RideHorse(ref, values)
    elseif msg == "DismountHorse"
        DismountHorse(ref)
    elseif msg == "Getup"
        Getup(ref)
    elseif msg == "DropItem"
        DropItem(ref, values)
    elseif msg == "DeathFlag"
        Death(ref, values)
    elseif msg == "Weather"
        SetWeather(ref, values)
    elseif msg == "SpawnPlayer"
        SpawnPlayer(ref, values)
    elseif msg == "TOD"
        SetTOD(values)
    elseif msg == "KOFA"
        KOFA(ref, values)
    elseif msg == "uPos"
        UpdatePlayerPosition(ref, values)
    endif
EndFunction