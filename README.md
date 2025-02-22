# GGBoy-Qt  
**A Minimal Qt Frontend for GGBoy-Core**

Basic graphical interface for the [GGBoy-Core](https://github.com/yourusername/ggboy-core) emulator, built with Qt 6 and SDL2. Provides essential playability with:  
- Savestates (4 slots via function keys)  
- Real-time audio channel toggling  
- Reset functionality
- Speedup emulation

## Getting Started
Clone with submodules using:  
```bash
git clone --recurse-submodules https://github.com/Georg-S/ggboy-qt.git
```

## Windows Build Requirements  
1. **Qt 6** installed with MSVC2019 64-bit components  
2. **SDL2** development libraries  
3. **Environment Variables** (set before building):  
   - `SDL2_INCLUDE_DIRS`: Path to SDL2 include folder  
   - `SDL2_LIBRARIES`: Path to SDL2 lib/x64 folder  
   - `QT_MSVC_64`: Path to Qt MSVC2019_64 compiler (e.g., `Qt/6.5.0/msvc2019_64/bin`)  

## Controls  
**Game Input**  

⬆️⬇️⬅️➡️ = W S A D
A Button = O
B Button = P
Start = Space
Select = Enter

**System Controls**  
- `R`: Hard reset  
- `F1-F4`: Save state to slots 1-4  
- `F5-F8`: Load state from slots 1-4  
- `F9-F12`: Toggle audio channels 1-4  

## Dependencies  
- [GGBoy-Core](https://github.com/Georg-S/ggboy-core) (emulation core)  
- Qt 6 Core/GUI/Widgets modules  
- SDL2 (audio subsystem)  