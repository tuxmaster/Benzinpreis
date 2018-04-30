cmake_minimum_required(VERSION 3.1)
project(Benzinpreis)

set(QT_MIN_VERSION "5.9.0")
option(USE_LD_GOLD "Use gold linker" ON)

if (USE_LD_GOLD)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE LD_VERSION)
    if ("${LD_VERSION}" MATCHES "GNU gold")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fuse-ld=gold")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=gold")
    else ()
	message(WARNING "Gold linker isn't available, using the default system linker.")
    endif ()
endif()

find_package( Qt5Widgets ${QT_MIN_VERSION} REQUIRED)
find_package( Qt5Network ${QT_MIN_VERSION} REQUIRED)
find_package( Qt5Sql ${QT_MIN_VERSION} REQUIRED)
find_package( Qt5Positioning ${QT_MIN_VERSION} REQUIRED)

include_directories( ${CMAKE_BINARY_DIR} )

set( QUELLEN
    "Quellen/Start.cpp"
    "Quellen/DlgHauptfenster.cpp"
    "Quellen/DlgHauptfenster.h"
    "Quellen/Steuerung.cpp"
    "Quellen/Steuerung.h"
    "Quellen/PLZ_Datenbank.cpp"
    "Quellen/PLZ_Datenbank.h"
    "Quellen/Vorgaben.h"
    "Quellen/Tankstellen.cpp"
    "Quellen/Tankstellen.h"
    "Quellen/Preissuche.h"
    "Quellen/Tankstelle.h"
    "Quellen/Öffnungszeiten.h"
    )

set( MOC
    "Quellen/DlgHauptfenster.h"
    "Quellen/Steuerung.h"
    "Quellen/PLZ_Datenbank.h"
    "Quellen/Tankstellen.h"
    )

set( UI
    "Dialoge/DlgHauptfenster.ui"
    )


qt5_wrap_ui( UI_HEADER ${UI} )
qt5_wrap_cpp( MOC_SRC ${MOC} )

add_executable( ${PROJECT_NAME} ${UI_HEADER} ${QUELLEN} ${MOC_SRC})

target_link_libraries( ${PROJECT_NAME} Qt5::Widgets Qt5::Network Qt5::Sql Qt5::Positioning )



