#include "pch.h"
#include "EntityComponent.h"
#include "Entity.h"

void EntityComponent::EnableComponent(bool isEnable)
{
    if (mComponentEnabled == isEnable)
        return;

    mComponentEnabled = isEnable;
    if (isEnable)
    {
        OnComponentEnabled();
    }
    else
    {
        OnComponentDisabled();
    }
}

void EntityComponent::Delete()
{
    mComponentDeleted = true;
}

void EntityComponent::OnComponentEnabled() 
{
    // do nothing
}

void EntityComponent::OnComponentDisabled() 
{
    // do nothing
}