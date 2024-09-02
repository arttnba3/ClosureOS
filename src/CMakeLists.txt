cmake_minimum_required(VERSION 3.16)
project(ClosureOS)

# basic config
set(CMAKE_CXX_STANDARD 23)

# languages we will use
enable_language(C)
enable_language(CXX)
enable_language(ASM)

# check for current compiler environment, temporarily support linux only
if (NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "Unsatisfied compilation environment, only support Linux now")
endif()

# global link script
if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${CMAKE_SOURCE_DIR}/arch/x86/linker.lds")
    include_directories("arch/x86/include")
else()
    message(FATAL_ERROR "Only support x86_64 now")
endif()

# compile and link as bare bone project
add_compile_options(-pipe -ffreestanding -nostdlib -fno-pie -fno-stack-protector -mcmodel=large -fno-asynchronous-unwind-tables -fno-exceptions)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -z max-page-size=0x1000 -Wl,--build-id=none -static")

# general include dirs
include_directories(${PROJECT_SOURCE_DIR}/include)

# seperate module
add_subdirectory(arch)
add_subdirectory(kernel)

# we create an empty file for add_executable(), as it must need at least one file to work
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/internal_empty_file_for_occupation_only.cpp" "")
add_executable(kernel.bin "${CMAKE_CURRENT_BINARY_DIR}/internal_empty_file_for_occupation_only.cpp")

# final output
target_link_libraries(kernel.bin
    PRIVATE
        Arch
        Kernel
)
