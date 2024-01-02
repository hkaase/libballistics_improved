cmake_minimum_required(VERSION 3.1)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
project(ballistics C CXX)

include_directories(include)
# don't need the tests
# add_subdirectory(test)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -finline-functions -O3")

# In the CMakeLists.txt of your library

# Detect if we are compiling with Emscripten
if(EMSCRIPTEN)
        set(TARGET_LIB_DIR "/usr/local/lib/wasm")  # Custom directory for WASM libs
        set(TARGET_INCLUDE_DIR "/usr/local/include/wasm")  # Custom directory for WASM headers
else()
        set(TARGET_LIB_DIR "/usr/local/lib")  # Default directory for native libs
        set(TARGET_INCLUDE_DIR "/usr/local/include")  # Default directory for native headers
endif()


add_executable(example example.c)
set_target_properties(example PROPERTIES SUFFIX ".html")
target_link_libraries(example PRIVATE m ballistics)
#install(TARGETS example DESTINATION bin)

add_library(ballistics STATIC
        angle.c
        atmosphere.c
        ballistics.c
        pbr.c
        )
target_link_libraries(ballistics PRIVATE m)
set_target_properties(ballistics PROPERTIES LINK_FLAGS "-Wl,--whole-archive")
install(TARGETS ballistics DESTINATION ${TARGET_LIB_DIR})
install(DIRECTORY include/ballistics DESTINATION ${TARGET_INCLUDE_DIR})

# To workaround the fact CLion doesn't have a way to do `make install`:
add_custom_target(install_${PROJECT_NAME}
        ${CMAKE_MAKE_PROGRAM} install
        DEPENDS ${PROJECT_NAME}
        COMMENT "Installing ${PROJECT_NAME}")
