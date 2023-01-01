
add_subdirectory(../deps/googletest gtest)


file(GLOB TEST_FILES test_*.cpp)
add_executable(unittests main.cpp ${TEST_FILES} ${APPLESAUCE_FILES})


target_compile_options(unittests  PUBLIC ${COMPILER_FLAGS})
target_include_directories(unittests PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_include_directories(unittests SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../deps/libpng)
target_include_directories(unittests SYSTEM PUBLIC ${PROJECT_BINARY_DIR}/deps/libpng)
target_link_libraries(unittests gtest glad glfw glm nlohmann_json png_static)
