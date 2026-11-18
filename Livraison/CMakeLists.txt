cmake_minimum_required(VERSION 3.16)
project(HydroConnectLivraison VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

qt_add_executable(HydroConnectLivraison
    main.cpp
    fenetre_principale.h fenetre_principale.cpp
    resources.qrc
    connection.h connection.cpp
    schema.h schema.cpp
    livraison.h livraison.cpp
    gestion_livraison.h gestion_livraison.cpp
    livraison_donutwidget.h livraison_donutwidget.cpp
    responsive.h
)

target_link_libraries(HydroConnectLivraison PRIVATE Qt6::Widgets Qt6::Sql)
