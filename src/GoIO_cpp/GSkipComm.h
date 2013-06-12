/*********************************************************************************

Copyright (c) 2010, Vernier Software & Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Vernier Software & Technology nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL VERNIER SOFTWARE & TECHNOLOGY BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**********************************************************************************/
#ifndef _SKIP_COMMUNICATION_H_
#define _SKIP_COMMUNICATION_H_ 1
/**HOST APPLICATION<->DEVICE COMMUNICATION PROTOCOL**************************************************/
//
// We use 8 byte USB HID packets to exchange data between a host computer based application program
// and the device. The USB HID layer is used simply as a transport layer on top of which the application
// protocol rests - hopefully the HID layer is passive and allows us to send the packets defined by the
// application protocol.
//
// Communication is based on 3 logical unidirectional packet streams:
//	1) Command output stream
//	2) Command Response input stream
//	3) Measurement input stream
//
// The Command output stream is implemented by issuing Set_Report requests across the USB Control pipe
// to endpoint 0.
//
// The two input streams share the USB Interrupt pipe attached to endpoint 1. The first byte in each
// input packet is a header which indicates whether the packet is a command response or a measurement packet.
//
#define SKIP_MASK_INPUT_PACKET_TYPE 0xC0
#define SKIP_INPUT_PACKET_TYPE_MEAS 0x00
#define SKIP_INPUT_PACKET_CMD_RESP  0x40
//SKIP_INPUT_PACKET_INIT_RESP indicates command response for SKIP_CMD_ID_INIT.
#define SKIP_INPUT_PACKET_INIT_RESP 0x80
//SKIP_INPUT_PACKET_NOTIFICATION is not used yet.
#define SKIP_INPUT_PACKET_NOTIFICATION 0xC0
//
// Low level application code stuffs input packets into 2 separate fifo queues: one for command responses
// and the other queue for measurements.
// if ((header & SKIP_MASK_INPUT_PACKET_TYPE) == SKIP_INPUT_PACKET_TYPE_MEAS), then packet goes in meas. queue,
// else put packet in command response queue.
//
/***************************************************************************************************/
// Go! Link is also known as Skip.
// Go! Temp is also known as Jonah.
// Go! Motion is also known as Cyclops.
//
// Skip, Jonah, and Cyclops use the same basic protocol. The set of commands supported by Skip is a superset
// of the command set supported by Jonah. Ditto for Cyclops.
//

#if defined (TARGET_OS_WIN)
#pragma pack(push)
#pragma pack(1)
#endif

#ifdef TARGET_OS_MAC
#pragma pack(1)
#endif

/***************************************************************************************************/
// Measurement packets have the following format:
//
typedef struct
{
	unsigned char nMeasurementsInPacket;//Note that (nMeasurementsInPacket & SKIP_MASK_INPUT_PACKET_TYPE) must = 0.
	unsigned char nRollingCounter;
	unsigned char meas0LsByte;
	unsigned char meas0MsByte;
	unsigned char meas1LsByte;
	unsigned char meas1MsByte;
	unsigned char meas2LsByte;
	unsigned char meas2MsByte;
} GSkipMeasurementPacket;
//
typedef struct
{
	unsigned char nMeasurementsInPacket;//Always = 1 for Cyclops.
	unsigned char nRollingCounter;
	unsigned char measLsByteLsWord;
	unsigned char measMsByteLsWord;
	unsigned char measLsByteMsWord;
	unsigned char measMsByteMsWord;
	unsigned char measurementType;//See CYCLOPS_MEAS_TYPE.
	unsigned char reserved;
} GCyclopsMeasurementPacket;
//
/***************************************************************************************************/
//
// Command packets have the following format:
//
typedef struct
{
	unsigned char cmd;
	unsigned char params[7];
} GSkipOutputPacket;
//
// The simplest commands do not use the params[] fields.
//
/***************************************************************************************************/
//
// All Skip packets are the same size as GSkipPacket.
//
typedef struct
{
    unsigned char data[8];
} GSkipPacket;
//
//
/***************************************************************************************************/

//The following command id is sent from the Skip USB CPU to the Skip main CPU to 'turn the SPI crank' so
//that we can get input packets back from the main CPU.
#define SKIP_FAKE_CMD_ID_POLL 0xD0

//
/***************************************************************************************************/
//
// For every Command packet sent from the host to the device, a corresponding Command Response packet
// is sent back.
//
// Many of the Command Responses are simple success/failure indications so they share a common format.
// ALL ERROR RESPONSES USE THIS FORMAT. Check error bit(20h) in the header to detect an error response.
//
typedef struct
{
	unsigned char header;	//7Ah for errors, or 5Ah if no error.
	unsigned char cmd;		//Matches cmd parameter sent in Command packet.
	unsigned char errorStatus;//See SKIP_STATUS_...declarations.
	unsigned char reserved[5];
} GSkipDefaultResponsePacket;
//
typedef struct
{
	unsigned char header;
	unsigned char cmd;		//Matches cmd parameter sent in Command packet.
	unsigned char responsePayload[6];
} GSkipGenericResponsePacket;
//
/***************************************************************************************************/
//SKIP_CMD_ID_GET_STATUS:
//
//Command packet format: GSkipOutputPacket.
//
//Command Response packet format:
typedef struct
{
	unsigned char header;	//5Eh (Skip and Cyclops) or 5Ch (Jonah)
	unsigned char cmd;		//SKIP_CMD_ID_GET_STATUS
	unsigned char status;	//Normal status value is 0. See SKIP_STATUS_...declarations.
	unsigned char minorVersionMasterCPU;	//Binary coded decimal
	unsigned char majorVersionMasterCPU;	//Binary coded decimal
	unsigned char minorVersionSlaveCPU;		//Binary coded decimal - not updated by Jonah
	unsigned char majorVersionSlaveCPU;		//Binary coded decimal - not updated by Jonah
	unsigned char reserved[1];
} GSkipGetStatusCmdResponsePacket;
//
/***************************************************************************************************/
//SKIP_CMD_ID_WRITE_LOCAL_NV_MEM_1BYTE:
//SKIP_CMD_ID_WRITE_LOCAL_NV_MEM_2BYTES:
//SKIP_CMD_ID_WRITE_LOCAL_NV_MEM_3BYTES:
//SKIP_CMD_ID_WRITE_LOCAL_NV_MEM_4BYTES:
//SKIP_CMD_ID_WRITE_LOCAL_NV_MEM_5BYTES:
//SKIP_CMD_ID_WRITE_LOCAL_NV_MEM_6BYTES:
//SKIP_CMD_ID_WRITE_REMOTE_NV_MEM_1BYTE:
//SKIP_CMD_ID_WRITE_REMOTE_NV_MEM_2BYTES:
//SKIP_CMD_ID_WRITE_REMOTE_NV_MEM_3BYTES:
//SKIP_CMD_ID_WRITE_REMOTE_NV_MEM_4BYTES:
//SKIP_CMD_ID_WRITE_REMOTE_NV_MEM_5BYTES:
//SKIP_CMD_ID_WRITE_REMOTE_NV_MEM_6BYTES:
//
//Command packet format:
#define SKIP_MASK_WRITE_NV_MEM_CMD_NUMBYTES 7
typedef struct
{
	unsigned char cmd;
	unsigned char addr;			//non-volatile memory destination address
	unsigned char payload[6];	//data to write to non-volatile memory.
} GSkipWriteNVMemCmdPacket;
//
//Command Response packet format: GSkipDefaultResponsePacket.
//
/***************************************************************************************************/
//
// The local nonvolatile memory on Skip is flash memory. It has the following format:
//
#define SKIP_VALID_FLASH_SIGNATURE 0x9C
typedef struct
{
	unsigned char version;
	unsigned char signature;    //A value of VALID_FLASH_SIGNATURE indicates that this record contains valid data.
	unsigned char ww;			//week in year(starting at 1) in BCD format
	unsigned char yy;			//last two digits of year in BCD format
	short		  vinOffset;	//Big endian - added to vin ADC reading.
	short		  vinLowOffset;	//Big endian - added to vin low ADC reading.
	float		  vinSlope;
	float		  vinLowSlope;
	unsigned char vinOffsetDac;
	unsigned char reserved[3];
} GSkipFlashMemoryRecord;
//
/***************************************************************************************************/
//SKIP_CMD_ID_READ_LOCAL_NV_MEM:
//SKIP_CMD_ID_READ_REMOTE_NV_MEM:
//
//Command packet format:
typedef struct
{
	unsigned char cmd;
	unsigned char addr;			//non-volatile memory source address
	unsigned char count;		//number of bytes to read
	unsigned char reserved[5];
} GSkipReadNVMemCmdPacket;
//
//Command Response packet format:
//
//The first response packet has the following format:
typedef struct
{
	unsigned char header;		//49h + (number NV mem bytes) + (10h if also last packet)
	unsigned char cmd;			//SKIP_CMD_ID_READ_LOCAL_NV_MEM or SKIP_CMD_ID_READ_REMOTE_NV_MEM
	unsigned char payload[6];	//data read from non-volatile memory.
} GSkipReadNVMemCmdFirstResponsePacket;
//
//The second through the last response packet has the following format:
typedef struct
{
	unsigned char header;		//40h + (number NV mem bytes) + (10h if last packet)
	unsigned char payload[7];	//data read from non-volatile memory.
} GSkipReadNVMemCmdResponsePacket;
//
/***************************************************************************************************/
//SKIP_CMD_ID_START_MEASUREMENTS:
//
//Command packet format: GSkipOutputPacket for Skip and Jonah.
//  For Cyclops:
typedef struct
{
	unsigned char cmd;	//SKIP_CMD_ID_START_MEASUREMENTS
	unsigned char triggerType;
	unsigned char lsbyteMeasurementCount;	//(count int16 == 0) => real Time
	unsigned char msbyteMeasurementCount;	//count int16 <= CYCLOPS_MAX_MEASUREMENT_COUNT.
	unsigned char realTimeMeasType;			//CYCLOPS_MEAS_TYPE.
	unsigned char flags;					//CYCLOPS_TEMPERATURE_COMP ...
	unsigned char filterType;               //CYCLOPS_FILTER.
	unsigned char reserved;
} GCyclopsStartMeasurementsCmdPacket;
//
//Command Response packet format: GSkipDefaultResponsePacket.
//
/***************************************************************************************************/
//SKIP_CMD_ID_STOP_MEASUREMENTS:
//
//Command packet format: GSkipOutputPacket.
//
//Command Response packet format: GSkipDefaultResponsePacket.
//
/***************************************************************************************************/
//SKIP_CMD_ID_INIT:
//
//This command should be sent every time an application program opens the device.
//
//In response to SKIP_CMD_ID_INIT:
//	1)Measurements are stopped.
//	2)The measurement period is set to its default value(0.5 seconds for Jonah, 0.1 seconds for Skip, not done for Cyclops).
//	3)The LED color is set to orange(Jonah only).
//	4)Any previously pending commands are aborted.
//	5)The error flags in the status value returned by SKIP_CMD_ID_GET_STATUS are set to 0.
//
//Most commands are ignored by the device if they are sent before the device has responded to the
//previously issued command. SKIP_CMD_ID_INIT is the exception to the rule - the device will stop whatever
//it was doing and respond to SKIP_CMD_ID_INIT whenever it is detected.
//
//Command packet format: GSkipOutputPacket for Skip and Jonah.
//  For Cyclops:
typedef struct
{
	unsigned char cmd;	//SKIP_CMD_ID_INIT
	unsigned char hostType;
	unsigned char forcePowerOnDefaults;
	unsigned char reserved[5];
} GCyclopsInitCmdPacket;
//
//Command Response packet format: GSkipDefaultResponsePacket - header = 9Ah.
//
/***************************************************************************************************/
//SKIP_CMD_ID_SET_MEASUREMENT_PERIOD:
//
//Command packet format:
typedef struct
{
	unsigned char cmd;	//SKIP_CMD_ID_SET_MEASUREMENT_PERIOD
	unsigned char lsbyteLswordMeasurementPeriod;
	unsigned char msbyteLswordMeasurementPeriod;
	unsigned char lsbyteMswordMeasurementPeriod;
	unsigned char msbyteMswordMeasurementPeriod;
	unsigned char reserved[3];
} GSkipSetMeasurementPeriodCmdPacket;
//
//Command Response packet format: GSkipDefaultResponsePacket.
//
/***************************************************************************************************/
//SKIP_CMD_ID_GET_MEASUREMENT_PERIOD:
//
//Command packet format: GSkipOutputPacket.
//
//Command Response packet format:
typedef struct
{
	unsigned char header;		//5Dh
	unsigned char cmd;			//SKIP_CMD_ID_GET_MEASUREMENT_PERIOD
	unsigned char lsbyteLswordMeasurementPeriod;
	unsigned char msbyteLswordMeasurementPeriod;
	unsigned char lsbyteMswordMeasurementPeriod;
	unsigned char msbyteMswordMeasurementPeriod;
	unsigned char reserved[2];
} GSkipGetMeasurementPeriodCmdResponsePacket;
//
/***************************************************************************************************/
//SKIP_CMD_ID_SET_LED_STATE:
//
#define SKIP_LED_COLOR_BLACK 0xC0
#define SKIP_LED_COLOR_RED 0x40
#define SKIP_LED_COLOR_GREEN 0x80
#define SKIP_LED_COLOR_RED_GREEN 0
#define SKIP_LED_BRIGHTNESS_MIN 0
#define SKIP_LED_BRIGHTNESS_MAX 0x10
//
//Command packet format:
typedef struct
{
	unsigned char cmd;	//SKIP_CMD_ID_SET_LED_STATE
	unsigned char color;
	unsigned char brightness;
	unsigned char reserved[5];
} GSkipSetLedStateCmdPacket;
//
//Command Response packet format: GSkipDefaultResponsePacket.
//
/***************************************************************************************************/
//SKIP_CMD_ID_GET_LED_STATE:
//
//Command packet format: GSkipOutputPacket.
//
//Command Response packet format:
typedef struct
{
	unsigned char header;		//5Bh
	unsigned char cmd;			//SKIP_CMD_ID_GET_LED_STATE
	unsigned char color;
	unsigned char brightness;
	unsigned char reserved[4];
} GSkipGetLedStateCmdResponsePacket;
//
/***************************************************************************************************/
//SKIP_CMD_ID_GET_SERIAL_NUMBER:
//
//Command packet format: GSkipOutputPacket.
//
//Command Response packet format:
typedef struct
{
	unsigned char header;		//5Fh
	unsigned char cmd;			//SKIP_CMD_ID_GET_SERIAL_NUMBER
	unsigned char ww;			//week in year(starting at 1) in BCD format
	unsigned char yy;			//last two digits of year in BCD format
	unsigned char lsbyteLswordSerialCounter;
	unsigned char msbyteLswordSerialCounter;
	unsigned char lsbyteMswordSerialCounter;
	unsigned char msbyteMswordSerialCounter;
} GSkipGetSerialNumberCmdResponsePacket;
//
/***************************************************************************************************/
//SKIP_CMD_ID_SET_VIN_OFFSET_DAC:
//
//This is used to calibrate the SKIP_ANALOG_INPUT_CHANNEL_VIN (+/-10v) hardware.
//
//Command packet format:
typedef struct
{
	unsigned char cmd;	//SKIP_CMD_ID_SET_VIN_OFFSET_DAC
	unsigned char dacSetting;
	unsigned char reserved[6];
} GSkipSetVinOffsetDacCmdPacket;
//
//Command Response packet format: GSkipDefaultResponsePacket.
//
/***************************************************************************************************/
//SKIP_CMD_ID_GET_SENSOR_ID:
//
//Command packet format: GSkipOutputPacket.
//
//Command Response packet format:
typedef struct
{
	unsigned char header;		//5Fh
	unsigned char cmd;			//SKIP_CMD_ID_GET_SENSOR_ID
	unsigned char lsbyteLswordSensorId;
	unsigned char msbyteLswordSensorId;
	unsigned char lsbyteMswordSensorId;
	unsigned char msbyteMswordSensorId;
	unsigned char reserved[2];
} GSkipGetSensorIdCmdResponsePacket;
//
/***************************************************************************************************/
//SKIP_CMD_ID_SET_ANALOG_INPUT_CHANNEL:
//
//Command packet format:
typedef struct
{
	unsigned char cmd;	//SKIP_CMD_ID_SET_ANALOG_INPUT_CHANNEL
	unsigned char analogInputChannel;
	unsigned char reserved[6];
} GSkipSetAnalogInputChannelCmdPacket;
//
//Command Response packet format: GSkipDefaultResponsePacket.
//
/***************************************************************************************************/
//SKIP_CMD_ID_GET_ANALOG_INPUT_CHANNEL:
//
//Command packet format: GSkipOutputPacket.
//
//Command Response packet format:
typedef struct
{
	unsigned char header;		//5Ah
	unsigned char cmd;			//SKIP_CMD_ID_GET_ANALOG_INPUT_CHANNEL
	unsigned char analogInputChannel;
	unsigned char reserved[5];
} GSkipGetAnalogInputChannelCmdResponsePacket;
//
/***************************************************************************************************/
//SKIP_CMD_ID_GET_VIN_OFFSET_DAC:
//
//Command packet format: GSkipOutputPacket.
//
//Command Response packet format:
typedef struct
{
	unsigned char header;		//5Ah
	unsigned char cmd;			//SKIP_CMD_ID_GET_VIN_OFFSET_DAC
	unsigned char dacSetting;
	unsigned char reserved[5];
} GSkipGetVinOffsetDacCmdResponsePacket;
//
/***************************************************************************************************/
//SKIP_CMD_ID_GET_MEASUREMENT_STATUS:
//
//Command packet format: GSkipOutputPacket.
//
//Command Response packet format:
typedef struct
{
	unsigned char header;	//5Fh
	unsigned char cmd;		//SKIP_CMD_ID_GET_MEASUREMENT_STATUS
	unsigned char flags;	//See SKIP_MEASURMENT_STATUS_MASKs and CYCLOPS_TEMPERATURE_COMP_MASK.
	unsigned char triggerType;	//See CYCLOPS_TRIGGER_TYPE.
	unsigned char lsbyteMeasurementCount;	//(count int16 < 0) => non real Time pending, abs(count) says how many.
	unsigned char msbyteMeasurementCount;	//(count == 0) if real Time is in progress.
    unsigned char dataRunSignature; //This gets incremented every time a new set of measurements is triggered.
	unsigned char filterType;       //CYCLOPS_FILTER.
} GSkipGetMeasurementStatusCmdResponsePacket;
//
/***************************************************************************************************/
//SKIP_CMD_ID_GET_MEASUREMENTS:
//
//Command packet format:
typedef struct
{
	unsigned char cmd;		//SKIP_CMD_ID_GET_MEASUREMENTS
	unsigned char lsbyteFirstMeasurementIndex;	//Int16 from 0 to (CYCLOPS_MAX_MEASUREMENT_COUNT-1)
	unsigned char msbyteFirstMeasurementIndex;
	unsigned char lsbyteMeasurementCount;		//Int16 from 1 to CYCLOPS_MAX_MEASUREMENT_COUNT.
	unsigned char msbyteMeasurementCount;
	unsigned char measType;	                    //CYCLOPS_MEAS_TYPE.
	unsigned char filterType;                   //CYCLOPS_FILTER.
	unsigned char reserved;
} GSkipGetMeasurementsCmdPacket;
//
//Command Response packet format:
//The first response packet has the following format:
typedef GSkipReadNVMemCmdFirstResponsePacket GSkipGetMeasurementsCmdFirstResponsePacket;
//
//The second through the last response packet has the following format:
typedef GSkipReadNVMemCmdResponsePacket GSkipGetMeasurementsCmdResponsePacket;
//
// The payload bytes in the response packets are used to transmit a byte stream. The byte stream
// data corresponds to an array of 32 bit integer measurements in little endian format. The units of
// the measurements are microns, microns/second, or microns/second**2.
//typedef struct
//{
//    unsigned char dataRunSignature; //This gets incremented every time a new set of measurements is triggered.
//    int measurements[CYCLOPS_MAX_MEASUREMENT_COUNT]; Note that this structure is packed, so the measurements are not DWORD aligned.
//} GSkipGetNonRealTimeMeasurementsCmdResponsePayload; //This is the response payload returned by GetNextResponse() after sending SKIP_CMD_ID_GET_MEASUREMENTS(Cyclops only).
//
//
/***************************************************************************************************/
//SKIP_CMD_ID_SET_TEMPERATURE:
//
//  Store the CYCLOPS_TEMPERATURE_COMP_TYPE_FIXED temperature.
//
//Command packet format:
typedef struct
{
	unsigned char cmd;	//SKIP_CMD_ID_SET_TEMPERATURE
	unsigned char lsbyteLswordFloatTemperatureC;//Temperature is in degrees Centigrade.
	unsigned char msbyteLswordFloatTemperatureC;//Format is 4 byte IEEE float.
	unsigned char lsbyteMswordFloatTemperatureC;//Ls byte is first.
	unsigned char msbyteMswordFloatTemperatureC;//Ms byte is last.
	unsigned char reserved[3];
} GSkipSetTemperatureCmdPacket;
//
//Command Response packet format:  GSkipDefaultResponsePacket.
//
/***************************************************************************************************/
//SKIP_CMD_ID_GET_TEMPERATURE:
//
//Command packet format:
typedef struct
{
	unsigned char cmd;	//SKIP_CMD_ID_GET_TEMPERATURE
	unsigned char compensationType;//See CYCLOPS_TEMPERATURE_COMP_TYPE.
	unsigned char reserved[6];
} GSkipGetTemperatureCmdPacket;
//
//Command Response packet format:
typedef struct
{
	unsigned char header;	//5Dh
	unsigned char cmd;		//SKIP_CMD_ID_GET_TEMPERATURE
	unsigned char lsbyteLswordFloatTemperatureC;//Temperature is in degrees Centigrade.
	unsigned char msbyteLswordFloatTemperatureC;//Format is 4 byte IEEE float.
	unsigned char lsbyteMswordFloatTemperatureC;//Ls byte is first.
	unsigned char msbyteMswordFloatTemperatureC;//Ms byte is last.
	unsigned char reserved[2];
} GSkipGetTemperatureCmdResponsePacket;
//
/***************************************************************************************************/
//
// The following declaration declares a single union type for all the different types of command
// packets. This allows us to access all the command packets with a single pointer variable.
//
typedef union
{
	GSkipOutputPacket generic;
    GCyclopsInitCmdPacket initCyclops;
    GCyclopsStartMeasurementsCmdPacket startMeasurementsCyclops;
	GSkipWriteNVMemCmdPacket writeNVMem;
	GSkipReadNVMemCmdPacket readNVMem;
	GSkipSetMeasurementPeriodCmdPacket setMeasurementPeriod;
	GSkipSetLedStateCmdPacket setLedState;
	GSkipSetAnalogInputChannelCmdPacket setAnalogInputChannel;
	GSkipSetVinOffsetDacCmdPacket setVinOffsetDac;
    GSkipGetMeasurementsCmdPacket getMeasurements;
    GSkipSetTemperatureCmdPacket setTemperature;
    GSkipGetTemperatureCmdPacket getTemperature;
} GSkipCmdPacket;
//
/***************************************************************************************************/
//
// The following declaration declares a single union type for all the different types of command
// response packets. This allows us to access all the command response packets with a single pointer variable.
//
typedef union
{
	GSkipDefaultResponsePacket generic;
	GSkipGetStatusCmdResponsePacket getStatus;
	GSkipReadNVMemCmdFirstResponsePacket readNVMemFirstResponse;
	GSkipReadNVMemCmdResponsePacket readNVMemNthResponse;
	GSkipGetMeasurementPeriodCmdResponsePacket getMeasurementPeriod;
	GSkipGetLedStateCmdResponsePacket getLedState;
	GSkipGetSerialNumberCmdResponsePacket getSerialNumber;
	GSkipGetSensorIdCmdResponsePacket getSensorId;
	GSkipGetAnalogInputChannelCmdResponsePacket getAnalogInputChannel;
	GSkipGetVinOffsetDacCmdResponsePacket getVinOffsetDac;
    GSkipGetMeasurementStatusCmdResponsePacket getMeasurementStatus;
    GSkipGetMeasurementsCmdFirstResponsePacket getMeasurementsFirstResponse;
    GSkipGetMeasurementsCmdResponsePacket getMeasurementsNthResponse;
    GSkipGetTemperatureCmdResponsePacket getTemperature;
} GSkipCmdResponsePacket;
//
/***************************************************************************************************/
//
// The Skip master CPU sends SKIP_FAKE_CMD_ID_POLL packets to query the slave for input packets.
// If the slave has no data to report, it sends back packets with an SKIP_IDLE_NOTIFICATION_HDR.
// The payload bytes of the SKIP_FAKE_CMD_ID_POLL contain values ramping from (START_TEST_RAMP-1) to (START_TEST_RAMP-7).
// The payload bytes of the idle response packet contain values ramping from (START_TEST_RAMP+1) to (START_TEST_RAMP+7).
#define SKIP_IDLE_NOTIFICATION_HDR SKIP_INPUT_PACKET_NOTIFICATION
#define SKIP_START_TEST_RAMP 0x61

/***************************************************************************************************/
//Packet parsing info:
/***************************************************************************************************/
#define SKIP_MASK_INPUT_PACKET_ERROR_FLAG 0x20

//The following mask is used to calculate the number of bytes in a command response packet not counting the header.
#define SKIP_MASK_CMD_RESP_NUMBYTES 7
#define SKIP_MAX_CMD_RESP_NUMBYTES 7
#define SKIP_MAX_READ_NV_MEM_DATA_BYTES_1ST_PACKET 6
#define SKIP_DDS_MAX_ADDRESS 0x7f
#define SKIP_MASK_CMD_RESP_1ST_PACKET_FLAG 8
#define SKIP_MASK_CMD_RESP_LAST_PACKET_FLAG 0x10
#define SKIP_CMD_RESP_DEFAULT_NUMBYTES 2
#define SKIP_CMD_RESP_ERROR_HDR (SKIP_INPUT_PACKET_CMD_RESP | SKIP_MASK_INPUT_PACKET_ERROR_FLAG | SKIP_MASK_CMD_RESP_1ST_PACKET_FLAG | SKIP_MASK_CMD_RESP_LAST_PACKET_FLAG | SKIP_CMD_RESP_DEFAULT_NUMBYTES)
#define SKIP_CMD_RESP_DEFAULT_HDR (SKIP_INPUT_PACKET_CMD_RESP | SKIP_MASK_CMD_RESP_1ST_PACKET_FLAG | SKIP_MASK_CMD_RESP_LAST_PACKET_FLAG | SKIP_CMD_RESP_DEFAULT_NUMBYTES)
#define SKIP_INIT_CMD_RESP_HDR (SKIP_INPUT_PACKET_INIT_RESP | SKIP_MASK_CMD_RESP_1ST_PACKET_FLAG | SKIP_MASK_CMD_RESP_LAST_PACKET_FLAG | SKIP_CMD_RESP_DEFAULT_NUMBYTES)
#define SKIP_GET_STATUS_CMD_RESP_HDR (SKIP_INPUT_PACKET_CMD_RESP | SKIP_MASK_CMD_RESP_1ST_PACKET_FLAG | SKIP_MASK_CMD_RESP_LAST_PACKET_FLAG | 6)
#define SKIP_GET_MEASUREMENT_PERIOD_CMD_RESP_HDR (SKIP_INPUT_PACKET_CMD_RESP | SKIP_MASK_CMD_RESP_1ST_PACKET_FLAG | SKIP_MASK_CMD_RESP_LAST_PACKET_FLAG | 5)
#define SKIP_GET_LED_STATE_CMD_RESP_HDR (SKIP_INPUT_PACKET_CMD_RESP | SKIP_MASK_CMD_RESP_1ST_PACKET_FLAG | SKIP_MASK_CMD_RESP_LAST_PACKET_FLAG | 3)
#define SKIP_GET_SERIAL_NUMBER_CMD_RESP_HDR (SKIP_INPUT_PACKET_CMD_RESP | SKIP_MASK_CMD_RESP_1ST_PACKET_FLAG | SKIP_MASK_CMD_RESP_LAST_PACKET_FLAG | 7)
#define SKIP_GET_SENSOR_ID_CMD_RESP_HDR (SKIP_INPUT_PACKET_CMD_RESP | SKIP_MASK_CMD_RESP_1ST_PACKET_FLAG | SKIP_MASK_CMD_RESP_LAST_PACKET_FLAG | 5)
#define SKIP_GET_ANALOG_INPUT_CHANNEL_CMD_RESP_HDR (SKIP_INPUT_PACKET_CMD_RESP | SKIP_MASK_CMD_RESP_1ST_PACKET_FLAG | SKIP_MASK_CMD_RESP_LAST_PACKET_FLAG | 2)
#define SKIP_GET_VIN_OFFSET_DAC_CMD_RESP_HDR (SKIP_INPUT_PACKET_CMD_RESP | SKIP_MASK_CMD_RESP_1ST_PACKET_FLAG | SKIP_MASK_CMD_RESP_LAST_PACKET_FLAG | 2)

#define SKIP_GET_MEASUREMENT_STATUS_CMD_RESP_HDR (SKIP_INPUT_PACKET_CMD_RESP | SKIP_MASK_CMD_RESP_1ST_PACKET_FLAG | SKIP_MASK_CMD_RESP_LAST_PACKET_FLAG | 7)
#define SKIP_GET_TEMPERATURE_CMD_RESP_HDR (SKIP_INPUT_PACKET_CMD_RESP | SKIP_MASK_CMD_RESP_1ST_PACKET_FLAG | SKIP_MASK_CMD_RESP_LAST_PACKET_FLAG | 5)

#if defined(TARGET_OS_WIN)
#pragma pack(pop)
#endif

#ifdef TARGET_OS_MAC
#pragma pack()
#endif

#endif  //_SKIP_COMMUNICATION_H_

