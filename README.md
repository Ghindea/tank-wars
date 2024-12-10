<img style="display: block;
    margin-left: auto;
    margin-right: auto;
    width: 30%;"
    src="./docs/demo.png"
    alt="https://github.com/Ghindea/QR_code_beta">
</img>

# **Tank Wars**
by _[Daniel Ghindea](https://www.github.com/Ghindea)_

**Tank Wars** is a simple 2D shooter game, based on [GFX Framework](docs/README.md), developed in C++ using the OpenGL graphics library. The player's objective is to destroy the enemy tank while avoiding being destroyed themselves.

## Contents
- [Building project](#building-project)
- [How it works](#how-it-works)
- [Controls](#controls)
- [Bibliography](#bibliography)
- [License](#license)

## Building project

**Note:** Make sure that your environment follows the prerequisites listed in the [GFX Framework documentation](docs/README.md/#white_check_mark-prerequisites).

Open a terminal and go into the root folder of the project, which contains the top-level `CMakeLists.txt` file.
Do not run CMake directly from the top-level folder (meaning, do not do this: `cmake .`). Instead, make a separate directory, as follows:

1.  `mkdir build`
2.  `cd build`
3.  Generate the project:
    `cmake ..`
4.  Build the project:
    -   Windows, one of the following:
        -   `cmake --build .`
    -   Linux and macOS, one of the following:
        -   `cmake --build .`
        -   or just `make`

That's it! :tada:

**Note:** When running the CMake generation step on Windows (meaning, `cmake ..`), you may receive errors that include the phrase `Permission denied`. If you currently have the framework on the `C:` drive, for example on the Desktop, you might not have write permissions. Experimentally, we've found that this happens on single-drive devices (meaning, for example, computers with no disk partition other than `C:`). As an alternative, move the framework on the top-level (directly in `C:`) or create another drive partition (for example `D:`) and move it there.



## How it works

## Controls
**Player 1:**
- Movement: `W`, `A`, `S`, `D`
- Fire: `Space`
- Change color: `1`

**Player 2:**
- Movement: `Up`, `Left`, `Down`, `Right`
- Fire: `Enter`
- Change color: `2`

**Gameplay:**
- Restart game: `R`
- Display trajectory: `L`
- Toggle ammo limit: `U`
- Sound on/off: `M`
- Night mode: `O`

## Bibliography

- [GFX Framework](docs/README.md)
- [OpenGL](https://www.opengl.org/)
- 

## License




   