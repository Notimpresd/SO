UNIX Treasure Hunt Game

This is a C-based command-line project that simulates a digital treasure hunt using the UNIX file system, process management, and signals. The project includes two main modules: (1) treasure_manager – handles treasure hunt creation, storage, and retrieval; and (2) treasure_hub – provides a user interface and manages a background monitor process for interaction.

Features include: binary file management for storing treasures, directory-based hunt management with logging, symbolic link creation for quick access to logs, background process handling via Unix signals, and an interactive command-line interface.

Prerequisites: A UNIX-like environment (Linux or macOS) with GCC (GNU Compiler Collection) and optionally `make`. A shell like Bash is required. Check installation with `gcc --version` and `make --version`.

To compile the project:

Option 1 – with Makefile:
    make

Option 2 – manually:
   ` gcc -o treasure_manager treasure_manager.c
    gcc -o treasure_hub treasure_hub.c`

This will create two executables: ./treasure_manager and ./treasure_hub

Usage examples:

To create a hunt:
   `` ./treasure_manager create <hunt_id>``

To add a treasure:
   ` ./treasure_manager add <hunt_id> "<name>" "<description>" <value>`

To list treasures:
   ` ./treasure_manager list <hunt_id>`

To view a treasure:
  `  ./treasure_manager view <hunt_id> <index>`

To remove a treasure:
`    ./treasure_manager remove <hunt_id> <index>`

To delete a hunt:
  `  ./treasure_manager delete <hunt_id>`

To run the hub:
   ` ./treasure_hub`

This launches an interface and a background process that communicates via signals like SIGUSR1. Use the interface to control the hunt or exit cleanly.

```File and directory structure:
- treasure_manager.c        - logic for managing hunts
- treasure_hub.c            - user interface and process management
- Makefile                  - optional build file
- <hunt_id>/                - directory created per hunt
  ├── treasures.dat         - binary treasure data
  └── logged_hunt           - log file for operations
- hunt_<id>_log_link        - symbolic link to log file
```

Possible improvements: input validation, user authentication, Windows compatibility via WSL, CLI UI enhancements.

Author: Notimpresd (https://github.com/Notimpresd)

License: MIT License
