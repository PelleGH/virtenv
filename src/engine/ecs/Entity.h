#pragma once
#include <cstdint>

struct Entity {
    std::uint32_t id;

    // id consists of 20 bit + 12 bit
    // The last 20 bits = index
    // The first 12 bits = generation (is used in order to reuse id:s)

    std::uint32_t getIndex() const{
        return id & 0xFFFFF;
    }

    std::uint32_t getGeneration() const{
        return (id >> 20) & 0xFFF;
    }

    // Possible future use to compare different entities
    bool operator == (const Entity& other) const{
        return id == other.id;
    }
};