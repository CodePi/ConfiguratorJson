cmake_minimum_required(VERSION 2.8)
project(ConfiguratorJson)
add_compile_options(-Wall -Werror)

set(CMAKE_CXX_STANDARD 11)

add_executable(TestConfigJS TestConfigJS.cpp)
add_executable(TestConfigJS2 TestConfigJS2.cpp)
add_executable(TestConfigJS3 TestConfigJS3.cpp)
add_executable(testOptionalCJS testOptionalCJS.cpp)

add_test(TestConfigJS TestConfigJS)
add_test(TestConfigJS2 TestConfigJS2)
add_test(TestConfigJS3 TestConfigJS3)
add_test(testOptionalCJS testOptionalCJS)
