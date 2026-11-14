# IceMan

A 2D digging game in C++ with OpenGL (freeglut). Dig through the ice to collect every oil barrel on each level while you dodge protesters and falling boulders.

## Running the game

### Windows (Visual Studio 2022)

1. Install Visual Studio 2022 with the **Desktop development with C++** workload.
2. Clone this repo, or download it as a ZIP.
3. Make sure `freeglut.dll` and `irrKlang.dll` are in the `IceMan/` folder, next to `main.cpp`. The game won't start without them. If they're missing, get the **32-bit** DLLs from freeglut 3.0 (MSVC build) and irrKlang 1.5.
4. Open `IceMan.sln`.
5. Set the configuration to **Debug** and the platform to **x86**.
6. Press **Ctrl+F5** to build and run.

If you get `Cannot find dig1.tga in Assets`, the game is running from the wrong folder. Go to **Project → Properties → Debugging** and set **Working Directory** to `$(ProjectDir)`.

### Linux

```bash
sudo apt install g++ freeglut3-dev
cd IceMan
g++ -std=c++14 -O2 -I. -o IceMan *.cpp -lglut -lGLU -lGL
./IceMan
```

Run it from inside the `IceMan/` folder so it can find `Assets/`. There's no sound on Linux.

## Controls

| Key | Action |
|---|---|
| Arrow keys or W A S D | Move and dig. The first press turns you, the next presses move you. |
| Space | Squirt water at protesters |
| Z | Use sonar to reveal nearby oil and gold |
| Tab or T | Drop a gold nugget to bribe a protester |
| Enter | Start or continue |
| Q | Quit |
