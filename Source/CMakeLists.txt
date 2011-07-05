project(r5ge CXX)
cmake_minimum_required(VERSION 2.8)
enable_language(CXX)

set(SYSWINDOW_SRCS Engine/SysWindow/Source/SysWindow.cpp)
if(UNIX)
	SET(SYSWINDOW_SRCS ${SYSWINDOW_SRCS}
		Engine/SysWindow/Source/_X11Window.cpp
	)
	SET(OS Linux)
elseif(APPLE)
	SET(SYSWINDOW_SRCS ${SYSWINDOW_SRCS}
		Engine/SysWindow/Source/_CocoaClasses.mm
		Engine/SysWindow/Source/_CocoaWindow.mm
	)
	SET(OS Mac)
elseif(WIN32)
	SET(SYSWINDOW_SRCS ${SYSWINDOW_SRCS}
		Engine/SysWindow/Source/_MSWindow.cpp
	)
	SET(OS Win)
else()
	message(FATAL_ERROR "This probably shouldn't happen... Unknown system")
endif()

aux_source_directory(Engine/Serialization/Source SERIALIZATION_SRCS)
aux_source_directory(Engine/Font/Source FONT_SRCS)
aux_source_directory(Engine/UI/Source UI_SRCS)
aux_source_directory(Engine/Sound/Source SOUND_SRCS)
aux_source_directory(Engine/Math/Source MATH_SRCS)
aux_source_directory(Engine/Network/Source NEWORK_SRCS)
aux_source_directory(Engine/Image/Source IMAGE_SRCS)
aux_source_directory(Engine/OpenGL/Source OPENGL_SRCS)
aux_source_directory(Engine/Core/Source CORE_SRCS)
aux_source_directory(Engine/Noise/Source NOISE_SRCS)
aux_source_directory(Engine/Basic/Source BASIC_SRCS)

include_directories(Engine/Sound/Include Engine/Font/Include)

if(CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(-D_DEBUG)
	SET (LIBRARY_OUTPUT_PATH ../Debug/${OS}/)
else()
	SET (LIBRARY_OUTPUT_PATH ../Release/${OS}/)
endif()

add_library(r5ge STATIC
	${SYSWINDOW_SRCS}
	${SERIALIZATION_SRCS}
	${FONT_SRCS}
	${UI_SRCS}
	${SOUND_SRCS}
	${MATH_SRCS}
	${NEWORK_SRCS}
	${IMAGE_SRCS}
	${OPENGL_SRCS}
	${CORE_SRCS}
	${NOISE_SRCS}
	${BASIC_SRCS}
)
