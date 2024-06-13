set(TARGET_NAME Boot)
set(SOURCE_FILE)

file(GLOB SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/*.S"
                      "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
                      "${CMAKE_CURRENT_SOURCE_DIR}/boot_font.o")

if(SOURCE_FILE)
    add_library(${TARGET_NAME} ${SOURCE_FILE})
else()
    message(FATAL_ERROR "no source files provided for boot")
endif()
