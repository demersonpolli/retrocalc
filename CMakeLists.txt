cmake_minimum_required(VERSION 3.10)
project(retrocalc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(retrocalc src/main.cpp src/welcome.cpp src/spreadsheet.cpp src/matrix.cpp src/terminal.cpp src/display.cpp)
