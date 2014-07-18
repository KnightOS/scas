cmake_minimum_required(VERSION 2.8.5)
project(sass C)
set(CMAKE_C_FLAGS "-g -DENABLE_HOOKS -DENABLE_READ_BYTE_HOOK=1 -DENABLE_WRITE_BYTE_HOOK=1")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
add_definitions("-Wall")

add_executable(sass
    main.c
    meta/log.c
    meta/runtime.c
)

include_directories(
    meta/
)

ADD_CUSTOM_TARGET(man ALL)

ADD_CUSTOM_COMMAND(
  TARGET man
  SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/sass.1.txt
  COMMAND a2x --no-xmllint --doctype manpage --format manpage ${CMAKE_CURRENT_SOURCE_DIR}/sass.1.txt 
  OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/sass.1
)

ADD_CUSTOM_COMMAND(
  TARGET man
  SOURCE man
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/sass.1
)

INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/sass.1
    DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1
)

set(CMAKE_BUILD_TYPE Release)
