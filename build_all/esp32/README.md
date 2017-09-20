
# ESP32 Gate Build

This project exercises the ESP32 cross-compiler against the Azure IoT C SDK. This project is Linux-only, and is built by the `linux_esp32_c.sh` script in this project's parent directory.

### Project environment
The ESP32 build tools expect two pieces of environment information
* **IDF_PATH** - this export must be set to the directory containing the ESP32 SDK
* **ESP32_TOOLS** - this export must contain the ESP32 toolchain, which the `linux_esp32_c.sh` script will add to the PATH

### Project structure
The ESP32 build tools expect the project to be contained in two directories:
* **components** - each subdirectory containing a component.mk file will be built and treated as a library
* **main** - the contents of main are treated as project files, and must contain an app_main() function

### Project setup

Here are the instructions for setting up the Linux environment for the ESP32 cross-compiler:

##### Install prerequisites:  

`sudo apt-get install git wget make libncurses-dev flex bison gperf python python-serial`


##### Install the ESP32 toolchain:

Extract:
https://dl.espressif.com/dl/xtensa-esp32-elf-linux64-1.22.0-61-gab8375a-5.2.0.tar.gz
Into /home/jenkins/esp32: 

`mkdir -p /home/jenkins/esp32`

`cd /home/jenkins/esp32`

`tar -xzf ~/Downloads/xtensa-esp32-elf-linux64-1.22.0-61-gab8375a-5.2.0.tar.gz`

##### Create an export for the toolchain location:<br/>
`export ESP32_TOOLS="/home/jenkins/esp32"`

##### Install the ESP32 SDK:

`cd /home/jenkins` 

`git clone --recursive https://github.com/espressif/esp-idf.git esp32-idf`

`git checkout 53893297299e207029679dc99b7fb33151bdd415`

##### Export the ESP32 SDK location:

`export IDF_PATH="/home/jenkins/esp32-idf"`

# Get Started with Microsoft Azure IoT Starter Kit - ESP32-DevKitC ("Core Board")

This sample was tested with Espressif's **ESP32-DevKitC ("Core Board")**, but many other kits would work as well.

Don't have a kit yet? Click [here](http://esp32.net/)

This sample was modified from [this one](https://github.com/ustccw/AzureESP32.git).

## Step 1 - Download the ESP32 SDK

Clone the [Espressif IoT Development Framework](https://github.com/espressif/esp-idf) repository with the following command:

`git clone https://github.com/espressif/esp-idf.git --recursive`

## Step 2 - Set up the ESP32 toolchain

Follow the instructions for setting up the ESP32 toolchain found [here](http://esp-idf.readthedocs.io/en/latest/#setup-toolchain).

## Step 3 - Set IDF_PATH

Set the IDF_PATH environment variable to point to the location of the **esp-idf** directory that you cloned in Step 1.

If you're using MSYS on Windows, a good place to set the IDF_PATH variable is in the `~\msys32\home\user\.bashrc` file that gets created the first time you run `msys2_shell.cmd`.

## Step 4 - Install the Azure IoT C SDK

Create an `azure-iot` directory in the ESP32 SDK's `components` directory:<br/>
`mkdir $IDF_PATH/components/azure-iot`

Clone the Azure IoT C SDK into the `azure-iot` directory as `sdk`:<br/>
`cd $IDF_PATH/components/azure-iot`<br/>
`git clone --recursive  https://github.com/Azure/azure-iot-sdk-c.git sdk`

Copy the `component.mk` file for ESP32 into the `azure-iot` directory:<br/>
`cp sdk/c-utility/build_all/esp32/sdk/component.mk .`

## Step 5 - Create your new project

Create a directory for your new project and make that directory current:<br/>
`mkdir <myproject>`<br/>
`cd <myproject>`

Copy the project structure:<br/>
`cp -a $IDF_PATH/components/azure-iot/sdk/c-utility/build_all/esp32/proj/. .`

Copy the sample files:<br/>
`cp $IDF_PATH/components/azure-iot/sdk/iothub_client/samples/iothub_client_sample_mqtt/iothub_client_sample_mqtt.* main`


## Step 6 - Set your device's connection string

Create and IoT Hub and an associated device identity [as shown here](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-csharp-csharp-getstarted).
Then open the `main/iothub_client_sample_mqtt.c` file in the `main` directory of your project and find the line near the top that reads 

```c
static const char* connectionString = "";

```

and set the value of the connectionString variable to the be the connection string of the device identity that you created.

## Step 7- Configure the make process

Using the toolchain you installed in Step 2 (MSYS, for example), navigate to the location of the sample code you downloaded in Step 4 and run the following command:

`make menuconfig`

This command will bring up a configuration dialog.

1. Under "Serial flasher config --->Default serial port" set the serial port ID to that of your ESP32 device. (On Windows you can find the serial port ID under Computer Management.)

1. Under "Example Configuration --->" enter your WiFi router SSID and password.

1. Save the configuration and exit the dialog.

## Step 8 - Run the make process

Build the sample with the simple command:

`make`

This will produce a iothub_client_sample_mqtt.bin file, a partitions_singleapp.bin file, a bootloader/bootloader.bin file, plus associated maps.

## Step 9 - Flash the ESP32 device

Make sure the ESP32 device is plugged in and run the command:

`make flash`

This will flash the project onto the ESP32 device. Alternate methods of flashing the device can be found [here](https://espressif.com/en/support/download/other-tools)

## Step 10 - Monitor the device output

The sample program sends status output to the device's serial port at a default 115200 baud. You monitor this output by connecting to the serial port with any terminal program such as [Putty](http://www.putty.org/).

# Contributing

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.
