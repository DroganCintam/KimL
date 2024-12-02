cmake_minimum_required(VERSION 3.11)

project(KimL)

set(CMAKE_CXX_STANDARD 11)

add_library(libkiml STATIC "")

file(GLOB LIBKIML_SRC "libkiml/src/*.cpp")
file(GLOB LIBKIML_INC "libkiml/include/kiml")
target_sources(libkiml PRIVATE ${LIBKIML_SRC})
target_include_directories(libkiml PRIVATE ${LIBKIML_INC})
set_target_properties(libkiml PROPERTIES PREFIX "")

add_executable(kiml "")

file(GLOB KIML_SRC "kiml/*.cpp")
file(GLOB KIML_INC "kiml" "libkiml/include")
target_sources(kiml PRIVATE ${KIML_SRC})
target_include_directories(kiml PRIVATE ${KIML_INC})
target_link_libraries(kiml libkiml)

add_executable(simpl "")

file(GLOB SIMPL_SRC "simpl/main.cpp" "simpl/src/*.cpp")
file(GLOB SIMPL_INC "simpl/include/simpl" "libkiml/include")
target_sources(simpl PRIVATE ${SIMPL_SRC})
target_include_directories(simpl PRIVATE ${SIMPL_INC})
target_link_libraries(simpl libkiml)

add_executable(kimln "")

file(GLOB KIMLN_SRC "kimln/*.cpp")
file(GLOB KIMLN_INC "kimln" "libkiml/include")
target_sources(kimln PRIVATE ${KIMLN_SRC})
target_include_directories(kimln PUBLIC ${KIMLN_INC})
target_link_libraries(kimln libkiml)
