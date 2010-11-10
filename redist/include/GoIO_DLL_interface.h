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
#ifndef _GOIO_DLL_INTERFACE_H_
#define _GOIO_DLL_INTERFACE_H_

/***************************************************************************************************************************

	This file documents the 'C' interface to GoIO_DLL.

	This library is implemented as GoIO_DLL.dll on Windows and as libGoIO_DLL.dylib on the Mac.

	Go! Link, Go! Temp, Go! Motion, and Vernier Mini GC	devices may be accessed by this library.

	The Vernier Mini GC device is implemented internally as a Go! Link interface with a fixed GC sensor plugged in, so most
	of the comments describing the Go! Link interface apply to the Mini GC as well. The Mini GC does have its own USB 
	product id so it can be distinguished from a Go! Link.

	The GoIO API is fairly broad, so knowing where to start is hard. The documentation for the GoIO_Sensor_Open() and the 
	GoIO_Sensor_SendCmdAndGetResponse() functions are good starting places. The GoIO_DeviceCheck sample program gives a good
	overview.
	
***************************************************************************************************************************/
#ifdef TARGET_OS_LINUX
#ifdef __cplusplus
	#define GOIO_DLL_INTERFACE_DECL extern "C" __attribute__ ((visibility("default")))
#else
	#define GOIO_DLL_INTERFACE_DECL __attribute__ ((visibility("default")))
#endif
#else
#ifdef __cplusplus
	#define GOIO_DLL_INTERFACE_DECL extern "C"
#else
	#define GOIO_DLL_INTERFACE_DECL
#endif
#endif

#include "GSkipCommExt.h"
#include "GSensorDDSMem.h"
#include "GVernierUSB.h"

#ifdef TARGET_OS_MAC
	#define GOIO_MAX_SIZE_DEVICE_NAME 255
#endif

#if defined (TARGET_OS_WIN) || defined (TARGET_OS_LINUX)
	#define GOIO_MAX_SIZE_DEVICE_NAME 260
#endif

#ifndef GTYPE_NUMS
#define GTYPE_NUMS
typedef short gtype_int16;
typedef unsigned short gtype_uint16;
#ifdef TARGET_OS_DAQ
typedef long int gtype_int32;
typedef unsigned long int gtype_uint32;
#else
typedef int gtype_int32;
typedef unsigned int gtype_uint32;
#endif
typedef long long gtype_int64;
typedef unsigned long long gtype_uint64;
typedef unsigned char gtype_bool;
typedef double gtype_real64;
typedef float gtype_real32;
#endif

typedef void *GOIO_SENSOR_HANDLE;

#define SKIP_TIMEOUT_MS_DEFAULT 2000
#define SKIP_TIMEOUT_MS_READ_DDSMEMBLOCK 2000
#define SKIP_TIMEOUT_MS_WRITE_DDSMEMBLOCK 4000


/***************************************************************************************************************************
	Function Name: GoIO_Init()
	
	Purpose:	Call GoIO_Init() once before making any other GoIO function calls.
				GoIO_Init() and GoIO_Uninit() should be called from the same thread.

				This routine attempts to grab the mutex that controls access to Vernier
				sensors. If Logger Pro, Logger Lite, or another GoIO application is currently
				running, then this routine will fail to grab the device control mutex, so it
				will return -1. Subsequent calls to GoIO functions will also fail until
				the application is able to run GoIO_Init() successfully.

	Return:		0 iff successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Init();

/***************************************************************************************************************************
	Function Name: GoIO_Uninit()
	
	Purpose:	Call GoIO_Uninit() once to 'undo' GoIO_Init().
				GoIO_Init() and GoIO_Uninit() should be called from the same thread.

	Return:		0 iff successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Uninit();

/***************************************************************************************************************************
	Function Name: GoIO_Diags_SetDebugTraceThreshold()
	
	Purpose:	GoIO lib generates a variety of debugging messages when it runs. Each message is assigned a severity
				when it is generated. Only messages that are assigned a priority >= the debug trace threshold are actually
				sent to the debug output. Call GoIO_Diags_SetDebugTraceThreshold(GOIO_TRACE_SEVERITY_LOWEST) for max
				debug output.
				
				On windows systems, these messages are passed to the OutputDebugString() function.
				On Mac and Linux systems, these messages are sent to STDOUT and/or STDERR.

	Return:		0 iff successful, else -1.

****************************************************************************************************************************/
#define GOIO_TRACE_SEVERITY_LOWEST 1
#define GOIO_TRACE_SEVERITY_LOW 10
#define GOIO_TRACE_SEVERITY_MEDIUM 50
#define GOIO_TRACE_SEVERITY_HIGH 100
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Diags_SetDebugTraceThreshold(
	gtype_int32 threshold);//[in] Only trace messages marked with a severity >= threshold are actually sent to the debug output.

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Diags_GetDebugTraceThreshold(
	gtype_int32 *pThreshold);//[out]

/***************************************************************************************************************************
	Function Name: GoIO_GetDLLVersion()
		Added in version 2.00.
	
	Purpose:	This routine returns the major and minor version numbers for the instance of GoIO_DLL that is
				currently running.

				If a function is not guaranteed to be present in all supported versions of GoIO_DLL, then the line
				"Added in version 'major.minor'" will appear in the function description in this file.

				It is our intention that all versions of GoIO_DLL created subsequent to a given version, will be
				backwards compatible with the older version. You should be able to replace an old version of GoIO_DLL
				with a newer version and everything should still work without rebuilding your application.

				Note that version major2.minor2 is later than version major1.minor1 
				iff. ((major2 > major1) || ((major2 == major1) && (minor2 > minor1))).
				
				Backwards compatibility is definitely our intention, but we do not absolutely guarantee it. If you think
				that you have detected a backwards compatibility bug, then please report it to Vernier Software & Technology.
				Calling GoIO_GetVersion() from your application is a way to identify precisely which version of
				GoIO_DLL you are actually using.


	Return:		0 iff successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_GetDLLVersion(
	gtype_uint16 *pMajorVersion, //[o]
	gtype_uint16 *pMinorVersion); //[o]

/***************************************************************************************************************************
	Function Name: GoIO_UpdateListOfAvailableDevices()
	
	Purpose:	This routine queries the operating system to build a list of available devices
				that have the specified USB vendor id and product id. Only Go! Link, Go! Temp, Go! Motion, and Vernier Mini GC
				vendor and product id's are supported.

	Return:		number of devices found.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_UpdateListOfAvailableDevices(
	gtype_int32 vendorId,	//[in]
	gtype_int32 productId);	//[in]

/***************************************************************************************************************************
	Function Name: GoIO_GetNthAvailableDeviceName()
	
	Purpose:	Get a unique device name string for the n'th device in the list of known devices with a specified
				USB vendor id and product id. This routine will only succeed if GoIO_UpdateListOfAvailableDevices()
				has been called previously.

	Return:		0 iff successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_GetNthAvailableDeviceName(
	char *pBuf,			//[out] ptr to buffer to store device name string.
	gtype_int32 bufSize,//[in] number of bytes in buffer pointed to by pBuf. Strlen(pBuf) < bufSize, because the string is NULL terminated.
	gtype_int32 vendorId,	//[in] USB vendor id
	gtype_int32 productId,	//[in] USB product id
	gtype_int32 N);			//[in] index into list of known devices, 0 => first device in list.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_Open()
	
	Purpose:	Open a specified Go! device and the attached sensor.. 
	
				If the device is already open, then this routine will fail.

				In addition to establishing basic communication with the device, this routine will initialize the
				device. Each GOIO_SENSOR_HANDLE sensor object has an associated DDS memory record. If the physical 
				sensor being opened is a 'smart' sensor with its own physical DDS memory, then this routine will copy
				the contents of the memory on the device to the sensor object's DDS memory record. If the physical 
				sensor does not have DDS memory, then the associated DDS memory record is set to default values.

				The following commands are sent to Go! Temp devices:
					SKIP_CMD_ID_INIT,
					SKIP_CMD_ID_READ_LOCAL_NV_MEM. - read DDS record

				The following commands are sent to Go! Link and Vernier Mini GC devices:
					SKIP_CMD_ID_INIT,
					SKIP_CMD_ID_GET_SENSOR_ID,
					SKIP_CMD_ID_READ_REMOTE_NV_MEM, - read DDS record if this is a 'smart' sensor
					SKIP_CMD_ID_SET_ANALOG_INPUT_CHANNEL. - based on sensor EProbeType

				SKIP_CMD_ID_GET_SENSOR_ID is superfluous when sent to the Mini GC, but the Mini GC is implemented internally
				as a Go! Link with a fixed sensor plugged in.

				Only SKIP_CMD_ID_INIT is sent to Go! Motion. Go! Motion does not contain DDS memory, but this routine
				initializes the sensor's associated DDS memory record with calibrations for both meters and feet.

				Since the device stops sending measurements in response to SKIP_CMD_ID_INIT, an application must send
				SKIP_CMD_ID_START_MEASUREMENTS to the device in order to receive measurements.

				At any given time, a sensor is 'owned' by only one thread. The thread that calls this routine is the
				initial owner of the sensor. If a GoIO() call is made from a thread that does not own the sensor object
				that is passed in, then the call will generally fail. To allow another thread to access a sensor,
				the owning thread should call GoIO_Sensor_Unlock(), and then the new thread must call GoIO_Sensor_Lock().
  
	Return:		handle to open sensor device if successful, else NULL.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL GOIO_SENSOR_HANDLE GoIO_Sensor_Open(
	const char *pDeviceName,	//[in] NULL terminated string that uniquely identifies the device. See GoIO_GetNthAvailableDeviceName().
	gtype_int32 vendorId,		//[in] USB vendor id
	gtype_int32 productId,		//[in] USB product id
	gtype_int32 strictDDSValidationFlag);//[in] insist on exactly valid checksum if 1, else use a more lax validation test.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_Close()
	
	Purpose:	Close a specified sensor. After this routine runs the hSensor handle is no longer valid.
	
	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_Close(
	GOIO_SENSOR_HANDLE hSensor);//[in] handle to open sensor.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetOpenDeviceName()
	
	Purpose:	Get the unique device name string and USB id's for a specified open device.

	Return:		0 iff successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_GetOpenDeviceName(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	char *pBuf,				//[out] ptr to buffer to store device name string.
	gtype_int32 bufSize,	//[in] number of bytes in buffer pointed to by pBuf. Strlen(pBuf) < bufSize, because the string is NULL terminated.
	gtype_int32 *pVendorId,	//[out]
	gtype_int32 *pProductId);//[out]

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_Lock()
	
	Purpose:	Lock a specified sensor so that no other thread can access it. This routine will fail if the sensor
				is currently locked by another thread.

				GoIO_Sensor_Lock() increments the lock count for the specified sensor by 1. In order for a second
				thread to acquire access to the sensor, the first thread must call GoIO_Sensor_Unlock() until the lock
				count reaches 0. Note that the lock count is set to 1 by GoIO_Sensor_Open().
				
				It is ok for the lock count to be greater than 1 when GoIO_Sensor_Close() is called.

				This is an advanced routine that most users should not use. It is only necessary if an application
				needs to access a single sensor from multiple threads.
	
	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_Lock(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	gtype_int32 timeoutMs);		//[in] wait this long to acquire the lock before failing.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_Unlock()
	
	Purpose:	Decrement the lock count for the specified sensor by 1. This routine will fail if the sensor
				is not currently locked by the calling thread.

				In order for a second thread to acquire access to the sensor, the first thread must call 
				GoIO_Sensor_Unlock() until the lock count reaches 0. Note that the lock count is set to 1 by 
				GoIO_Sensor_Open().

				If the lock count reaches zero, it is important to make a call to GoIO_Sensor_Lock() to lock the
				device. This will prevent multiple threads from simultaneously accessing the device, which can cause
				unpredictable results.
				
				It is ok for the lock count to be greater than 1 when GoIO_Sensor_Close() is called.

				This is an advanced routine that most users should not use. It is only necessary if an application
				needs to access a single sensor from multiple threads.
	
	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_Unlock(
	GOIO_SENSOR_HANDLE hSensor);//[in] handle to open sensor.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_ClearIO()
	
	Purpose:	Clear the input/output communication buffers for a specified sensor. 
				This also expties the GoIO Measurement Buffer, which holds measurements that have been retrieved from 
				the sensor.
	
	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_ClearIO(
	GOIO_SENSOR_HANDLE hSensor);//[in] handle to open sensor.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_SendCmdAndGetResponse()
	
	Purpose:	Send a command to the specified Go! device hardware and wait for a response. 

				Note that GoIO_Sensor_SendCmdAndGetResponse() will ASSERT if you send a SKIP_CMD_ID_START_MEASUREMENTS
				while GoIO_Sensor_GetNumMeasurementsAvailable() says measurements are available. 
				This will not occur the first time SKIP_CMD_ID_START_MEASUREMENTS is sent after GoIO_Sensor_Open()
				because GoIO_Sensor_Open() automatically calls GoIO_Sensor_ClearIO().

				So...if you are restarting measurements, you should clear any old measurements in the GoIO Measurement 
				Buffer first by reading in the measurements until the Buffer is empty, or you should call 
				GoIO_Sensor_ClearIO().
	
	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_SendCmdAndGetResponse(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	unsigned char cmd,		//[in] command code. See SKIP_CMD_ID_* in GSkipCommExt.h.
	void *pParams,			//[in] ptr to cmd specific parameter block, may be NULL. See GSkipCommExt.h.
	gtype_int32 nParamBytes,//[in] # of bytes in (*pParams).
	void *pRespBuf,			//[out] ptr to destination buffer, may be NULL. See GSkipCommExt.h.
	gtype_int32 *pnRespBytes,//[in, out] ptr to size of of pRespBuf buffer on input, size of response on output, may be NULL if pRespBuf is NULL.
	gtype_int32 timeoutMs);	//[in] # of milliseconds to wait for a reply before giving up. Go! devices should reply to almost all the 
							//currently defined commands within SKIP_TIMEOUT_MS_DEFAULT(1000) milliseconds. In fact, typical response
							//times are less than 50 milliseconds. See SKIP_TIMEOUT_MS_* definitions.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetLastCmdResponseStatus()
	
	Purpose:	Get error information for the device. 
	
				In principle, any command sent to the device can result in an error. 
				If GoIO_Sensor_SendCmdAndGetResponse() ever fails and returns a non-zero return code,
				you can gather additional info about what went wrong by calling GoIO_Sensor_GetLastCmdResponseStatus().

	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_GetLastCmdResponseStatus(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	unsigned char *pLastCmd,	//[out] last cmd sent to the sensor
	unsigned char *pLastCmdStatus,//[out] status of last command sent to the sensor.
								  //If command ran successfully and the device reported good status, then this will be be SKIP_STATUS_SUCCESS(aka 0).
								  //If no response has been reported back from the device, then this will be SKIP_STATUS_ERROR_COMMUNICATION.
								  //If the device reported a failure, then this will be a cmd specific error, eg SKIP_STATUS_ERROR_...
	unsigned char *pLastCmdWithErrorRespSentOvertheWire, //[out] last cmd sent that caused the device to report back an error.
	unsigned char *pLastErrorSentOvertheWire);//[out] last error that came back from the device 'over the wire'.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_SendCmd()
	
	Purpose:	GoIO_Sensor_SendCmd() is an advanced function. You should usually use 
				GoIO_Sensor_SendCmdAndGetResponse() instead. After calling GoIO_Sensor_SendCmd(), you must call
				GoIO_Sensor_GetNextResponse() before sending any more commands to the device.

				The main reason that GoIO_Sensor_SendCmd() is made available to the user is to allow a program to send
				SKIP_CMD_ID_START_MEASUREMENTS commands to several different devices as close together as possible so that
				measurements start at about the same time on separate devices.
	
	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_SendCmd(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	unsigned char cmd,	//[in] command code
	void *pParams,			//[in] ptr to cmd specific parameter block, may be NULL. See GSkipCommExt.h.
	gtype_int32 nParamBytes);//[in] # of bytes in (*pParams).

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetNextResponse()
	
	Purpose:	GoIO_Sensor_GetNextResponse() is an advanced function. You should usually use 
				GoIO_Sensor_SendCmdAndGetResponse() instead. After calling GoIO_Sensor_SendCmd(), you must call
				GoIO_Sensor_GetNextResponse() before sending any more commands to the device.


	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_GetNextResponse(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	void *pRespBuf,				//[out] ptr to destination buffer, may be NULL. See GSkipCommExt.h.
	gtype_int32 *pnRespBytes,	//[in, out] ptr to size of of pRespBuf buffer on input, size of response on output, may be NULL if pRespBuf is NULL.
	unsigned char *pCmd,		//[out] identifies which command this response is for. Ptr must NOT be NULL!
	gtype_int32 *pErrRespFlag,	//[out] flag(1 or 0) indicating that the response contains error info. Ptr must NOT be NULL!
	gtype_int32 nTimeoutMs);	//[in] # of milliseconds to wait before giving up.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetMeasurementTickInSeconds()
	
	Purpose:	The measurement period for Go! devices is specified in discrete 'ticks', so the actual time between 
				measurements is an integer multiple of the tick time. The length of time between ticks is different 
				for Go! Link versus Go! Temp. 
	
	Return:		If hSensor is not valid, then this routine returns -1.0, else the return value = the length of time
				in seconds between ticks.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_real64 GoIO_Sensor_GetMeasurementTickInSeconds(
	GOIO_SENSOR_HANDLE hSensor);//[in] handle to open sensor.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetMinimumMeasurementPeriod()
	
	Return:		If hSensor is not valid, then this routine returns -1.0, else the return value = minimum measurement
				period in seconds that is supported by the device.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_real64 GoIO_Sensor_GetMinimumMeasurementPeriod(
	GOIO_SENSOR_HANDLE hSensor);//[in] handle to open sensor.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetMaximumMeasurementPeriod()
	
	Return:		If hSensor is not valid, then this routine returns -1.0, else the return value = maximum measurement
				period in seconds that is supported by the device.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_real64 GoIO_Sensor_GetMaximumMeasurementPeriod(
	GOIO_SENSOR_HANDLE hSensor);//[in] handle to open sensor.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_SetMeasurementPeriod()
	
	Purpose:	Set the measurement period to a specified number of seconds. The Go! sensor will report measurements
				to the computer at the measurement period interval once measurements have been started. These 
				measurements are held in the GoIO Measurement Buffer. 
				See documentation for GoIO_Sensor_GetNumMeasurementsAvailable().

				This routine sends SKIP_CMD_ID_SET_MEASUREMENT_PERIOD to the sensor with the parameter block configured 
				for the desiredPeriod.

				Because the measurement period is constrained to be a multiple of the tick returned by
				GoIO_Sensor_GetMeasurementTickInSeconds(), and because it must be 
					>= GoIO_Sensor_GetMinimumMeasurementPeriod(), and
					<= GoIO_Sensor_GetMaximumMeasurementPeriod(), 
				the actual period is different than the desiredPeriod.

				You can determine the actual period by calling GoIO_Sensor_GetMeasurementPeriod().

				This routine will fail if we are currently collecting measurements from the sensor. Note that collection
				is started by sending SKIP_CMD_ID_START_MEASUREMENTS to the sensor, and stopped by sending 
				SKIP_CMD_ID_STOP_MEASUREMENTS.

	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_SetMeasurementPeriod(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	gtype_real64 desiredPeriod,	//[in] desired measurement period in seconds.
	gtype_int32 timeoutMs);		//[in] # of milliseconds to wait for a reply before giving up. SKIP_TIMEOUT_MS_DEFAULT is recommended.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetMeasurementPeriod()

	Purpose:	This routine sends SKIP_CMD_ID_GET_MEASUREMENT_PERIOD to the sensor.
	
	Return:		1000000.0 if not successful, else the measurement period in seconds.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_real64 GoIO_Sensor_GetMeasurementPeriod(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	gtype_int32 timeoutMs);		//[in] # of milliseconds to wait for a reply before giving up. SKIP_TIMEOUT_MS_DEFAULT is recommended.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetNumMeasurementsAvailable()
	
	Purpose:	Report the number of measurements currently stored in the GoIO Measurement Buffer. 

				After SKIP_CMD_ID_START_MEASUREMENTS has been sent to the sensor, the sensor starts
				sending measurements to the host computer. These measurements are stored in the 
				GoIO Measurement Buffer. A separate GoIO Measurement Buffer is maintained for each
				open sensor.

				The delay between sending SKIP_CMD_ID_START_MEASUREMENTS and the appearance of the first
				measurement in the GoIO Measurement Buffer varies according to the type of Go! device.

				Go! device type	                Delay before first measurement
				-----------------               --------------------------------
				Go! Temp                        ~510 milliseconds
				Go! Link                        ~10 milliseconds
				Mini GC                         ~10 milliseconds
				Go! Motion                      ~ measurement period + 10 milliseconds

				The 10 millisecond delay specifed for Go! Link is just the approximate delay required for the
				data to come back from the device via USB. Go! Link actually performs the first measurement
				immediately.

				Subsequent measurements are sent at the currently configured measurement period interval. 
				See GoIO_Sensor_SetMeasurementPeriod().

				Call GoIO_Sensor_ReadRawMeasurements() to retrieve measurements from the
				GoIO Measurement Buffer. The GoIO Measurement Buffer is guaranteed to hold at least 1200
				measurements. The buffer is circular, so if you do not service it often enough, the
				oldest measurements in the buffer are lost. If you wish to capture all the 
				measurements coming from the sensor, you must call GoIO_Sensor_ReadRawMeasurements()
				often enough so that the GoIO_Sensor_GetNumMeasurementsAvailable() does not reach 1200.
				On the other hand, we reserve the right to make the Measurement Buffer > 1200 measurements, so
				do not assume that you can empty the buffer simply by reading in 1200 measurements.

				Each of the following actions clears the GoIO Measurement Buffer:
					1) Call GoIO_Sensor_ReadRawMeasurements() with count set to GoIO_Sensor_GetNumMeasurementsAvailable(), or
					2) Call GoIO_Sensor_GetLatestRawMeasurement(), or
					3) Call GoIO_Sensor_ClearIO().

				The GoIO Measurement Buffer is empty after GoIO_Sensor_Open() is called. It does not
				start filling up until SKIP_CMD_ID_START_MEASUREMENTS is sent to the sensor.

				Note that the sensor stops sending measurements to the host computer after
				SKIP_CMD_ID_STOP_MEASUREMENTS or SKIP_CMD_ID_INIT is sent, but sending these commands
				does not clear the GoIO Measurement Buffer.

	Return:		number of measurements currently stored in the GoIO Measurement Buffer. 

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_GetNumMeasurementsAvailable(
	GOIO_SENSOR_HANDLE hSensor);//[in] handle to open sensor.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_ReadRawMeasurements()
	
	Purpose:	Retrieve measurements from the GoIO Measurement Buffer. The measurements reported
				by this routine are actually removed from the GoIO Measurement Buffer.

				After SKIP_CMD_ID_START_MEASUREMENTS has been sent to the sensor, the sensor starts
				sending measurements to the host computer. These measurements are stored in the 
				GoIO Measurement Buffer. A separate GoIO Measurement Buffer is maintained for each
				open sensor. See the description of GoIO_Sensor_GetNumMeasurementsAvailable().

				Note that for Go! Temp and Go! Link, raw measurements range from -32768 to 32767.
				Go! Motion raw measurements are in microns and can range into the millions.

				To convert a raw measurement to a voltage use GoIO_Sensor_ConvertToVoltage().
				To convert a voltage to a sensor specific calibrated unit, use GoIO_Sensor_CalibrateData().

				WARNING!!! IF YOU ARE COLLECTING MORE THAN 50 MEASUREMENTS A SECOND FROM GO! LINK,
				READ THIS: The GoIO Measurement Buffer is packet oriented. If you are collecting 50 or
				less measurements per second, then each packet contains only 1 measurement, and there is
				no problem.

				If you are collecting more than 50 measurements a second, then each packet may contain 2 
				or 3 measurements. Depending on the exact measurement period, all the packets will 
				contain 2, or all the packets will contain 3 measurements. IF THE LAST MEASUREMENT COPIED
				INTO pMeasurementsBuf IS NOT THE LAST MEASUREMENT IN ITS PACKET, THEN MEASUREMENTS MAY
				BE LOST.
				There are a couple of safe workarounds to this problem:
					1) Always set the maxCount parameter to a multiple of 6, or
					2) Always set the maxCount parameter to GoIO_Sensor_GetNumMeasurementsAvailable().

	Return:		number of measurements retrieved from the GoIO Measurement Buffer. This routine
				returns immediately, so the return value may be less than maxCount.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_ReadRawMeasurements(
	GOIO_SENSOR_HANDLE hSensor,		//[in] handle to open sensor.
	gtype_int32 *pMeasurementsBuf,	//[out] ptr to loc to store measurements.
	gtype_int32 maxCount);	//[in] maximum number of measurements to copy to pMeasurementsBuf. See warning above.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetLatestRawMeasurement()
	
	Purpose:	Report the most recent measurement put in the GoIO Measurement Buffer. If no 
				measurements have been placed in the GoIO Measurement Buffer since it was
				created byGoIO_Sensor_Open(), then report a value of 0. 
				
				This routine also empties the GoIO Measurement Buffer, so GoIO_Sensor_GetNumMeasurementsAvailable()
				will report 0 after calling GoIO_Sensor_GetLatestRawMeasurement().

				After SKIP_CMD_ID_START_MEASUREMENTS has been sent to the sensor, the sensor starts
				sending measurements to the host computer. These measurements are stored in the 
				GoIO Measurement Buffer. A separate GoIO Measurement Buffer is maintained for each
				open sensor. See the description of GoIO_Sensor_GetNumMeasurementsAvailable().

				Note that for Go! Temp and Go! Link, raw measurements range from -32768 to 32767.
				Go! Motion raw measurements are in microns and can range into the millions.

				To convert a raw measurement to a voltage use GoIO_Sensor_ConvertToVoltage().
				To convert a voltage to a sensor specific calibrated unit, use GoIO_Sensor_CalibrateData().

	Return:		most recent measurement put in the GoIO Measurement Buffer. If no 
				measurements have been placed in the GoIO Measurement Buffer since it was
				created byGoIO_Sensor_Open(), then report a value of 0.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_GetLatestRawMeasurement(
	GOIO_SENSOR_HANDLE hSensor);//[in] handle to open sensor.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_ConvertToVoltage()
	
	Purpose:	Convert a raw measurement integer value into a real voltage value.
				Depending on the type of sensor(see GoIO_Sensor_GetProbeType()), the voltage
				may range from 0.0 to 5.0, or from -10.0 to 10.0 . For Go! Motion, voltage returned is simply distance
				in meters.

	Return:		voltage corresponding to a specified raw measurement value.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_real64 GoIO_Sensor_ConvertToVoltage(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	gtype_int32 rawMeasurement);//[in] raw measurement obtained from GoIO_Sensor_GetLatestRawMeasurement() or 
								//GoIO_Sensor_ReadRawMeasurements().

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_CalibrateData()
	
	Purpose:	Convert a voltage value into a sensor specific unit.
				What units this routine produces can be determined by calling
				GoIO_Sensor_DDSMem_GetCalPage(hSensor, GoIO_Sensor_DDSMem_GetActiveCalPage(),...) .

	Return:		value in sensor specific units corresponding to a specified voltage.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_real64 GoIO_Sensor_CalibrateData(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	gtype_real64 volts);		//[in] voltage value obtained from GoIO_Sensor_ConvertToVoltage();

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetProbeType()
	
	Purpose:	Find out the probe type. See EProbeType in GSensorDDSMem.h.

				For Go! Link devices, this attribute is dependent on the OperationType in the SensorDDSRecord. 
				See GoIO_Sensor_DDSMem_GetOperationType().
				If (2 == OperationType) then the sensor is kProbeTypeAnalog10V, else kProbeTypeAnalog5V.

				Note that for Go! Link devices, SKIP_CMD_ID_SET_ANALOG_INPUT_CHANNEL must be sent to the device with
				the analogInputChannel parameter set to SKIP_ANALOG_INPUT_CHANNEL_VIN_LOW for kProbeTypeAnalog5V,
				or set to SKIP_ANALOG_INPUT_CHANNEL_VIN for kProbeTypeAnalog10V devices. Generally, application
				programs do not have to deal with this, because GoIO_Sensor_Open() automatically sends
				SKIP_CMD_ID_SET_ANALOG_INPUT_CHANNEL to the device with the appropriate parameters.

				Go! Temp => kProbeTypeAnalog5V.
				Go! Motion => kProbeTypeMD.

	Return:		EProbeType.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_GetProbeType(
	GOIO_SENSOR_HANDLE hSensor);//[in] handle to open sensor.


/***************************************************************************************************************************/
/***************************************************************************************************************************/
/***************************************************************************************************************************
GoIO_Sensor_DDSMem_* routines:

Each GoIO sensor object maintains a SensorDDSRecord of type GSensorDDSRec. The fields in this record describe the sensor.
The GoIO_Sensor_DDSMem_* routines allow an application program to access the SensorDDSRecord.
See GSensorDDSMem.h for the declaration of GSensorDDSRec.

Sensors come in 3 basic flavors:
1) Smart sensors,
2) AutoId capable sensors that are not smart, and
3) Legacy sensors.

Smart sensors store the entire 128 byte GSensorDDSRec record on the actual sensor hardware. This allows calibration
data that is specific to a given sensor to be stored on the sensor. Smart sensors report an AutoId number
in response to the SKIP_CMD_ID_GET_SENSOR_ID command that is >= kSensorIdNumber_FirstSmartSensor. 

If a sensor reports an AutoId >= kSensorIdNumber_FirstSmartSensor, then GoIO_Sensor_Open() will get the GSensorDDSRec 
info from the sensor hardware and store in the GoIO sensor object's SensorDDSRecord. 

Note that Go! Motion pretends to be smart: its id is > kSensorIdNumber_FirstSmartSensor, and GoIO_Sensor_Open() sets 
up its GSensorDDSRec info with calibrations for meters and feet. However, there is no way to store a modified
GSensorDDSRec record on the sensor permanently, which truly smart sensors do support.

AutoId capable sensors that are not smart report an AutoId value > 0 and < kSensorIdNumber_FirstSmartSensor.

Legacy (aka Dumb) sensors report an AutoId = 0.

Non smart sensors just get default values for most of the fields in the SensorDDSRecord. The default calibration 
parameters are configured so that GoIO_Sensor_CalibrateData() just reports volts.

For all 3 types of sensors, SensorDDSRecord.SensorNumber = AutoId after GoIO_Sensor_Open(). However, for
legacy sensors that means SensorDDSRecord.SensorNumber will be 0 which is not informative. In fact, Go! Link
reports an AutoID of 0 even if no sensor is plugged in!

All the GoIO_Sensor_DDSMem_* routines return 0 if successful, or a negative number if an error occurs. 
The GoIO_Sensor_DDSMem_* routines that do not take a timeout parameter will not fail if hSensor is valid and owned by
the calling thread.

****************************************************************************************************************************/
/***************************************************************************************************************************/
/***************************************************************************************************************************/

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_DDSMem_WriteRecord()
	
	Purpose:	Write the SensorDDSRecord into a smart sensor's DDS memory hardware. 
				Before the record is written to the sensor, SensorDDSRecord.Checksum is recalculated.
				Before the record is written to the sensor, a copy of the record is marshalled into little endian format,
				and then this copy is actually written to the sensor.

				This routine sends SKIP_CMD_ID_WRITE_LOCAL_NV_MEM_*(Go! Temp) or SKIP_CMD_ID_WRITE_REMOTE_NV_MEM_*(Go! Link)
				commands to the device. To alter DDS memory on the sensor, we recommend using 
				GoIO_Sensor_DDSMem_WriteRecord rather than the low level SKIP_CMD_ID_WRITE commands.

				WARNING: Be careful about using this routine. Changing a smart sensor's DDS memory can cause the sensor
				to stop working with Logger Pro.
	
	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_WriteRecord(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	gtype_int32 timeoutMs);//[in] # of milliseconds to wait for a reply before giving up. SKIP_TIMEOUT_MS_WRITE_DDSMEMBLOCK is recommended.


/***************************************************************************************************************************
	Function Name: GoIO_Sensor_DDSMem_ReadRecord()
	
	Purpose:	Read the SensorDDSRecord from a smart sensor's DDS memory hardware. 
				After the record is read from the sensor, it is unmarshalled from little endian format into processor
				specific format.

				This routine sends SKIP_CMD_ID_READ_LOCAL_NV_MEM(Go! Temp) or SKIP_CMD_ID_READ_REMOTE_NV_MEM(Go! Link)
				commands to the device. GoIO_Sensor_Open() automatically calls this routine, so you usually do not
				need to.
	
	Return:		0 if successful, 
				else if data validation fails then -2,
				else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_ReadRecord(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	gtype_int32 strictDDSValidationFlag,//[in] insist on exactly valid checksum if 1, else use a more lax validation test.
	gtype_int32 timeoutMs);//[in] # of milliseconds to wait for a reply before giving up. SKIP_TIMEOUT_MS_READ_DDSMEMBLOCK is recommended.
						   //Note that SKIP_TIMEOUT_MS_READ_DDSMEMBLOCK is much longer than the typical time required which is < 500 ms.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_DDSMem_SetRecord()
	
	Purpose:	Update the SensorDDSRecord. 
				This routine does not send data to the sensor's DDS memory hardware - use GoIO_Sensor_DDSMem_WriteRecord for that.

	Return:		0 if hSensor is valid, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetRecord(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	const GSensorDDSRec *pRec);	//[in] ptr to data to copy into the SensorDDSRecord.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_DDSMem_GetRecord()
	
	Purpose:	Get a copy of the SensorDDSRecord. 
				This routine does not read data from the sensor's DDS memory hardware - use GoIO_Sensor_DDSMem_ReadRecord for that.

	Return:		0 if hSensor is valid, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetRecord(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	GSensorDDSRec *pRec);		//[out] ptr to dest buf to copy the SensorDDSRecord into.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_DDSMem_CalculateChecksum()
	
	Return:		0 if hSensor is valid, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_CalculateChecksum(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	unsigned char *pChecksum);	//[out] ptr to checksum calculated by XOR'ing the first 127 bytes of the SensorDDSRecord.

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_DDSMem_ClearRecord()
	
	Purpose:	Clear the DDS record for the specified sensor. This puts default values in the record's fields.
				The sensor name fields are set to blank. The sensor id is set to 0, and a single linear calibration is
				set up with a gain of 1.0 and an offest of 0. The calibration units field is set to "volts".
				The OperationType is set to imply a probe type of kProbeTypeAnalog5V.

	Return:		0 if hSensor is valid, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_ClearRecord(
	GOIO_SENSOR_HANDLE hSensor);	//[in] handle to open sensor.

/***************************************************************************************************************************/
/***************************************************************************************************************************/
/***************************************************************************************************************************
Languages that can handle GSensorDDSRec can access the DDS fields using GoIO_Sensor_DDSMem_SetRecord and GetRecord.
Languages such as Basic are unlikely to be able to handle a complex record like GSensorDDSRec, especially because of
byte alignment issues. The individual field level API's defined below provide an alternate way to access the SensorDDSRecord.
****************************************************************************************************************************/
/***************************************************************************************************************************/
/***************************************************************************************************************************/

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetMemMapVersion(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char MemMapVersion);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetMemMapVersion(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pMemMapVersion);

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_DDSMem_SetSensorNumber()
	
	Purpose:	Normally SensorDDSRecord.SensorNumber is automatically set inside GoIO_Sensor_Open().
				However, if the current sensor is a legacy version, then SensorDDSRecord.SensorNumber = 0, so
				you might want to use GoIO_Sensor_DDSMem_SetSensorNumber() to change it.
				
	SIDE EFFECTS:	
				If the new SensorDDSRecord.SensorNumber is set to an (id < kSensorIdNumber_FirstSmartSensor) and (id > 0),
				then the rest of the fields in the DDS record are  populated with default values appropriate for the
				new sensor id.

				If the GoIO_Sensor_DDSMem_SetSensorNumber() causes the probeType to change, then you should
				send a SKIP_CMD_ID_SET_ANALOG_INPUT_CHANNEL command to the sensor. See GoIO_Sensor_GetProbeType().

	Return:		0 if hSensor is valid and sensor is connected to a Go! Link, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetSensorNumber(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	unsigned char SensorNumber);//[in]

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_DDSMem_GetSensorNumber()
	
	Purpose:	Retrieve SensorDDSRecord.SensorNumber. 
	
				If sendQueryToHardwareflag != 0, then send a SKIP_CMD_ID_GET_SENSOR_ID to the sensor hardware, and then call
				GoIO_Sensor_DDSMem_SetSensorNumber(new sensor id).
				SendQueryToHardwareflag is ignored for Go! Temp.

				If the sensor hardware reports a new SensorNumber, then a user has probably changed what sensor is
				plugged into a Go! Link. The simplest course of action in this case is to call
				GoIO_Sensor_GetOpenDeviceName() to get the device id, call GoIO_Sensor_Close(), and then
				call GoIO_Sensor_Open() with the values obtained from GoIO_Sensor_GetOpenDeviceName() to reopen the
				device. This will cause the GoIO_Sensor object to be properly configured for the new sensor.

	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetSensorNumber(
	GOIO_SENSOR_HANDLE hSensor,		//[in] handle to open sensor. 
	unsigned char *pSensorNumber,	//[out] ptr to SensorNumber.
	gtype_int32 sendQueryToHardwareflag,//[in] If sendQueryToHardwareflag != 0, then send a SKIP_CMD_ID_GET_SENSOR_ID to the sensor hardware. 
	gtype_int32 timeoutMs);//[in] # of milliseconds to wait for a reply before giving up. SKIP_TIMEOUT_MS_DEFAULT is recommended.

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetSerialNumber(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char leastSigByte_SerialNumber,
	unsigned char midSigByte_SerialNumber,
	unsigned char mostSigByte_SerialNumber);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetSerialNumber(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pLeastSigByte_SerialNumber,
	unsigned char *pMidSigByte_SerialNumber,
	unsigned char *pMostSigByte_SerialNumber);

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetLotCode(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char YY_BCD,
	unsigned char WW_BCD);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetLotCode(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pYY_BCD,
	unsigned char *pWW_BCD);

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetManufacturerID(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char ManufacturerID);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetManufacturerID(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pManufacturerID);

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetLongName(
	GOIO_SENSOR_HANDLE hSensor,
	const char *pLongName);//[in] ptr to NULL terminated string.
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetLongName(
	GOIO_SENSOR_HANDLE hSensor,			//[in] handle to open sensor. 
	char *pLongName,					//[out] ptr to buffer for NULL terminated output string.
	gtype_uint16 maxNumBytesToCopy);	//[in] size of pLongName buffer.

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetShortName(
	GOIO_SENSOR_HANDLE hSensor,
	const char *pShortName);//[in] ptr to NULL terminated string.
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetShortName(
	GOIO_SENSOR_HANDLE hSensor,			//[in] handle to open sensor. 
	char *pShortName,					//[out] ptr to buffer for NULL terminated output string. 
	gtype_uint16 maxNumBytesToCopy);	//[in] size of pShortName buffer.

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetUncertainty(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char Uncertainty);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetUncertainty(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pUncertainty);

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetSignificantFigures(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char SignificantFigures);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetSignificantFigures(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pSignificantFigures);

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetCurrentRequirement(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char CurrentRequirement);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetCurrentRequirement(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pCurrentRequirement);

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetAveraging(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char Averaging);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetAveraging(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pAveraging);

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetMinSamplePeriod(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_real32 MinSamplePeriod);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetMinSamplePeriod(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_real32 *pMinSamplePeriod);

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetTypSamplePeriod(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_real32 TypSamplePeriod);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetTypSamplePeriod(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_real32 *pTypSamplePeriod);

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetTypNumberofSamples(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_uint16 TypNumberofSamples);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetTypNumberofSamples(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_uint16 *pTypNumberofSamples);

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetWarmUpTime(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_uint16 WarmUpTime);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetWarmUpTime(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_uint16 *pWarmUpTime);

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetExperimentType(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char ExperimentType);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetExperimentType(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pExperimentType);

//GoIO_Sensor_DDSMem_SetOperationType() can change the probe type. See GoIO_Sensor_GetProbeType().
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetOperationType(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char OperationType);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetOperationType(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pOperationType);

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetCalibrationEquation(
	GOIO_SENSOR_HANDLE hSensor,
	char CalibrationEquation);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetCalibrationEquation(
	GOIO_SENSOR_HANDLE hSensor,
	char *pCalibrationEquation);

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetYminValue(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_real32 YminValue);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetYminValue(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_real32 *pYminValue);

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetYmaxValue(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_real32 YmaxValue);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetYmaxValue(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_real32 *pYmaxValue);

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetYscale(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char Yscale);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetYscale(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pYscale);

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetHighestValidCalPageIndex(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char HighestValidCalPageIndex);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetHighestValidCalPageIndex(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pHighestValidCalPageIndex);

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetActiveCalPage(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char ActiveCalPage);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetActiveCalPage(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pActiveCalPage);

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetCalPage(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char CalPageIndex,
	gtype_real32 CalibrationCoefficientA,
	gtype_real32 CalibrationCoefficientB,
	gtype_real32 CalibrationCoefficientC,
	const char *pUnits);//[in] ptr to NULL terminated string.
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetCalPage(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char CalPageIndex,
	gtype_real32 *pCalibrationCoefficientA,
	gtype_real32 *pCalibrationCoefficientB,
	gtype_real32 *pCalibrationCoefficientC,
	char *pUnits,						//[out] ptr to buffer for NULL terminated output string. 
	gtype_uint16 maxNumBytesToCopy);	//[in] size of pUnits buffer.

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetChecksum(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char Checksum);
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetChecksum(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pChecksum);

//Advanced API's:
GOIO_DLL_INTERFACE_DECL void GoIO_Diags_SetIOTraceEnableFlag(
	gtype_bool flag);

GOIO_DLL_INTERFACE_DECL void GoIO_Diags_GetIOTraceEnableFlag(
	gtype_bool *pFlag);			//[out] ptr to loc to store IO trace enable flag.

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Diags_GetNumInputTraceBytesAvailable(	//returns # of bytes available.
	GOIO_SENSOR_HANDLE hSensor);	//[in] handle to open sensor.

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Diags_ReadInputTraceBytes(				//returns # of bytes copied.
	GOIO_SENSOR_HANDLE hSensor,		//[in] handle to open sensor.
	unsigned char *pInputTraceBuf,	//[out] ptr to loc to store trace info.
	gtype_int32 maxCount);	//[in] maximum number of bytes to copy into pInputTraceBuf.

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Diags_GetNumOutputTraceBytesAvailable(	//returns # of bytes available.
	GOIO_SENSOR_HANDLE hSensor);	//[in] handle to open sensor.

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Diags_ReadOutputTraceBytes(			//returns # of bytes copied.
	GOIO_SENSOR_HANDLE hSensor,		//[in] handle to open sensor.
	unsigned char *pOutputTraceBuf,	//[out] ptr to loc to store trace info.
	gtype_int32 maxCount);	//[in] maximum number of bytes to copy into pOutputTraceBuf.

#endif //_GOIO_DLL_INTERFACE_H_
