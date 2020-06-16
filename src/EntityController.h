#pragma once

// base class of entity controllers
class EntityController: public cxx::noncopyable
{
    decl_rtti_base(EntityController)

public:
    virtual ~EntityController();

};