#include "stdafx.h"
#include "CreatureActivityUtils.h"
#include "SimplePool.h"

cxx::uniqueptr<CreatureActivity> CreatureActivityUtils::Construct()
{
    static SimplePool<CreatureActivity> activitiesPool = {
        [](CreatureActivity* instance)
        {
            instance->emplace<CreatureActivity_None>();
        }};

    CreatureActivity* objectInstance = activitiesPool.Acquire();
    cxx_assert(objectInstance);

    return std::move(cxx::uniqueptr<CreatureActivity> (objectInstance, [](CreatureActivity* instance)
        {
            if (instance)
            {
                activitiesPool.Return(instance);
            }
        }));
}