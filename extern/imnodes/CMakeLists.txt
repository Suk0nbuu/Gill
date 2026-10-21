add_library(imnodes STATIC imnodes.cpp imnodes.h)
target_include_directories(imnodes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(imnodes PUBLIC imgui)