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
#ifndef _CYCLOPS_COMMUNICATION_EXT_H_
#define _CYCLOPS_COMMUNICATION_EXT_H_

/***************************************************************************************************/
// Go! Motion is also known as Cyclops.
//
// This file contains declarations for parameter and response structures used by the Skip/Cyclops support
// function SendCmdAndGetResponse(). This stuff should really go into GSkipCommExt.h, but that is part of
// the public interface to GoIO_DLL, and we do not want to have to explain these odd Cyclops extensions,
// so we are tucking the info away in here.
//
// The Cyclops extensions decribed here are not really necessary to take measurements. The commands described
// in GSkipCommExt.h are all that you need to retrieve measurements when Cyclops is connected to the
// host computer by a USB cable. However, if you want to gather measurements with Cyclops when it is
// disconnected from the computer, and then send them to the computer later on, then you will need the
// commands described here.
//
// All Skip family devices have the ability to start sending measurements up in 'real time' in response
// to SKIP_CMD_ID_START_MEASUREMENTS. That includes Cyclops. Measurements are sent back continuously
// to the host computer until the Skip family device receives a SKIP_CMD_ID_STOP_MEASUREMENTS command.
//
// Cyclops supports an additional mode. If a non zero measurement count parameter is sent to Cyclops with
// the SKIP_CMD_ID_START_MEASUREMENTS command, then a fixed number of measurements are taken and stored on
// Cyclops. These measurements remain on Cyclops until they are retrieved with a SKIP_CMD_ID_GET_MEASUREMENTS
// command.
//
/***************************************************************************************************/

//Cyclops specific commands:
#define SKIP_CMD_ID_GET_MEASUREMENT_STATUS 0x30
#define SKIP_CMD_ID_GET_MEASUREMENTS 0x31
#define SKIP_CMD_ID_SET_TEMPERATURE 0x32
#define SKIP_CMD_ID_GET_TEMPERATURE 0x33

#ifdef SKIP_LAST_CMD_ID
#undef SKIP_LAST_CMD_ID
#define SKIP_LAST_CMD_ID SKIP_CMD_ID_GET_TEMPERATURE
#endif

#if defined (TARGET_OS_WIN)
#pragma pack(push)
#pragma pack(1)
#endif

#ifdef TARGET_OS_MAC
#pragma pack(1)
#endif

/***************************************************************************************************/
#define SKIP_HOST_TYPE_UNKNOWN 0
#define SKIP_HOST_TYPE_CALCULATOR 1
#define SKIP_HOST_TYPE_COMPUTER 2
typedef struct
{
	unsigned char hostType;                 //SKIP_HOST_TYPE.
	unsigned char forcePowerOnDefaults;     //Put Cyclops in power on state if this is non zero.
} GCyclopsInitParams;           //Parameter block(optional) passed into SendCmd() with SKIP_CMD_ID_INIT for Cyclops.

/***************************************************************************************************/
#define CYCLOPS_TRIGGER_TYPE_IMMEDIATE 0
#define CYCLOPS_TRIGGER_TYPE_BUTTON 1
#define CYCLOPS_TRIGGER_TYPE_DELAYED 7

#define CYCLOPS_MEAS_TYPE_DISTANCE 0
#define CYCLOPS_MEAS_TYPE_VELOCITY 1
#define CYCLOPS_MEAS_TYPE_ACCEL 2

#define CYCLOPS_TEMPERATURE_COMP_TYPE_DYNAMIC 0
#define CYCLOPS_TEMPERATURE_COMP_TYPE_FIXED 1
#define CYCLOPS_TEMPERATURE_COMP_TYPE_NONE 2
#define CYCLOPS_TEMPERATURE_COMP_MASK 3

#define CYCLOPS_FILTER_NONE 0
#define CYCLOPS_FILTER_5POINT_SG_SMOOTHING 1
#define CYCLOPS_FILTER_9POINT_SG_SMOOTHING 2
#define CYCLOPS_FILTER_17POINT_SG_SMOOTHING 3
#define CYCLOPS_FILTER_29POINT_SG_SMOOTHING 4
#define CYCLOPS_FILTER_3POINT_MEDIAN_PRUNING 5
#define CYCLOPS_FILTER_5POINT_MEDIAN_PRUNING 6
#define CYCLOPS_FILTER_LIGHT_REAL_TIME_TRACKING 7
#define CYCLOPS_FILTER_MEDIUM_REAL_TIME_TRACKING 8
#define CYCLOPS_FILTER_HEAVY_REAL_TIME_TRACKING 9
//Note: Filters 1-6 are illegal in real time sampling mode, and generate an error.
//Filters 7-9 are illegal in non realtime sampling mode, and generate an error if used there

//The filterType parameter passed to Cyclops with the SKIP_CMD_ID_START_MEASUREMENTS command is ignored
//unless we are starting real time measurements, in which case it should be in the range 7 to 9, or 0.
//
//To control the filterType with non realtime measurements, pass in a filterType parameter with the
//SKIP_CMD_ID_GET_MEASUREMENTS command.

#define CYCLOPS_MAX_MEASUREMENT_COUNT 512

typedef struct
{
	unsigned char triggerType;              //CYCLOPS_TRIGGER_TYPE.
	unsigned char lsbyteMeasurementCount;	//(count int16 == 0) => real Time
	unsigned char msbyteMeasurementCount;	//count int16 <= CYCLOPS_MAX_MEASUREMENT_COUNT.
	unsigned char realTimeMeasType;			//CYCLOPS_MEAS_TYPE.
	unsigned char flags;					//CYCLOPS_TEMPERATURE_COMP ...
	unsigned char filterType;               //CYCLOPS_FILTER.
} GCyclopsStartMeasurementsParams; //Parameter block(optional) passed into SendCmd() with SKIP_CMD_ID_START_MEASUREMENTS for Cyclops.
                                   //Skip and Jonah never need parameters with SKIP_CMD_ID_START_MEASUREMENTS.
								   //If you do not send this parameter block to Cyclops, then reasonable default values are used(real time measurements).
/***************************************************************************************************/

typedef struct
{
	unsigned char lsbyteFirstMeasurementIndex;	//Int16 from 0 to (CYCLOPS_MAX_MEASUREMENT_COUNT-1)
	unsigned char msbyteFirstMeasurementIndex;
	unsigned char lsbyteMeasurementCount;		//Int16 from 1 to CYCLOPS_MAX_MEASUREMENT_COUNT.
	unsigned char msbyteMeasurementCount;
	unsigned char measType;	                    //CYCLOPS_MEAS_TYPE.
	unsigned char filterType;                   //CYCLOPS_FILTER.
} GSkipGetMeasurementsParams;//Parameter block passed into SendCmd() with SKIP_CMD_ID_GET_MEASUREMENTS.

typedef struct
{
    unsigned char dataRunSignature; //This gets incremented every time a new set of measurements is triggered.
    int measurements[CYCLOPS_MAX_MEASUREMENT_COUNT];// Note that this structure is packed, so the measurements are not DWORD aligned.
													// Each measurement is stored as a 4 byte little endian integer when this structure is
													// returned by SendCmdAndGetResponse(), so the caller needs to reorder the bytes
													// in big endian environments. The Measurements are in microns.
} GSkipGetNonRealTimeMeasurementsCmdResponsePayload; //This is the response payload returned by GetNextResponse() after sending SKIP_CMD_ID_GET_MEASUREMENTS(Cyclops only).

/***************************************************************************************************/
typedef struct
{
	unsigned char lsbyteLswordFloatTemperatureC;//Temperature is in degrees Centigrade.
	unsigned char msbyteLswordFloatTemperatureC;//Format is 4 byte IEEE float.
	unsigned char lsbyteMswordFloatTemperatureC;//Ls byte is first.
	unsigned char msbyteMswordFloatTemperatureC;//Ms byte is last.
} GSkipSetTemperatureParams;//Parameter block passed into SendCmd() with SKIP_CMD_ID_SET_TEMPERATURE.

typedef struct
{
	unsigned char compensationType;//See CYCLOPS_TEMPERATURE_COMP_TYPE.
} GSkipGetTemperatureParams;//Parameter block passed into SendCmd() with SKIP_CMD_ID_GET_TEMPERATURE.

typedef GSkipSetTemperatureParams GSkipGetTemperatureCmdResponsePayload; //This is the response payload returned by GetNextResponse()
//after sending SKIP_CMD_ID_GET_TEMPERATURE.

/***************************************************************************************************/
#define SKIP_MEASURMENT_STATUS_MASK_REALTIME_MEAS_ENABLED 0x80
#define SKIP_MEASURMENT_STATUS_MASK_NONREALTIME_MEAS_ENABLED 0x40
#define SKIP_MEASURMENT_STATUS_MASK_TRIGGERED 0x20

// Flag set to indicate data stored in unit was collected in REAL-TIME mode
// Flag clear to indicate data stored in unit was collected in NON-REAL TIME mode
#define SKIP_MEASURMENT_STATUS_MASK_REALTIMEDATA 0x10

typedef struct
{
	unsigned char flags;	//See SKIP_MEASURMENT_STATUS_MASKs and CYCLOPS_TEMPERATURE_COMP_MASK.
	unsigned char triggerType;	//See CYCLOPS_TRIGGER_TYPE.
	unsigned char lsbyteMeasurementCount;	//(count int16 < 0) => non real Time pending, abs(count) says how many.
	unsigned char msbyteMeasurementCount;	//(count == 0) if real Time is in progress.
    unsigned char dataRunSignature; //This gets incremented every time a new set of measurements is triggered.
	unsigned char filterType;       //CYCLOPS_FILTER.
} GSkipGetMeasurementStatusCmdResponsePayload; //This is the response payload returned by GetNextResponse() after sending SKIP_CMD_ID_GET_MEASUREMENT_STATUS(Cyclops only).

/***************************************************************************************************/

#if defined (TARGET_OS_WIN)
#pragma pack(pop)
#endif

#ifdef TARGET_OS_MAC
#pragma pack()
#endif

#endif //_CYCLOPS_COMMUNICATION_EXT_H_

