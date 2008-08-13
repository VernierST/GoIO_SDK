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

#endif // _GSENSORDDSMEM_H_
