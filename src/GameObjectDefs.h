#pragma once

//////////////////////////////////////////////////////////////////////////

class GameObject;
class GameObjectController;
class GameObjectManager;

//////////////////////////////////////////////////////////////////////////

using GameObjectClassId = unsigned int;
// well known object class ids
enum : GameObjectClassId 
{
    GameObjectClassId_Null = 0,
    GameObjectClassId_GoldPile = 1,
    GameObjectClassId_GoldBag = 2,
    GameObjectClassId_GoldChest = 3,
    GameObjectClassId_Chicken = 9,
    GameObjectClassId_DungeonHeart = 13,
    GameObjectClassId_Egg = 47,
    GameObjectClassId_TempleHand = 66,
    GameObjectClassId_TreasuryPillar = 76,
    GameObjectClassId_HatcheryPillar = 78,
    GameObjectClassId_PitPillar = 79,
    GameObjectClassId_WorkshopPillar = 80,
    GameObjectClassId_TorturePillar = 82,
    GameObjectClassId_GraveyardPillar = 84,
    GameObjectClassId_CasinoPillar = 85,
    GameObjectClassId_3DFrontEndBanner1 = 89,
    GameObjectClassId_TempleCandlestick = 111,
    GameObjectClassId_3DFrontEndBanner2 = 134,
    GameObjectClassId_3DFrontEndBanner3 = 135,
    GameObjectClassId_3DFrontEndBanner4 = 136,
    GameObjectClassId_3DFrontEndGemHolder = 131,
    GameObjectClassId_3DFrontEndChain = 132,

};

//////////////////////////////////////////////////////////////////////////

enum eGameObjectCategory
{
    eGameObjectCategory_Normal,
    eGameObjectCategory_Special,
    eGameObjectCategory_SpellBook,
    eGameObjectCategory_Crate,
    eGameObjectCategory_Lair,
    eGameObjectCategory_Gold,
    eGameObjectCategory_Food,
    eGameObjectCategory_LevelGem,
};
enum_serialize_decl(eGameObjectCategory);

//////////////////////////////////////////////////////////////////////////

// values are 1 to 1 mapped to dk objects init state
enum eGameObjectState
{
    eGameObjectState_None = 0,
    eGameObjectState_BeingDropped,
    eGameObjectState_BeingSlapped,
    eGameObjectState_PrisonArrive,
    eGameObjectState_TempleArrive,
    eGameObjectState_GoldArrive,
    eGameObjectState_HatcheryArrive,
    eGameObjectState_LairArrive,
    eGameObjectState_Pecking,
    eGameObjectState_MoveTo,
    eGameObjectState_Wander,
    eGameObjectState_WanderWithinPlayer,
    eGameObjectState_CreateCallToArms,
    eGameObjectState_CallToArms,
    eGameObjectState_ResendCallToArms,
    eGameObjectState_DestroyCallToArms,
    eGameObjectState_DoNothing,
    eGameObjectState_BoulderState,
    eGameObjectState_Spinning,
    eGameObjectState_DestroyCallToArmsCopy,
    eGameObjectState_DestroyCallToArmsCopyInit,
    eGameObjectState_Rotate,
    eGameObjectState_BeingPickedUp,
    eGameObjectState_Dodge,
    eGameObjectState_FreeChicken,
    eGameObjectState_EscapeFromFight,
    eGameObjectState_ActivateAnim,
    eGameObjectState_WorkshopArrive,
    eGameObjectState_Kicked,
    eGameObjectState_Shake,
    eGameObjectState_TortureWheelSpin,
    eGameObjectState_GoldProcess,
    eGameObjectState_Hatching,
    eGameObjectState_Flare,
    eGameObjectState_TombstoneScaleUp,
    eGameObjectState_TombstoneScaleDown,
    eGameObjectState_Special,
    eGameObjectState_HeartConstruction,
    eGameObjectState_Destructor,
    eGameObjectState_ManaVault,
    eGameObjectState_TortureCoals,
    eGameObjectState_ThreatBurn,
    eGameObjectState_IdleEffect,
    eGameObjectState_FrontendLevelGem,
    eGameObjectState_PrisonDoorOpen,
    eGameObjectState_PrisonDoorClose,
    eGameObjectState_OpenPrisonDoorBar,
    eGameObjectState_ClosePrisonDoorBar,
    eGameObjectState_PlayWinAnimIndicatorToSmiles,
    eGameObjectState_PlayWinAnimIndicatorToMoney,
    eGameObjectState_PortalGem,
    eGameObjectState_MyPetDungeonLevelIndicator,
    eGameObjectState_EntranceArrive,
    eGameObjectState_SpellBookIdle,
    eGameObjectState_MpdLevelUp,
    eGameObjectState_MpdLevelDown,
    eGameObjectState_JackInTheBoxState,
    eGameObjectState_CryptDancer,
    eGameObjectState_CryptSpecialDancer,
    eGameObjectState_GemChallengeJewel,
    eGameObjectState_Plinth,
    eGameObjectState_SpecialPlinth,
    eGameObjectState_DropOffPoint,
    eGameObjectState_GemExitPoint,

    eGameObjectState_MAX
};

//////////////////////////////////////////////////////////////////////////

enum eGameObjectMeshId
{
    eGameObjectMeshId_Main,
    eGameObjectMeshId_Additional1,
    eGameObjectMeshId_Additional2,
    eGameObjectMeshId_Additional3,
    eGameObjectMeshId_Additional4,
};

//////////////////////////////////////////////////////////////////////////
