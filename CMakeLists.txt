cmake_minimum_required(VERSION 3.5)
project(obs-v4l2sink)
include(GNUInstallDirs)

find_package(LibObs REQUIRED)

set (CMAKE_CXX_STANDARD 11)
set(CMAKE_PREFIX_PATH "${QTDIR}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)


set(virtualoutput_SOURCES
	src/v4l2sink.cpp
	src/v4l2sinkproperties.cpp)

set(virtualoutput_HEADERS
	src/v4l2sink.h
	src/v4l2sinkproperties.h)

add_library(v4l2sink MODULE
	${virtualoutput_SOURCES}
	${virtualoutput_HEADERS})
	
target_link_libraries(v4l2sink
	libobs
	Qt5::Core
	Qt5::Widgets)

set_target_properties(v4l2sink PROPERTIES PREFIX "")

install(TARGETS v4l2sink
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/obs-plugins)

install(DIRECTORY locale/
	DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/obs/obs-plugins/v4l2sink/locale")

