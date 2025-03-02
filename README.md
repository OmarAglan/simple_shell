# Simple Shell

A cross-platform shell designed specifically to provide comprehensive Arabic language support for the Baa programming language ecosystem. This shell addresses critical limitations in standard terminals for Arabic text handling.

## Key Features

- **Full Arabic Language Support**
  - Complete UTF-8 implementation
  - Right-to-left text display
  - Bidirectional text handling
  - Arabic input method support
  - Arabic localization

- **Baa Language Integration**
  - Purpose-built environment for Baa development
  - First-class support for Arabic programming
  - Consistent terminal behavior across platforms

- **Core Shell Capabilities**
  - Basic command execution
  - Environment variable handling
  - Command history
  - Built-in commands (cd, exit, env, etc.)
  - Signal handling (Ctrl+C)
  - Input/output redirection

- **Cross-Platform Compatibility**
  - Windows and Unix/Linux support
  - Consistent behavior across operating systems
  - Platform-specific optimizations with common API

- **Stand-alone Operation**
  - Can be installed as your primary shell
  - Custom console window with enhanced capabilities
  - Modern UI elements on Windows

## Building from Source

### Prerequisites

- C compiler (GCC 4.8+ for Unix/Linux, MSVC 2019+ for Windows)
- CMake 3.10 or higher
- Make (Unix/Linux) or Visual Studio (Windows)

### Build Instructions

#### Unix/Linux
```bash
mkdir build && cd build
cmake ..
make
```

#### Windows (Visual Studio)
```powershell
mkdir build
cd build
cmake ..
cmake --build .
```

### Building a Stand-Alone Shell

To build a fully stand-alone shell with minimal dependencies:

#### Unix/Linux
```bash
mkdir build && cd build
cmake .. -DBUILD_STATIC=ON
make
```

#### Windows
```powershell
# Option 1: Using the provided batch file (recommended)
.\build_standalone.bat

# Option 2: Manual build
mkdir build_standalone
cd build_standalone
cmake .. -G "MinGW Makefiles" -DBUILD_STATIC=ON -DWIN_GUI=ON
mingw32-make
```

The Windows standalone version (`-DWIN_GUI=ON`) creates a true standalone application that:
- Runs independently without requiring cmd.exe or PowerShell
- Has its own console window with proper UTF-8 and Arabic support
- Supports all shell features including Arabic and RTL text

## Installation

### Unix/Linux

Use the provided installation script:

```bash
chmod +x install_linux.sh
./install_linux.sh
```

Options:
- `--prefix=DIR`: Install binaries in DIR (default: /usr/local/bin)
- `--set-default`: Set as default shell for current user
- `--help`: Display help and exit

### Windows

Run the PowerShell installation script as administrator:

```powershell
powershell -ExecutionPolicy Bypass -File install_windows.ps1
```

Options:
- `-InstallDir <path>`: Install binaries in specified directory
- `-CreateShortcut`: Create desktop shortcut
- `-Help`: Display help and exit

## Usage

Run the shell:
```bash
./hsh
```

The shell supports both interactive and non-interactive modes.

### Interactive Mode
```bash
$ ./hsh
($) /bin/ls
hsh main.c shell.c
($) exit
$
```

### Non-interactive Mode
```bash
$ echo "/bin/ls" | ./hsh
hsh main.c shell.c test_ls_2
$
```

## Arabic Language Support

This shell provides comprehensive Arabic language support that standard terminals typically lack:

- **Full RTL Support**: Proper display of right-to-left text following the Unicode Bidirectional Algorithm
- **Arabic Input**: Support for Arabic keyboard layouts and input methods
- **Character Shaping**: Proper joining of Arabic letters (initial, medial, final, and isolated forms)
- **Localization**: Complete Arabic interface with translated messages
- **Mixed Text**: Handling of mixed Arabic/Latin text with proper bidirectional resolution

To switch to Arabic mode:
```
lang ar
```

To switch back to English:
```
lang en
```

## Setting as Default Shell

### Unix/Linux

After installation, you can set the shell as your default login shell:

```bash
chsh -s /usr/local/bin/hsh
```

You'll need to log out and log back in for the change to take effect.

### Windows

The shell can be set as the default terminal application by:

1. Creating a shortcut to the shell
2. Setting it as the default program for command line operations
3. Alternatively, use the Windows Terminal app and configure it to use hsh.exe

#### Running as Standalone Application

When built with the `-DWIN_GUI=ON` option, the shell will run as a standalone application with its own console window. This version:

- Does not require cmd.exe or PowerShell to be running
- Has better Unicode and RTL text support
- Can be launched directly by double-clicking the executable
- Can be pinned to the taskbar or Start menu

## Built-in Commands

- `cd [directory]`: Change current directory
- `exit [status]`: Exit the shell
- `env`: Print environment variables
- `setenv VARIABLE VALUE`: Set an environment variable
- `unsetenv VARIABLE`: Remove an environment variable
- `help`: Display help information
- `history`: Display command history
- `lang [en|ar]`: Switch language between English and Arabic

## Integration with Baa Language

This shell is specifically designed to integrate with the Baa programming language ecosystem:

- **Environment**: Provides the proper terminal environment for Baa language development
- **Text Support**: Handles Arabic text input/output required by Baa
- **Consistent Experience**: Ensures consistent behavior across platforms
- **Future Extensions**: Will support Baa-specific syntax highlighting, completion, and debugging

## Project Structure

```
.
├── AUTHORS
├── CMakeLists.txt
├── README.md
├── CHANGELOG.md
├── install_linux.sh
├── install_windows.ps1
├── docs/
│   ├── ARABIC_SUPPORT_GUIDE.md
│   ├── BAA_INTEGRATION.md
│   ├── CONTRIBUTING.md
│   ├── DEVELOPMENT_TASKS.md
│   ├── DOCUMENTATION_INDEX.md
│   ├── PROJECT_STATUS.md
│   └── ROADMAP.md
├── src/
│   ├── builtin.c
│   ├── environ.c
│   ├── errors.c
│   ├── locale.c
│   ├── utf8.c
│   └── ...
└── include/
    └── shell.h
```

## Documentation

Comprehensive documentation is available in the `docs/` directory:

- `PROJECT_STATUS.md`: Current implementation status
- `ARABIC_SUPPORT_GUIDE.md`: Technical details of Arabic support
- `BAA_INTEGRATION.md`: Integration with Baa language
- `DEVELOPMENT_TASKS.md`: Prioritized development tasks
- `ROADMAP.md`: Detailed development roadmap

For an overview of all documentation, see `docs/DOCUMENTATION_INDEX.md`.

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Create a Pull Request

See `docs/CONTRIBUTING.md` for detailed contribution guidelines.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
