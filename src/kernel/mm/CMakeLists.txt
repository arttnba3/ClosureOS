set(TARGET_NAME MM)
set(SOURCE_FILE)

file(GLOB SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
                      "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

if(SOURCE_FILE)
    add_library(${TARGET_NAME} ${SOURCE_FILE})
else()
    message(FATAL_ERROR "no source files provided for mm")
endif()
