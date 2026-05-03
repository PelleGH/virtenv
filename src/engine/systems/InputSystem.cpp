#include "InputSystem.h"
#include "../scene/Scene.h"
#include "../ecs/Components.h"
#include "raylib.h"

void InputSystem::update(Scene& scene)
{
    ComponentStorage& components = scene.getComponentStorage();
    auto& playerInputs = components.GetComponents<PlayerInput>();

    for (auto& [entity, input] : playerInputs)
    {
        input.up = IsKeyDown(KEY_W);
        input.down = IsKeyDown(KEY_S);
        input.left = IsKeyDown(KEY_A);
        input.right = IsKeyDown(KEY_D);

        input.attack = IsKeyDown(KEY_SPACE);
        input.interact = IsKeyDown(KEY_E);
    }
}