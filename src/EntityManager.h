#pragma once

class EntityManager: public cxx::noncopyable
{
public:
    // readonly
    std::vector<Entity*> mEntitiesList;

public:
    // one time initialization/shutdown routine
    bool Initialize();
    void Deinit();

    void EnterWorld();
    void ClearWorld();
    void UpdateFrame();

    // @param position: Position on scene
    // @param direction: Direction vector, must be normalized
    // @param scaling: Scaling
    Entity* CreateEntity(const glm::vec3& position, const glm::vec3& direction, float scaling);
    Entity* CreateEntity();

    // destroy entity immediately, pointer becomes invalid
    void DestroyEntity(Entity* entity);

private:
    void DestroyEntities();

private:
    cxx::object_pool<Entity> mEntitiesPool;
};

extern EntityManager gEntityManager;