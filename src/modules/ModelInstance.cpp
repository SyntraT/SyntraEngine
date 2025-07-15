#include "Syngine/modules/ModelInstance.hpp"

#include "Syngine/modules/MeshInstance.hpp"
#include "Syngine/modules/Model.hpp"
#include "Syngine/modules/Scene.hpp"
#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/modules/Shader.hpp"
#include "Syngine/world/WorldObject.hpp"
#include <Syngine/modules/Mesh.hpp>
#include <Syngine/utils/GameUtils.hpp>

ModelInstance::ModelInstance(Model* model, Coordination coords) : model(model) {
    setTransform(coords.getTransform());

    for (auto& pair : model->meshes) {
        meshInstances.emplace(pair.first, MeshInstance(pair.second));
    }
}

bool ModelInstance::shouldDiscard(Scene_T snapshot, const glm::mat4& transform) {
    for (auto& meshInstance : meshInstances) {
        MeshInstance& ref = meshInstance.second;
        glm::mat4 worldTransform = transform * ref.getTransform();
        if (!ref.shouldDiscard(snapshot, worldTransform)) {
            return false;
        }
    }
    return true;
}

void ModelInstance::renderDV(Scene_T snapshot, Shader shader, Screenbuffer screen) {
    if (!model->loaded) {
        return;
    }
    if (model->renderable_meshes.empty()) {
        for (auto& pair : meshInstances) {
            MeshInstance& ref = pair.second;
            if (!ref.shouldDiscard(snapshot, transform * ref.getTransform())) {
                ref.render(shader, screen);
            }
        }
        return;
    }
    for (const std::string& meshName : model->renderable_meshes) {
        auto pair = meshInstances.find(meshName);

        if (pair != meshInstances.end()) {
            MeshInstance& ref = pair->second;

            if (!ref.shouldDiscard(snapshot, transform * ref.getTransform())) {
                ref.render(shader, screen);
            }
        }
    }
}

void ModelInstance::render(Shader shader, Screenbuffer screen) {
    if (!model->loaded) {
        return;
    }
    if (model->renderable_meshes.empty()) {
        for (auto& pair : meshInstances) {
            MeshInstance& ref = pair.second;
            ref.render(shader, screen);
        }
        return;
    }
    for (const std::string& meshName : model->renderable_meshes) {
        auto pair = meshInstances.find(meshName);

        if (pair != meshInstances.end()) {
            MeshInstance& ref = pair->second;
            ref.render(shader, screen);
        }
    }
}

Model* ModelInstance::getModel() {
    return this->model;
}