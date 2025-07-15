#include "Syngine/modules/PointShadowMapper.hpp"
#include "Syngine/engine/Config.hpp"
#include "Syngine/modules/Scene.hpp"
#include "Syngine/modules/Shader.hpp"
#include "Syngine/modules/Skybox.hpp"
#include <vector>

PointShadowMapper::PointShadowMapper(Scene *scene, unsigned int uv)
        : PointShadowMapper(scene, uv, uv) {}

PointShadowMapper::PointShadowMapper(Scene *scene, unsigned int width, unsigned int height) 
        : scene(scene), shadowWidth(width), shadowHeight(height) {}

PointShadowMapper::~PointShadowMapper() {
    glDeleteTextures(1, &depthArrayTCB);
    glDeleteFramebuffers(1, &depthMapFBO);
}

void PointShadowMapper::updateProjection() {
}

glm::mat4 PointShadowMapper::fetchShadowProjection(PointShadow pointShadow, float fovy, float aspect) {
    return glm::perspective(glm::radians(fovy), aspect, pointShadow.near, pointShadow.far);
}

glm::mat4 PointShadowMapper::fetchShadowProjection(PointShadow pointShadow) {
    return fetchShadowProjection(pointShadow, 89.4666f, static_cast<float>(shadowWidth) / static_cast<float>(shadowHeight));
}

std::vector<glm::mat4> PointShadowMapper::fetchShadowTransforms(PointLight light, PointShadow pointShadow) {
    glm::mat4 projection = fetchShadowProjection(pointShadow);
    glm::vec3 lightPos = light.position;

    return {
        (projection * glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0))),
        (projection * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0))),
        (projection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0))),
        (projection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0))),
        (projection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0))),
        (projection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0,-1.0), glm::vec3(0.0, -1.0, 0.0)))
    };
}

void PointShadowMapper::updateNumPointLights(unsigned int numPointLights) {
    depthShader.reloadProgram({
        {SHADER_PDEPTH_KEY_NR_POINT_LIGHTS, std::to_string(scene->getPointLights().size())}
    });

    if (depthArrayTCB) glDeleteTextures(1, &depthArrayTCB);

    glGenTextures(1, &depthArrayTCB);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthArrayTCB);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 6 * numPointLights, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    std::vector<PointShadow> pointShadowsCpy = this->pointShadows;
    pointShadows.clear();
    if (numPointLights > pointShadowsCpy.size()) {
        for (unsigned int i = 0; i < numPointLights - pointShadowsCpy.size(); i++) {
            pointShadows.push_back({});
        }
    }
}

void PointShadowMapper::create() {
    updateNumPointLights(scene->getPointLights().size());
    glGenFramebuffers(1, &depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PointShadowMapper::renderDepth(Screenbuffer screen, Scene *scene) {
    glViewport(0, 0, screen.getWidth(), screen.getHeight());
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthArrayTCB);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthArrayTCB, 0);

    glClear(GL_DEPTH_BUFFER_BIT);

    std::vector<PointLight> pointLights = scene->getPointLights();

    for (unsigned int lightIndex = 0; lightIndex < pointLights.size(); lightIndex++) {
        PointLight pointLight = pointLights[lightIndex];
        PointShadow pointShadow = pointShadows[lightIndex];

        std::vector<glm::mat4> shadowTransforms = fetchShadowTransforms(pointLight, pointShadow);

        depthShader.use();
        depthShader.setInt("lightIndex", lightIndex);
        depthShader.setVec3f("lightPos", pointLight.position);
        depthShader.setFloat("far_plane", pointShadow.far);
        for (int i = 0; i < 6; ++i) {
            depthShader.setMatrix4("shadowTransforms[" + std::to_string(i) + "]", shadowTransforms[i], 1, GL_FALSE);
        }
    
        Screenbuffer shadowScreen(depthMapFBO, shadowWidth, shadowHeight);
        scene->getBatchRenderTable()->forEach([&](const std::string& key, ShaderRenderable* renderable) {
            if (!dynamic_cast<Skybox*>(renderable)) {
                renderable->render(depthShader, shadowScreen);
            }
        });
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, screen.getFBO());
    glViewport(0, 0, screen.getWidth(), screen.getHeight());
}

std::vector<PointShadow> PointShadowMapper::getPointShadows() {
    return this->pointShadows;
}

unsigned int PointShadowMapper::getDepthMapFBO() {
    return this->depthMapFBO;
}

unsigned int PointShadowMapper::getDepthArrayTCB() {
    return this->depthArrayTCB;
}