#pragma once

#include "Shader.hpp"
#include "Syngine/modules/Scene.hpp"
#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/modules/Shader.hpp"
#include "glm/fwd.hpp"

struct PointShadow {
    float bias = 0.05f;
    float near = 1.0f;
    float far = 30.0f;
    float strength = 0.7f;
};

class PointShadowMapper {
private:
    Shader depthShader = Shader("shaders/pdepthVertex.glsl", "shaders/pdepthFrag.glsl", "shaders/pdepthGeo.glsl");
    GLuint depthMapFBO = 0, depthArrayTCB = 0;
    std::vector<PointShadow> pointShadows;
    Scene *scene;
public:
    unsigned int shadowWidth = 1024, shadowHeight = 1024;

    PointShadowMapper(Scene *scene, unsigned int uv = 1024);

    PointShadowMapper(Scene *scene, unsigned int width, unsigned int height);

    ~PointShadowMapper();

    void create();

    void updateNumPointLights(unsigned int numPointLights);

    void updateProjection();

    glm::mat4 fetchShadowProjection(PointShadow pointShadow, float fovy, float aspect);

    glm::mat4 fetchShadowProjection(PointShadow pointShadow);

    std::vector<glm::mat4> fetchShadowTransforms(PointLight light, PointShadow pointShadow);

    bool isCreated() {
        return depthMapFBO && depthArrayTCB;
    }

    void renderDepth(Screenbuffer screen, Scene *scene);

    std::vector<PointShadow> getPointShadows();

    unsigned int getDepthMapFBO();

    unsigned int getDepthArrayTCB();
};