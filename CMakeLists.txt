cmake_minimum_required(VERSION 3.16.0)
project(ProMuxer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt6_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/FFmpegSetupDialog.cpp
    src/core/FileProcessor.cpp
    src/core/MuxingTask.cpp
    src/core/MediaAnalyzer.cpp
)

set(HEADERS
    src/ui/MainWindow.h
    src/ui/FFmpegSetupDialog.h
    src/core/FileProcessor.h
    src/core/MuxingTask.h
    src/core/MediaAnalyzer.h
)

set(UI_FILES
    src/ui/MainWindow.ui
)

qt6_add_executable(ProMuxer ${SOURCES} ${HEADERS})
qt6_add_resources(ProMuxer "resources" PREFIX "/" FILES)

# Process UI files
qt6_add_resources(ProMuxer "ui_resources" PREFIX "/" FILES ${UI_FILES})

target_link_libraries(ProMuxer 
    PRIVATE 
    Qt6::Core 
    Qt6::Widgets
)

# Windows specific settings
if(WIN32)
    set_target_properties(ProMuxer PROPERTIES
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "ProMuxer"
    )
    # Copy Qt DLLs to output directory for easy deployment
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(DEBUG_SUFFIX "d")
    else()
        set(DEBUG_SUFFIX "")
    endif()
endif()

# Install rules
install(TARGETS ProMuxer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)