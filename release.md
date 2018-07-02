16174prog03 Release Notes
=========================

Feature List
------------

- **Clock**
  - Getting timestamp from Cloud Server.
- **Cluster Control Server**
  - Transmits time message every 1 minute.
  - Will not transmit time message if local clock in not **SYNCHRONISED**.
  - Collecting time is 10 minutes.
  - Downloading time is 50 minutes.
- GPS location is always 0.0, 0.0
- **Cloud Uploader:**
  - Gateway State is uploaded every minute.
  - Node state is uploaded after TCP/IP link opened, and then every 10 minutes.
  - Format gateway line: `gw,IP6ADDR,MA.MI.REL,LAT,LON`
  - Format node line: `nd,IP6ADDR,STATUS,MA.MI.REL,STRATUM,LAT,LON,WAITING,CURRENT`
  - Format node line: `nd,IP6ADDR`
  - Format data line: `da,TIMESTAMP,ACCELX,Y,Z,GYROX,Y,Z,MAGX,Y,Z`

- **Network**
  - Maximum number of neighbours: 40
  - Maximum number of routes: 100




Revision History
----------------

### Version: V2.0.5

* **Date**     : 29-11-2017
* **CVS Tag**  : C16174prog03-V2.0.5
* **Contract** : 16868
* **Checksum** : 0x013E52F1

**Notes:**

- Fixed: Over the Air reprogramming via Bluetooth link.
- Fixed: Radio driver not using ACK's to detect if message was received.
- Fixed: Network not finding alternative route when a node goes down.
- Fixed: Byte-order of Bluetooth mac address displayed on Android phone app.
- Using IPv6 address to identify the Gateway to the cloud server.
- Added 'network' command to the shell.




--------------------------------------------------------------------------------

### Version: V2.0.4

* **Date**     : 18/10/2017
* **CVS Tag**  : C16174prog03-V2.0.4
* **Contract** : 16868
* **Checksum** : 0x013C9015

**Notes:**

- Added method to reset the modem. All open TCP/IP links are gracefully closed
  before resetting the Modem.
- Resetting the Modem if can't open/reopen TCP link after 10 minutes trying.
  This was due to Modem loosing IP address on Cellular network and the Modem
  Controller not picking up on this and resetting the modem.
- Added shell command to reset the modem 'modem reset'.
- Fixed -- possible lost data when Modem fails to send data. The data buffer is
  retransmitting the next time the Modem link is opened.
- The dirty flag is set on Node when download is finished. This will update the
  Cloud very quickly.
- Fixed -- failing to flash new firmware using phone.




--------------------------------------------------------------------------------

### Version: V2.0.3

* **Date**     : 10/10/2017
* **CVS Tag**  : C16174prog03-V2.0.3
* **Contract** : 16868
* **Checksum** : 0x013C5792

**Notes:**

- Fixed -- not sending Long Node at regular intervals. Now sending at 10 minute
  intervals.
- Added `is_dirty` flag to the `SensorNode` object.
- Sending Long Node message whenever `SensorNode` is flagged as dirty.
- Flagging node as dirty whenever key properties have changed in message from
  low-pan network.
- Flagging all nodes as dirty when TCP link to Cloud is opened. This will result
  in Long Node Message being sent to the Cloud for each Node straight after link
  is opened.




--------------------------------------------------------------------------------

### Version: V2.0.2

* **Date**     : 03/10/2017
* **CVS Tag**  : C16174prog03-V2.0.2
* **Contract** : 16868
* **Checksum** : 0x013C4EB1

**Notes:**

- Getting timestamp from Cloud Server.
- Improved detection of dropped TCP connection to Cloud Server.
  - Stopped using counter to record failed sending of data and now reading size
    of transmit buffer from Modem.
  - Don't attempt to send data if TCP link has been closed.




--------------------------------------------------------------------------------

### Version: V2.0.1

* **Date**     : 21/09/2017
* **CVS Tag**  : C16174prog03-V2.0.1
* **Contract** : 16868
* **Checksum** : 0x013BFDEC

**Notes:**

- Disabled GPS unit on the Modem board.
- Getting time from the Real-Time-Clock on the Modem board.
- Configuring the Modem to use NTP to initialise and keep the RTC in sync.
- The local clock is synchronised with the Modem's RTC every 10 minutes.
- GPS location is always 0.0, 0.0




--------------------------------------------------------------------------------

### Version: V2.0.0

* **Date**     : 12/09/2017
* **CVS Tag**  : C16174PROG03-V2.0.0
* **Contract** : 16868
* **Checksum** : 0x013B5088

**Notes:**

- Fixed -- GPS often fails to synchronize with external PPS signal.
- Storing Radio setup (PAN Channel and PAN ID) in EEPROM store.
- Bluetooth interface can edit PAN Channel and PAN ID.
- Once an hour checks the connection status of each node.
  - Nodes are flagged as STALE if no communications for over ???? hours
  - Nodes are deleted if no communications for over ???? hours




--------------------------------------------------------------------------------

### Version: V1.0.0

* **Date**     : 01/03/2017
* **CVS Tag**  : C16174PROG03-V1.0.0
* **Contract** : 16174
* **Checksum** : 0x0133DF28

**Notes:**

- First release

--------------------------------------------------------------------------------
