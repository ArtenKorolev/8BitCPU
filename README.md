# MOS Technology 6502 Emulator
This project is a software emulator of MOS Technology 6502 microprocessor. It behaves according to 6502 specification. It implements instruction set, addressing modes and CPU flags. It is capable of running original 6502 programs. The project also includes emulators of old 6502-based systems with virtual monitor and keyboard (only Apple ][ is supported so far).

## Key features 🧨
1) Full **accordance** to 6502 specification
2) *All* official 6502 instructions are **supported**
3) **Meaningful** log messages
4) 6502-based systems' emulators are **included**
5) Virtual **monitor and keyboard**
6) **ROM images** with original 6502/Apple ][ programs

## Architecture 🏛️
There are four main components in this project:
1) CPU. It is responsible for **instructions' pipeline**. Code's location: `source/core/cpu`
2) Memory. It is responsible for encapsulating **writing/reading RAM and memory-mapped I/O**. Code's location: `source/core/memory`
3) Platform. This component includes I/O implementations for **specific system** (Apple ][, Commodore 64, etc.). Code's location: `source/platform/...`
4) Emulator. The **entry point**. It initializes all the components and starts the CPU. Code's location: `source/core/emulator.*`

## Installing and building 🛠️
To install and build this project follow the steps:
- **Clone** this repository: `git clone https://github.com/ArtenKorolev/8BitCPU`
- Create build folder & run **CMake** with your genereator: `mkdir build; cd build; cmake -S .. -B . -G<your generator>`
- **Run your generator**, for example Make: `make`
- **Place a ROM file** in the build folder (notice that it should be *64 Kb in size* and has *name "mem.bin"*): `cp ... mem.bin`
- **Run** the emulator: `./emu8`

## Command line arguments ⌨️
You can choose log level by command line arguments:
- **Info**: `-linfo`
- **Warning**: `-lwarn`
- **Error**: `-lerr`
  
The default is NO_LOG option

## Examples 🔭
### Applesoft BASIC
<img width="426" height="682" alt="изображение" src="https://github.com/user-attachments/assets/5df78dda-71f1-477d-96ff-cfe8f996db5c" />

### WozMonitor
<img width="420" height="692" alt="Screenshot 2026-03-19 at 21 22 52" src="https://github.com/user-attachments/assets/ca3b5185-0a5c-4c0e-a8fe-3306382c9fed" />

### Integer BASIC
<img width="420" height="693" alt="изображение" src="https://github.com/user-attachments/assets/3be1b65a-d399-4a0a-aff9-f405c0dedfb7" />


