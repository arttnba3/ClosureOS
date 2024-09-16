set(TARGET_NAME Arch)

add_library(${TARGET_NAME} INTERFACE)

# check for current architecture
if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
    add_subdirectory(x86)
    target_link_libraries(${TARGET_NAME}
        INTERFACE
            X86)
else()
    message(FATAL_ERROR "Temporarily nly support x86_64 now")
endif()
