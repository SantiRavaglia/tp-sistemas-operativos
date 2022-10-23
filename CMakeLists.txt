cmake_minimum_required(VERSION 3.16)
project(test_tp_sistemas)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Threads)
add_executable(test_tp_sistemas
        main.cpp gameMaster.cpp config.h config.cpp gameMaster.h equipo.cpp equipo.h definiciones.h )
target_link_libraries(test_tp_sistemas Threads::Threads)
