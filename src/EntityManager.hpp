#pragma once
#include "Entity.hpp"

#include <map>
#include <memory>
#include <vector>

using EntityVec = std::vector<std::shared_ptr<Entity>>;
using EntityMap = std::map<std::string, EntityVec>;

class EntityManager
{
    EntityVec m_entities;
    EntityVec m_entitiesToAdd;
    EntityMap m_entityMap;
    size_t m_totalEntities = 0;

    void removeDeadEntities(EntityVec &vec)
    {
        vec.erase(std::remove_if(vec.begin(), vec.end(),
                            [](const std::shared_ptr<Entity> &e) 
                            { 
                                return !e->isAlive();    
                            }),
                  vec.end());
    }

  public:
    EntityManager() = default;

    void update()
    {
        // add all entites we want to add
        for (auto &e : m_entitiesToAdd)
        {
            m_entities.push_back(e);
            m_entityMap[e->m_tag].push_back(e);
        }

        m_entitiesToAdd.clear();

        removeDeadEntities(m_entities);

        // remove dead entities from each vector in the entity map
        for (auto &[tag, entityVec] : m_entityMap)
        {
            removeDeadEntities(entityVec);
        }
    }

    std::shared_ptr<Entity> addEntity(const std::string &tag)
    {
        auto e = std::make_shared<Entity>(tag, m_totalEntities++);
        m_entitiesToAdd.push_back(e);
        return e;
    }

    const EntityVec &getEntities() const
    {
        return m_entities;
    }

    const EntityVec &getEntities(const std::string &tag) const
    {
        static const EntityVec empty;
        auto it = m_entityMap.find(tag);
        return (it != m_entityMap.end()) ? it->second : empty;
    }

    const EntityMap &getEntityMap() const
    {
        return m_entityMap;
    }
};
