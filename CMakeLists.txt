cmake_minimum_required(VERSION 3.16)

project(earth3d VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
find_package(OpenGL REQUIRED)
find_package(GLU REQUIRED)

# Создаем директорию для текстур в папке сборки
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/textures)

# Копируем текстуры при сборке
file(COPY
    ${CMAKE_SOURCE_DIR}/textures/earth.jpg
    ${CMAKE_SOURCE_DIR}/textures/earth_height.png
    ${CMAKE_SOURCE_DIR}/textures/earth_normal.png
    ${CMAKE_SOURCE_DIR}/textures/earth_night.jpg
    ${CMAKE_SOURCE_DIR}/textures/earth_clouds.jpg
    ${CMAKE_SOURCE_DIR}/textures/earth_specular.jpg
    ${CMAKE_SOURCE_DIR}/textures/earth_temperature.jpg
    ${CMAKE_SOURCE_DIR}/textures/earth_snow.jpg
    DESTINATION ${CMAKE_BINARY_DIR}/textures
)

set(PROJECT_SOURCES
        main.cpp


        earthwidget.cpp
        earthwidget.h
        resources.qrc

)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(earth3d
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        satellite.h
        camera.h camera.cpp
        renderer.h
        fps_renderer.h fps_renderer.cpp
        earth_renderer.h earth_renderer.cpp
        satellite_renderer.h satellite_renderer.cpp
        trajectory_renderer.h trajectory_renderer.cpp
        renderer.cpp
        satellite_info_renderer.h satellite_info_renderer.cpp
        tile_texture_manager.h tile_texture_manager.cpp
        atmosphere_renderer.h atmosphere_renderer.cpp


    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET earth3d APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(earth3d SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(earth3d
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(earth3d PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    OpenGL::GL
    OpenGL::GLU
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.earth3d)
endif()
set_target_properties(earth3d PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS earth3d
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(earth3d)
endif()
