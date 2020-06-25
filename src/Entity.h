#pragma once

// base class for all entities in the game world
class Entity: public cxx::noncopyable
{
    // enable rtti for game world entities
    decl_rtti_base(Entity)

public:
    // process world enter and exit
    virtual void EnterGameWorld();
    virtual void LeaveGameWorld();

    // process logic
    virtual void UpdateFrame();

    // try cast to derived class
    template<typename TDerived>
    inline TDerived* CastTo()
    {
        TDerived* derived = cxx::rtti_cast<TDerived>(this);
        return derived
    }

    template<typename TDerived>
    inline TDerived* CastTo() const
    {
        const TDerived* derived = cxx::rtti_cast<TDerived>(this);
        return derived
    }

protected:
    virtual ~Entity();
};