# Study-with-this

- A lightweight Pomodoro + lofi app: Pomodoro helper should not be resource-hungry 😂.
- A weekend side project, recreating [study-with-me-with-this](https://github.com/stannam/study-with-me-with-this).
- The child of a sleepless Friday night, written for fun to make something I'd actually use and to practice C + SDL2.

# Features

<img width="800" alt="image" src="https://github.com/user-attachments/assets/1d9a5fc8-e1b6-4f4d-be55-85e285ec28b4" />

- Helps with the [Pomodoro Technique](https://en.wikipedia.org/wiki/Pomodoro_Technique)  
  - During a work session, the red circle shrinks and local lofi music plays continuously.  
  - During a break session, the circle regrows.  
- 𝓣𝓱𝓪𝓽'𝓼 𝓲𝓽!

# Installation
1. **Install dependencies**:
    - Need to install three libraries: SDL2, SDL2_ttf and SDL2_mixer.
    - On macOS, you can use `brew`:
      ```bash
      brew install sdl2 sdl2_mixer
      ```
    - On linux, ... you should know better than me. use apt, dnf, pacman, or whatever package manager your system uses. 
      ```bash
      sudo apt-get install libsdl2-dev libsdl2-mixer-dev libsdl2-ttf-dev
      ```
      ```bash
      sudo pacman install libsdl2-dev libsdl2-mixer-dev libsdl2-ttf-dev
      ```

2. **Clone the repository**:
    ```bash
    git clone https://github.com/your-username/study-with-this.git
    cd study-with-this
    ```

3. **Build the project** (using `Makefile`):
    ```bash
    make
    ```

4. **Run the application**:
    ```bash
    ./study-with-me
    ```

# Why recreate SWMWT?
1. Why not?  
2. SWMWT is written in Python + PyQt5, which  
   - takes soooooo long (~10 seconds) to launch.
   - was tricky to get running on Ubuntu.

# License
- Software: GNU3; Font: SIL OPEN FONT LICENSE
- See [LICENSE](./LICENSE)
