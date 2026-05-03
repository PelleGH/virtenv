#pragma once

#include <cstdint>
#include <cstddef>
#include <functional>

struct Entity {
    std::uint32_t id;

    std::uint32_t getIndex() const {
        return id & 0xFFFFF;
    }

    std::uint32_t getGeneration() const {
        return (id >> 20) & 0xFFF;
    }

    bool operator==(const Entity& other) const {
        return id == other.id;
    }
};

// Specialize std::hash for Entity so it can be used in unordered containers in ComponentArray
namespace std {
    template <>
    struct hash<Entity> {
        std::size_t operator()(const Entity& entity) const {
            return std::hash<std::uint32_t>{}(entity.id);
        }
    };
}