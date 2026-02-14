# Moumou

Moumou is an implementation of a lightweight card game “Moumou” (see [complete rules](rules.md)) with multiple frontends and language bindings. The project includes a console C version, a LÖVE (Lua) version, and other platform-specific implementations (e.g., Gamebuino), demonstrating game logic reuse across environments.

## 🎯 Features

- Pure C console version — portable, lightweight implementation

- LÖVE 2D version — graphical UI using the [LÖVE framework](https://love2d.org/)

- Gamebuino Classic version — [embedded console](https://gamebuino.com/gamebuino-classic) port

Multiple language and platform explorations in one tree


## 🚀 Getting Started
### C (Console)

Build and run the C version:

    cd console
    make
    ./moumou


This version runs in a terminal and enforces game rules without graphics.

### LÖVE 2D (Lua)

Install LÖVE then run:

    cd love2d
    love .


This launches the graphical version of the game using the LÖVE engine.

### Gamebuino Classic

Install anduino-cli then run:

    cd gamebuino
    make

Use [Gamebuino emulator](https://github.com/33d/gbsim) or a real device to run the game.

## 📄 Rules

See [rules.md](rules.md) for full Moumou gameplay rules and scoring details.

## 📦 License

This repository is open source.
