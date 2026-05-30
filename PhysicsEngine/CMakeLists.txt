# Minimum CMake version. 3.24 gives us modern FetchContent behavior.
cmake_minimum_required(VERSION 3.24)

# Project declaration. CXX = C++ language. We'll add CUDA in Phase 4.
project(PhysicsEngine LANGUAGES CXX)

# Use C++20. Modern, well-supported in VS2022, gives us nice features.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)   # use -std=c++20 not -std=gnu++20

# Put all built binaries in build/bin so shaders can find them at runtime.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---- Dependencies via FetchContent ----
include(FetchContent)

# GLFW: window + input + GL context creation.
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
# Turn off GLFW's docs/tests/examples to speed up build.
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLM: header-only math library. Vectors, matrices, quaternions.
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

# GLAD: OpenGL function loader. We use a pre-generated GL 4.5 core profile.
# We vendor this one because GLAD's repo is a generator, not a library.
# (We'll add the GLAD files in the next step.)

# ---- Shader files (listed so VS shows them in the project tree, not compiled) ----
set(SHADER_FILES
    shaders/triangle.vert
    shaders/triangle.frag
)
set_source_files_properties(${SHADER_FILES} PROPERTIES VS_TOOL_OVERRIDE "None")

# ---- Our executable target ----
add_executable(physics_engine
    PhysicsEngine.cpp
    external/glad/src/glad.c
    ${SHADER_FILES}
)

# Tell our target where to find headers.
target_include_directories(physics_engine PRIVATE
    external/glad/include
)

# Link against dependencies. PRIVATE = only physics_engine sees these.
target_link_libraries(physics_engine PRIVATE
    glfw
    glm::glm
)

# Pass the shader directory path to the code as a #define, so main.cpp
# can find shaders/ relative to the source tree without copying them.
target_compile_definitions(physics_engine PRIVATE
    SHADER_DIR="${CMAKE_SOURCE_DIR}/shaders"
)

# On Windows, link the OpenGL library.
if(WIN32)
    target_link_libraries(physics_engine PRIVATE opengl32)
endif()