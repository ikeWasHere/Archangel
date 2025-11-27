#pragma once

#include "Components.hpp"
#include <string>
#include <tuple>

class EntityManager;

using ComponentTuple = std::tuple<
    CTransform,
    CShape,
    CCollision,
    CInput,
    CScore,
    CLifespan>;

class Entity
{
    friend class EntityManager;

    ComponentTuple m_components;
    bool m_alive = true;
    std::string m_tag = "default";
    size_t m_id = 0;

public:
    Entity() = default;

    template <typename T, typename... Args>
    T &add(Args &&...args)
    {
        auto &component = std::get<T>(m_components);
        component = T(std::forward<Args>(args)...);
        component.exists = true;
        return component;
    }

    template <typename T>
    T &get()
    {
        return std::get<T>(m_components);
    }

    template <typename T>
    bool has() const
    {
        return std::get<T>(m_components).exists;
    }

    size_t id() const
    {
        return m_id;
    }

    bool isAlive() const
    {
        return m_alive;
    }

    void destroy()
    {
        m_alive = false;
    }

    const std::string &tag() const
    {
        return m_tag;
    }
};
