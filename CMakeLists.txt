cmake_minimum_required(VERSION 3.16)
project(Mandelbrot_CPP)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Executable/bin)
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} \
        -std=gnu++2a \
        -pedantic \
        -Wall \
        -Wextra\
         \
        -Wconversion \
        -Wenum-compare \
        -Woverloaded-virtual \
    "
    )
set(STATIC_BUILD TRUE)
if (CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    set(STATIC_BUILD TRUE)
endif ()

if (STATIC_BUILD)
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc")
endif ()

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)


include(FetchContent)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/Multithreading/ThreadPool.h
        src/DebugTools/DebugTools.cpp
        src/DebugTools/DebugTools.h
        src/Utility/Types.h
        src/Utility/Functions.h
        src/MainWindow.cpp
        src/MainWindow.h
        src/Fractal/MandelbrotFractal.cpp
        src/Fractal/MandelbrotFractal.h
        src/Fractal/TaskIterators.h
        src/Fractal/Zoomer.cpp
        src/Fractal/Zoomer.h
        src/Utility/Types.cpp
        src/Fractal/Config.h
        src/Fractal/FractalCalcMethods.cpp
        src/Fractal/FractalCalcMethods.h
        src/Multithreading/ThreadPoolInstance.h

)

set(ENV_ROOT "D:/Prog/Env")
set(SFML_SOURCE_DIR "${ENV_ROOT}/Libraries/SFML-2.6.1-64")
target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-window sfml-system)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

add_custom_command(
        TARGET ${PROJECT_NAME}
        COMMENT "Copy font"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/resources/arial.ttf $<TARGET_FILE_DIR:${PROJECT_NAME}>
        VERBATIM)

install(TARGETS ${PROJECT_NAME})



