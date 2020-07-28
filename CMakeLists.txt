cmake_minimum_required(VERSION 3.13)
project(Mandelbrot_CPP)

set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} \
        -std=gnu++2a \
        -pedantic \
        -Wall \
        -Wextra\
         \
        -Wconversion \
        -Wenum-compare \
        -Wfloat-equal \
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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Executable/bin)
set(ENV_ROOT "D:/Programms/ProgrammingEnvironment")


add_executable(${PROJECT_NAME}
    main.cpp

    )


set(SFML_ROOT "${ENV_ROOT}/Librares/SFML-2.5.1")
set(SFML_STATIC_LIBRARIES ${STATIC_BUILD})
set(SFML_DIR "${SFML_ROOT}/lib/cmake/SFML")
find_package(SFML 2.5 COMPONENTS graphics REQUIRED)
target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-window sfml-system)


set(GMP_DIR "${ENV_ROOT}/Librares/gmp-6.1.1")
include_directories(${PROJECT_NAME} ${GMP_DIR}/include)
target_link_libraries(${PROJECT_NAME}
    "${GMP_DIR}/lib/libgmp.dll.a"
    "${GMP_DIR}/lib/libgmpxx.dll.a"
    )



