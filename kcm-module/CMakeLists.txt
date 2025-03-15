# Projekt modułu KCM
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Źródła modułu KCM
set(KCM_SOURCES
    src/factory-reset-kcm.cpp
    src/factory-reset-helper.cpp
)

# Nagłówki
set(KCM_HEADERS
    src/factory-reset-kcm.h
    src/factory-reset-helper.h
)

# Utwórz bibliotekę modułową
add_library(kcm_factory_reset MODULE 
    ${KCM_SOURCES}
    ${KCM_HEADERS}
)

# Ścieżki do nagłówków
target_include_directories(kcm_factory_reset PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Linkuj z bibliotekami Qt i KDE
target_link_libraries(kcm_factory_reset
    Qt5::Core
    Qt5::Widgets
    KF5::CoreAddons
    KF5::ConfigWidgets
    KF5::I18n
    KF5::WidgetsAddons
)

# Instalacja modułu KCM
install(TARGETS kcm_factory_reset DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)

# Instalacja plików konfiguracyjnych
install(FILES resources/factory-reset.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES resources/factory-reset.json DESTINATION ${KDE_INSTALL_KCFGDIR})

# Komunikat informacyjny
message(STATUS "Moduł KCM skonfigurowany do kompilacji i instalacji")