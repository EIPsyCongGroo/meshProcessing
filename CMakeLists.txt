cmake_minimum_required(VERSION 3.16)
project(Simplification)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(VCPKG_TARGET_TRIPLET x64-windows)
include(C:/src/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake)

# Libigl
include(libigl)
# Enable the target igl::glfw
igl_include(glfw)
igl_include(imgui)

find_package(openmesh  REQUIRED)

add_definitions(-D_USE_MATH_DEFINES)
# Add your project files
file(GLOB SRC_FILES *.cpp)
file(GLOB HEADER_FILES *.h *.hpp)
add_executable(${PROJECT_NAME}  ${SRC_FILES} ${HEADER_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC igl::glfw igl::imgui igl::opengl)
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENMESH_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENMESH_LIBRARIES})
