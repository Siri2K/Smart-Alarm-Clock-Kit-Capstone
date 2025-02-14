cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
set(CMAKE_BUILD_TYPE Release)

project(Watch)

# Automatically add all .c files in Component and subdirectories
file(GLOB COMPONENT_SOURCES src/Component/**/*.c)
file(GLOB SERVICE_SOURCES src/Services/**/*.c)

# Target all Source Files
target_sources(app PRIVATE 
    src/main.c
    ${COMPONENT_SOURCES} # Add all component source files
    ${SERVICE_SOURCES} # Add all Services source files
)

# include Component Folder + Subdirectories
target_include_directories(app PRIVATE
    src
    src/Component
    src/Services
)
