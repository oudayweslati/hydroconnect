cmake_minimum_required(VERSION 3.16)

project(SmartWaterFactory VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Fonctionne avec Qt 6 ou Qt 5
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

add_executable(SmartWaterFactory
    main.cpp
    mainwindow.h
    mainwindow.cpp
    connexion.h
    connexion.cpp
    equipement_dao.h
    equipement_dao.cpp
    gestion_equipement.h
    gestion_equipement.cpp
    statistiques_dialog.h
    statistiques_dialog.cpp
    historique_dialog.h
    historique_dialog.cpp
    maintenance_predictive.h
    maintenance_predictive.cpp
    xlsx_writer.h
    xlsx_writer.cpp
    logo-data.h
)

target_link_libraries(SmartWaterFactory PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
)

set_target_properties(SmartWaterFactory PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
