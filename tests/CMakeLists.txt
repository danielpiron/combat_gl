
add_subdirectory(../deps/googletest gtest)

add_executable(unittests main.cpp)
target_compile_options(unittests  PUBLIC ${COMPILER_FLAGS})
target_link_libraries(unittests gtest)

