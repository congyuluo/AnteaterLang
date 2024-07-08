# AnteaterLang and Anteater IDE

## Overview

AnteaterLang is a programming language which supports easy C acceleration. This repository includes all the necessary scripts and documentation to install, use, and uninstall AnteaterLang and Anteater IDE.

## File System

- **Development Documentation.md:** Contains documentation for development.
- **install_anteaterlang.sh:** Script to install AnteaterLang.
- **LICENSE:** The license file for the project.
- **sample_source:** Contains sample source files.
- **cmake-build-debug:** Contains the debug build files.
- **src:** Source directory for the project.
- **ide_src:** Source directory for the IDE.
- **install_anteater_ide.sh:** Script to install Anteater IDE.
- **uninstall_anteater_ide.sh:** Script to uninstall Anteater IDE.
- **uninstall_anteaterlang.sh:** Script to uninstall AnteaterLang.

## Installation Instructions

### Installing AnteaterLang

1. **Clone the Repository:**
   ```sh
   git clone https://github.com/your-username/your-repository.git
   cd your-repository
    ```
   
2. **Run the Installation Script:**
    ```sh
    ./install_anteaterlang.sh
    ```
   
3. **Verify Installation:**
    ```sh
    antlang --version
    ```
   
### Installing Anteater IDE

1. **Clone the Repository:**
   ```sh
   ./install_anteater_ide.sh
   ```
   
## Uninstallation Instructions

### Uninstalling AnteaterLang

1. **Run the Uninstallation Script:**
    ```sh
    ./uninstall_anteaterlang.sh
    ```
   
### Uninstalling Anteater IDE

1. **Run the Uninstallation Script:**
    ```sh
    ./uninstall_anteater_ide.sh
    ```
   
## Usage Instructions

### Running a Script with AnteaterLang

To run a script using AnteaterLang, use the antlang command followed by the script path:

```sh
antlang path/to/your_script.ant
```

### Compiling C Acceleration Source with AnteaterLang

To compile C acceleration source, use the antcompile command:

```sh
antcompile path/to/your_c_source.c
```

### Using the antlang Command

- To run a script with accLib:
  ```sh
  antlang accLib path/to/source
  ```
  
- To run a regular source:
  ```sh
  antlang path/to/source
  ```
  
