cmake_minimum_required(VERSION 3.16)

project(PhotoChopped VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Svg)

set(PROJECT_SOURCES
    main.cpp
    windows/mainwindow.cpp
    windows/mainwindow.h
    windows/mainwindow.ui
    canvas/canvas.cpp
    canvas/canvas.h
    tools/tool.h
    tools/brush.cpp
    tools/brush.h
    tools/eraser.cpp
    tools/eraser.h
    icons.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PhotoChopped
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        tools/pipette.h tools/pipette.cpp
        windows/newimagedialog.h windows/newimagedialog.cpp
        tools/pencil.h tools/pencil.cpp
        tools/bucket.h tools/bucket.cpp
    )
else()
    if(ANDROID)
        add_library(PhotoChopped SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(PhotoChopped
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(PhotoChopped PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
)

target_include_directories(PhotoChopped PRIVATE
    windows
    canvas
    tools
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.PhotoChopped)
endif()
set_target_properties(PhotoChopped PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS PhotoChopped
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PhotoChopped)
endif()