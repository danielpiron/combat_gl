
add_subdirectory(../deps/googletest gtest)

add_executable(unittests main.cpp)
target_compile_options(unittests  PUBLIC ${COMPILER_FLAGS})
target_include_directories(unittests PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(unittests gtest glad glfw)

