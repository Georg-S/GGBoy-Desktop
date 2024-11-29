cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
project(GGBoyDesktop)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(Core)

set(HEADERS 
	"include/Audio.hpp"
	"include/Video.hpp"
	"include/Inputhandling.hpp"
	"include/EmulatorMain.hpp"
	)
	
set(SOURCES 
	"src/Audio.cpp"
	"src/Video.cpp"
	"src/Inputhandling.cpp"
	"src/main.cpp"
	"src/EmulatorMain.cpp"
	)

#source_group("" FILES ${miscFiles})

if	(WIN32)
	set(SDL2_INCLUDE_DIRS $ENV{SDL2_INCLUDE})
	set(SDL2_LIBRARIES "$ENV{SDL2_BIN}/SDL2.lib")
endif(WIN32)

add_executable(GGBoyDesktop 
	${SOURCES}
	${HEADERS}
	)
target_include_directories(GGBoyDesktop PUBLIC "include" ${SDL2_INCLUDE_DIRS})
target_link_libraries(GGBoyDesktop "GGBoyCore" ${SDL2_LIBRARIES})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT GGBoyDesktop)

if (MSVC)
    add_custom_command(TARGET GGBoyDesktop
                        POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "$ENV{SDL2_BIN}/SDL2.dll"
                        $<TARGET_FILE_DIR:GGBoyDesktop>)
endif (MSVC)