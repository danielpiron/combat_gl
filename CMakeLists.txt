cmake_minimum_required(VERSION 3.0.0)
project(combat_gl)

if(MSVC)
    set(COMPILER_FLAGS /W4)
else()
	set(COMPILER_FLAGS ${CLANG_WARNINGS} -O2 -Werror -Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

file(GLOB APPLESAUCE_FILES src/applesauce/*.cpp src/util/*.cpp)

add_subdirectory(tests)

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)

add_subdirectory(deps/glfw)
add_subdirectory(deps/glad)
add_subdirectory(deps/glm)


#
# Begins: IMGUI Integration
#
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui)
set(IMGUI_SOURCES ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp
                  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
#
# Ends: IMGUI Integration
#


# if(MSVC)
#     add_subdirectory(deps/zlib)
#     set(PNG_BUILD_ZLIB ON)
#     set(ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib ${PROJECT_BINARY_DIR}/deps/zlib)
#     set(ZLIB_LIBRARIES ${PROJECT_BINARY_DIR}/deps/zlib/Debug/zlibstaticd.lib)
# endif()

set(PNG_TESTS OFF CACHE BOOL "Build libpng tests" FORCE)
add_subdirectory(deps/libpng)


add_executable(combat_gl src/main.cpp ${APPLESAUCE_FILES} ${IMGUI_SOURCES})
target_link_libraries(combat_gl
 glfw
 glad
 nlohmann_json
 png_static
 )

target_compile_options(combat_gl PUBLIC ${COMPILER_FLAGS})
target_include_directories(combat_gl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(combat_gl SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/glfw/include)
target_include_directories(combat_gl SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/glad/include)
target_include_directories(combat_gl SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/glm)
target_include_directories(combat_gl SYSTEM PUBLIC ${IMGUI_DIR})
target_include_directories(combat_gl SYSTEM PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_include_directories(combat_gl SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/libpng)
target_include_directories(combat_gl SYSTEM PUBLIC ${PROJECT_BINARY_DIR}/deps/libpng)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/basic.vs.glsl assets/shaders/basic.vs.glsl COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/basic.fs.glsl assets/shaders/basic.fs.glsl COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/shadow.vs.glsl assets/shaders/shadow.vs.glsl COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/shadow.fs.glsl assets/shaders/shadow.fs.glsl COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/quad.vs.glsl assets/shaders/quad.vs.glsl COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/quad.fs.glsl assets/shaders/quad.fs.glsl COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/gltf/tenk6a.gltf assets/gltf/tenk6a.gltf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/gltf/tenk7.gltf assets/gltf/tenk7.gltf COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/assets/gltf/wall-and-floor.gltf" "assets/gltf/wall-and-floor.gltf" COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/textures/Checker.png assets/textures/Checker.png COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/assets/textures/White Square.png" "assets/textures/White Square.png" COPYONLY)
