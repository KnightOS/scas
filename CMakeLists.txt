cmake_minimum_required(VERSION 2.8.5)
project(scass C)
set(CMAKE_C_FLAGS "-g")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
add_definitions("-Wall")

include_directories(
    assembler/
    scass/
    common/
)

FILE(GLOB common ${PROJECT_SOURCE_DIR}/common/*.c)
FILE(GLOB assembler ${PROJECT_SOURCE_DIR}/assembler/*.c)
FILE(GLOB linker ${PROJECT_SOURCE_DIR}/linker/*.c)
FILE(GLOB scass ${PROJECT_SOURCE_DIR}/scass/*.c)

add_executable(scass
    ${common}
    ${assembler}
    ${linker}
    ${scass}
)

ADD_CUSTOM_TARGET(
    man ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/bin/scass.1
)

ADD_CUSTOM_COMMAND(
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scass.1.txt
    COMMAND a2x --no-xmllint --doctype manpage --format manpage ${CMAKE_CURRENT_SOURCE_DIR}/scass.1.txt
    COMMAND test "${CMAKE_CURRENT_SOURCE_DIR}" != "${CMAKE_CURRENT_BINARY_DIR}/bin" && mv -f ${CMAKE_CURRENT_SOURCE_DIR}/scass.1 ${CMAKE_CURRENT_BINARY_DIR}/bin || true
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/scass.1
    VERBATIM
)
set_directory_properties(${CMAKE_CURRENT_BINARY_DIR} ADDITIONAL_MAKE_CLEAN_FILES scass.1)

INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/scass.1
    DESTINATION man/man1
)

INSTALL(
    TARGETS scass
    RUNTIME DESTINATION bin
)

set(CMAKE_BUILD_TYPE Release)
