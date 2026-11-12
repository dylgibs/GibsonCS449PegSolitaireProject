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

#  Game logic: pure C++, no Qt
add_library(pegcore STATIC
    src/core/Move.h
    src/core/BoardLayout.h
    src/core/BoardLayout.cpp
    src/core/Game.h
    src/core/Game.cpp
    src/core/GameRecord.h
    src/core/GameRecord.cpp
    src/core/GameRecordFile.h
    src/core/GameRecordFile.cpp
    src/core/Replay.h
    src/core/Replay.cpp
    src/core/Solver.h
    src/core/Solver.cpp
    src/core/MoveStrategy.h
    src/core/MoveStrategy.cpp
    src/core/AutoPlayer.h
    src/core/AutoPlayer.cpp
    src/core/RandomState.h
    src/core/RandomState.cpp
)
target_include_directories(pegcore PUBLIC src/core)

#  GUI (Qt Widgets)
add_executable(PegSolitaire
    src/gui/main.cpp
    src/gui/mainwindow.h
    src/gui/mainwindow.cpp
    src/gui/mainwindow.ui
)
target_link_libraries(PegSolitaire PRIVATE pegcore Qt${QT_VERSION_MAJOR}::Widgets)

# Unit tests
add_executable(tst_game tests/tst_game.cpp)
target_link_libraries(tst_game PRIVATE pegcore Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME tst_game COMMAND tst_game)

add_executable(tst_boards tests/tst_boards.cpp)
target_link_libraries(tst_boards PRIVATE pegcore Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME tst_boards COMMAND tst_boards)

add_executable(tst_records tests/tst_records.cpp)
target_link_libraries(tst_records PRIVATE pegcore Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME tst_records COMMAND tst_records)

add_executable(tst_auto tests/tst_auto.cpp)
target_link_libraries(tst_auto PRIVATE pegcore Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME tst_auto COMMAND tst_auto)

add_executable(tst_randomstart tests/tst_randomstart.cpp)
target_link_libraries(tst_randomstart PRIVATE pegcore Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME tst_randomstart COMMAND tst_randomstart)
