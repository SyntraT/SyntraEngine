cmake_minimum_required(VERSION 3.25)
project(SyntraEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_TOOLCHAIN_FILE "vcpkg/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "Vcpkg toolchain file")


# Check for clang++ in non visual studio generator environments
find_program(CLANGXX_PATH clang++)

if(CLANGXX_PATH)
    message(STATUS "Found Clang++ at: ${CLANGXX_PATH}")
    set(CMAKE_CXX_COMPILER ${CLANGXX_PATH})
else()
    # Check for MSVC compiler cl.exe
    find_program(MSVC_PATH cl)

    if(MSVC_PATH)
        message(STATUS "Found MSVC (cl.exe) at: ${MSVC_PATH}")
        set(CMAKE_CXX_COMPILER ${MSVC_PATH})
    else()
        # If neither compiler is found, abort
        message(FATAL_ERROR "Neither Clang++ nor MSVC (cl.exe) were found on this system.")
    endif()
endif()

# Now declare project after compiler is selected
project(SyntraEngine LANGUAGES CXX)

# Confirm final selected compiler
message(STATUS "Using C++ compiler: ${CMAKE_CXX_COMPILER}")

# Detect Vulkan SDK via environment variable
if(NOT DEFINED ENV{VULKAN_SDK})
    message(FATAL_ERROR "VULKAN_SDK environment variable not set. Please install the Vulkan SDK and set VULKAN_SDK environment variable.")
endif()

message(STATUS "Using Vulkan SDK from: $ENV{VULKAN_SDK}")

# Set include and library paths based on platform
set(VULKAN_INCLUDE_DIR "$ENV{VULKAN_SDK}/Include")

if(WIN32)
    set(VULKAN_LIBRARY "$ENV{VULKAN_SDK}/Lib/vulkan-1.lib")
elseif(APPLE)
    set(VULKAN_LIBRARY "$ENV{VULKAN_SDK}/lib/libvulkan.dylib")
elseif(UNIX)
    set(VULKAN_LIBRARY "$ENV{VULKAN_SDK}/lib/libvulkan.so")
else()
    message(FATAL_ERROR "Unsupported platform.")
endif()



# Source files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "include/*.h" "include/*.hpp")

add_executable(SyntraEngine ${SRC_FILES} ${HEADER_FILES})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES ${SRC_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "include" FILES ${HEADER_FILES})


# Dependencies via vcpkg
find_package(fmt CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(volk CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(Bullet CONFIG REQUIRED)



# Include directories
target_include_directories(SyntraEngine
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
		${VULKAN_INCLUDE_DIR}
)


# Link libraries to target
target_link_libraries(SyntraEngine PRIVATE
    fmt::fmt
    assimp::assimp
    glad::glad
    glfw
    OpenGL::GL
    volk::volk
    SDL2::SDL2
    glm::glm
    imgui::imgui
    vk-bootstrap::vk-bootstrap
    GPUOpen::VulkanMemoryAllocator
	BulletDynamics
    BulletCollision
    LinearMath
	${VULKAN_LIBRARY}
)

# Installation (optional)
install(TARGETS SyntraEngine DESTINATION bin)
