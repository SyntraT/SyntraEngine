cmake_minimum_required(VERSION 3.25)
project(SyntraEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "Vcpkg toolchain file")
endif()

# Prefer clang if available
if(NOT MSVC)
  find_program(CLANGXX clang++)
  if(CLANGXX)
    set(CMAKE_CXX_COMPILER ${CLANGXX} CACHE FILEPATH "C++ compiler")
  endif()
endif()

# Source files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "include/*.h" "include/*.hpp")

add_executable(SyntraEngine ${SRC_FILES} ${HEADER_FILES})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES ${SRC_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "include" FILES ${HEADER_FILES})

# Include directories
target_include_directories(SyntraEngine
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows/include
)

# Dependencies via vcpkg
find_package(fmt CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(volk CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(Bullet CONFIG REQUIRED)


# Link libraries to target
target_link_libraries(SyntraEngine PRIVATE
    fmt::fmt
    assimp::assimp
    glad::glad
    glfw
    OpenGL::GL
    volk::volk
    SDL3::SDL3
    glm::glm
    Vulkan::Vulkan
    imgui::imgui
    vk-bootstrap::vk-bootstrap
    GPUOpen::VulkanMemoryAllocator
	BulletDynamics
    BulletCollision
    LinearMath
)

# Installation (optional)
install(TARGETS SyntraEngine DESTINATION bin)
