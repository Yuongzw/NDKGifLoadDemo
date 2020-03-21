cmake_minimum_required(VERSION 3.4.1)

#file(GLOB gif_source ${CMAKE_SOURCE_DIR}/giflib/*.c ${CMAKE_SOURCE_DIR}/giflib/*.h)
#include_directories(giflib)


add_library(
        native-lib

        SHARED

        native-lib.cpp
        dgif_lib.c
        gifalloc.c
        )

#set(LIBS)
#
#list(APPEND LIBS log jnigraphics)




find_library(
        log-lib
        log)

find_library(
        jnigraphics-lib
        jnigraphics)


target_link_libraries(
        native-lib
        ${log-lib}
        ${jnigraphics-lib})