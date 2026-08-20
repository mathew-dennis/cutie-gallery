cmake_minimum_required(VERSION 3.16)
project(cutie-gallery VERSION 1.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Qml Concurrent)

qt_standard_project_setup()

set(cutie-gallery_SOURCES
	src/main.cpp
	src/imagescanner.cpp
	src/thumbnailprovider.cpp
)

qt_add_resources(APP_RESOURCES
	${CMAKE_SOURCE_DIR}/src/qml/qml.qrc)

qt_add_executable(cutie-gallery ${cutie-gallery_SOURCES} ${APP_RESOURCES})

target_include_directories(cutie-gallery PUBLIC src)

target_link_libraries(cutie-gallery PUBLIC Qt::Core Qt::Quick Qt::Qml Qt::Concurrent)

install(TARGETS cutie-gallery RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES cutie-gallery.desktop DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
install(FILES cutie-gallery.svg DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps")
