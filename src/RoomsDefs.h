#pragma once

// known room class names
#define ROOM_NAME_DUNGEON_HEART "Dungeon Heart"
#define ROOM_NAME_TEMPLE "Temple"
#define ROOM_NAME_TREASURY "Treasury"
#define ROOM_NAME_LAIR "Lair"
#define ROOM_NAME_PORTAL "Portal"
#define ROOM_NAME_HATCHERY "Hatchery"
#define ROOM_NAME_LIBRARY "Library"
#define ROOM_NAME_TRAINING_ROOM "Training Room"
#define ROOM_NAME_WOODEN_BRIDGE "Wooden Bridge"
#define ROOM_NAME_GUARD_ROOM "Guard Room"
#define ROOM_NAME_WORKSHOP "Work Shop"
#define ROOM_NAME_PRISON "Prison"
#define ROOM_NAME_TORTURE_CHAMBER "Torture Chamber"
#define ROOM_NAME_GRAVEYARD "Graveyard"
#define ROOM_NAME_CASINO "Casino"
#define ROOM_NAME_COMBAT_PIT "Combat Pit"
#define ROOM_NAME_STONE_BRIDGE "Stone Bridge"
#define ROOM_NAME_HERO_GATE_FINAL "Hero Gate [ Final ]"
#define ROOM_NAME_HERO_GATE_2X2 "Hero Gate [ 2x2 ]"
#define ROOM_NAME_HERO_GATE_FRONTEND "Hero Gate [ FrontEnd ]"
#define ROOM_NAME_HERO_STONE_BRIDGE "Hero Stone Bridge"
#define ROOM_NAME_HERO_GATE_3X1 "Hero Gate [ 3x1 ]"
#define ROOM_NAME_HERO_PORTAL "Hero Portal"

// forwards
class GenericRoom;
class WallSection;

// weak pointer to room instance
using RoomHandle = cxx::handle<GenericRoom>;

// room instance unique identifier
using RoomInstanceID = unsigned long long;