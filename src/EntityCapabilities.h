#pragma once

//////////////////////////////////////////////////////////////////////////

class EntityCapability { public: virtual ~EntityCapability() {} };

//////////////////////////////////////////////////////////////////////////

// entity acts as gold container
// usage: game object
class GoldContainerCapability: public EntityCapability
{
public:
    virtual long GetStoredGoldAmount() const = 0;
    virtual long GetStoredGoldCapacity() const = 0; // returns 0 if capacity is unlimited

    // changes amount of gold in the container
    // typically not used directly by anyone other than the Parent Room Controller,
    // as it may cause resource synchronization issues
    virtual long StoreGold(long goldAmount) = 0; // returns amount of gold actually stored
    virtual long DisposeGold(long goldAmount) = 0; // returns amount of gold actually disposed
};

//////////////////////////////////////////////////////////////////////////

class MoneyStorageRoomCapability: public EntityCapability
{
public:
    virtual long GetStoredGoldAmount() const = 0;
    virtual long GetStoredGoldCapacity() const = 0; // returns 0 if capacity is unlimited
    virtual long StoreGold(long goldAmount) = 0; // returns amount of gold actually stored
    virtual long DisposeGold(long goldAmount) = 0; // returns amount of gold actually disposed
    virtual long StoreGold(long goldAmount, const Point2D& tileLocation) = 0;
    virtual bool GetTileToStoreGold(Point2D& tileLocation) = 0;
};

//////////////////////////////////////////////////////////////////////////

class ObjectStorageRoomCapability: public EntityCapability
{
public:
    virtual int GetStoredObjectsCount() = 0;
    virtual int GetStoredObjectsCapacity() = 0;
};

//////////////////////////////////////////////////////////////////////////