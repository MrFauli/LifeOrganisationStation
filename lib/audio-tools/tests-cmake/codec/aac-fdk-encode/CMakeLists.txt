cmake_minimum_required(VERSION 3.20)

# set the project name
project(aac-fdk-encode)
set (CMAKE_CXX_STANDARD 11)
set (DCMAKE_CXX_FLAGS "-Werror")
# add_compile_options(-Wstack-usage=1024)

include(FetchContent)

# Build with arduino-audio-tools
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../.. ${CMAKE_CURRENT_BINARY_DIR}/arduino-audio-tools )
endif()

# Build with arduino-fdk-aac
FetchContent_Declare(fdk_aac GIT_REPOSITORY "https://github.com/pschatzmann/arduino-fdk-aac.git" GIT_TAG main )
FetchContent_GetProperties(fdk_aac)
if(NOT fdk_aac_POPULATED)
    FetchContent_Populate(fdk_aac)
    add_subdirectory(${fdk_aac_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/fdk_aac)
endif()

# build sketch as executable
add_executable (aac-fdk-encode aac-fdk-encode.cpp )
# set preprocessor defines
target_compile_definitions(aac-fdk-encode PUBLIC -DARDUINO -DIS_DESKTOP)

# specify libraries
target_link_libraries(aac-fdk-encode arduino_emulator fdk_aac arduino-audio-tools)

