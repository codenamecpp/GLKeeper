#pragma once

//////////////////////////////////////////////////////////////////////////

enum eMapInteractionMode
{
    eMapInteractionMode_Free, // can tag terrain, pick creatures, interact with objects
    eMapInteractionMode_CastSpell, // can cast spells
    eMapInteractionMode_Build, // can tag terrain, construct rooms
    eMapInteractionMode_Sell, // can sell rooms
    eMapInteractionMode_PlaceTrap, // can place traps
};

//////////////////////////////////////////////////////////////////////////

struct GameplayCameraInfo
{
    glm::vec2 mPosition {}; // world
    float mHeight {}; // zoom
    float mRotation {}; // angle degrees
};

//////////////////////////////////////////////////////////////////////////