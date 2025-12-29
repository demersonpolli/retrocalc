# RetroCalc: A VisiCalc-Inspired Spreadsheet

RetroCalc is an open-source, cross-platform (Linux, Unix, Mac, Windows) C++ project that aims to recreate the experience and functionality of the original VisiCalc spreadsheet—the first widely-used electronic spreadsheet program. The goal is to provide a fast, keyboard-driven, terminal-based spreadsheet for modern systems, with a retro feel and extensible codebase.

## Project Vision
- **Faithful to VisiCalc**: Reproduce the look, feel, and workflow of the original VisiCalc, including cell navigation, formula entry, and command structure.
- **Modern C++**: Clean, portable code using C++17 and CMake.
- **Terminal UI**: Runs in a terminal/console, using only keyboard input for maximum speed and nostalgia.
- **Extensible**: Designed for easy addition of new features and commands.

## Key Features (Planned)
- Spreadsheet grid with cell navigation (arrow keys, jump to cell)
- Cell types: label, value, formula
- In-place editing and formula entry
- Save/load sheets to file (JSON or CSV)
- Command mode for advanced operations
- Cross-platform compatibility

## Main Commands (Planned)
- `/` : Enter command mode
- `/B` : Blank (clear) the current cell
- `/C` : Clear the entire sheet
- `/E` : Edit the current cell
- `/S` : Enter storage submode (save/load)
    - `/SS` : Save sheet
    - `/SL` : Load sheet
- `/J` : Jump to a specific cell (e.g., `/JA1`)
- Arrow keys: Move active cell
- Enter: Edit cell
- ESC: Cancel/exit modes

## Getting Started
1. Clone the repository
2. Build with CMake and your C++17 compiler
3. Run `retrocalc` in your terminal

## Contributing
Contributions are welcome! Please open issues or pull requests for features, bug fixes, or suggestions.

## License
MIT License

---
Inspired by the legacy of VisiCalc and the spirit of retro computing.
