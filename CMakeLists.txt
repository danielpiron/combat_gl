cmake_minimum_required(VERSION 3.0.0)
project(combat_gl)

if(MSVC)
    set(COMPILER_FLAGS /W4)
else()
	set(COMPILER_FLAGS ${CLANG_WARNINGS} -g -Werror -Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

#include(CTest)
#enable_testing()
#add_subdirectory(tests)

add_subdirectory(deps/glfw)
add_subdirectory(deps/glad)

# if(MSVC)
#     add_subdirectory(deps/zlib)
#     set(PNG_BUILD_ZLIB ON)
#     set(ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib ${PROJECT_BINARY_DIR}/deps/zlib)
#     set(ZLIB_LIBRARIES ${PROJECT_BINARY_DIR}/deps/zlib/Debug/zlibstaticd.lib)
# endif()

# set(PNG_TESTS OFF CACHE BOOL "Build libpng tests" FORCE)
# add_subdirectory(deps/libpng)

add_executable(combat_gl${PLAYER_SOURCE} src/main.cpp)
target_link_libraries(combat_gl
 glfw
 glad
# png_static
 )

target_compile_options(combat_gl PUBLIC ${COMPILER_FLAGS})
target_include_directories(combat_gl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(combat_gl SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/glfw/include)
target_include_directories(combat_gl SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/glad/include)
# target_include_directories(combat_gl SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/glm)
# target_include_directories(combat_gl SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/libpng)
# target_include_directories(combat_gl SYSTEM PUBLIC ${PROJECT_BINARY_DIR}/deps/libpng)
