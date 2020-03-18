#pragma once

enum eCreatureFightStyle
{
    eCreatureFightStyle_NonFighter,

    // Blitzers are ideally suited to offensive strikes. When confronted with enemy troops, 
    // Blitzers storm towards the front line in an attempt to break through and reach the 
    // enemy support creatures. Once the enemy support has been disabled, they turn their 
    // attentions back to the other creatures.
    eCreatureFightStyle_Blitzer, // Dark Angel, Mistress, Skeleton, Vampire

    // Support creatures are generally unsuited to physical, hand-to-hand combat, but their ranged
    // attacks are ideally suited to providing cover and support to the Blitzers and Flankers.
    eCreatureFightStyle_Support, // Dark Elf, Warlock, Elven Archer, Wizard

    // Blockers are ideal for defending vital areas of your dungeon. In combat situations, 
    // they don’t storm in on the offensive, but rather hold their position in an attempt 
    // to prevent enemy troops from gaining ground.
    eCreatureFightStyle_Blocker, // Bile Demon, Black Knight, Maiden, Troll, Giant, Guard, Knight, Royal Guard
                
    // Flankers attempt to position themselves behind the enemy targets, where they can then 
    // let rip and inflict the maximum damage possible.
    eCreatureFightStyle_Flanker,
};

enum eCreatureJobClass
{
    // Thinker is the job class of Warlocks, Vampires, Maidens, Monks, and Wizards. 
    // It indicates that these creatures' first priority (after food, wages, and sleep) is going to a Library 
    // and researching spells until there is nothing left to research.
    eCreatureJobClass_Thinker,

    // Fighter is the job class of most of the creatures - that is to say, every creature that is not a Thinker, 
    // a Worker, or a Scout. It indicates that fighting is what these creatures do best, even if they take it 
    // upon themselves to do other things when the need arises; for example, Dark Angels will research if there 
    // is still research to be done, and Bile Demons and Giants will usually manufacture Traps and Doors if there 
    // are any unfilled blueprints, but all of these creatures are still classified as Fighters.
    eCreatureJobClass_Fighter,

    // Worker
    eCreatureJobClass_Worker,

    // Scout
    eCreatureJobClass_Scout,
};

enum eCreatureAnimationID
{
    // primary animations count 36
    eCreatureAnimation_Walk,
    eCreatureAnimation_Run,
    eCreatureAnimation_Dragged_Pose,
    eCreatureAnimation_Recoil_Forwards,
    eCreatureAnimation_Melee,
    eCreatureAnimation_Magic,
    eCreatureAnimation_Die,
    eCreatureAnimation_Happy,
    eCreatureAnimation_Angry,
    eCreatureAnimation_Stunned_Pose,
    eCreatureAnimation_Swing,
    eCreatureAnimation_Sleep_Pose,
    eCreatureAnimation_Eat,
    eCreatureAnimation_Research,
    eCreatureAnimation_Null_NotUsed1,
    eCreatureAnimation_Null_NotUsed2,
    eCreatureAnimation_Torture,
    eCreatureAnimation_Null_NotUsed3,
    eCreatureAnimation_Drink,
    eCreatureAnimation_Idle1,
    eCreatureAnimation_Recoil_Backwards,
    eCreatureAnimation_Building,
    eCreatureAnimation_Pray,
    eCreatureAnimation_Fallback,
    eCreatureAnimation_Elec,
    eCreatureAnimation_Electrocute,
    eCreatureAnimation_Getup,
    eCreatureAnimation_Dance,
    eCreatureAnimation_Drunk1,
    eCreatureAnimation_Entrance,
    eCreatureAnimation_Idle2,
    eCreatureAnimation_Special1,
    eCreatureAnimation_Special2,
    eCreatureAnimation_Drunk2,
    eCreatureAnimation_Special3,
    eCreatureAnimation_Null_NotUsed4,

    // additional animations
    eCreatureAnimation_DrunkIdle,
    eCreatureAnimation_Melee2,
    eCreatureAnimation_Special4,
    eCreatureAnimation_Special5,
    eCreatureAnimation_Special6,
    eCreatureAnimation_Special7,
    eCreatureAnimation_Special8,
    eCreatureAnimation_WalkBack,
    eCreatureAnimation_Pose_Frame,
    eCreatureAnimation_Walk2,
    eCreatureAnimation_Death_Pose,

    eCreatureAnimation_COUNT,

    // special animations, remapped
    eCreatureAnimation_Horny_GemIntro       = 2,
    eCreatureAnimation_Horny_Footstamp      = 5,
    eCreatureAnimation_Horny_Melee2         = 12,
    eCreatureAnimation_Horny_Melee3         = 13,
    eCreatureAnimation_Horny_Roar1          = 19,
    eCreatureAnimation_Horny_PickUpGem      = 22,
    eCreatureAnimation_Horny_StampOnChicken = 24,
    eCreatureAnimation_Horny_Roar2          = eCreatureAnimation_Special3,
    eCreatureAnimation_Imp_Dig              = 4,
    eCreatureAnimation_Imp_ClaimLand        = 11,
    eCreatureAnimation_Imp_ClaimFloor       = 12,
    eCreatureAnimation_Imp_Drag             = 18,
    eCreatureAnimation_Imp_Jump1            = eCreatureAnimation_Special1,
    eCreatureAnimation_Imp_Jump2            = eCreatureAnimation_Special3,
    eCreatureAnimation_Imp_Idle3            = eCreatureAnimation_Special4,
    eCreatureAnimation_Imp_Idle4            = eCreatureAnimation_Special5,
    eCreatureAnimation_Imp_Idle5            = eCreatureAnimation_Special6,
    eCreatureAnimation_Imp_Idle6            = eCreatureAnimation_Special7,
    eCreatureAnimation_Imp_Dig2             = eCreatureAnimation_Special8,
    eCreatureAnimation_Dwarf_ClaimWall      = 11,
    eCreatureAnimation_Dwarf_ClaimLand      = 12,
    eCreatureAnimation_Dwarf_Drag           = 18,
    eCreatureAnimation_Vampire_IntoBat      = eCreatureAnimation_Special1,
    eCreatureAnimation_Vampire_EatCorpse1   = eCreatureAnimation_Special2,
    eCreatureAnimation_Vampire_EatCorpse2   = eCreatureAnimation_Special3,
    eCreatureAnimation_Mistress_Whip        = eCreatureAnimation_Special1,
    eCreatureAnimation_Skeleton_Attention   = eCreatureAnimation_Special1,
    eCreatureAnimation_DarkAngel_Aggressive = eCreatureAnimation_Special1,
    eCreatureAnimation_Knight_Happy         = eCreatureAnimation_Special3,
    eCreatureAnimation_Thief_Loot           = eCreatureAnimation_Special3,
    eCreatureAnimation_Rogue_Loot           = eCreatureAnimation_Special3,
};