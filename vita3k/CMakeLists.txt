set(VITA3K_ORG_NAME "Vita3K")
set(VITA3K_APP_NAME "Vita3K")
set(VITA3K_APP_VER_HI 0)
set(VITA3K_APP_VER_MID 2)
set(VITA3K_APP_VER_LO 1)
set(VITA3K_APP_VERSION "v${VITA3K_APP_VER_HI}.${VITA3K_APP_VER_MID}.${VITA3K_APP_VER_LO}")

execute_process(
	COMMAND git rev-list HEAD --count
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_COUNT
	OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(
	COMMAND git rev-parse --short HEAD
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
	COMMAND git rev-parse --abbrev-ref HEAD
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_BRANCH
	OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMD git config --get remote.origin.url)

execute_process(COMMAND ${CMD}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_REPO
	OUTPUT_STRIP_TRAILING_WHITESPACE)

set(VITA3K_OFFICIAL_UPDATE_BUILD 0)

if (NOT GIT_REPO STREQUAL "")
	string(FIND ${GIT_REPO} "git@github.com:" IS_GIT_SSH)
	if (IS_GIT_SSH EQUAL 0)
		string(REPLACE "git@github.com:" "" GIT_REPO ${GIT_REPO})
	else()
		string(REPLACE "https://github.com/" "" GIT_REPO ${GIT_REPO})
	endif()
	string(REPLACE "/" ";" GIT_REPO ${GIT_REPO})
	list(POP_BACK GIT_REPO)
endif()

if(GIT_HASH STREQUAL "")
	set(VITA3K_GIT_REV "Development version")
	set(GIT_COUNT 0)
else()
	set(VITA3K_GIT_REV "${GIT_HASH}-${GIT_REPO}/${GIT_BRANCH}")
endif()

file(WRITE ${CMAKE_BINARY_DIR}/vita3k/config/gitver.h
	"#define APP_VER_HI ${VITA3K_APP_VER_HI}\n"
	"#define APP_VER_MID ${VITA3K_APP_VER_MID}\n"
	"#define APP_VER_LO ${VITA3K_APP_VER_LO}\n"
	"#define APP_NUMBER ${GIT_COUNT}\n"
)

include_directories(${CMAKE_BINARY_DIR}/vita3k/config)

find_package(Boost REQUIRED COMPONENTS filesystem)
include_directories(${Boost_INCLUDE_DIRS})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(app)
add_subdirectory(audio)
add_subdirectory(camera)
add_subdirectory(input)
add_subdirectory(config)
add_subdirectory(cpu)
add_subdirectory(ctrl)
add_subdirectory(codec)
add_subdirectory(compat)
add_subdirectory(dialog)
add_subdirectory(display)
add_subdirectory(features)
add_subdirectory(glutil)
add_subdirectory(updater)
add_subdirectory(gxm)
add_subdirectory(ime)
add_subdirectory(net)
add_subdirectory(ngs)
add_subdirectory(np)
add_subdirectory(emuenv)
add_subdirectory(http)
add_subdirectory(io)
add_subdirectory(kernel)
add_subdirectory(lang)
add_subdirectory(mem)
add_subdirectory(patch)
add_subdirectory(module)
add_subdirectory(modules)
add_subdirectory(motion)
add_subdirectory(nids)
add_subdirectory(overlay)
add_subdirectory(regmgr)
add_subdirectory(renderer)
add_subdirectory(rtc)
add_subdirectory(shader)
add_subdirectory(threads)
add_subdirectory(touch)
add_subdirectory(util)
add_subdirectory(gdbstub)
add_subdirectory(packages)
add_subdirectory(vkutil)

# iOS exclusive executable target
add_executable(vita3k MACOSX_BUNDLE
	main_ios.cpp
	interface.cpp
	interface.h
	performance.cpp
	ios/AppDelegate.swift
	ios/ViewController.swift
)

set_target_properties(vita3k PROPERTIES
	MACOSX_BUNDLE_GUI_IDENTIFIER "com.allydevs0.VitaiOS"
	MACOSX_BUNDLE_BUNDLE_NAME "VitaiOS"
	MACOSX_BUNDLE_BUNDLE_VERSION "${VITA3K_APP_VERSION}"
	MACOSX_BUNDLE_SHORT_VERSION_STRING "${VITA3K_APP_VERSION}"
	MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/ios/Info.plist"
	XCODE_ATTRIBUTE_ENABLE_BITCODE "NO"
	XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
	XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
	XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
	XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
	XCODE_ATTRIBUTE_SWIFT_OBJC_BRIDGING_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/ios/VitaiOS-Bridging-Header.h"
	OUTPUT_NAME "VitaiOS"
)

target_include_directories(vita3k PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(vita3k PRIVATE
	app audio compat config cppcommon ctrl dialog display ime lang
	gdbstub gxm io miniz modules motion packages patch renderer
	shader touch util psvpfsparser
	"-framework UIKit"
	"-framework Foundation"
	"-framework QuartzCore"
	"-framework Metal"
	"-framework AVFoundation"
	"-framework CoreAudio"
	"-framework AudioToolbox"
)

# Copy bundle resources after build
add_custom_command(
	TARGET vita3k
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_CURRENT_SOURCE_DIR}/../data"
		"$<TARGET_FILE_DIR:vita3k>/../Resources/data"
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_CURRENT_SOURCE_DIR}/shaders-builtin/"
		"$<TARGET_FILE_DIR:vita3k>/../Resources/shaders-builtin"
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_CURRENT_SOURCE_DIR}/icons"
		"$<TARGET_FILE_DIR:vita3k>/../Resources/icons"
	COMMENT "Copying bundle resources"
)

# Embed MoltenVK dylib into the .app bundle
if(DEFINED MOLTENVK_DYLIB AND EXISTS "${MOLTENVK_DYLIB}")
	target_sources(vita3k PRIVATE ${MOLTENVK_DYLIB})
	set_source_files_properties(${MOLTENVK_DYLIB} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
endif()
