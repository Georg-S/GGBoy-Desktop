cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
project(GGBoyDesktop)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(GGBoy-Core)

if	(WIN32)
	set(SDL2_INCLUDE_DIRS $ENV{SDL2_INCLUDE})
	set(SDL2_LIBRARIES "$ENV{SDL2_BIN}/SDL2.lib")
	list(APPEND CMAKE_PREFIX_PATH $ENV{QT_MSVC_64})
endif(WIN32)

find_package(Qt6 REQUIRED Widgets)

set(HEADERS 
	"include/MainWindow.hpp"
	"include/Audio.hpp"
	"include/Video.hpp"
	"include/Inputhandling.hpp"
	"include/EmulatorMain.hpp"
	"include/InformationWindow.hpp"
	)
	
set(SOURCES 
	"src/MainWindow.cpp"
	"src/Audio.cpp"
	"src/Video.cpp"
	"src/Inputhandling.cpp"
	"src/main.cpp"
	"src/EmulatorMain.cpp"
	"src/InformationWindow.cpp"
	)
	
set(QT_UI_FILES
	"include/MainWindow.ui"
	"include/InformationWindow.ui"
	)


source_group("\\Header Files" FILES ${HEADERS})
source_group("\\Source Files" FILES ${SOURCES})
source_group("UI Files" FILES ${QT_UI_FILES})
# Put all other files via regex into the "Other" folder, mainly so that the solution doesn't get cluttered by Qt generated files
source_group("Other" REGULAR_EXPRESSION "\\.h$|\\.cpp$|\\.stamp$|\\.rule$")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(GGBoyDesktop 
	${SOURCES}
	${HEADERS}
	${QT_UI_FILES}
	)
target_include_directories(GGBoyDesktop PUBLIC "include" ${SDL2_INCLUDE_DIRS})
target_link_libraries(GGBoyDesktop "GGBoyCore" ${SDL2_LIBRARIES} Qt6::Widgets)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT GGBoyDesktop)

if (MSVC)
    add_custom_command(TARGET GGBoyDesktop
                        POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "$ENV{SDL2_BIN}/SDL2.dll"
                        $<TARGET_FILE_DIR:GGBoyDesktop>)
endif (MSVC)