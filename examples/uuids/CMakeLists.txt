cmake_minimum_required(VERSION 2.6.4)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")
set(PROJECT_EXAMPLE_NAME uuids)

file(GLOB EXAMPLE_SRC_FILES ${CASS_ROOT_DIR}/examples/uuids/*.c)
include_directories(${INCLUDES})
add_executable(${PROJECT_EXAMPLE_NAME} ${EXAMPLE_SRC_FILES})
target_link_libraries(${PROJECT_EXAMPLE_NAME} ${PROJECT_LIB_NAME_TARGET} ${CASS_LIBS})
add_dependencies(${PROJECT_EXAMPLE_NAME} ${PROJECT_LIB_NAME_TARGET})

set_property(TARGET ${PROJECT_EXAMPLE_NAME} PROPERTY FOLDER "Examples")
