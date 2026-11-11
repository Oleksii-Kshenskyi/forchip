cmake_minimum_required(VERSION 3.28)

project(forchip VERSION 0.1)

set(CMAKE_BUILD_TYPE Debug)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(fetch_raylib)

set(FORCHIP_SRC ${CMAKE_SOURCE_DIR}/src)
set(FORCHIP_INCLUDES ${CMAKE_SOURCE_DIR}/include)

file(GLOB FORCHIP_SRC_FILES CONFIGURE_DEPENDS "${FORCHIP_SRC}/*.cpp")

include_directories(${FORCHIP_INCLUDES})

add_executable(${PROJECT_NAME}
    ${FORCHIP_SRC_FILES}
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
