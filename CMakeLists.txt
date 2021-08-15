project(sj)

set(TARGET_NAME "${PROJECT_NAME}")

find_package(BOOST REQUIRED COMPONENTS intrusive)

file(GLOB_RECURSE headers include/*.h)

set(sources
  src/background_thread.cpp
  src/job.cpp
  src/scheduler.cpp
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
  Boost::headers
)

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)

add_subdirectory(qt-integration)