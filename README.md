Accessing the board
--------------------

    minicom -D /dev/ttyUSB0 -b 115200 -o

Drivers: http://www.ftdichip.com/FTDrivers.htm

Commands
--------------------

    HISTORY  
      CLEAR       - clear history
      [SHOW]      - show history
    HELP          - display list of all available commands
    SYS
      REBOOT      - reboot system 
      HALT        - freeze the CPU (may be useful for debugging purposes)
      DIAG        - display peak stack usage
      CPU         - display information about CPU
      BOARD       - display information about board
      [INFO]      - display system information
      BUILD       - display information about current firmware build
      UPTIME      - display system uptime
    MATRIX
      HUMIDITY    - display humidity (in %) from the sensor on the switching matrix
      TEMPERATURE - display temperature (deg C) from the sensor on the switching matrix
      [INFO]      - display current settings
      MEASUREMENT - get/set measurement type (Valid types IV/CV)
                    Examples:
                    MATRIX.MEASUREMENT IV
                    MATRIX.MEASUREMENT ?
      CHANNEL     - select channel
                    Examples:
                    MATRIX.CHANNEL 12
                    MATRIX.CHANNEL ?
      SHORTALL    - shorts all channels to ground
                    Examples:
                    MATRIX.SHORTALL
    PROBECARD
      HUMIDITY    - display humidity (in %) from the sensor on the probecard
      TEMPERATURE - display temperature (deg C) from the sensor on the probecard
    UI
      REPRESENTATION - set/get the representation of the channel number displayed 
                    on the 7 segment display (Valid values dec/oct/hex)
                    Examples:
                    UI.REPRESENTATION ?    # show the current representation 
                    UI.REPRESENTATION DEC  # change the representation to decimal
      TIMEOUT     - get/set how long display should be on in the AUTO mode
                    Examples:
                    UI.TIMEOUT 5 #2 seconds
                    UI.TIMEOUT ? # shows the current value
      DISPLAY     - get/set display mode. Options ON/OFF/AUTO
                    Examples:
                    UI.DISPLAY AUTO
                    UI.DISPLAY ?
      [INFO]      - informations about UI

User interface
---------------

Display shows the currently selected channel. By default the number is given 
in octal representation (this behavior can be changed with 
UI.REPRESENTATION command). When the display shows '---' it means that
all channels are shorted to ground. 

The current channel can be changed with NEXT and PREV buttons. Pressing two 
buttons at the same time will short all the channels to ground. 

The measurement type can be changed by MODE button. The currently selected
measurement type is indicated by IV/CV LEDs.

Build configurations
--------------------

Directory structure and build scripts are organized so that sources, headers
and build configuration options related to specific levels of target system
architecture are separated and may be reused as long as the given part is
the same.

The build system allows for building different applications for different
systems (i.e. permutations of hardware architecture and - optionally -
operating system).

Things related to specific application are placed in appropriate subdirectory
under app/.

Further distinction is done within the system part, which is divided into
three layers:
- CPU - configuration and sources related to specific processor architecture,
  determined either by the processor architecture itself or by toolchain
  and C library used to build application for the architecture.
  All such files are placed in appropriate subdirectories
  under cpu/.
- SoC - optional subcategory under CPU, comprising things dedicated
  to specific System-on-Chip or microcontroller, which includes e.g.
  options to link drivers for hardware incorporated in particular chips.
- Board - defines configuration of target system on which the application
  will run. This includes specification of CPU (and optionally SoC) and
  drivers for devices installed on board.

Peripheral device drivers are placed in a separate directory drivers/,
as some of them (like for instance the well-known 16550 UART) are not
specific to any particular CPU, microcontroller or board.

Each valid permutation of APP, BOARD, SOC and CPU is present in top level
Makefile in form of app_(optional system spec)_config, for example
default_config.

To build a permutation, invoke make with desired permutation name, then make
alone:

    $ make default_config
    $ make clean        # optional
    $ make

Programming (flashing)
----------------------

Switching matrix can be reprogrammed in two ways:

1) Using on board ISP connector (J8). When AVRISP mkII programmer is connected, 
   one can use command shown below to program the chip.

    $   make prog_avrisp2

2) Using build in boot loader. To enter the bootloader, one has to keep PB2(LED)
   button pressed during power up (or reset) of the board. When bootloader 
   is active LD5 (HV) LED should be lit. Boot loader uses AVR911 protocol, one
   can use the command shown below to program the chip.

    $ make prog_avr911

   To exit bootloader one should reset the board with PB1 (RESET) button.

Programming with AVR Studio
---------------------------

0) Download firmware from: https://gitlab.cern.ch/skulis/HGCSensorProbeCard/tree/master/fw/build/hex

1) Start AVR Studio 7.0

2) **Tools** -> **Device programming**

a) Select your **Toool** 

b) Select **ATxmega128A1U** as **Device**

c) Select **PDI** as **Interface**

d) Click **Apply**

e) Go to **Device information** and you should see a page shown below:

![device](https://gitlab.cern.ch/skulis/HGCSensorProbeCard/raw/master/doc/img/prog/avr01.png)

3) Go to **Fuses** tab, configure all fields as shown below and click **Program**

![fuses](https://gitlab.cern.ch/skulis/HGCSensorProbeCard/raw/master/doc/img/prog/avr02.png)

4) Go to Memories tab, select **Erace Chip** and click **Erase now**

5) Unselect **Erase Flash before programming**, select **Xmega_Bootloader.hex** file and click **Program** (as shown below).

![boot loader](https://gitlab.cern.ch/skulis/HGCSensorProbeCard/raw/master/doc/img/prog/avr04.png)

6) Select **progfile.hex** file and click **Program** (as shown below).

![program](https://gitlab.cern.ch/skulis/HGCSensorProbeCard/raw/master/doc/img/prog/avr05.png)

