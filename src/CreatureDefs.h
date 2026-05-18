#pragma once

//////////////////////////////////////////////////////////////////////////

class CreatureController;
class CreatureManager;
class Creature;

class CreatureActivity_None;
class CreatureActivity_GoTo;
class CreatureActivity_GoToBed;
class CreatureActivity_GoToFood;
class CreatureActivity_Idle;
class CreatureActivity_Explore;
class CreatureActivity_Sleep;
class CreatureActivity_Eat;

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
    eCreatureJob_Sleep,
    eCreatureJob_Eat,
    eCreatureJob_Research,
    eCreatureJob_Train,
    eCreatureJob_Manufacture,
    eCreatureJob_Guard,
    eCreatureJob_Torture,
    eCreatureJob_Pray,
    eCreatureJob_Drink,
    eCreatureJob_Leave,
    eCreatureJob_DestroyEnemyRooms,
    eCreatureJob_DestroyWalls,
    eCreatureJob_StealGold,
    eCreatureJob_StealSpells,
    eCreatureJob_StealManufactureCrates,
    eCreatureJob_StealEnemyGold,
    eCreatureJob_Sulk,
    eCreatureJob_Rebel,
    eCreatureJob_KillCreatures,
    eCreatureJob_KillPlayer,
    eCreatureJob_Tunnelling,
    eCreatureJob_Wait,
    eCreatureJob_SendToActionPoint,
    eCreatureJob_Explore,
    eCreatureJob_CombatPitSpectate,
    eCreatureJob_JailBreak,

    eCreatureJob_COUNT
};

//////////////////////////////////////////////////////////////////////////

using CreatureTypeId = unsigned int;
enum : CreatureTypeId
{
    CreatureTypeId_Null = 0, // not valid id
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

enum eCreatureActivity
{
    eCreatureActivity_None,
    eCreatureActivity_GoToLocation,
    eCreatureActivity_GoToBed,
    eCreatureActivity_GoToFood,
    eCreatureActivity_Idle,
    eCreatureActivity_Explore,
    eCreatureActivity_Sleep,
    eCreatureActivity_Eat,
};

//////////////////////////////////////////////////////////////////////////

// activity runtime status
enum eCreatureActivityStatus: unsigned char
{
    eCreatureActivityStatus_Init, // initial state
    eCreatureActivityStatus_Running, // in progress
    eCreatureActivityStatus_Cancelling, // interruption requested
    eCreatureActivityStatus_Finished, // completed
};

// activity completion result
enum eCreatureActivityResult: unsigned char
{
    eCreatureActivityResult_None, // activity was not completed
    eCreatureActivityResult_Success,
    eCreatureActivityResult_Failed,
    eCreatureActivityResult_Cancelled
};

//////////////////////////////////////////////////////////////////////////

