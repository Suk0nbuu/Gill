add_library(cgltf STATIC
        cgltf.h
        cgltf.cpp
        cgltf_write.h
)

target_include_directories(cgltf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})