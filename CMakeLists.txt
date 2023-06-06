cmake_minimum_required(VERSION 3.14)

project(qjsonpath LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(qjsonpath
  main.cpp
  qjsonpath.h
  qjsonpath.cpp
)
target_link_libraries(qjsonpath Qt${QT_VERSION_MAJOR}::Core)

install(TARGETS qjsonpath
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_compile_definitions(DEBUG_HANDLE_JSON_ATTRIBUTE)
