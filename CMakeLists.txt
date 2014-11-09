cmake_minimum_required(VERSION 2.8.5)
project(scas C)
set(CMAKE_C_FLAGS "-g")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
add_definitions("-Wall")
set(CMAKE_BUILD_TYPE Debug)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(INSTRUCTION_SET_PATH "${PROJECT_SOURCE_DIR}/tables/" CACHE PATH "Location to install instruction set tables.")
else()
    set(INSTRUCTION_SET_PATH "/usr/share/scas/instruction_sets/" CACHE PATH "Location to install instruction set tables.")
endif()
add_definitions(-DINSTRUCTION_SET_PATH="${INSTRUCTION_SET_PATH}/")

include_directories(
    assembler/
    linker/
    scas/
    common/
)

FILE(GLOB common ${PROJECT_SOURCE_DIR}/common/*.c)
FILE(GLOB assembler ${PROJECT_SOURCE_DIR}/assembler/*.c)
FILE(GLOB linker ${PROJECT_SOURCE_DIR}/linker/*.c)
FILE(GLOB scas ${PROJECT_SOURCE_DIR}/scas/*.c)
FILE(GLOB tables ${PROJECT_SOURCE_DIR}/tables/*)

add_executable(scas
    ${common}
    ${assembler}
    ${linker}
    ${scas}
)

ADD_CUSTOM_TARGET(
    man ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/bin/scas.1
)

ADD_CUSTOM_COMMAND(
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scas.1.txt
    COMMAND a2x --no-xmllint --doctype manpage --format manpage ${CMAKE_CURRENT_SOURCE_DIR}/scas.1.txt
    COMMAND test "${CMAKE_CURRENT_SOURCE_DIR}" != "${CMAKE_CURRENT_BINARY_DIR}/bin" && mv -f ${CMAKE_CURRENT_SOURCE_DIR}/scas.1 ${CMAKE_CURRENT_BINARY_DIR}/bin || true
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bin/scas.1
    VERBATIM
)
set_directory_properties(${CMAKE_CURRENT_BINARY_DIR} ADDITIONAL_MAKE_CLEAN_FILES scas.1)

INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/scas.1
    DESTINATION man/man1
)

INSTALL(
    FILES ${tables}
    DESTINATION share/scas/instruction_sets/
)

INSTALL(
    TARGETS scas
    RUNTIME DESTINATION bin
)
