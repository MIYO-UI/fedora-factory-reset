cmake_minimum_required(VERSION 3.10)
project(fedora-factory-reset VERSION 1.0.0 LANGUAGES CXX)

# Ustawienia globalne
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Znajdź pakiety ECM
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

# Włącz polityki KDE
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

# Znajdź wymagane pakiety Qt i KDE
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(KF5 REQUIRED COMPONENTS I18n ConfigWidgets CoreAddons WidgetsAddons)

# Opcje kompilacji
option(BUILD_RECORDER "Skompiluj program rejestratora pakietów" ON)
option(BUILD_KCM "Skompiluj moduł KCM (wtyczkę KDE)" ON)

# Dodaj subprojekty
if(BUILD_RECORDER)
    add_subdirectory(recorder)
endif()

if(BUILD_KCM)
    add_subdirectory(kcm-module)
endif()

# Konfigurowanie pakietu RPM
set(CPACK_PACKAGE_NAME "fedora-factory-reset")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "admin@example.com")
set(CPACK_PACKAGE_VENDOR "Fedora")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Narzędzie do przywracania systemu Fedora do stanu fabrycznego")
set(CPACK_PACKAGE_DESCRIPTION "
Pakiet zawiera:
- Rejestrator pakietów uruchamiany przy pierwszym starcie systemu
- Wtyczkę do KDE do przywracania systemu do stanu fabrycznego
")
set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0")
set(CPACK_RPM_PACKAGE_GROUP "System Environment/Base")
set(CPACK_RPM_PACKAGE_REQUIRES "dnf, kf5-kconfigwidgets, kf5-ki18n")
set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")

# Nie kompresuj man i info stron
set(CPACK_RPM_SPEC_MORE_DEFINE "%define _build_id_links none")

# Włącz CPack
include(CPack)

# Podsumowanie
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)