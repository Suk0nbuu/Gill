cmake_minimum_required(VERSION 3.20)
project(GILL C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
)
# GLFW ships its own examples/tests/docs we don't need — skip building them
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

include(FetchContent)

FetchContent_Declare(
        magic_enum
        GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
        GIT_TAG        v0.9.5
)

FetchContent_MakeAvailable(magic_enum)





add_subdirectory(extern/glad)
add_subdirectory(extern/imgui)
add_subdirectory(extern/stb_image)
add_subdirectory(lib/mathpp)
add_subdirectory(lib/engine)
add_subdirectory(extern/cgltf)




add_executable(gill source/gill/main.cpp
        source/gill/app.hpp
        source/gill/app.cpp
        extern/cgltf/cgltf.cpp)



add_custom_target(copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/asset"
        "${CMAKE_CURRENT_BINARY_DIR}/asset"
        COMMENT "Copying assets to build directory"
)

add_dependencies(gill copy_assets)



target_link_libraries(gill PRIVATE glad engine magic_enum::magic_enum)

