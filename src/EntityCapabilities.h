#pragma once

//////////////////////////////////////////////////////////////////////////

class IEntityCapability { public: virtual ~IEntityCapability() {} };

//////////////////////////////////////////////////////////////////////////

// entity acts as gold container
// usage: game object
class IGoldContainer: public IEntityCapability
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

class ITreasuryRoom: public IEntityCapability
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

class IStorageRoom: public IEntityCapability
{
public:
    virtual int GetStoredObjectsCount() = 0;
    virtual int GetStoredObjectsCapacity() = 0;
};

//////////////////////////////////////////////////////////////////////////

// entity acts as piece of campaign level selection table: level or arrow
class I3DMapTableLevel: public IEntityCapability
{
public:
    virtual void SelectLevel() = 0;
    virtual void DeselectLevel() = 0;
};

//////////////////////////////////////////////////////////////////////////

class I3DMapTableRoom: public IEntityCapability
{
public:
    virtual bool IsLevel(EntityHandle objectHandle) const = 0;
    virtual bool SelectLevel(EntityHandle objectHandle) = 0;
    virtual void DeselectLevel() = 0;
    virtual bool GetLevelKey(EntityHandle objectHandle, CampaignLevelKey& levelKey) const = 0;
};

//////////////////////////////////////////////////////////////////////////