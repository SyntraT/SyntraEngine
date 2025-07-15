#include "Syngine/modules/Scene.hpp"
#include "GLFW/glfw3.h"
#include "Syngine/Syngine.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/ShadowMapper.hpp"
#include "Syngine/modules/PointShadowMapper.hpp"
#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/modules/Shader.hpp"
#include "Syngine/world/WorldObject.hpp"
#include "Syngine/utils/GameUtils.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Syngine/engine/Config.hpp"

Scene::Scene(Camera* camera, GameWindow* window)
    : camera(camera),
      screenWidth(window->getWidth()),
      screenHeight(window->getHeight()),
      near(0.1f),
      far(100.0f),
      fieldOfView(65.0f) {
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, screenWidth, screenHeight);
    aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    updateProjection();
    setupShaders();
}

Scene::Scene(Camera* camera, float FOVDegrees, float near, float far, int width, int height)
    : camera(camera),
      screenWidth(width),
      screenHeight(height),
      near(near),
      far(far),
      fieldOfView(FOVDegrees) {
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, screenWidth, screenHeight);
    aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    updateProjection();
    setupShaders();
}

Scene::~Scene() {
    glfwSetWindowUserPointer(callbacksGLFWWindow, lastWindowUserData);
}

void Scene::setDirectionalLight(DirLight light) {
    this->dirLight = light;
    updateUniforms();
}

void Scene::setupShaders() {
    screenShader.init();
    screenShader.use();
    screenShader.setVec2f("uv", screenWidth, screenHeight);
    batchShader.init({
        {SHADER_BATCH_KEY_NR_POINT_LIGHTS, "0"},
        {SHADER_BATCH_KEY_NR_SPOT_LIGHTS, "0"},
        {SHADER_BATCH_KEY_HAS_SHADOWS, SHADER_VAL_OFF}
    });
    setDirectionalLight({});
}

void Scene::render(Screenbuffer screen) {
    camera->updateViewMatrix();

    if (shadowMapper && shadowMapper->isCreated()) {
        shadowMapper->renderDepth(screen, this);
        batchShader.use();
        batchShader.setMatrix4("lightSpaceMatrix", shadowMapper->getLightSpaceMatrix(), 1, GL_FALSE);
        batchShader.setFloat("shadowBiasMin", shadowMapper->biasMin);
        batchShader.setFloat("shadowBiasMax", shadowMapper->biasMax);
        batchShader.setFloat("shadowStrength", shadowMapper->strength);
        batchShader.setTexture("shadowMap", GL_TEXTURE_2D, 2, shadowMapper->getDepthMapTCB());
    }
    if (pointShadowMapper && pointShadowMapper->isCreated()) {
        pointShadowMapper->renderDepth(screen, this);
        batchShader.use();
        batchShader.setTexture("pshadowMap", GL_TEXTURE_2D_ARRAY, 3, pointShadowMapper->getDepthArrayTCB());
    }
    batchShader.use();
    batchShader.setMatrix4("view", camera->getViewMatrix(), 1, GL_FALSE);
    batchShader.setMatrix4("projection", projection, 1, GL_FALSE);
    batchShader.setVec3f("cameraPos", camera->getPosition());
    batchRenderTable->forEach([&](const std::string& key, ShaderRenderable* renderable) {
        GameUtils::renderDV(renderable, this, batchShader, screen);
    });
}

void Scene::withShadows(ShadowMapper* shadowMapper) {
    if (shadowMapper == nullptr) {
        batchShader.reloadProgram({
            {SHADER_BATCH_KEY_NR_POINT_LIGHTS, std::to_string(pointLights.size())},
            {SHADER_BATCH_KEY_NR_SPOT_LIGHTS, std::to_string(spotLights.size())},
            {SHADER_BATCH_KEY_HAS_SHADOWS, SHADER_VAL_OFF},
            {SHADER_BATCH_KEY_HAS_POINT_SHADOWS, hasPointShadows() ? SHADER_VAL_ON : SHADER_VAL_OFF}
        });
        updateUniforms();
        return;
    }
    if (!shadowMapper->isCreated()) return;
    this->shadowMapper = shadowMapper;
    batchShader.reloadProgram({
        {SHADER_BATCH_KEY_NR_POINT_LIGHTS, std::to_string(pointLights.size())},
        {SHADER_BATCH_KEY_NR_SPOT_LIGHTS, std::to_string(spotLights.size())},
        {SHADER_BATCH_KEY_HAS_SHADOWS, SHADER_VAL_ON},
        {SHADER_BATCH_KEY_HAS_POINT_SHADOWS, hasPointShadows() ? SHADER_VAL_ON : SHADER_VAL_OFF}
    });
    updateUniforms();
}

void Scene::withPointShadows(PointShadowMapper* pointShadowMapper) {
    if (pointShadowMapper == nullptr) {
        batchShader.reloadProgram({
            {SHADER_BATCH_KEY_NR_POINT_LIGHTS, std::to_string(pointLights.size())},
            {SHADER_BATCH_KEY_NR_SPOT_LIGHTS, std::to_string(spotLights.size())},
            {SHADER_BATCH_KEY_HAS_SHADOWS, hasShadows() ? SHADER_VAL_ON : SHADER_VAL_OFF},
            {SHADER_BATCH_KEY_HAS_POINT_SHADOWS, SHADER_VAL_OFF}
        });
        updateUniforms();
        return;
    }
    if (!pointShadowMapper->isCreated()) return;
    this->pointShadowMapper = pointShadowMapper;
    batchShader.reloadProgram({
        {SHADER_BATCH_KEY_NR_POINT_LIGHTS, std::to_string(pointLights.size())},
        {SHADER_BATCH_KEY_NR_SPOT_LIGHTS, std::to_string(spotLights.size())},
        {SHADER_BATCH_KEY_HAS_SHADOWS, hasShadows() ? SHADER_VAL_ON : SHADER_VAL_OFF},
        {SHADER_BATCH_KEY_HAS_POINT_SHADOWS, SHADER_VAL_ON}
    });
    updateUniforms();
}

void Scene::withCallbacks(GameWindow* window) {
    GLFWwindow* glfwWindow = window->getGLFWWindowPtr();

    lastWindowUserData = glfwGetWindowUserPointer(glfwWindow);

    WindowUserData* data = new WindowUserData();
    data->scene = this;
    
    glfwSetWindowUserPointer(glfwWindow, data);
    glfwSetFramebufferSizeCallback(glfwWindow, [](GLFWwindow* glfwWindow, int width, int height) {
        WindowUserData* data = static_cast<WindowUserData*>(glfwGetWindowUserPointer(glfwWindow));
        if (data && data->scene) {
            data->scene->setScreenLayout(width, height);
        }
    });
}

void Scene::updateProjection() {
    updateProjection(glm::perspective(glm::radians(fieldOfView), aspectRatio, near, far));
}

void Scene::updateUniforms(Shader batchShader) {
    batchShader.use();
    batchShader.setVec3f("dirLight.direction", dirLight.direction);
    batchShader.setVec3f("dirLight.ambient", dirLight.ambient);
    batchShader.setVec3f("dirLight.diffuse", dirLight.diffuse);
    batchShader.setVec3f("dirLight.specular", dirLight.specular);
    for (unsigned int i = 0; i < pointLights.size(); i++) {
        std::string is = std::to_string(i);
        batchShader.setVec3f("pointLights[" + is + "].ambient", pointLights[i].ambient);
        batchShader.setVec3f("pointLights[" + is + "].diffuse", pointLights[i].diffuse);
        batchShader.setVec3f("pointLights[" + is + "].specular", pointLights[i].specular);
        batchShader.setFloat("pointLights[" + is + "].constant", pointLights[i].constant);
        batchShader.setFloat("pointLights[" + is + "].linear", pointLights[i].linear);
        batchShader.setFloat("pointLights[" + is + "].quadratic", pointLights[i].quadratic);
    }
    for (unsigned int i = 0; i < spotLights.size(); i++) {
        std::string is = std::to_string(i);
        batchShader.setVec3f("spotLights[" + is + "].position", spotLights[i].position);
        batchShader.setVec3f("spotLights[" + is + "].direction", spotLights[i].direction);
        batchShader.setFloat("spotLights[" + is + "].cutOff", spotLights[i].cutOff);
        batchShader.setFloat("spotLights[" + is + "].outerCutOff", spotLights[i].outerCutOff);
        batchShader.setVec3f("spotLights[" + is + "].ambient", spotLights[i].ambient);
        batchShader.setVec3f("spotLights[" + is + "].diffuse", spotLights[i].diffuse);
        batchShader.setVec3f("spotLights[" + is + "].specular", spotLights[i].specular);
    }
    if (hasPointShadows()) {
        std::vector<PointShadow> pointShadows = pointShadowMapper->getPointShadows();

        for (unsigned int i = 0; i < pointLights.size(); i++) {
            PointShadow pointShadow = pointShadows[i];
            std::string is = std::to_string(i);

            batchShader.setFloat("pointShadows[" + is + "].bias", pointShadow.bias);
            batchShader.setFloat("pointShadows[" + is + "].near", pointShadow.near);
            batchShader.setFloat("pointShadows[" + is + "].far", pointShadow.far);
            batchShader.setFloat("pointShadows[" + is + "].strength", pointShadow.strength);
        }
    }
}

void Scene::updateProjection(glm::mat4 customPerspective) {
    projection = customPerspective;
    batchShader.use();
    batchShader.setMatrix4("projection", projection, 1, GL_FALSE);
}

void Scene::setScreenLayout(int width, int height) {
    glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;
    aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    screenShader.use();
    screenShader.setVec2f("uv", width, height);
    updateProjection();
}

void Scene::setPointLights(const std::vector<PointLight>& pointLights) {
    this->pointLights = pointLights;
    batchShader.reloadProgram({
        {SHADER_BATCH_KEY_NR_POINT_LIGHTS, std::to_string(pointLights.size())},
        {SHADER_BATCH_KEY_NR_SPOT_LIGHTS, std::to_string(spotLights.size())},
        {SHADER_BATCH_KEY_HAS_SHADOWS, hasShadows() ? SHADER_VAL_ON : SHADER_VAL_OFF},
        {SHADER_BATCH_KEY_HAS_POINT_SHADOWS, hasPointShadows() ? SHADER_VAL_ON : SHADER_VAL_OFF}
    });
    updateUniforms();
    if (pointShadowMapper && pointShadowMapper->isCreated()) {
        pointShadowMapper->updateNumPointLights(pointLights.size());
    }
}

void Scene::setSpotLights(const std::vector<SpotLight>& spotLights) {
    this->spotLights = spotLights;
    batchShader.reloadProgram({
        {SHADER_BATCH_KEY_NR_POINT_LIGHTS, std::to_string(pointLights.size())},
        {SHADER_BATCH_KEY_NR_SPOT_LIGHTS, std::to_string(spotLights.size())},
        {SHADER_BATCH_KEY_HAS_SHADOWS, hasShadows() ? SHADER_VAL_ON : SHADER_VAL_OFF},
        {SHADER_BATCH_KEY_HAS_POINT_SHADOWS, hasPointShadows() ? SHADER_VAL_ON : SHADER_VAL_OFF}
    });
    updateUniforms();
}

void Scene::setPointLight(PointLight light, unsigned int index) {
    if (index >= pointLights.size()) return;
    pointLights[index] = light;
    updateUniforms();
}

void Scene::setSpotLight(SpotLight light, unsigned int index) {
    if (index >= spotLights.size()) return;
    spotLights[index] = light;
    updateUniforms();
}

void Scene::setZBufferLayout(float near, float far) {
    this->near = near;
    this->far = far;
    updateProjection();
}

void Scene::setAspectRatio(float aspectRatio) {
    this->aspectRatio = aspectRatio;
    updateProjection();
}

void Scene::setFieldOfView(float FOVDegrees) {
    fieldOfView = FOVDegrees;
    updateProjection();
}

float Scene::getScreenWidth() {
    return screenWidth;
}

float Scene::getScreenHeight() {
    return screenHeight;
}

float Scene::getZNear() {
    return near;
}

float Scene::getZFar() {
    return far;
}

float Scene::getFieldOfViewDegrees() {
    return fieldOfView;
}

float Scene::getAspectRatio() {
    return aspectRatio;
}

bool Scene::hasShadows() {
    return shadowMapper != nullptr && shadowMapper->isCreated();
}

bool Scene::hasPointShadows() {
    return pointShadowMapper != nullptr && pointShadowMapper->isCreated();
}

std::vector<PointLight> Scene::getPointLights() {
    return this->pointLights;
}

std::vector<SpotLight> Scene::getSpotLights() {
    return this->spotLights;
}

Scene_T Scene::getSnapshot() {
    Scene_T res;
    res.cameraPos = camera->getPosition();
    res.cameraDir = camera->getDirection();
    res.cameraUp = camera->getUp();
    res.cameraRight = camera->getRight();
    res.aspectRatio = aspectRatio;
    res.FOV = fieldOfView;
    res.zNear = near;
    res.zFar = far;
    return res;
}

ShadowMapper* Scene::getShadowMapper() {
    return shadowMapper;
}

Camera* Scene::getCamera() {
    return camera;
}

RenderTable<ShaderRenderable>* Scene::getBatchRenderTable() {
    return batchRenderTable;
}

glm::mat4 Scene::getProjection() {
    return projection;
}

glm::mat4 Scene::getViewMatrix() {
    return camera->getViewMatrix();
}

Shader Scene::getBatchShader() {
    return batchShader;
}

Shader Scene::getScreenShader() {
    return screenShader;
}