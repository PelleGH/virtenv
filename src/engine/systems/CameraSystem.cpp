#include "CameraSystem.h"

#include "../scene/Scene.h"
#include "../ecs/Components.h"

void CameraSystem::init(Camera3D& camera)
{
    camera = { 0 };
    camera.position = { 0.0f, 5.0f, 5.0f };
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

void CameraSystem::update(Scene& scene, Camera3D& camera)
{
    const SceneData& data = scene.getData();

    if (data.camera.mode == "fixed")
    {
        camera.position = {
            data.camera.positionX,
            data.camera.positionY,
            data.camera.positionZ
        };

        camera.target = {
            data.camera.targetX,
            data.camera.targetY,
            data.camera.targetZ
        };

        return;
    }

    ComponentStorage& components = scene.getComponentStorage();
    auto& players = components.GetComponents<PlayerInput>();

    for (auto& [entity, input] : players)
    {
        if (!components.HasComponent<TransformComponent>(entity))
            continue;

        TransformComponent& transform =
            components.GetComponent<TransformComponent>(entity);

        camera.target = {
            transform.x,
            transform.y,
            transform.z
        };

        camera.position = {
            transform.x + data.camera.positionX,
            transform.y + data.camera.positionY,
            transform.z + data.camera.positionZ
        };

        return;
    }
}