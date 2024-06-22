set(Name "npassm")

cmake_minimum_required(VERSION 3.20)
project(${Name} VERSION 0.1)

add_executable(${Name} src/main.c)

find_package(OpenSSL REQUIRED)
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

target_link_libraries(${Name} m)
target_link_libraries(${Name} ${CURSES_LIBRARIES})
target_link_libraries(${Name} OpenSSL::Crypto)
