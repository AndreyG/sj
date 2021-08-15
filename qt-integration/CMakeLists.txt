project(sj-qt)

set(TARGET_NAME "${PROJECT_NAME}")

find_package(Qt5Widgets REQUIRED)

file(GLOB_RECURSE headers include/*.h)

set(sources
  src/qscheduler.cpp
  src/progress.cpp
  ${headers}
)

add_library(${TARGET_NAME} STATIC ${sources})

foreach(FILE ${sources})
    # Get the directory of the source file
    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)

    # Remove common directory prefix to make the group
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")

    # Make sure we are using windows slashes
    string(REPLACE "/" "\\" GROUP "${GROUP}")

    source_group("${GROUP}" FILES "${FILE}")
endforeach()

target_include_directories(${TARGET_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(${TARGET_NAME}
  sj
  Qt5::Widgets
)

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)