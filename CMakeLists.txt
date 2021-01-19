cmake_minimum_required(VERSION 3.13)
project(Sp1 C)

set(CMAKE_C_STANDARD 99)

add_executable(Sp1 server.c client.c game.c game.h message_in.c message_in.h game_manager.c game_manager.h cards.c cards.h)

find_package (Threads REQUIRED)

target_link_libraries(Sp1
        ${CMAKE_THREAD_LIBS_INIT}
        )