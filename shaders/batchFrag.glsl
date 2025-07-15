#version 330 core

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define HAS_SHADOWS ${HAS_SHADOWS=0}
#define HAS_POINT_SHADOWS ${HAS_POINT_SHADOWS=0}

#define NR_POINT_LIGHTS ${NR_POINT_LIGHTS=0}
#define NR_SPOT_LIGHTS ${NR_SPOT_LIGHTS=0}

#if NR_POINT_LIGHTS
uniform PointLight pointLights[NR_POINT_LIGHTS];
#endif
#if NR_SPOT_LIGHTS
uniform SpotLight spotLights[NR_SPOT_LIGHTS];
#endif
uniform DirLight dirLight;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform float shininess;
uniform float shadowStrength = 0.7;

#if HAS_SHADOWS
in vec4 fragPosLightSpace;

uniform sampler2D shadowMap;

uniform float shadowBiasMax = 0.05;
uniform float shadowBiasMin = 0.005;
#endif

#if HAS_POINT_SHADOWS && NR_POINT_LIGHTS
uniform sampler2DArray pshadowMap;

struct PointShadow {
    float bias;
    float far_plane;
    float strength;
};

uniform PointShadow pointShadows[NR_POINT_LIGHTS];
#endif

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;

out vec4 FragColor;

uniform vec3 cameraPos;
uniform float opacity = 1.0;

float specularStrength = 1.0;

#if HAS_SHADOWS
float calculateShadow(DirLight light, vec4 fragPosLightSpace);
#endif

#if HAS_POINT_SHADOWS && NR_POINT_LIGHTS
float calculateShadow(uint lightIndex);
#endif

vec3 calculateDirectionalLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 viewDir);
vec3 calculatePointLight(PointLight light, vec3 normal, vec3 viewDir, int lightIndex);

void main() {
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(cameraPos - fragPos);
    vec3 result = calculateDirectionalLight(dirLight, norm, viewDir);

#if NR_POINT_LIGHTS
    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += calculatePointLight(pointLights[i], norm, viewDir, i);
    }
#endif
#if NR_SPOT_LIGHTS
    for (int i = 0; i < NR_SPOT_LIGHTS; i++) {
        result += calculateSpotLight(spotLights[i], norm, viewDir);
    }
#endif

    FragColor = vec4(result, opacity);
}

#if HAS_SHADOWS
float calculateShadow(DirLight light, vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    vec3 lightDir = normalize(-light.direction);
    float bias = max(shadowBiasMax * (1.0 - dot(normal, lightDir)), shadowBiasMin);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            if (projCoords.z - bias > pcfDepth)
                shadow += 1.0;
        }
    }
    shadow /= 9.0;

    return shadowStrength * shadow;
}
#endif

#if HAS_POINT_SHADOWS && NR_POINT_LIGHTS
void directionToFaceUV(vec3 dir, out int faceIndex, out vec2 uv) {
    vec3 absDir = abs(dir);
    float maxAxis;
    vec2 planar;

    if (absDir.x >= absDir.y && absDir.x >= absDir.z) {
        maxAxis = absDir.x;
        if (dir.x > 0.0) {
            faceIndex = 0;
            planar = vec2(-dir.z, -dir.y);
        } else {
            faceIndex = 1;
            planar = vec2(dir.z, -dir.y);
        }
    } else if (absDir.y >= absDir.z) {
        maxAxis = absDir.y;
        if (dir.y > 0.0) {
            faceIndex = 2;
            planar = vec2(dir.x, dir.z);
        } else {
            faceIndex = 3;
            planar = vec2(dir.x, -dir.z);
        }
    } else {
        maxAxis = absDir.z;
        if (dir.z > 0.0) {
            faceIndex = 4;
            planar = vec2(dir.x, -dir.y);
        } else {
            faceIndex = 5;
            planar = vec2(-dir.x, -dir.y);
        }
    }
    uv = planar / maxAxis * 0.5 + 0.5;
}

float calculateShadow(uint lightIndex) {
    PointLight light = pointLights[lightIndex];
    PointShadow pShadow = pointShadows[lightIndex];

    vec3 fragToLight = fragPos - light.position;
    int faceIndex;
    vec2 uv;
    directionToFaceUV(normalize(fragToLight), faceIndex, uv);
    int layer = int(lightIndex) * 6 + faceIndex;

    float closestDepth = texture(pshadowMap, vec3(uv, float(layer))).r;

    closestDepth *= pShadow.far_plane;

    float currentDepth = length(fragToLight);
    float shadow = currentDepth - pShadow.bias > closestDepth ? 1.0 : 0.0;

    return pShadow.strength * shadow;
}  
#endif

vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 viewDir) {
    vec3 ambient = light.ambient * texture(texture_diffuse1, texCoord).rgb;

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = texture(texture_diffuse1, texCoord).rgb * diff * light.diffuse;

    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = texture(texture_specular1, texCoord).rgb * spec * light.specular;

    // spotlight (soft edges)
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = (light.cutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    diffuse *= intensity;
    specular *= intensity;

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 calculateDirectionalLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, texCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, texCoord));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, texCoord));

#if HAS_SHADOWS
    return (ambient + (1.0 - calculateShadow(light, fragPosLightSpace)) * (diffuse + specular));
#else
    return (ambient + diffuse + specular);
#endif
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 viewDir, int lightIndex) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, texCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, texCoord));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, texCoord));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

#if HAS_POINT_SHADOWS
    float shadow = calculateShadow(uint(lightIndex));
    return ambient + (1.0 - shadow) * (diffuse + specular);
#else
    return (ambient + diffuse + specular);
#endif
}