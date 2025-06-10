# touch_piano-final-project
In this project, we present the design and implementation of a portable digital piano system based on the STM32 microcontroller platform. Motivated by the idea of creating a compact, low-cost musical instrument capable of real-time performance, we developed a prototype that combines capacitive touch sensing with wireless communication and software sound synthesis.

The input interface consists of a piano-shaped capacitive touch sensor array connected to an STM32 development board, which detects touch signals corresponding to specific musical notes. The STM32 device processes the input and transmits the data wirelessly to a host computer using a Wi-Fi client-server model. On the computer side, a Python-based server receives the signals and triggers the playback of the corresponding notes using synthesized instrument sounds.

Our goal is to build a fully functional, standalone piano input system that supports physical keys, pitch control, and potentially instrument sound switching. This project demonstrates the integration of embedded hardware with wireless networking and interactive audio software, enabling an expressive and extensible platform for musical interaction.
## How to use this repository
### STM32 B-L475E-IOT01A
1. Change the port number, wifi name, password in  `main.c`
2. Move `Wifi_Client_Server` directory to your workspace
3. Start STM32CubeIDE
4. Click File -> Import -> General -> Existing Projects into Workspace
5. Select root directory -> STM32CubeIDE
CubeIDE will auto detect `.project` and `.cproject`.
### Python Server
1. In your terminal, install FluidSynth.
macOS:
```
brew install fluidsynth
```
Ubuntu / Debian：
```
sudo apt install fluidsynth libfluidsynth-dev
```
2. install pyFluidSynth module
```
pip install pyFluidSynth
```

3. Run `python piano.py`

Open your wifi/hotspot, run the python server. Then your STM32 should connect to your python server, and you can start to play the piano.
