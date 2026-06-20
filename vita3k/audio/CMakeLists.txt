add_library(
    audio
    STATIC
    src/audio.cpp
    src/impl/sdl_audio.cpp)

if (NOT VITA3K_BUILD_IOS)
    target_sources(audio PRIVATE src/impl/cubeb_audio.cpp)
    target_link_libraries(audio PRIVATE cubeb)
endif()

target_include_directories(audio PUBLIC include)
target_link_libraries(audio PRIVATE util SDL3::SDL3)
