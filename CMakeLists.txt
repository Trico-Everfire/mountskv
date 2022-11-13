cmake_minimum_required(VERSION 3.23)

project( mountskv VERSION 1.0.0 LANGUAGES CXX )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
set(CMAKE_INSTALL_RPATH $ORIGIN)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "..." FORCE)
endif()

if(UNIX)
    set(CMAKE_CXX_STANDARD 20)  # required for g++ to compile

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(QT_INCLUDE "/usr/include/x86_64-linux-gnu/qt5")

    if(NOT DEFINED QT_BASEDIR)
        set(QT_BASEDIR "$ENV{HOME}/Qt/5.15.2/gcc_64/")
    endif()
    if(DEFINED QT_BASEDIR)
        set(CMAKE_PREFIX_PATH "${QT_BASEDIR}/lib/cmake")
    else()
        message(FATAL_ERROR "--!@ Please define your QT install dir with -DQT_BASEDIR=C:/your/qt5/here")
    endif()
elseif(WIN32)
    set(CMAKE_CXX_STANDARD 23)  # required for MSVC to support c++ 20 modules

    # On Windows, the install dir must be specified, as it's not standardized
    if(DEFINED QT_BASEDIR)
        # Add it to the prefix path so find_package can find it
        list(APPEND CMAKE_PREFIX_PATH ${QT_BASEDIR})
        set(QT_INCLUDE ${QT_BASEDIR}/include)
        # CMake has an odd policy that links a special link lib for Qt on newer versions of CMake. Enable it so we don't get spammed, and I get to write less
        cmake_policy(SET CMP0020 NEW)
    else()
        message(FATAL_ERROR "--!@ Please define your QT install dir with -DQT_BASEDIR=C:/your/qt5/here")
    endif()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui Network)

set(SRC
        src/main.cpp src/MainView.cpp src/MainView.h src/widgets/DraggableTreeWidget.h)

add_executable(${PROJECT_NAME} ${SRC} res/resource.qrc)

#add_library(keyvalues)
#add_library(fmt)
add_subdirectory( libs/speedykv )
add_subdirectory( libs/fmt )
add_subdirectory( libs/SteamAppPathProvider )

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network keyvalues fmt SAPP)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libs/speedykv")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libs/SteamAppPathProvider")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libs/fmt/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${QT_INCLUDE} ${QT_INCLUDE}/QtWidgets" "${QT_INCLUDE}/QtGui" "${QT_INCLUDE}/QtCore" "${QT_INCLUDE}/QtNetwork")

if(WIN32)
    # Copy these to bundle them with the program in releases
    configure_file("${QT_BASEDIR}/bin/Qt5Core.dll"    "${CMAKE_BINARY_DIR}/Qt5Core.dll"    COPYONLY)
    configure_file("${QT_BASEDIR}/bin/Qt5Cored.dll"    "${CMAKE_BINARY_DIR}/Qt5Cored.dll"    COPYONLY)
    configure_file("${QT_BASEDIR}/bin/Qt5Gui.dll"     "${CMAKE_BINARY_DIR}/Qt5Gui.dll"     COPYONLY)
    configure_file("${QT_BASEDIR}/bin/Qt5Guid.dll"     "${CMAKE_BINARY_DIR}/Qt5Guid.dll"     COPYONLY)
    configure_file("${QT_BASEDIR}/bin/Qt5Widgets.dll" "${CMAKE_BINARY_DIR}/Qt5Widgets.dll" COPYONLY)
    configure_file("${QT_BASEDIR}/bin/Qt5Widgetsd.dll" "${CMAKE_BINARY_DIR}/Qt5Widgetsd.dll" COPYONLY)
    configure_file("${QT_BASEDIR}/bin/Qt5Network.dll" "${CMAKE_BINARY_DIR}/Qt5Network.dll" COPYONLY)
    configure_file("${QT_BASEDIR}/bin/Qt5Networkd.dll" "${CMAKE_BINARY_DIR}/Qt5Networkd.dll" COPYONLY)
    configure_file("${QT_BASEDIR}/plugins/platforms/qwindows.dll"         "${CMAKE_BINARY_DIR}/platforms/qwindows.dll"         COPYONLY)
    configure_file("${QT_BASEDIR}/plugins/platforms/qwindowsd.dll"        "${CMAKE_BINARY_DIR}/platforms/qwindowsd.dll"        COPYONLY)
elseif(UNIX)
    configure_file("${QT_BASEDIR}/lib/libQt5Core.so.5.15.2" "${CMAKE_BINARY_DIR}/libQt5Core.so.5" COPYONLY)
    configure_file("${QT_BASEDIR}/lib/libQt5Gui.so.5.15.2" "${CMAKE_BINARY_DIR}/libQt5Gui.so.5" COPYONLY)
    configure_file("${QT_BASEDIR}/lib/libQt5Widgets.so.5.15.2" "${CMAKE_BINARY_DIR}/libQt5Widgets.so.5" COPYONLY)
    configure_file("${QT_BASEDIR}/lib/libQt5Network.so.5.15.2" "${CMAKE_BINARY_DIR}/libQt5Network.so.5" COPYONLY)
    configure_file("${QT_BASEDIR}/lib/libicui18n.so.56" "${CMAKE_BINARY_DIR}/libicui18n.so.56" COPYONLY)
    configure_file("${QT_BASEDIR}/lib/libicuuc.so.56" "${CMAKE_BINARY_DIR}/libicuuc.so.56" COPYONLY)
    configure_file("${QT_BASEDIR}/lib/libicudata.so.56" "${CMAKE_BINARY_DIR}/libicudata.so.56" COPYONLY)
    configure_file("${QT_BASEDIR}/plugins/imageformats/libqtga.so" "${CMAKE_BINARY_DIR}/libqtga.so")
    configure_file("${QT_BASEDIR}/plugins/platforms/libqminimal.so" "${CMAKE_BINARY_DIR}/platforms/libqminimal.so" COPYONLY)
    configure_file("${QT_BASEDIR}/plugins/platforms/libqxcb.so" "${CMAKE_BINARY_DIR}/platforms/libqxcb.so" COPYONLY)
endif()