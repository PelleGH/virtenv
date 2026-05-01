#pragma once

#include <vector>
#include <queue>
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

class EntityManager{
    private:
        //Current generation (version) of index (ID)
        std::vector <std::uint32_t> generations;

        //Queue for entities that dies during game
        std::queue <Entity> entityToDestroy;

        //Queue from old index that is ready to be reuse
        std::queue <std::uint32_t> oldIndex;

        //Next index to be given 
        std::uint32_t nextIndex = 0; 

        //Counter for active entities
        std::uint32_t activeEntities = 0;

    public:
        Entity createEntity();

        //Add entities that dies during game
        void destroyEntityQueue(Entity e);

        //Executes at the end
        void destroyEntity();

        //Checks if entity is alive
        bool isAlive(Entity e) const;

        std::uint32_t getActiveEntities() const;
};