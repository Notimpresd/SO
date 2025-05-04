# Treasure Hunt Game System

## Overview

This project implements a UNIX-based treasure hunt game system in C. It simulates digital treasure hunts where users can create, manage, and participate in treasure hunts. The system uses file operations to store treasure data, process management, and Unix signals for inter-process communication and control. The project is structured in phases, with Phase 1 focusing on file system management and Phase 2 extending functionality with processes and signals.

---

## Project Components

### treasure_manager

- Manages treasure data files organized by hunt IDs.
- Supports adding, listing, viewing, and removing treasures, as well as removing entire hunts.
- Stores treasures in binary files inside hunt-specific directories.
- Logs all operations to a `logged_hunt` file within the hunt directory.
- Creates symbolic links to log files in the root directory for convenience.

### treasure_hub

- Provides an interactive command-line interface for users.
- Starts and manages a background monitor process that handles user requests.
- Uses Unix signals (`SIGUSR1`, `SIGTERM`, `SIGCHLD`) to communicate with the monitor.
- Supports commands for listing hunts, treasures, viewing treasure details, and controlling the monitor lifecycle.

---

## Phase 1: File Systems

### Features

- Supports fixed-size binary records for treasures.
- Creates directories per hunt to organize treasure files.
- Uses system calls like `open()`, `close()`, `read()`, `write()`, `lseek()`, and `stat()` for file handling.
- Performs data validation and error handling for robust operation.
- Operation logging for audit and debugging.
- Symbolic links to log files named `logged_hunt-<HUNT_ID>` are created at the program root.

### Usage Examples

```bash
./treasure_manager add Hunt001
./treasure_manager list Hunt001
./treasure_manager view Hunt001 1
./treasure_manager remove_treasure Hunt001 1
./treasure_manager remove_hunt Hunt001
