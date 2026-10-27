cmake_minimum_required(VERSION 3.16)
project(PegSolitaire LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Works with Qt 6 or Qt 5.15+
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Test)

enable_testing()

# ---- Game logic: pure C++, no Qt ----
add_library(pegcore STATIC
    src/core/Move.h
    src/core/BoardLayout.h
    src/core/BoardLayout.cpp
    src/core/Game.h
    src/core/Game.cpp
)
target_include_directories(pegcore PUBLIC src/core)

# ---- GUI (Qt Widgets) ----
add_executable(PegSolitaire
    src/gui/main.cpp
    src/gui/mainwindow.h
    src/gui/mainwindow.cpp
    src/gui/mainwindow.ui
)
target_link_libraries(PegSolitaire PRIVATE pegcore Qt${QT_VERSION_MAJOR}::Widgets)

# ---- Unit tests (QtTest, xUnit style) ----
add_executable(tst_game tests/tst_game.cpp)
target_link_libraries(tst_game PRIVATE pegcore Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME tst_game COMMAND tst_game)
