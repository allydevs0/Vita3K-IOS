add_library(
	util
	STATIC
	src/arm.cpp
	src/float_to_half.cpp
	src/fs_utils.cpp
	src/hash.cpp
	src/instrset_detect.cpp
	src/logging.cpp
	src/net_utils.cpp
	src/string_utils.cpp
	src/tracy.cpp
	src/vita_theme_utils.cpp
)

target_include_directories(util PUBLIC include)
target_link_libraries(util PUBLIC ${Boost_LIBRARIES} fmt spdlog http mem)
target_link_libraries(util PRIVATE crypto SDL3::SDL3)

find_package(CURL QUIET)
if(CURL_FOUND)
	target_link_libraries(util PRIVATE CURL::libcurl)
endif()
