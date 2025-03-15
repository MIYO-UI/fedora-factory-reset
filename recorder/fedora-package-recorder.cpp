#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <filesystem>
#include <ctime>

namespace fs = std::filesystem;

const std::string PACKAGES_FILE = "/var/lib/factory-reset/installed-packages.txt";
const std::string TIMESTAMP_FILE = "/var/lib/factory-reset/installation-timestamp.txt";
const std::string CONFIG_DIR = "/var/lib/factory-reset";

// Zapisuje listę wszystkich zainstalowanych pakietów
bool saveInstalledPackages() {
    try {
        // Utwórz katalog, jeśli nie istnieje
        if (!fs::exists(CONFIG_DIR)) {
            fs::create_directories(CONFIG_DIR);
        }

        // Wykonaj komendę dnf list installed i zapisz wynik do pliku
        std::string command = "dnf list installed > " + PACKAGES_FILE;
        int result = std::system(command.c_str());
        
        if (result != 0) {
            std::cerr << "Błąd podczas wykonywania komendy dnf" << std::endl;
            return false;
        }

        // Zapisz aktualny czas instalacji
        std::ofstream timestamp(TIMESTAMP_FILE);
        if (!timestamp) {
            std::cerr << "Nie można utworzyć pliku znacznika czasu" << std::endl;
            return false;
        }
        
        std::time_t now = std::time(nullptr);
        timestamp << now << std::endl;
        timestamp.close();
        
        // Ustaw odpowiednie uprawnienia do plików
        std::system(("chmod 644 " + PACKAGES_FILE).c_str());
        std::system(("chmod 644 " + TIMESTAMP_FILE).c_str());
        std::system(("chmod 755 " + CONFIG_DIR).c_str());
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Wystąpił błąd: " << e.what() << std::endl;
        return false;
    }
}

// Sprawdza, czy program został już wcześniej uruchomiony
bool isFirstRun() {
    return !fs::exists(PACKAGES_FILE);
}

// Wyłącza usługę systemd, aby nie uruchamiała się ponownie
void disableService() {
    std::system("systemctl disable fedora-package-recorder.service");
}

int main() {
    if (isFirstRun()) {
        std::cout << "Pierwsze uruchomienie - zapisywanie zainstalowanych pakietów..." << std::endl;
        
        if (saveInstalledPackages()) {
            std::cout << "Pakiety zostały zapisane pomyślnie." << std::endl;
        } else {
            std::cerr << "Nie udało się zapisać listy pakietów!" << std::endl;
            return 1;
        }
    } else {
        std::cout << "Pakiety zostały już wcześniej zapisane." << std::endl;
    }
    
    // Wyłączenie usługi po pierwszym uruchomieniu
    disableService();
    
    return 0;
}