cmake_minimum_required(VERSION 2.8.5)
project(sass C)
set(CMAKE_C_FLAGS "-g")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
add_definitions("-Wall")

add_executable(sass
    main.c
    meta/log.c
    meta/runtime.c
)

include_directories(
    meta/
    assemble/
)

ADD_CUSTOM_TARGET(
  man ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/sass.1
)

ADD_CUSTOM_COMMAND(
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sass.1.txt
  COMMAND a2x --no-xmllint --doctype manpage --format manpage ${CMAKE_CURRENT_SOURCE_DIR}/sass.1.txt
  COMMAND test "${CMAKE_CURRENT_SOURCE_DIR}" != "${CMAKE_CURRENT_BINARY_DIR}" && mv -f ${CMAKE_CURRENT_SOURCE_DIR}/sass.1 ${CMAKE_CURRENT_BINARY_DIR} || true
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sass.1
  VERBATIM
)
set_directory_properties(${CMAKE_CURRENT_BINARY_DIR} ADDITIONAL_MAKE_CLEAN_FILES sass.1)

INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/sass.1
    DESTINATION man/man1
)

INSTALL(
    TARGETS sass
    RUNTIME DESTINATION bin
)

set(CMAKE_BUILD_TYPE Release)
