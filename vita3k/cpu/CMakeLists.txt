set(SOURCE_LIST
include/cpu/state.h
include/cpu/common.h
include/cpu/functions.h
include/cpu/impl/interface.h
include/cpu/disasm/functions.h
include/cpu/disasm/state.h
src/disasm.cpp
src/cpu.cpp
)

if (USE_UNICORN_CPU)
    list(APPEND SOURCE_LIST include/cpu/impl/unicorn_cpu.h src/unicorn_cpu.cpp)
else()
    list(APPEND SOURCE_LIST include/cpu/impl/dynarmic_cpu.h src/dynarmic_cpu.cpp)
endif()

add_library(
cpu
STATIC
${SOURCE_LIST}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_LIST})

target_include_directories(cpu PUBLIC include)
target_link_libraries(cpu PUBLIC mem util)

if (USE_UNICORN_CPU)
    target_link_libraries(cpu PRIVATE unicorn capstone merry::mcl)
else()
    target_link_libraries(cpu PRIVATE dynarmic capstone merry::mcl)
endif()
