set(TARGET_NAME MM)
set(SOURCE_FILE)
set(C_SOURCE_FILE)
set(CXX_SOURCE_FILE)

file(GLOB C_SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB CXX_SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
list(APPEND SOURCE_FILE ${C_SOURCE_FILE})
list(APPEND SOURCE_FILE ${CXX_SOURCE_FILE})

if(SOURCE_FILE)
    add_library(${TARGET_NAME} ${SOURCE_FILE})
    target_sources(MM
        PUBLIC
            FILE_SET CXX_MODULES FILES
            ${CXX_SOURCE_FILE}
    )
else()
    message(FATAL_ERROR "no source files provided for mm")
endif()
