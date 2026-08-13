#pragma once

//////////////////////////////////////////////////////////////////////////

class GameObject;
class Room;
class Entity;

struct EntityMsg;

//////////////////////////////////////////////////////////////////////////

enum eEntityType
{
    eEntityType_None,
    eEntityType_GameObject,
    eEntityType_Creature,
    eEntityType_Room,
    // add more
};

//////////////////////////////////////////////////////////////////////////

// unique identifier of entity instance within game world
using EntityUid = uint64_t;

//////////////////////////////////////////////////////////////////////////

union EntityHandle
{
public:
    EntityHandle() {}
    EntityHandle(eEntityType entType, uint32_t entGeneration, uint32_t entIndex)
        : mType(entType)
        , mGeneration(entGeneration)
        , mIndex(entIndex)
        , mFlags()
    {
    }
    inline bool WasSet() const { return mType > eEntityType_None; }
    inline bool operator == (const EntityHandle& rhs) const { return mPacked == rhs.mPacked; }
    inline bool operator != (const EntityHandle& rhs) const { return mPacked != rhs.mPacked; }
    template<typename T> inline bool operator == (const T& rhs) const = delete;
    template<typename T> inline bool operator != (const T& rhs) const = delete;
    inline operator bool () const
    {
        return WasSet();
    }
    inline eEntityType GetEntityType() const 
    { 
        return static_cast<eEntityType>(mType); 
    }
    // shortcuts
    inline bool IsGameObject()  const { return mType == eEntityType_GameObject; }
    inline bool IsCreature()    const { return mType == eEntityType_Creature; }
    inline bool IsRoom()        const { return mType == eEntityType_Room; }
public:
    struct
    {
        uint64_t mType      : 16; // 65.536 unique values
        uint64_t mGeneration: 24; // 16.777.216 unique values
        uint64_t mIndex     : 20; // 1.048.576 unique values
        uint64_t mFlags     : 4;
    };
    uint64_t mPacked = 0;
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

struct EntityLifecycleFlags
{
public:
    EntityLifecycleFlags()
        : mWasSpawned(false)
        , mWasDeleted(false)
        , mWasDespawned(false)
    {}
    unsigned char mWasSpawned : 1; // whether entity was activated
    unsigned char mWasDeleted : 1; // whether entity marked as pending deletion
    unsigned char mWasDespawned : 1; // whether entity was completely removed from world
};

//////////////////////////////////////////////////////////////////////////
