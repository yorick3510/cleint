cmake_minimum_required(VERSION 3.20)
project(client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)

# SDL2 configuratie
#set(SDL2_PATH "D:/SDL2-2.30.10/x86_64-w64-mingw32")
#find_package(SDL2 REQUIRED)
#include_directories(${SDL2_INCLUDE_DIR})

# Maak executables aan
add_executable(main main.cpp)


# Koppel bibliotheken
if(WIN32)
    # Koppel ws2_32 voor Windows sockets
    target_link_libraries(main ws2_32)


    # Alleen SDL2 voor de client (indien gebruikt)
    target_link_libraries(main ${SDL2_LIBRARIES})
endif()
