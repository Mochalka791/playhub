# Casino – Vendor Setup

Before opening the Visual Studio solution, you need to download two dependencies
and place them in the `Casino/vendor/` folder.

---

## 1. SDL2

Download the **Development Libraries (VC)** for Windows from:
https://github.com/libsdl-org/SDL/releases  (e.g. `SDL2-devel-2.30.x-VC.zip`)

Extract and copy so the structure looks like:

```
Casino/vendor/SDL2/
  include/
    SDL.h
    SDL_opengl.h
    ... (all SDL2 headers)
  lib/
    x64/
      SDL2.lib
      SDL2main.lib
      SDL2.dll         <-- copy this next to Casino.exe after build
```

---

## 2. ImGui

Clone or download the latest **Dear ImGui** release from:
https://github.com/ocornut/imgui/releases

Copy the following files into `Casino/vendor/imgui/`:

```
Casino/vendor/imgui/
  imgui.h
  imgui.cpp
  imgui_internal.h
  imgui_draw.cpp
  imgui_tables.cpp
  imgui_widgets.cpp
  imconfig.h
  imstb_rectpack.h
  imstb_textedit.h
  imstb_truetype.h
  backends/
    imgui_impl_sdl2.h
    imgui_impl_sdl2.cpp
    imgui_impl_opengl3.h
    imgui_impl_opengl3.cpp
    imgui_impl_opengl3_loader.h
```

---

## 3. Build

1. Open `Casino.sln` in Visual Studio 2022
2. Select **x64 | Debug** or **x64 | Release**
3. Build → Build Solution  (Ctrl+Shift+B)
4. Copy `SDL2.dll` from `vendor/SDL2/lib/x64/` next to the compiled `.exe`
   (usually `bin/Debug/Casino.exe`)

---

## Gameplay

| Screen     | Description                                      |
|------------|--------------------------------------------------|
| Menu       | Start or exit                                    |
| Player Setup | Enter your name and choose a starting balance  |
| Lobby      | Choose Slot Machine, Blackjack, or Roulette      |
| Highscores | View top 10 players by net profit (session only) |
