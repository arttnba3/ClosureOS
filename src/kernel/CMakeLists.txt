set(TARGET_NAME Kernel)
set(SOURCE_FILE)

file(GLOB SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
                      "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

if(SOURCE_FILE)
    add_library(${TARGET_NAME} "")
    target_sources(${TARGET_NAME} PUBLIC ${SOURCE_FILE})
else()
    add_library(${TARGET_NAME} INTERFACE)
endif()

add_subdirectory(mm)

target_link_libraries(${TARGET_NAME}
    INTERFACE
        Kernel.MM)
