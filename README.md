# Fedora Factory Reset

A comprehensive tool for restoring Fedora Linux systems to factory settings, consisting of two main components:

1. **Package Recorder** - A service that runs at the first system boot and records the list of originally installed packages
2. **KDE Settings Plugin** - A KCM (KDE Config Module) for the KDE System Settings that enables users to restore the system to its factory state

## Features

- Records the initial system state automatically on first boot
- Provides a user-friendly KDE interface for system restoration
- Offers options to:
  - Remove additional packages installed after the initial setup
  - Reinstall missing packages that were part of the original installation
  - Remove all user data and accounts
- Integrates with the Fedora's DNF package manager
- Includes a systemd service for automated operation

## Project Structure

```
fedora-factory-reset/
├── CMakeLists.txt                 # Main CMake configuration
├── README.md                      # This file
├── recorder/                      # Package recorder component
│   ├── CMakeLists.txt             # CMake file for recorder
│   ├── fedora-package-recorder.cpp # Recorder source code
│   └── fedora-package-recorder.service # Systemd service file
└── kcm-module/                    # KDE plugin component
    ├── CMakeLists.txt             # CMake file for KCM module
    ├── src/                       # Source directory
    │   ├── factory-reset-kcm.cpp  # KCM module implementation
    │   ├── factory-reset-kcm.h    # KCM module header
    │   ├── factory-reset-helper.cpp # Helper class implementation
    │   └── factory-reset-helper.h # Helper class header
    └── resources/                 # Resources directory
        ├── factory-reset.desktop  # Desktop file for KDE menu integration
        └── factory-reset.json     # Plugin configuration file
```

## Requirements

- C++ compiler with C++17 support
- CMake (version 3.10 or newer)
- Qt 5 (Core and Widgets libraries)
- KDE Frameworks 5 (KConfigWidgets, KI18n, KCoreAddons, KWidgetsAddons)
- Fedora Linux with DNF package manager
- KDE Plasma desktop environment

## Installation

### Building from Source

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/fedora-factory-reset.git
   cd fedora-factory-reset
   ```

2. Create a build directory and compile:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

3. Install the components:
   ```bash
   sudo make install
   ```

### Enabling the Package Recorder Service

After installation, enable the package recorder service to run at system boot:

```bash
sudo systemctl enable fedora-package-recorder.service
```

For first-time installation, you can start the service immediately:

```bash
sudo systemctl start fedora-package-recorder.service
```

### Creating an RPM Package

The project includes CPack configuration for building RPM packages:

```bash
cd build
make package
```

This will create an RPM package that you can distribute and install on other Fedora systems.

## Usage

### Package Recorder

The package recorder service automatically runs at the first system boot and records the list of originally installed packages to `/var/lib/factory-reset/installed-packages.txt`. It disables itself after the first run.

### KDE Settings Plugin

The factory reset functionality can be accessed through the KDE System Settings:

1. Open KDE System Settings
2. Navigate to "System Administration" section
3. Click on "Factory Reset"

The plugin offers two main options:
- **Remove additional packages and restore original ones**: This will uninstall any packages that weren't part of the original installation and reinstall any missing original packages
- **Remove all user data**: This will delete all user accounts and their home directories

To perform a factory reset:
1. Select the desired options
2. Check the confirmation box to acknowledge that the operation is irreversible
3. Click the "Restore System to Factory Settings" button
4. Confirm the action in the dialog that appears

After the reset operation completes, the system will automatically reboot to apply all changes.

## Development

To build the project with debugging information:

```bash
mkdir debug-build && cd debug-build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

To build only a specific component:

```bash
cmake -DBUILD_RECORDER=ON -DBUILD_KCM=OFF ..  # Build only the recorder
cmake -DBUILD_RECORDER=OFF -DBUILD_KCM=ON ..  # Build only the KCM module
```

## License

This project is licensed under the GNU General Public License v3.0 (GPL-3.0).