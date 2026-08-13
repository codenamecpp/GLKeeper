#pragma once

//////////////////////////////////////////////////////////////////////////

class CreatureController;
using CreatureControllerPtr = cxx::uniqueptr<CreatureController>;

class CreatureManager;
class Creature;
class CreatureTaskManager;

class CreatureTask;
using CreatureTaskPtr = cxx::uniqueptr<CreatureTask>;

class CreatureState;
using CreatureStatePtr = cxx::uniqueptr<CreatureState>;

class CreatureAction;
using CreatureActionPtr = cxx::uniqueptr<CreatureAction>;

//////////////////////////////////////////////////////////////////////////

enum eCreatureFightStyle
{
    eCreatureFightStyle_NonFighter,
    eCreatureFightStyle_Blitzer, // Dark Angel, Mistress, Skeleton, Vampire
    eCreatureFightStyle_Support, // Dark Elf, Warlock, Elven Archer, Wizard
    eCreatureFightStyle_Blocker, // Bile Demon, Black Knight, Maiden, Troll, Giant, Guard, Knight, Royal Guard
    eCreatureFightStyle_Flanker
};

enum eCreatureJobClass
{
    eCreatureJobClass_Worker,
    eCreatureJobClass_Thinker, // Warlocks, Vampires, Maidens, Monks, Wizards
    eCreatureJobClass_Fighter,
    eCreatureJobClass_Scout,
};

enum eCreatureJob
{
    eCreatureJob_None,

    eCreatureJob_Wander,
    eCreatureJob_Sleep,
    eCreatureJob_Eat,
    eCreatureJob_Research,
    eCreatureJob_Train,
    eCreatureJob_Manufacture,
    eCreatureJob_Guard,
    eCreatureJob_Flee,
    eCreatureJob_Torture, // torture a captive
    eCreatureJob_TortureVolunteer,
    eCreatureJob_Pray,
    eCreatureJob_Drink,
    eCreatureJob_Leave,
    eCreatureJob_DestroyEnemyRooms,
    eCreatureJob_DestroyWalls,
    eCreatureJob_StealGold,
    eCreatureJob_StealSpells,
    eCreatureJob_StealCrates, // worker job?
    eCreatureJob_StealEnemyGold,
    eCreatureJob_Sulk,
    eCreatureJob_Rebel,
    eCreatureJob_KillCreatures,
    eCreatureJob_KillPlayer,
    eCreatureJob_Wait,
    eCreatureJob_SendToActionPoint,
    eCreatureJob_Explore,
    eCreatureJob_CombatPitSpectate,
    eCreatureJob_CombatPitFight,
    eCreatureJob_JailBreak,
    eCreatureJob_Gamble,
    eCreatureJob_Dance,
    eCreatureJob_Celebrate,
    eCreatureJob_CallToArms,
    eCreatureJob_CollectWages,
    eCreatureJob_MakeHome,
    eCreatureJob_DisarmTraps,

    // worker jobs
    eCreatureJob_Dig,
    eCreatureJob_Mine,
    eCreatureJob_Claim,
    eCreatureJob_ReinforceWall,
    eCreatureJob_RepairWall,
    eCreatureJob_CarryCorpseToGraveyard,
    eCreatureJob_CarryEnemyToPrison,
    eCreatureJob_CarryFallenToLair,
    eCreatureJob_CarrySpecialToLibrary,
    eCreatureJob_CarryGoldToTreasury,
    eCreatureJob_CarryCrateToWorkshop,
    eCreatureJob_InstallTrap,
    eCreatureJob_InstallDoor,

    // add more

    eCreatureJob_COUNT
};

enum eCreatureState
{
    eCreatureState_None, // init state

    eCreatureState_Idle,
    eCreatureState_Working,
    eCreatureState_Stunned,
    eCreatureState_Frozen,
    eCreatureState_Unconscious,
    eCreatureState_Tortured,
    eCreatureState_Dead,
    eCreatureState_InHand,
    eCreatureState_InPrison,
    eCreatureState_Dropped,
    eCreatureState_Slapped,
    eCreatureState_GetUp,
    eCreatureState_EnteringDungeon,

    // add more

    eCreatureState_COUNT
};

enum eCreatureAction
{
    eCreatureAction_IdleStanding,
    eCreatureAction_Wander,
    eCreatureAction_WalkToPoint,
    eCreatureAction_FaceTarget,

    // worker actions
    eCreatureAction_Digging,
    eCreatureAction_Mining,
    eCreatureAction_CarryGoldToTreasury,
    eCreatureAction_ReinforceWall,
    eCreatureAction_ClaimFloor,

    // add more

    eCreatureAction_COUNT
};

//////////////////////////////////////////////////////////////////////////

using CreatureTypeId = unsigned int;
enum : CreatureTypeId
{
    CreatureTypeId_Null = 0, // not valid id
    CreatureTypeId_Imp = 1,
    // todo
    CreatureTypeId_COUNT 
};

//////////////////////////////////////////////////////////////////////////

enum CreatureAnimationID
{
    // primary animations count 36
    CreatureAnimation_Walk = 0,
	CreatureAnimation_Run,
    CreatureAnimation_Dragged_Pose,
    CreatureAnimation_Recoil_Forwards,
    CreatureAnimation_Melee,
    CreatureAnimation_Magic,
    CreatureAnimation_Die,
    CreatureAnimation_Happy,
    CreatureAnimation_Angry,
    CreatureAnimation_Stunned_Pose,
    CreatureAnimation_Swing,
    CreatureAnimation_Sleep_Pose,
    CreatureAnimation_Eat,
    CreatureAnimation_Research,
    CreatureAnimation_Null_NotUsed1,
    CreatureAnimation_Dejected_Pose,
    CreatureAnimation_Torture,
    CreatureAnimation_Null_NotUsed2,
    CreatureAnimation_Drink,
    CreatureAnimation_Idle1,
    CreatureAnimation_Recoil_Backwards,
    CreatureAnimation_Building, // bile demon
    CreatureAnimation_Pray,
    CreatureAnimation_Fallback,
    CreatureAnimation_Elec,
    CreatureAnimation_Electrocute,
    CreatureAnimation_Getup,
    CreatureAnimation_Dance,
    CreatureAnimation_Drunk1,
    CreatureAnimation_Entrance,
    CreatureAnimation_Idle2,
    CreatureAnimation_Special1,
    CreatureAnimation_Special2,
    CreatureAnimation_Drunk2,
    CreatureAnimation_Special3,
    CreatureAnimation_Null_NotUsed3,

    // additional animations
    CreatureAnimation_DrunkIdle,
    CreatureAnimation_Melee2,
    CreatureAnimation_Special4,
    CreatureAnimation_Special5,
    CreatureAnimation_Special6,
    CreatureAnimation_Special7,
    CreatureAnimation_Special8,
    CreatureAnimation_WalkBack,
    CreatureAnimation_Pose_Frame,
    CreatureAnimation_Walk2,
    CreatureAnimation_Die_Pose,

    CreatureAnimation_COUNT,

    // special animations, remapped
    CreatureAnimation_Horny_GemIntro = 2,
    CreatureAnimation_Horny_Footstamp = 5,
    CreatureAnimation_Horny_Melee2 = 12,
    CreatureAnimation_Horny_Melee3 = 13,
    CreatureAnimation_Horny_Roar1 = 19,
    CreatureAnimation_Horny_PickUpGem = 22,
    CreatureAnimation_Horny_StampOnChicken = 24,
    CreatureAnimation_Horny_Roar2 = CreatureAnimation_Special3,
    CreatureAnimation_Imp_Dig = 4,
    CreatureAnimation_Imp_ClaimLand = 11,
    CreatureAnimation_Imp_ClaimFloor = 12,
    CreatureAnimation_Imp_Drag = 18,
    CreatureAnimation_Imp_Jump1 = CreatureAnimation_Special1,
    CreatureAnimation_Imp_Jump2 = CreatureAnimation_Special3,
    CreatureAnimation_Imp_Idle3 = CreatureAnimation_Special4,
    CreatureAnimation_Imp_Idle4 = CreatureAnimation_Special5,
    CreatureAnimation_Imp_Idle5 = CreatureAnimation_Special6,
    CreatureAnimation_Imp_Idle6 = CreatureAnimation_Special7,
    CreatureAnimation_Imp_Dig2 = CreatureAnimation_Special8,
    CreatureAnimation_Dwarf_ClaimWall = 11,
    CreatureAnimation_Dwarf_ClaimLand = 12,
    CreatureAnimation_Dwarf_Drag = 18,
    CreatureAnimation_Vampire_IntoBat = CreatureAnimation_Special1,
    CreatureAnimation_Vampire_EatCorpse1 = CreatureAnimation_Special2,
    CreatureAnimation_Vampire_EatCorpse2 = CreatureAnimation_Special3,
    CreatureAnimation_Mistress_Whip = CreatureAnimation_Special1,
    CreatureAnimation_Skeleton_Attention = CreatureAnimation_Special1,
    CreatureAnimation_DarkAngel_Aggressive = CreatureAnimation_Special1,
    CreatureAnimation_Knight_Happy = CreatureAnimation_Special3,
    CreatureAnimation_Thief_Loot = CreatureAnimation_Special3,
    CreatureAnimation_Rogue_Loot = CreatureAnimation_Special3,
};

enum_serialize_decl(CreatureAnimationID);

//////////////////////////////////////////////////////////////////////////

// unique identifier of creature task
using CreatureTaskUid = uint64_t;

//////////////////////////////////////////////////////////////////////////

