
add_subdirectory(../deps/googletest gtest)

set(JSON_MultipleHeaders OFF)
add_subdirectory(../deps/json json)

file(GLOB TEST_FILES test_*.cpp)
add_executable(unittests main.cpp ${TEST_FILES} ${APPLESAUCE_FILES})

target_compile_options(unittests  PUBLIC ${COMPILER_FLAGS})
target_include_directories(unittests PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(unittests gtest glad glfw glm nlohmann_json)
