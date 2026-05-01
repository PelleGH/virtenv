#include "EntityManager.h"

Entity EntityManager::createEntity(){
    //New entity with unique ID
    std::uint32_t index;

    //Checks if possible to reuse an old ID
    if (!oldIndex.empty()){
        index = oldIndex.front();
        oldIndex.pop();

    }else{
        //Creates space for new ID
        index = nextIndex;
        nextIndex = nextIndex + 1;
        generations.push_back(0);
    }

    activeEntities = activeEntities + 1;

    //Creates the new ID
    std::uint32_t currentGeneration = generations[index];
    std::uint32_t newID = (currentGeneration << 20) + index;

    return Entity {newID};
}

void EntityManager::destroyEntityQueue(Entity e){
    entityToDestroy.push(e);
}

void EntityManager::destroyEntity(){

    while (!entityToDestroy.empty()){

        //Gets the first entity and remove it
        Entity e = entityToDestroy.front();
        entityToDestroy.pop();

        //Possible future "security check"
        if (!isAlive(e)){
            continue;
        }

        std::uint32_t index = e.getIndex();

        //Increase generation for index (in order to reuse)
        generations[index] = (generations[index] + 1) & 0xFFF;

        //Add to queue for old index to be reuse
        oldIndex.push(index);

        activeEntities = activeEntities - 1;
    }
}

bool EntityManager::isAlive(Entity e) const{
    std::uint32_t index = e.getIndex();

    //Security checks so that the ID is valid
    if (index >= generations.size()){
        return false;
    }

    return generations[index] == e.getGeneration();
}

std::uint32_t EntityManager::getActiveEntities() const{
    return activeEntities;
}
