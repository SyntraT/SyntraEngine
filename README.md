# Syntra Engine

Syntra Engine is a lightweight, User Friendly, Beta (Work in progress) 3D game engine built with OpenGL. It’s designed to be optimized, modular, and easy to integrate into custom projects.

Goals:

- ~~Cross-Platform~~
- ~~Lightweight,~~ Optimized
- ~~Frustum Culling, Dynamic environment Mapping + Fresnel's Law~~ (along with other community development ideas within learnopengl itself & engines such as Source 2, etc.)
- ~~Simple & Easy to Use in Code~~ (IN PROGRESS)
- Normal Mapping (as well as other chapters <https://www.learnopengl.com> 29.1+)
- 3D/2D Audio Support (Sydio Project + OpenAL)
- GLTF + Bones & Rigid Animation Support
- Graphical Interface
  - QT + Window/Frames Management
- Physics
  - ~~Bullet Support~~ (IN PROGRESS)
    - PhysX Support

## How to Build

**Required Libraries**
Before building, ensure the following libraries are installed:

- **pip** (Package installer for python) 
- **Vulkan** (At least v1.3)
- **OpenGL** (Atleast v3.3)
- **Cmake** (At least v3.25)

Then clone the codebase and run ModelsDownloader.py and extract models folder into the same directory that ModelsDownloader.py is currently in. Afterward, delete the models.rar file.

### Install Vcpkg
1 - Go to repo directory:

```bash
git clone https://github.com/microsoft/vcpkg.git
```
On windows:
```bash
.\vcpkg\bootstrap-vcpkg.bat
```
On linux and macOs:
```bash
./vcpkg/bootstrap-vcpkg.sh
```

### Building the project:

### Mac OS

```bash
cmake --preset macos-release
cmake --build --preset macos-release
```

### Linux

```bash
cmake --preset linux-release
cmake --build --preset linux-release
```

### Windows

```bash
cmake --preset windows-release
cmake --build --preset windows-release
```
