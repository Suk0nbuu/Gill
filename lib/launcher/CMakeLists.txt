add_library(launcher STATIC
        include/core/launcher.hpp
        source/core/launcher.cpp
        include/project/project.hpp
        source/project/project.cpp
        include/project/projectList.hpp
        source/project/projectList.cpp
)

target_include_directories(launcher PUBLIC include)

target_link_libraries(launcher PRIVATE engine)