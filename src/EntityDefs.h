#pragma once

//////////////////////////////////////////////////////////////////////////

class GameObject;
class Room;
class Entity;

struct EntityMsg;

//////////////////////////////////////////////////////////////////////////

enum eEntityType : uint8_t
{
    eEntityType_None,
    eEntityType_GameObject,
    eEntityType_Creature,
    eEntityType_Room,
    // add more

    eEntityType_COUNT
};

//////////////////////////////////////////////////////////////////////////

// unique identifier of entity instance within game world
using EntityUid = uint64_t;

//////////////////////////////////////////////////////////////////////////

union EntityHandle
{
public:
    EntityHandle()
        : mPacked(0)
    {
    }
    EntityHandle(eEntityType entType, uint32_t entSubType, uint32_t entGeneration, uint32_t entIndex)
        : mPacked(0)
    {
        mType = entType;
        mSubType = entSubType;
        mGeneration = entGeneration;
        mIndex = entIndex;
    }
    inline bool WasSet() const { return mType > eEntityType_None; }
    inline bool operator == (const EntityHandle& rhs) const { return mPacked == rhs.mPacked; }
    inline bool operator != (const EntityHandle& rhs) const { return mPacked != rhs.mPacked; }
    template<typename T> inline bool operator == (const T& rhs) const = delete;
    template<typename T> inline bool operator != (const T& rhs) const = delete;
    inline explicit operator bool () const
    {
        return WasSet();
    }
    inline eEntityType GetEntityType() const 
    { 
        return static_cast<eEntityType>(mType); 
    }
    // shortcuts
    inline bool IsGameObject() const { return mType == eEntityType_GameObject; }
    inline bool IsGameObject(uint32_t subType) const 
    { 
        return IsGameObject() && (mSubType == subType); 
    }

    inline bool IsCreature() const { return mType == eEntityType_Creature; }
    inline bool IsCreature(uint32_t subType) const 
    { 
        return IsCreature() && (mSubType == subType); 
    }

    inline bool IsRoom() const { return mType == eEntityType_Room; }
    inline bool IsRoom(uint32_t subType) const 
    { 
        return IsRoom() && (mSubType == subType);
    }

public:
    struct
    {
        uint64_t mType      :  8; // 256 unique values
        uint64_t mSubType   : 12; // 4.096 unique values
        uint64_t mGeneration: 24; // 16.777.216 unique values
        uint64_t mIndex     : 20; // 1.048.576 unique values
    };
    uint64_t mPacked;
};

static_assert(sizeof(EntityHandle) == sizeof(uint64_t), "EntityHandle size mismatch");

//////////////////////////////////////////////////////////////////////////

struct EntityTransform
{
public:
    EntityTransform() = default;
    EntityTransform(const glm::vec3& entPosition, cxx::angle_t entOrientation)
        : mPosition(entPosition)
        , mOrientation(entOrientation)
    {
    }
    inline glm::vec2 GetPosition2d() const { return {mPosition.x, mPosition.z}; }
public:
    glm::vec3 mPosition;
    cxx::angle_t mOrientation;
};

//////////////////////////////////////////////////////////////////////////

struct EntityFlags
{
public:
    EntityFlags()
        : mWasSpawned(false)
        , mWasDeleted(false)
        , mWasDespawned(false)
        , mIsUnplaced(false)
    {}
    bool mWasSpawned : 1; // whether entity was activated
    bool mWasDeleted : 1; // whether entity marked as pending deletion
    bool mWasDespawned : 1; // whether entity was completely removed from world
    bool mIsUnplaced : 1; // whether entity has no world placement (e.g., in hand)
};

//////////////////////////////////////////////////////////////////////////
