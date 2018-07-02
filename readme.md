16174prog03 Read Me
===================

The files in this folder are the source code for the microcontroller in the
ALC Gateway.

For release notes on the various versions of this firmware see the text in the
file `release.md` file.




Project Directory Structure
---------------------------

The structure of the project is shown below.

    16174prog03
    +---doc                                     Documentation
    +---dox                                     Doxygen documentation
    +---release                                 Released binaries
    |   \---developer                           Development only binaries
    +---source
    |   +---inc                                 Header files
    |   |   +---bt                              Project-Specific Bluetooth
    |   |   +---databuffers                     Internal RAM storage for nodes and data
    |   |   +---gps                             GPS localtion and time module
    |   |   +---modem                           Modem driver and controller
    |   |   +---net                             Network (cloud) uploader
    |   |   +---shell                           Project-Specific Shell (UART) commands
    |   |   \---storage                         Non-volatile settings
    |   +---src                                 Source files
    |   |   +---bt                              Project-Specific Bluetooth sources
    |   |   +---databuffers                     Internal RAM storage for nodes and data
    |   |   +---gps                             GPS localtion and time module
    |   |   +---modem                           Modem driver and controller
    |   |   +---net                             Network (cloud) uploader
    |   |   +---shell                           Project-Specific Shell (UART) commands
    |   |   \---storage                         Non-volatile settings
    |   \---tests                               Unit-Test files
    |       +---alc_rtcc_arch                   Tests for real-time-clock
    |       +---data_upload_msg                 Tests for data-upload messages
    |       +---sensor_data_list                Tests for storage (data-list)
    |       +---sensor_data_pool                Tests for storage (data-pool)
    |       +---sensor_node                     Tests for sensor-node module
    |       +---sensor_node_list                Tests for sensor-node-list module
    |       \---sensor_node_pool                Tests for sensor-node-pool module
    +---templates                               Blank templates for source files
    \---_reports                                Computer generated report files




Version Control
---------------

This project is under version control using Git. The repository is stored at:

    T:\repos\software\projects\16xxx\16174prog03.git




Issue Tracking
--------------

Issues are tracked using Bugzilla. The URL is:

* [http://2000-server/bugs/editproducts.cgi?product=16174prog03](http://2000-server/bugs/editproducts.cgi?product=16174prog03)




Supporting Modules
------------------

The following software modules are required:

- contiki
- contiki-alc
- stm32_bluenrg




Build-Process
-------------

The project is make based. The source code for the **16174prog03** project and
supporting modules needs to be put into the same folder. Then just cd to the
16174prog03 folder and type `make all`

A number of operations are performed before each build of the firmware.
The `make_pre.mke` makefile controls the pre-build step. The following
operations are performed:

1. The source code is checked by PC-Lint.
2. Unit-tests are run on some of the code.
3. The project is compiled to produce the binary file:
   `source\16174prog03.16174a03-gateway.bin`

Note: The build process is terminated if any of the PC-Lint checks or unit-tests
fail.




Tools
-----

The following tools are used:

- GNU ARM C Compiler (v5.3 2016q1)
- Git
- PC-Lint
- Python (v3.4 or above)
