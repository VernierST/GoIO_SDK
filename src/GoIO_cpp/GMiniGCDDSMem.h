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
#ifndef _GMINIGCDDSMEM_H_
#define _GMINIGCDDSMEM_H_
// GMiniGCDDSMem.h
//
// Declare the GMiniGCDDSRec structure. This is the memory record stored on a Vernier Mini Gas Chromatograph that is readable 
// and writeable from a host computer. The GMiniGCDDSRec stucture is a variant of the GSensorDDSRec.
//
// The GMiniGCDDSRec is used as a shared memory communication area. Fields in the record can be modified by
// the host computer or by the GC. This means that some sort of locking mechanism is required to keep the record coherent.
// Once the host computer has written the first byte of the GMiniGCDDSRec, the GC should regard the GMiniGCDDSRec as
// incoherent until the last byte of the GMiniGCDDSRec(the checksum) has been written.
// Similarly, once the host computer has read the first byte of the GMiniGCDDSRec, the GC should not alter the contents of
// GMiniGCDDSRec until the last byte(the checksum) has been read. The host computer is required to read and write the
// entire GMiniGCDDSRec in discrete chunks. This is accomplished via GoIO_Sensor_DDSMem_ReadRecord() and
// GoIO_Sensor_DDSMem_WriteRecord().
// 
// Most of the fields in the GMiniGCDDSRec are written only by the host computer, the GC does not alter them. These
// fields should be read back by the host computer with same values last written by the computer.
//
// The fields in the structure that are declared as shorts and floats are stored on the chromatograph in little endian
// format. These same fields are typically manipulated on a host computer in platform specific endian format,
// eg. little endian for Wintel machines, and big endian for Power PC Macs, so marshalling is performed.
//
// Note that we are also assuming that the platform specific implementaton of float is 4 bytes long(we ASSERT this)
// and IEEE compliant.
//

//Get rid of POWERPC_MAC_BROKEN define after GC firmware is updated.
//#define POWERPC_MAC_BROKEN 1

#include "GSensorDDSMem.h"

#if defined (TARGET_OS_WIN)
#pragma pack(push)
#pragma pack(1)
#endif

#ifdef TARGET_OS_MAC
#pragma pack(1)
#endif

//This is unfortunate, but gcc 3.x does not support pragma pack(gcc 4.x does!).
//We are stuck with gcc 3.x for now, so we use _XPACK1 .
//Note that some docs on the web mentioned problems with using typedefs and
//__attribute__, so we are putting the typedef on a separate line.
#ifndef _XPACK1
#ifdef TARGET_OS_LINUX
#define _XPACK1 __attribute__((__packed__))
#else
#define _XPACK1
#endif
#endif

struct tagGMiniGCDDSRec
{
	unsigned char	MemMapVersion;			//Should be 2?	
	unsigned char	SensorNumber;			//Identifies type of sensor. For GC, this is 80.
	unsigned char	SensorSerialNumber[3];	//SensorSerialNumber[0] is lsbyte of int16 serial number.
											//SensorSerialNumber[1] is msbyte of int16 serial number.
											//SensorSerialNumber[2] contains 2 BCD digits for GC firmware version(written by GC).
	unsigned char	SensorLotCode[2];		//Lot code as 2-byte BCD date, [0] = YY, [1] == WW.
	unsigned char	ManufacturerID;
	char			SensorLongName[20];
	char			SensorShortName[12];
	unsigned char	Uncertainty;
	unsigned char	SignificantFigures;		//sig figs in high nibble, decimal precision in low nibble
	unsigned char	CurrentRequirement;		//Number of mA(average) required to power sensor.
	unsigned char	Averaging;
	float			MinSamplePeriod;		//seconds
	float			TypSamplePeriod;		//seconds
	unsigned short	TypNumberofSamples;
	unsigned short	WarmUpTime;				//Time (in seconds) required for the sensor to have power before reaching equilibrium.
	unsigned char	ExperimentType;
	unsigned char	OperationType;			//This is a LabPro specific field.
											//Go! devices use this field to infer probe type(5 volt or 10 volt). See EProbeType.
	char			CalibrationEquation;	//See EEquationType.
	float			YminValue;
	float			YmaxValue;
	unsigned char	Yscale;
	unsigned char	HighestValidCalPageIndex;//First index is 0. This field MUST contain 2.
	unsigned char	ActiveCalPage;

	float			CalibrationCoefficientA;
	float			CalibrationCoefficientB;
	float			ColumnTemperature;		//Present reading of column temperature in degrees C.(written by GC)
	char			Units[7];				//Unterminated ASCII string.

	float			StartTemperature;		//Ranges from 30 to 120 degrees C.
	float			HoldTime1;				//Ranges from 0 to 2700 seconds.
	float			RampRate;				//Ranges from 0 to 10 degrees C per minute.
	char			Status[7];				//NULL terminated ASCII string which is WRITTEN BY BOTH HOST AND GC! 
																	//See Status discussion below.
	float			FinalTemperature;		//Ranges from StartTemperature to 120 degrees C.
	float			HoldTime2;				//Ranges from 0 to 3600 seconds.
#ifdef POWERPC_MAC_BROKEN
	float			PressureReading;		//kPA(written by GC)
	float			PressureSetting;		//Pressure setting in kiloPascals(kPA).
#else
	float			PressureSetting;		//Pressure setting in kiloPascals(kPA).

	char			PressureReading[4];		//Pressure reading(written by GC) in kPA converted to 4 byte unterminated ASCII string.
											//This can be converted to a double by adding a terminating NULL and then calling
											//the C library function atof().
#endif
	char			Spare[3];				//These spare characters can be used by the host store additional info on the GC.

	unsigned char	Checksum;				//Result of XORing bytes 0-126.
} _XPACK1;
typedef struct tagGMiniGCDDSRec GMiniGCDDSRec;

#if defined (TARGET_OS_WIN)
#pragma pack(pop)
#endif

#ifdef TARGET_OS_MAC
#pragma pack()
#endif

//The GMiniGCDDSRec.Status field is used to issue commands from the host computer and to report state back from the gas
//chromatograph. So both the host computer and the GC write to this field.
//The values stored in the Status field are NULL terminated ASCII strings.
//
#define MINIGC_CMD_STOP "STOP"
#define MINIGC_CMD_GETRDY "GETRDY"
#define MINIGC_CMD_RUN "RUN"
//The host can write the following commands into Status: MINIGC_CMD_STOP, MINIGC_CMD_GETRDY, and MINIGC_CMD_RUN.
//MINIGC_CMD_STOP - stop the heater, the temperature profile and go back to "power up" state. Retain last profile stored. 
//	This can be used to turn the heater off even if the software is still connected. The "STOP" cmd can be issued at any time.
//MINIGC_CMD_GETRDY - This initiates the GC to turn the heater on and begin ramping to the start temperature stored in the profile.
//MINIGC_CMD_RUN - Begin running the profile. This will be used  to allow the host software to control the start of the temperature profile. 
//
#define MINIGC_STATE_STOP "STOP"
#define MINIGC_STATE_NOTRDY "NOTRDY"
#define MINIGC_STATE_READY "READY"
#define MINIGC_STATE_RUN "RUN"
#define MINIGC_STATE_ERR_PREFIX "ERR"
//The GC can report the following states in Status: MINIGC_STATE_STOP, MINIGC_STATE_NOTRDY", MINIGC_STATE_READY, MINIGC_STATE_RUN, and 
// and some error states that are prefixed with the ASCII string MINIGC_STATE_ERR_PREFIX.
//MINIGC_STATE_STOP - heater is off, unit is "power up" state. Last written profile is active.
//MINIGC_STATE_NOTRDY - GC is in GETRDY mode and heating to the start temperature in the profile but has not reached the start temperature.
//MINIGC_STATE_READY - GC is in GETRDY mode and has reached the start temperature.
//MINIGC_STATE_RUN - GC is running the profile.
//"ERR###" state - Error condition where ### represent three digits that we can assign to error conditions.
//	Clear the "ERR###" state by issuing a MINIGC_CMD_STOP or MINIGC_CMD_GETRDY command?????need to verify this.


//For languages that do not support packed 'C' structures directly, eg Basic, applications can access fields in the 
//GMiniGCDDSRec directly using the GoIO_Sensor_DDSMem_Set... and GoIO_Sensor_DDSMem_Get... functions.
//GMiniGCDDSRec is carefully laid out to match GSensorDDSRec so that all these functions work on both records.
//The new fields specific to the GMiniGCDDSRec correspond to fields in the GCalibrationPages, so they can all be
//accessed via the GoIO_Sensor_DDSMem_SetCalPage() and GoIO_Sensor_DDSMem_GetCalPage() functions.
//The new GMiniGCDDSRec fields map to the GSensorDDSRec fields as follows:
//	GMiniGCDDSRec.CalibrationCoefficientA maps to GSensorDDSRec.CalibrationPage[0].CalibrationCoefficientA.
//	GMiniGCDDSRec.CalibrationCoefficientB maps to GSensorDDSRec.CalibrationPage[0].CalibrationCoefficientB.
//	GMiniGCDDSRec.ColumnTemperature maps to GSensorDDSRec.CalibrationPage[0].CalibrationCoefficientC.
//	GMiniGCDDSRec.Units maps to GSensorDDSRec.CalibrationPage[0].Units.
//	GMiniGCDDSRec.StartTemperature maps to GSensorDDSRec.CalibrationPage[1].CalibrationCoefficientA.
//	GMiniGCDDSRec.HoldTime1 maps to GSensorDDSRec.CalibrationPage[1].CalibrationCoefficientB.
//	GMiniGCDDSRec.RampRate maps to GSensorDDSRec.CalibrationPage[1].CalibrationCoefficientC.
//	GMiniGCDDSRec.Status maps to GSensorDDSRec.CalibrationPage[1].Units.
//	GMiniGCDDSRec.FinalTemperature maps to GSensorDDSRec.CalibrationPage[2].CalibrationCoefficientA.
//	GMiniGCDDSRec.HoldTime2 maps to GSensorDDSRec.CalibrationPage[2].CalibrationCoefficientB.
//	GMiniGCDDSRec.PressureSetting maps to GSensorDDSRec.CalibrationPage[2].CalibrationCoefficientC.
//	GMiniGCDDSRec.PressureReading maps to GSensorDDSRec.CalibrationPage[2].Units[0..3].
//	GMiniGCDDSRec.Spare maps to GSensorDDSRec.CalibrationPage[2].Units[4..6].
//So, for example, you can modify StartTemperature, HoldTime1, RampRate, and Status with the following call:
//	GoIO_Sensor_DDSMem_SetCalPage(hSensor, 1, newStartTemperature, newHoldTime1, newRampRate, newStatus);

#endif

