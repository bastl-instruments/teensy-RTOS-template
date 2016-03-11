#  Teensy 3.1 Project Makefile
#  Karl Lunt
#  with modifications by Kevin Cuzner
#  with further modifications by Jozef Henzl

#  Project Name
PROJECT=app_osc

#  Project directory structure
SRCDIR = src
FREERTOS_DIR = FreeRTOS
AUDIOLIB_DIR = audio
# AUDIOLIB_DIR = /home/xoza/src/Audio/
OUTPUTDIR = bin
OBJDIR = obj

#  Type of CPU/MCU in target hardware
CPU = cortex-m4
MCU = mk20dx256

#  Build the list of object files needed.  All object files will be built in
#  the working directory, not the source directories.
#
#  You will need as a minimum your $(PROJECT).o file.
#  You will also need code for startup (following reset) and
#  any code needed to get the PLL configured.

#  Project C & C++ files which are to be compiled
CPP_FILES = $(wildcard $(SRCDIR)/*.cpp)
C_FILES = $(wildcard $(SRCDIR)/*.c) 
APP_C_FILES = $(wildcard $(PROJECT)/*.c)
APP_CPP_FILES = $(wildcard $(PROJECT)/*.cpp) 
# C_FILES +=  $(wildcard kinetis_i2c/*.c)
FREERTOS_FILES = $(wildcard $(FREERTOS_DIR)/*.c) 
FREERTOS_FILES += $(wildcard $(FREERTOS_DIR)/portable/GCC/ARM_CM3/*.c)

AUDIOLIB_FILES_CPP = $(wildcard $(AUDIOLIB_DIR)/*.cpp) 
AUDIOLIB_FILES_C = $(wildcard $(AUDIOLIB_DIR)/*.c) 


#  Change project C & C++ files into object files
OBJ_FILES := $(addprefix $(OBJDIR)/,$(notdir $(APP_CPP_FILES:.cpp=.o))) $(addprefix $(OBJDIR)/,$(notdir $(APP_C_FILES:.c=.o))) 
OBJ_FILES += $(addprefix $(OBJDIR)/,$(notdir $(CPP_FILES:.cpp=.o))) $(addprefix $(OBJDIR)/,$(notdir $(C_FILES:.c=.o)))
OBJ_FILES += $(addprefix $(OBJDIR)/,$(notdir $(FREERTOS_FILES:.c=.o))) 
OBJ_FILES += $(addprefix $(OBJDIR)/,$(notdir $(AUDIOLIB_FILES_CPP:.cpp=.o))) $(addprefix $(OBJDIR)/,$(notdir $(AUDIOLIB_FILES_C:.c=.o)))

#  Here we add any teensyduino or "library" object files. We compile these files
#  into the local obj directory, just like the Arduino IDE would do. Projects
#  won't be sharing the object files for these. Thus, we only put things here
#  that need to be compiled locally (*.c, *.cpp, *.s). No libries (*.a, *.lib)
#  go here.
OBJ_FILES += $(OBJDIR)/usb_dev.o $(OBJDIR)/usb_mem.o $(OBJDIR)/usb_desc.o $(OBJDIR)/usb_keyboard.o $(OBJDIR)/usb_seremu.o  $(OBJDIR)/usb_serial.o $(OBJDIR)/usb_midi.o 
OBJ_FILES += $(OBJDIR)/pins_teensy.o $(OBJDIR)/analog.o 
OBJ_FILES += $(OBJDIR)/nonstd.o
OBJ_FILES += $(OBJDIR)/DMAChannel.o
OBJ_FILES += $(OBJDIR)/AudioStream.o
OBJ_FILES += $(OBJDIR)/eeprom.o
# $(OBJDIR)/yield.o 

#  Next we need to define some things in order for Teensyduino to work.
#  Teensyduino in generaly will not throw any errors or generate any warnings if
#  these are not defined unless they are specifically needed. In particular, the
#  PLL will not be set up and the chip will be put running at 16Mhz if F_CPU
#  is not defined since mk20dx128.c will not bother trying to set up the PLL.

#  CPU Frequency (for PLL)
F_CPU = 96000000
#  Chip name (for PLL and other things in mk20dx128)
CHIP = MK20DX256

#  Select the toolchain by providing a path to the top level
#  directory; this will be the folder that holds the
#  arm-none-eabi subfolders. On linux, this should be /usr or /usr/local.
TOOLPATH = /home/xoza/src-old/arduino-1.6.5-r5/hardware/tools/arm/

#  Holds the base path to the teensyduino installation. We point specifically
#  to the teensy3 path since we are only compiling for the teensy 3.0 or
#  teensy 3.1. This path contains the *.h, *.c, and *.cpp files that we need
#  in order to initialize the chip. We could write our own and that could be
#  fun, but this makefile will use the ones that come with Teensyduino for
#  efficiency's sake.
TEENSY3X_BASEPATH = $(HOME)/src-old/arduino-1.6.5-r5/hardware/teensy/avr/cores/teensy3

#
#  Select the target type.  This is typically arm-none-eabi.
#  If your toolchain supports other targets, those target
#  folders should be at the same level in the toolchain as
#  the arm-none-eabi folders.
TARGETTYPE = arm-none-eabi

#  Describe the various include and source directories needed.
#  These usually point to files from whatever distribution
#  you are using (such as Freescale examples).  This can also
#  include paths to any needed GCC includes or libraries.
TEENSY3X_INC     = $(TEENSY3X_BASEPATH)
GCC_INC          = $(TOOLPATH)/$(TARGETTYPE)/include
FREERTOS_INC	 = FreeRTOS/include/ -IFreeRTOS/portable/GCC/ARM_CM3/
AUDIOLIB_INC	= -I$(AUDIOLIB_DIR) -I$(AUDIOLIB_DIR)/utility


#  All possible source directories other than '.' must be defined in
#  the VPATH variable.  This lets make tell the compiler where to find
#  source files outside of the working directory.  If you need more
#  than one directory, separate their paths with ':'.
#  
#  We don't particularly use this since we explicitly specify the src directory,
#  but we could change things to actually use this.
VPATH = $(TEENSY3X_BASEPATH)

				
#  List of directories to be searched for include files during compilation
INCDIRS  = -I$(GCC_INC)
INCDIRS += -I$(TEENSY3X_INC)
INCDIRS += -I$(FREERTOS_INC)
INCDIRS += -I$(PROJECT)

INCDIRS += -I/home/xoza/src-old/arduino-1.6.5-r5/hardware/teensy/avr/libraries/SPI/
INCDIRS += -I/home/xoza/src-old/arduino-1.6.5-r5/hardware/teensy/avr/libraries/Wire/

INCDIRS += -Iinclude
INCDIRS += -I.


# Name and path to the linker script
LSCRIPT = mk20dx256.ld


OPTIMIZATION = 3
DEBUG = 

#  List the directories to be searched for libraries during linking.
#  Optionally, list archives (libxxx.a) to be included during linking.
#  
#  These values are copied from the Makefile that ships with Teensyduino.
LIBDIRS  =
LIBS = -lm

#  Compiler options
GCFLAGS = -Wall -fno-common -mcpu=$(CPU) -mthumb -MMD -O$(OPTIMIZATION) $(DEBUG) 
GCFLAGS += $(INCDIRS)
GCFLAGS += -DF_CPU=$(F_CPU) -D__$(CHIP)__ -DUSB_SERIAL -DLAYOUT_US_ENGLISH -DTEENSYDUINO=121
GCFLAGS += -Wall -pedantic -std=gnu++11 -Wextra -Wno-unused-parameter

# You can uncomment the following line to create an assembly output
# listing of your C files.  If you do this, however, the sed script
# in the compilation below won't work properly.
# GCFLAGS += -c -g -Wa,-a,-ad 


#  Assembler options
ASFLAGS = -mcpu=$(CPU)

# Uncomment the following line if you want an assembler listing file
# for your .s files.  If you do this, however, the sed script
# in the assembler invocation below won't work properly.
#ASFLAGS += -alhs


#  Linker options
#  Since we are using gcc as the linker rather than ld, we need to pass linker
#  specific options with the -Wl,<option>,<value> format.
LDFLAGS  = -Os -Wl,--gc-sections -mcpu=cortex-m4 -mthumb -T$(LSCRIPT) -Wl,-Map,$(OUTPUTDIR)/$(PROJECT).map -lstdc++
LDFLAGS += $(LIBDIRS)
LDFLAGS += $(LIBS)


#  Tools paths
#
#  Define an explicit path to the GNU tools used by make.
#  If you are ABSOLUTELY sure that your PATH variable is
#  set properly, you can remove the BINDIR variable.
#
BINDIR = $(TOOLPATH)/bin

CC = arm-none-eabi-gcc
AS = $(BINDIR)/arm-none-eabi-as
AR = $(BINDIR)/arm-none-eabi-ar
LD = $(BINDIR)/arm-none-eabi-ld
OBJCOPY = $(BINDIR)/arm-none-eabi-objcopy
SIZE = $(BINDIR)/arm-none-eabi-size
OBJDUMP = $(BINDIR)/arm-none-eabi-objdump

#  Define a command for removing folders and files during clean.  The
#  simplest such command is Linux' rm with the -f option.  You can find
#  suitable versions of rm on the web.
REMOVE = rm -rf

#########################################################################

all:: $(OUTPUTDIR)/$(PROJECT).hex $(OUTPUTDIR)/$(PROJECT).bin 

$(OUTPUTDIR)/$(PROJECT).bin: $(OUTPUTDIR)/$(PROJECT).elf
	$(OBJCOPY) -O binary -j .text -j .data $(OUTPUTDIR)/$(PROJECT).elf $(OUTPUTDIR)/$(PROJECT).bin

$(OUTPUTDIR)/$(PROJECT).hex: $(OUTPUTDIR)/$(PROJECT).elf
	$(OBJCOPY) -R .stack -O ihex $(OUTPUTDIR)/$(PROJECT).elf $(OUTPUTDIR)/$(PROJECT).hex

#  Linker invocation
$(OUTPUTDIR)/$(PROJECT).elf: $(OBJ_FILES)
	@mkdir -p $(dir $@)
	$(CC) $(OBJ_FILES) $(LDFLAGS) -o $(OUTPUTDIR)/$(PROJECT).elf

stats: $(OUTPUTDIR)/$(PROJECT).elf
	$(SIZE) $(OUTPUTDIR)/$(PROJECT).elf
	
dump: $(OUTPUTDIR)/$(PROJECT).elf
	$(OBJDUMP) -h $(OUTPUTDIR)/$(PROJECT).elf    

clean:
	@echo $(OBJ_FILES)
	$(REMOVE) $(OBJDIR)
	$(REMOVE) $(OUTPUTDIR)

upload: $(OUTPUTDIR)/$(PROJECT).hex
	teensy_loader_cli -mmcu=$(MCU) -s -w -v  $(OUTPUTDIR)/$(PROJECT).hex

#  The toolvers target provides a sanity check, so you can determine
#  exactly which version of each tool will be used when you build.
#  If you use this target, make will display the first line of each
#  tool invocation.
#  To use this feature, enter from the command-line:
#    make -f $(PROJECT).mak toolvers
toolvers:
	$(CC) --version | sed q
	$(AS) --version | sed q
	$(LD) --version | sed q
	$(AR) --version | sed q
	$(OBJCOPY) --version | sed q
	$(SIZE) --version | sed q
	$(OBJDUMP) --version | sed q
	
#########################################################################
#  Default rules to compile .c and .cpp file to .o
#  and assemble .s files to .o

#  There are two options for compiling .c files to .o; uncomment only one.
#  The shorter option is suitable for making from the command-line.
#  The option with the sed script on the end is used if you want to
#  compile from Visual Studio; the sed script reformats error messages
#  so Visual Studio's IntelliSense feature can track back to the source
#  file with the error.
$(OBJDIR)/%.o : $(SRCDIR)/%.c
#     @echo Compiling $<, writing to $@...
	@mkdir -p $(dir $@)
	$(CC) $(GCFLAGS) -c $< -o $@ > $(basename $@).lst
#	$(CC) $(GCFLAGS) -c $< -o $@ 2>&1 | sed -e 's/\(\w\+\):\([0-9]\+\):/\1(\2):/'

$(OBJDIR)/%.o : $(AUDIOLIB_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(GCFLAGS) -c $< -o $@ > $(basename $@).lst

$(OBJDIR)/%.o : $(AUDIOLIB_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(GCFLAGS) -c $< -o $@ > $(basename $@).lst

$(OBJDIR)/%.o : $(FREERTOS_DIR)/%.c 
	@mkdir -p $(dir $@)
	$(CC) $(GCFLAGS) -c $< -o $@ > $(basename $@).lst

$(OBJDIR)/%.o : $(FREERTOS_DIR)/portable/GCC/ARM_CM3/%.c
	@mkdir -p $(dir $@)
	$(CC) $(GCFLAGS) -c $< -o $@ > $(basename $@).lst

$(OBJDIR)/%.o : $(TEENSY3X_BASEPATH)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(GCFLAGS) -c $< -o $@ > $(basename $@).lst

$(OBJDIR)/%.o : $(PROJECT)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(GCFLAGS) -c $< -o $@


$(OBJDIR)/%.o : $(PROJECT)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(GCFLAGS) -c $< -o $@
	
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(GCFLAGS) -c $< -o $@

$(OBJDIR)/%.o : $(TEENSY3X_BASEPATH)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(GCFLAGS) -c $< -o $@

#  There are two options for assembling .s files to .o; uncomment only one.
#  The shorter option is suitable for making from the command-line.
#  The option with the sed script on the end is used if you want to
#  compile from Visual Studio; the sed script reformats error messages
#  so Visual Studio's IntelliSense feature can track back to the source
#  file with the error.
$(OBJDIR)/%.o : $(SRCDIR)/%.s
#     @echo Assembling $<, writing to $@...
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -o $@ $<  > $(basename $@).lst
#	$(AS) $(ASFLAGS) -o $@ $<  2>&1 | sed -e 's/\(\w\+\):\([0-9]\+\):/\1(\2):/'
#########################################################################
