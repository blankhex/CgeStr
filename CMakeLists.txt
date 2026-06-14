cmake_minimum_required(VERSION 3.10)
project(CgeStr LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SOURCES
    Rune.c
    Str.c
    UCD.c
    Utf8.c
    Utf16.c
)

set(HEADERS
    CgeStr.h
)

add_library(CgeStr STATIC ${SOURCES} ${HEADERS})

target_include_directories(CgeStr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS CgeStr
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${HEADERS} DESTINATION include)
