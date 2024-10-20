
################################################################
# Project hex-a-hop
#
# run cmake with:
# cmake -DCMAKE_INSTALL_PREFIX=./local \
#       -DCMAKE_BUILD_TYPE=Release ..
################################################################

project(hex-a-hop)

# version number
set(hah_MAJOR 1)
set(hah_MINOR 1)
set(hah_PATCH 0)
set(hah_VERSION ${hah_MAJOR}.${hah_MINOR}.${hah_PATCH})
set(hah_DESCRIPTION "Puzzle Game")
set(hah_HOMEPAGE "http://hexahop.sourceforge.net")
set(hah_EXENAME "hex-a-hop")
set(hah_PACKAGENAME "Hex-a-hop")

# Minimum cmake version required
cmake_minimum_required(VERSION 2.6.2)


################################################################
# configure header files, add compiler flags
################################################################

add_definitions(-Wall)

if(APPLE)
	add_definitions(-DRELATIVE_PATHS)
endif(APPLE)

# Need these modules to do subsequent checks.
include(CheckIncludeFiles)

# check if header file exists
check_include_files(windows.h HAVE_WINDOWS_H)

# On windows systems the math library is not separated so do not specify
# it unless you are on a non-windows system.
if(NOT WIN32)
  find_library(HAVE_LIBM NAMES m PATHS /usr/local/lib /usr/lib)
  if(NOT HAVE_LIBM)
    message(FATAL_ERROR "Cannot find required math library")
  endif(NOT HAVE_LIBM)
else(NOT WIN32)
  set(HAVE_LIBM)
endif(NOT WIN32)

# create config.h
configure_file(
  ${CMAKE_SOURCE_DIR}/src/config.h.cmake
  ${CMAKE_BINARY_DIR}/src/config.h
)
include_directories(${CMAKE_BINARY_DIR}/src)

add_definitions(-DHAVE_CONFIG_H)

################################################################
# Find needed libraries
################################################################

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

find_package(SDL_image REQUIRED)
include_directories(${SDLIMAGE_INCLUDE_DIR})

if(NOT NINTENDO_DS)
	find_package(SDL_mixer REQUIRED)
	include_directories(${SDLMIXER_INCLUDE_DIR})
else(NOT NINTENDO_DS)
	add_definitions(-DDISABLE_SOUND)
endif(NOT NINTENDO_DS)

find_package(SDL_ttf REQUIRED)
include_directories(${SDLTTF_INCLUDE_DIR})

################################################################
# game executable
################################################################

# source files
set(
  hah_SRCS
		src/gfx.cpp
		src/gfx_list.h
		src/hex_puzzzle.cpp
		src/i18n.cpp
		src/i18n.h
		src/level_list.h
		src/menus.h
		src/packfile.h
		src/savestate.h
		src/sfx.cpp
		src/sfx.h
		src/state.h
		src/system-directory.c
		src/system-directory.h
		src/system-relative.c
		src/system-relative.h
		src/text.cpp
		src/text.h
		src/tiletypes.h
		src/video.h
)

if(NINTENDO_DS)
	set(
		hah_data
			data/DejaVuSans-Bold.ttf
			data/levels.dat
			data/nds/emi.png
			data/nds/gradient.png
			data/nds/map.png
			data/nds/map_top.png
			data/nds/tiles.png
			data/nds/tiles_reflect.png
			data/nds/title.png
	)
else(NINTENDO_DS)
	set(
		hah_data
			data/DejaVuSans-Bold.ttf
			data/emi.png
			data/gradient.png
			data/hex-a-hop-16.png
			data/levels.dat
			data/map.png
			data/map_top.png
			data/tiles.png
			data/tiles_reflect.png
			data/title.png
	)
endif(NINTENDO_DS)

set(
  hah_sound
		data/music-ending.ogg
		data/music-game-1.ogg
		data/music-game.ogg
		data/sound-builder-nonfree.ogg
		data/sound-collapse-nonfree.ogg
		data/sound-crack.ogg
		data/sound-death.ogg
		data/sound-explode-big.ogg
		data/sound-explode-small.ogg
		data/sound-floater-enter.ogg
		data/sound-found-antiice-nonfree.ogg
		data/sound-found-jump-nonfree.ogg
		data/sound-ice.ogg
		data/sound-laser.ogg
		data/sound-lift-down-nonfree.ogg
		data/sound-lift-up-nonfree.ogg
		data/sound-spinner-nonfree.ogg
		data/sound-trampoline.ogg
		data/sound-used-antiice.ogg
		data/sound-used-jump.ogg
		data/sound-win.ogg
)

set(
  hah_RSRCS
		data/hex-a-hop.icns
)

if(APPLE)
	add_executable(
		hex-a-hop MACOSX_BUNDLE
			${hah_SRCS}
			${hah_data}
			${hah_sound}
			${hah_RSRCS}
	)

	# Mac OS X bundle specific settings
	set(MACOSX_BUNDLE true)
	set(MACOSX_BUNDLE_BUNDLE_NAME hex-a-hop)
	set(MACOSX_BUNDLE_INFO_STRING "hex-a-hop ${hah_VERSION}")
	set(MACOSX_BUNDLE_LONG_VERSION_STRING "${hah_VERSION}")
	set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${hah_VERSION}")
	set(MACOSX_BUNDLE_BUNDLE_VERSION "${hah_VERSION}")
  set(MACOSX_BUNDLE_ICON_FILE hex-a-hop.icns)

  # paths for data files in bundle
  set_source_files_properties(
    ${hah_RSRCS}
    PROPERTIES
      MACOSX_PACKAGE_LOCATION Resources
  )
  set_source_files_properties(
    ${hah_data}
    ${hah_sound}
    PROPERTIES
      MACOSX_PACKAGE_LOCATION Resources/data
  )
else(APPLE)
	add_executable(
		hex-a-hop
			${hah_SRCS}
	)
endif(APPLE)

target_link_libraries(
	hex-a-hop
		${SDLIMAGE_LIBRARY}
		${SDLMIXER_LIBRARY}
		${SDLTTF_LIBRARY}
		${SDL_LIBRARY}
		SDLmain
)

if(NINTENDO_DS)
	target_link_libraries(hex-a-hop freetype bz2 png z filesystem fat mm9)

	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/nitrofs)
	file(INSTALL ${hah_data} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/nitrofs)

	nds_create_rom(hex-a-hop
		NAME "${hah_PACKAGENAME}"
		SUBTITLE1 "${hah_DESCRIPTION}"
		SUBTITLE2 "${hah_HOMEPAGE}"
		ICON ${CMAKE_CURRENT_SOURCE_DIR}/data/nds/icon.bmp
		NITROFS ${CMAKE_CURRENT_BINARY_DIR}/nitrofs)
endif(NINTENDO_DS)

#####################################################################
# Installation
#####################################################################

# executable
install(
	TARGETS hex-a-hop
		RUNTIME DESTINATION .
		BUNDLE DESTINATION .
)

# copy SDL frameworks into app bundle for Mac OS X
if(APPLE)
	INSTALL(DIRECTORY /Users/smekal/Library/Frameworks/SDL.framework
					DESTINATION hex-a-hop.app/Contents/Frameworks)
	INSTALL(DIRECTORY /Users/smekal/Library/Frameworks/SDL_mixer.framework
					DESTINATION hex-a-hop.app/Contents/Frameworks)
	INSTALL(DIRECTORY /Users/smekal/Library/Frameworks/SDL_ttf.framework
					DESTINATION hex-a-hop.app/Contents/Frameworks)
endif(APPLE)


#####################################################################
# Package
#####################################################################

# CPack version numbers for release tarball name.
set(CPACK_PACKAGE_VERSION_MAJOR ${hah_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${hah_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${hah_PATCH})

# To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
# where CPackConfig.cmake is created by including CPack
# And then there's ways to customize this as well
set(CPACK_DMG_VOLUME_NAME "hex-a-hop")
set(CPACK_BINARY_DRAGNDROP ON)
include(CPack)


################################################################
# summary
################################################################

message( "" )
message( "Summary:" )
message( "  CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}" )
message( "  WIN32 = ${WIN32}" )
message( "  UNIX = ${UNIX}" )
message( "  APPLE = ${APPLE}" )
message( "  CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}" )
message( "  CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}" )
message( "  CMAKE_OSX_ARCHITECTURES = ${CMAKE_OSX_ARCHITECTURES}" )
message( "  CMAKE_GENERATOR = ${CMAKE_GENERATOR}" )
message( "  SDL_INCLUDE_DIR = ${SDL_INCLUDE_DIR}" )
message( "  SDL_LIBRARY = ${SDL_LIBRARY}" )
message( "  SDLIMAGE_INCLUDE_DIR = ${SDLIMAGE_INCLUDE_DIR}" )
message( "  SDLIMAGE_LIBRARY = ${SDLIMAGE_LIBRARY}" )
message( "  SDLMIXER_INCLUDE_DIR = ${SDLMIXER_INCLUDE_DIR}" )
message( "  SDLMIXER_LIBRARY = ${SDLMIXER_LIBRARY}" )
message( "  SDLTTF_INCLUDE_DIR = ${SDLTTF_INCLUDE_DIR}" )
message( "  SDLTTF_LIBRARY = ${SDLTTF_LIBRARY}" )
message( "" )
