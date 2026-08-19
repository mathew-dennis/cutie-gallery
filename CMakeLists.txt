cmake_minimum_required(VERSION 3.16)
project(cutie-gallery VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Concurrent)

add_executable(cutie-gallery
    main.cpp
    src/imagescanner.cpp
    src/imagescanner.h
    src/thumbnailprovider.cpp
    src/thumbnailprovider.h
    gallery.qrc
)

target_include_directories(cutie-gallery PRIVATE src)

target_link_libraries(cutie-gallery PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::Concurrent
)

install(TARGETS cutie-gallery DESTINATION bin)
install(FILES cutie-gallery.desktop DESTINATION share/applications)
