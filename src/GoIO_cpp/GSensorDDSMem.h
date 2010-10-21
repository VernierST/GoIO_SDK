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
#ifndef _GSENSORDDSMEM_H_
#define _GSENSORDDSMEM_H_
// GSensorDDSMem.h
//
// Declare the structure of the memory record stored on a smart sensor.
//
// The fields in the structure that are declared as shorts and floats are stored on the sensor in little endian
// format. These same fields are typically manipulated on a host computer in platform specific endian format,
// eg. little endian for Wintel machines, and big endian for Macs, so marshalling is performed.
//
// Note that we are also assuming that the platform specific implementaton of float is 4 bytes long(we ASSERT this)
// and IEEE compliant.
//

typedef enum
{
	kSensorIdNumber_Unknown = 0,
	kSensorIdNumber_Voltage10 = 2,
	kSensorIdNumber_FirstSmartSensor = 20,
   	kSensorIdNumber_GoTemp  = 60 ,
   	kSensorIdNumber_GoMotion  = 69
} ESensorIdNumber;//This must be consistent with SensorMap.xml.

#if !defined VST_EEQUATIONTYPE // To avoid multiply defined problems GoIO/NGIO/LoggerPro
#define VST_EEQUATIONTYPE
typedef enum
{
	kEquationType_None = 0,
	kEquationType_Linear = 1, 					// linear, y = bx + a
	kEquationType_Quadratic = 2,				// y = cx^2 + bx + a
	kEquationType_Power = 3, 					// y = a * x^b
	kEquationType_ModifiedPower = 4, 			// y = a * b^x
	kEquationType_Logarithmic = 5, 				// y = a + b * ln(x)
	kEquationType_ModifiedLogarithmic = 6, 		// y = a + b * ln(1/x)
	kEquationType_Exponential = 7,				// y = a * e^(b * x)
	kEquationType_ModifiedExponential = 8,		// y = a * e^(b / x)
	kEquationType_Geometric = 9,				// y = a * x^(b * x)
	kEquationType_ModifiedGeometric = 10,		// y = a * x^(b / x)
	kEquationType_ReciprocalLog = 11,			// y = 1 / (a + b * ln(c * x))
	kEquationType_SteinhartHart = 12,			// y = 1 / (a + b * ln(1000*x) + c * (ln(1000*x)^3) )
	kEquationType_Motion = 13,
	kEquationType_Rotary = 14,
	kEquationType_HeatPulser = 15,
	kEquationType_DropCounter = 16
} EEquationType;
#endif

#if !defined VST_EPROBETYPE // To avoid multiply defined problems between GoIO/NGIO/LoggerPro 
#define VST_EPROBETYPE
typedef enum
{
	kProbeTypeNoProbe = 0,
	kProbeTypeTime,
	kProbeTypeAnalog5V = 2, //Skip supports this!
	kProbeTypeAnalog10V = 3,//Skip supports this!
	kProbeTypeHeatPulser,
	kProbeTypeAnalogOut,
	kProbeTypeMD = 6,
	kProbeTypePhotoGate = 7,
//	kProbeTypePhotoGatePulseWidth,  no longer allowed (never used)
//	kProbeTypePhotoGatePulsePeriod, no longer allowed (never used)
	kProbeTypeDigitalCount = 10,
	kProbeTypeRotary,
	kProbeTypeDigitalOut,
	kProbeTypeLabquestAudio,
	kNumProbeTypes
} EProbeType;
#endif

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

#define MAX_CALIBRATION_UNITS_CHARS_ON_SENSOR 7

struct tagGCalibrationPage
{
	float		CalibrationCoefficientA;
	float		CalibrationCoefficientB;
	float		CalibrationCoefficientC;
	char		Units[MAX_CALIBRATION_UNITS_CHARS_ON_SENSOR];
} _XPACK1;
typedef struct tagGCalibrationPage GCalibrationPage;

struct tagGSensorDDSRec
{
	unsigned char	MemMapVersion;
	unsigned char	SensorNumber;			//Identifies type of sensor; (SensorNumber >= 20) generally implies that
											//GSensorDDSRec is stored on the sensor hardware. Such sensors are called 'smart'.
	unsigned char	SensorSerialNumber[3];	//[0][1][2] - serial number as 3-byte integer, Little-Endian (LSB first).
	unsigned char	SensorLotCode[2];		//Lot code as 2-byte BCD date, [0] = YY, [1] == WW.
	unsigned char	ManufacturerID;
	char			SensorLongName[20];
	char			SensorShortName[12];
	unsigned char	Uncertainty;
	unsigned char	SignificantFigures;		//Ignore this field - it is only properly understood by legacy TI calculators.
											//GSensorDDSRec.SignificantFigures is not a reliable indicator of how many significant digits 
											//of a measurement to display.
											//An alternative criterion for determining this is the fact that for kProbeTypeAnalog5V
											//sensors, the least significant bit of the analog to digital converter used to take measurements
											//corresponds to a voltage measurement of 5.0/4096 = 0.00122 volts. Thus voltage measurements for 
											//kProbeTypeAnalog5V sensors are accurate to within about 0.00122 volts.
											//Note that almost all analog sensors are of type kProbeTypeAnalog5V.
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
	unsigned char	HighestValidCalPageIndex;//First index is 0.
	unsigned char	ActiveCalPage;
	GCalibrationPage	CalibrationPage[3];
	unsigned char	Checksum;				//Result of XORing bytes 0-126.
} _XPACK1;
typedef struct tagGSensorDDSRec GSensorDDSRec;

#if defined (TARGET_OS_WIN)
#pragma pack(pop)
#endif

#ifdef TARGET_OS_MAC
#pragma pack()
#endif

#endif

