#pragma once

//////////////////////////////////////////////////////////////////////////

class Player;
class EconomyService;

//////////////////////////////////////////////////////////////////////////

// Possible player identifiers
enum ePlayerID : unsigned int
{
    ePlayerID_Null, // not an identifier
    ePlayerID_Good,
    ePlayerID_Neutral,
    ePlayerID_Keeper1,
    ePlayerID_Keeper2,
    ePlayerID_Keeper3,
    ePlayerID_Keeper4,
    ePlayerID_Keeper5,
    ePlayerID_Keeper6,
    ePlayerID_Keeper7,
    ePlayerID_Keeper8,
    ePlayerID_COUNT
};

enum_serialize_decl(ePlayerID);

//////////////////////////////////////////////////////////////////////////

enum ePlayerType : unsigned int
{
    ePlayerType_Null, // nothing
    ePlayerType_Human,
    ePlayerType_AI,
    ePlayerType_COUNT
};

enum_serialize_decl(ePlayerType);

//////////////////////////////////////////////////////////////////////////

enum eComputerAI 
{
    eComputerAI_MasterKeeper,
    eComputerAI_Conqueror,
    eComputerAI_Psychotic,
    eComputerAI_Stalwart,
    eComputerAI_Greyman,
    eComputerAI_Idiot,
    eComputerAI_Guardian,
    eComputerAI_ThickSkinned,
    eComputerAI_Paranoid,
    eComputerAI_COUNT
};

//////////////////////////////////////////////////////////////////////////

// AI player preferences
struct ComputerAIPreferences
{
    // stuff
};