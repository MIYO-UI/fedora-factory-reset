# Projekt rejestratora pakietów
set(RECORDER_SOURCES
    fedora-package-recorder.cpp
)

# Utwórz plik wykonywalny
add_executable(fedora-package-recorder ${RECORDER_SOURCES})

# Upewnij się, że kompilator wspiera C++17
target_compile_features(fedora-package-recorder PRIVATE cxx_std_17)

# Instalacja pliku wykonywalnego
install(TARGETS fedora-package-recorder DESTINATION ${CMAKE_INSTALL_BINDIR})

# Instalacja pliku usługi systemd
install(FILES fedora-package-recorder.service 
        DESTINATION /usr/lib/systemd/system)

# Utwórz katalog konfiguracyjny
install(CODE "execute_process(COMMAND mkdir -p \$ENV{DESTDIR}/var/lib/factory-reset)")

# Ustaw odpowiednie uprawnienia
install(CODE "execute_process(COMMAND chmod 755 \$ENV{DESTDIR}/var/lib/factory-reset)")

# Opcjonalnie - włącz usługę systemd podczas instalacji
if(NOT DEFINED ENV{DESTDIR})
    install(CODE "execute_process(COMMAND systemctl enable fedora-package-recorder.service WORKING_DIRECTORY / RESULT_VARIABLE systemd_result)")
    install(CODE "if(NOT systemd_result EQUAL 0)
                   message(WARNING \"Failed to enable systemd service.\")
                 endif()")
endif()

# Komunikat informacyjny
message(STATUS "Rejestrator pakietów skonfigurowany do kompilacji i instalacji")