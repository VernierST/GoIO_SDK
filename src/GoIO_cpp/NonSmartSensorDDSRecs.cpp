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
#include "stdafx.h"

#include "GMBLSensor.h"
#include "NonSmartSensorDDSRecs.h"

#ifdef LIB_NAMESPACE
namespace LIB_NAMESPACE {
#endif

#ifdef TARGET_OS_WIN
#pragma warning(disable: 4996)
#pragma warning(disable: 4305)
#endif

GSensorDDSRec g_analogSensorDefaultDDSRecs[kSensorIdNumber_FirstSmartSensor];
GSensorDDSRec g_digitalSensorDefaultDDSRecs[kSensorIdNumber_FirstSmartSensor];

/*
Analog Signal Sensors (right hand BTC)
IDENT 	Sensor Type			Range					ID Num
Value
680		below this is no sensor						0
1K		Ex heart rate		N/A						13
1.5K	EKG 				N/A						15
2.2K	Thermocouple °C		-200°C to 1400°C		1		
3.3K	Resistance sensor	1KW to 100KW			4 deprecated
4.7K	TI Light sensor		0 to 1					12 
6.8K	Current sensor		-10 to +10 Amps			3
10K		Temp sensor °C		-25°C to 125°C			10
15K		Voltage sensor		-30 to +30 Volts		11
22K		Extra long temp°C	-50°C to 150°C			16
33K		Voltage sensor		-10 to +10 Volts		2
47K		Voltage sensor 		0 to 5 Volts			14
68K		CO2 gas sensor 		0 to 5000 ppm			17
100K	Oxygen gas sensor	0 to 27%				18
150K	Diff. Voltage Probe	-6 to +6 Volts			8
220K	Current Probe (DCP)	-0.6 to +0.6 Amps		9
301K	Not used as tolerance error could cause overlap
365K	Available
442K	Not used as tolerance error could cause overlap


Digital Signal Sensors (left hand BTC)
IDENT 	Sensor Type			Range					ID Num
Value			
10K		DCU											3
15K		Motion detector		½ meter to 6 meters		2
22K		Motion detector		½ meter to 6 meters		2
33K		Photogate			0 - 1					4
47K	 	Drop Counter?		0 - 1					5
68K		Rotary Motion	 	counts					6
100K	Radiation Monitor 	counts/interval			7
*/


// Sensor Record for AUTO-ID value 2.2k : Thermocouple Sensor
GSensorDDSRec Thermocouple_Sensor = 
{   // Auto-ID : 2.2k Thermocouple sensor
	1,								    // MemMapVersion
	1,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"Thermocouple",         			// SensorLongName
	"Temp",				    		    // SensorShortName
	0,							    	// Uncertainty
	0x22,							    // SigFigures
	1,							    	// CurrentRequirement
	-1,									// Averaging
	0.005,								// MinSamplePeriod
	0.5,								// TypSamplePeriod
	360,                                // TypNumberOfSamples
	0,                                  // WarmUpTime
	1,                                  // ExperimentType
	14,                                 // OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType)
	-200,                               // YminValue
	1400.0,                             // YmaxValue
	5,                                  // Yscale
	2,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { -27.6, 552.6, 0.0, "(C)" },     // CalibrationPage[0]
	  { -17.68, 1026.7, 0.0, "(F)" },   // CalibrationPage[1]
	  { 245.573, 552.6, 0.0, "(K)" }    // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for AUTO-ID value 33k : +/- 10V Voltage Sensor
GSensorDDSRec Voltage10_Sensor = 
{   // Auto-ID : 33k +/- 10v sensor
	1,								    // MemMapVersion
	2,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"Voltage (+/-10V)",    			    // SensorLongName
	"Voltage",				    		// SensorShortName
	0,							    	// Uncertainty
	0x45,							    // SigFigs
	1,							    	// CurrentRequirement
	-1,									// Averaging
	0.00002,                            // MinSamplePeriod
	0.1,                                // TypSamplePeriod
	150,                                // TypNumberOfSamples
	0,                                  // WarmUpTime
	1,                                  // ExperimentType
	2,									// OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType)
	-10.0,                              // YminValue
	10.0,                               // YmaxValue
	5,                                  // Yscale
	0,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 0.0, 1.0, 0.0, "(V)" },         // CalibrationPage[0]
	  { 0.0, 0.0, 0.0, "" },			// CalibrationPage[1]
	  { 0.0, 0.0, 0.0, "" }             // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for AUTO-ID value 15k : +/- 30V Voltage Sensor
GSensorDDSRec Voltage30_Sensor = 
{   // Auto-ID : 3.3k +/- 30v sensor
	1,								    // MemMapVersion
	11,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"Voltage (+/-30V)",    			    // SensorLongName
	"Voltage",				    		// SensorShortName
	0,							    	// Uncertainty
	0x45,							    // SigFigs
	1,							    	// CurrentRequirement
	-1,									// Averaging
	0.00002,                            // MinSamplePeriod
	0.1,                                // TypSamplePeriod
	150,                                // TypNumberOfSamples
	0,                                  // WarmUpTime
	1,                                  // ExperimentType
	2,									// OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType)
	-30.0,                              // YminValue
	30.0,                               // YmaxValue
	5,                                  // Yscale
	0,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 0.0, 3.0, 0.0, "(V)" },         // CalibrationPage[0]
	  { 0.0, 0.0, 0.0, "" },			// CalibrationPage[1]
	  { 0.0, 0.0, 0.0, "" }             // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for AUTO-ID value 6.8k : +/- 10A Current Sensor
GSensorDDSRec Current10_Sensor = 
{   // Auto-ID : 6.8k +/- 10A sensor
	1,								    // MemMapVersion
	3,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"Current Probe",    			    // SensorLongName
	"Current",				    		// SensorShortName
	0,							    	// Uncertainty
	0x45,							    // sigfigs
	-1,							    	// CurrentRequirement
	-1,                                 // Averaging
	-1.0,                               // MinSamplePeriod
	0.00002,                            // TypSamplePeriod
	180,                                // TypNumberOfSamples
	0,                                  // WarmUpTime
	1,                                  // ExperimentType
	14,                                  // OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType)
	-10.0,                              // YminValue
	10.0,                               // YmaxValue
	5,                                  // Yscale
	0,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 0.0, 1.0, 0.0, "(A)" },         // CalibrationPage[0]
	  { 0.0, 0.0, 0.0, "" },			// CalibrationPage[1]
	  { 0.0, 0.0, 0.0, "" }             // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for AUTO-ID value 150k : CV Voltage Sensor
GSensorDDSRec CV_Voltage_Sensor = 
{   // Auto-ID : 150k CV Voltage sensor
	1,								    // MemMapVersion
	8,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"Diff. Voltage",					// SensorLongName
	"Voltage",				    		// SensorShortName
	0,							    	// Uncertainty
	0x45,					            // sig figs
	7,							    	// CurrentRequirement
	-1,                                 // Averaging
	0.00002,                            // MinSamplePeriod
	0.1,                                // TypSamplePeriod
	180,                                // TypNumberOfSamples
	0,                                  // WarmUpTime
	1,                                  // ExperimentType
	14,                                 // OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType)
	-6.6,                               // YminValue
	6.6,                                // YmaxValue
	5,                                  // Yscale
	0,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 6.25, -2.5, 0.0, "(V)" },       // CalibrationPage[0]
	  { 0.0, 0.0, 0.0, "" },            // CalibrationPage[1]
	  { 0.0, 0.0, 0.0, "" }             // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for AUTO-ID value 220k : CV Current Sensor
GSensorDDSRec CV_Current_Sensor = 
{   // Auto-ID : 220k CV Current sensor
	1,								    // MemMapVersion
	9,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"Current",							// SensorLongName
	"Current",				    		// SensorShortName
	0,							    	// Uncertainty
	0x56,							    // sig figs
	7,							    	// CurrentRequirement
	-1,                                 // Averaging
	0.00002,                            // MinSamplePeriod
	0.1,                                // TypSamplePeriod
	180,                                // TypNumberOfSamples
	0,                                  // WarmUpTime
	1,                                  // ExperimentType
	14,                                 // OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType)
	-0.6,                               // YminValue
	0.6,                                // YmaxValue
	5,                                  // Yscale
	0,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 0.625, -0.25, 0.0, "(A)" },     // CalibrationPage[0]
	  { 0.0, 0.0, 0.0, "" },            // CalibrationPage[1]
	  { 0.0, 0.0, 0.0, "" }             // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for AUTO-ID value 10k : SS Temperature Sensor - degrees C (-25 to 125)
GSensorDDSRec TemperatureC_Sensor = 
{   // Auto-ID : 10k
	1,								    // MemMapVersion
	10,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"Stainless Temp",    	            // SensorLongName
	"Temp",				    		    // SensorShortName
	0,							    	// Uncertainty
	0x33,						        // Sig figs
	1,							    	// CurrentRequirement
	1,                                  // Averaging
	0.5,								// MinSamplePeriod
	0.5,                                // TypSamplePeriod
	360,                                // TypNumberOfSamples
	0,                                  // WarmUpTime
	1,                                  // ExperimentType
	14,                                 // OperationType       (EProbeType)
	kEquationType_SteinhartHart,        // CalibrationEquation (EEquationType)
	-20.0,								// YminValue
	125.0,								// YmaxValue
	10,                                 // Yscale
	0,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 1.02119e-3, 2.22468e-4, 1.33342e-7, "(C)" },  // CalibrationPage[0]
	  { 0.0, 0.0, 0.0, "" },            // CalibrationPage[1]
	  { 0.0, 0.0, 0.0, "" }             // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for AUTO-ID value 4.7k : Light Sensor - relative 0 to 1 (uW/cm^2)
GSensorDDSRec Light_Sensor = 
{   // Auto-ID : 4.7k
	1,								    // MemMapVersion
	12,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"TI Light Probe",    				// SensorLongName
	"Light",				    		// SensorShortName
	0,							    	// Uncertainty
	0x55,							    // SigFigs
	5,							    	// CurrentRequirement
	-1,                                 // Averaging
	0.001,								// MinSamplePeriod
	0.05,                               // TypSamplePeriod
	200,                                // TypNumberOfSamples
	0,                                  // WarmUpTime
	1,                                  // ExperimentType
	14,                                 // OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType)
	0.0,                                // YminValue
	1.0,                                // YmaxValue
	5,                                  // Yscale
	0,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 0.0006, 0.2, 0.0, "" },			// CalibrationPage[0]
	  { 0.0, 0.0, 0.0, "" },			// CalibrationPage[1]
	  { 0.0, 0.0, 0.0, "" }             // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for AUTO-ID value 1k : Ex Heart Rate Sensor
GSensorDDSRec ExHeartRate_Sensor = 
{   // Auto-ID : 1k Ex Heart Rate Sensor
	1,								    // MemMapVersion
	13,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"Heart Rate",         	            // SensorLongName
	"Heart Rt",				            // SensorShortName
	0,							    	// Uncertainty
	0x44,							    // SigFigs
	1,							    	// CurrentRequirement
	1,                                  // Averaging
	0.01,                               // MinSamplePeriod
	0.02,                               // TypSamplePeriod
	30000,                              // TypNumberOfSamples
	1,                                  // WarmUpTime
	1,                                  // ExperimentType
	14,                                 // OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType).
	0,									// YminValue
	3.0,								// YmaxValue
	5,                                  // Yscale
	0,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 0.0, 1.0, 0.0, "(v)" },			// CalibrationPage[0]
	  { 0.0, 0.0, 0.0, "" },            // CalibrationPage[1]
	  { 0.0, 0.0, 0.0, "" }             // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for AUTO-ID value 47k : 0-5V Voltage Sensor
GSensorDDSRec Voltage5_Sensor = 
{   // Auto-ID : 47k 0-5v sensor
	1,								    // MemMapVersion
	14,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"Raw Voltage (0-5V)",    			// SensorLongName
	"Voltage",				    		// SensorShortName
	1,							    	// Uncertainty
	0x66,							    // sig figs
	0x32,							    // CurrentRequirement
	1,                                  // Averaging
	0.00002,                            // MinSamplePeriod
	0.1,                                // TypSamplePeriod
	150,                                // TypNumberOfSamples
	0,                                  // WarmUpTime
	1,                                  // ExperimentType
	14,                                 // OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType)
	0.0,                                // YminValue
	5.0,                                // YmaxValue
	10,                                 // Yscale
	0,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 0.0, 1.0, 0.0, "(V)" },         // CalibrationPage[0]
	  { 0.0, 0.0, 0.0, "" },			// CalibrationPage[1]
	  { 0.0, 0.0, 0.0, "" }             // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for AUTO-ID value 1.5k : EKG Sensor
GSensorDDSRec EKG_Sensor = 
{   // Auto-ID : 1.5k EKG sensor
	1,								    // MemMapVersion
	15,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"EKG",         	  		            // SensorLongName
	"EKG",				    		    // SensorShortName
	0,							    	// Uncertainty
	0x55,							    // SigFigures
	-1,							    	// CurrentRequirement
	-1,                                 // Averaging
	0.005,                              // MinSamplePeriod
	0.01,                               // TypSamplePeriod
	200,                                // TypNumberOfSamples
	2,                                  // WarmUpTime
	1,                                  // ExperimentType
	14,                                 // OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType)
	-0.5,                               // YminValue
	4.0,                                // YmaxValue
	5,                                  // Yscale
	0,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 0.0, 1.0, 0.0, "(mV)" },		// CalibrationPage[0]
	  { 0.0, 0.0, 0.0, "" },            // CalibrationPage[1]
	  { 0.0, 0.0, 0.0, "" }             // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for AUTO-ID value 22k : ExtraLong Temp Sensor
GSensorDDSRec ExtraLong_Temp_Sensor = 
{   // Auto-ID : 22k
	1,								    // MemMapVersion
	16,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"Extra Long Temp",    				// SensorLongName
	"Temp",				    		    // SensorShortName
	0,							    	// Uncertainty
	0x33,						        // Sig figs
	7,							    	// CurrentRequirement
	-1,                                 // Averaging
	0.01,								// MinSamplePeriod
	0.5,                                // TypSamplePeriod
	360,                                // TypNumberOfSamples
	0,                                  // WarmUpTime
	1,                                  // ExperimentType
	14,                                 // OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType)
	-50.0,                              // YminValue
	150.0,                              // YmaxValue
	5,                                  // Yscale
	2,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { -53.073, 58.341, 0.0, "(C)" },  // CalibrationPage[0]
	  { -63.65, 105.15, 0.0, "(F)" },   // CalibrationPage[1]
	  { 220.1,   58.341, 0.0, "(K)" }   // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for AUTO-ID value 68k :CO2 Gas Sensor
GSensorDDSRec CO2_Gas_Sensor = 
{   // Auto-ID : 68k
	1,								    // MemMapVersion
	17,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"CO2 Gas",    	                    // SensorLongName
	"CO2 Gas",				            // SensorShortName
	0,							    	// Uncertainty
	0x12,						        // Sig figs
	47,							    	// CurrentRequirement
	-1,                                 // Averaging
	4.0,                                // MinSamplePeriod
	4.0,                                // TypSamplePeriod
	150,                                // TypNumberOfSamples
	90,                                 // WarmUpTime
	1,                                  // ExperimentType
	14,                                 // OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType)
	0.0,                                // YminValue
	5000.0,                             // YmaxValue
	5,                                  // Yscale
	2,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 0.0, 2000.0, 0.0, "(ppm)" },    // CalibrationPage[0]
	  { 0.0, 2.0, 0.0, "(ppt)" },       // CalibrationPage[1]
	  { 0.0, 0.2, 0.0, "(%)" }			// CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for AUTO-ID value 100k :O2 Gas Sensor
GSensorDDSRec O2_Gas_Sensor = 
{   // Auto-ID : 100k
	1,								    // MemMapVersion
	18,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"Oxygen Gas",    	                // SensorLongName
	"O2 Gas",				            // SensorShortName
	0,							    	// Uncertainty
	0x34,						        // Sig figs
	2,							    	// CurrentRequirement
	-1,                                 // Averaging
	1.0,                                // MinSamplePeriod
	1.0,								// TypSamplePeriod
	300,								// TypNumberOfSamples
	2,                                  // WarmUpTime
	1,                                  // ExperimentType
	14,                                 // OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType)
	15.0,                               // YminValue
	25.0,                               // YmaxValue
	5,                                  // Yscale
	2,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 0.0, 6.769, 0.0, "(%)" },       // CalibrationPage[0]
	  { 0.0, 67.69, 0.0, "(ppt)" },     // CalibrationPage[1]
	  { 0.0, 676.9, 0.0, "(ppm)" }      // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for AUTO-ID value 22k : Motion Detector - Meters
GSensorDDSRec Motion_Sensor = 
{   // Auto-ID : 22k Motion Detector (m) sensor
	1,								    // MemMapVersion
	2,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"Motion Detector",					// SensorLongName
	"Pos",				                // SensorShortName
	1,							    	// Uncertainty
	0x22,							    // SigFigs
	50,							    	// CurrentRequirement
	1,                                  // Averaging
	0.02,								// MinSamplePeriod
	0.05,                               // TypSamplePeriod
	100,                                // TypNumberOfSamples
	0,                                  // WarmUpTime
	1,                                  // ExperimentType
	kProbeTypeMD,                       // OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType).
	0.0,                                // YminValue
	5.0,                                // YmaxValue
	1,                                  // Yscale
	0,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 0.0, 1.0, 0.0, "" },			// CalibrationPage[0]
	  { 0.0, 0.0, 0.0, "" },			// CalibrationPage[1]
	  { 0.0, 0.0, 0.0, "" }             // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for AUTO-ID value 10k : DCU
GSensorDDSRec DCU_Sensor = 
{   // Auto-ID : 10k 
	1,								    // MemMapVersion
	3,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"DCU",								// SensorLongName
	"DCU",								// SensorShortName
	0,							    	// Uncertainty
	0x55,							    // SigFigs
	40,							    	// CurrentRequirement
	-1,                                 // Averaging
	0.001,                              // MinSamplePeriod
	1.00,                               // TypSamplePeriod
	500,                                // TypNumberOfSamples
	0,                                  // WarmUpTime
	1,                                  // ExperimentType
	kProbeTypeDigitalOut,				// OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType).
	0.0,                                // YminValue
	525.0,                              // YmaxValue
	25,                                 // Yscale
	0,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 0.0, 1.0, 0.0, "" },            // CalibrationPage[0]
	  { 0.0, 0.0, 0.0, "" },            // CalibrationPage[1]
	  { 0.0, 0.0, 0.0, "" }             // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for AUTO-ID value 33k : Photogate Sensor
GSensorDDSRec Photogate_Sensor = 
{   // Auto-ID : 33k Photogate (x) sensor
	1,								    // MemMapVersion
	4,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"Photogate",                        // SensorLongName
	"Photogate",				        // SensorShortName
	0,							    	// Uncertainty
	0x22,							    // SigFigs
	30,							    	// CurrentRequirement
	-1,                                 // Averaging
	0.0001,                             // MinSamplePeriod
	0.1,								// TypSamplePeriod
	180,                                // TypNumberOfSamples
	0,                                  // WarmUpTime
	1,                                  // ExperimentType
	kProbeTypePhotoGate,                // OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType).
	0.0,                                // YminValue
	1.0,								// YmaxValue
	25,                                 // Yscale
	0,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 0.0, 1.0, 0.0, "" },            // CalibrationPage[0]
	  { 0.0, 0.0, 0.0, "" },            // CalibrationPage[1]
	  { 0.0, 0.0, 0.0, "" }             // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

GSensorDDSRec Drop_Counter = 
{   //
	1,								    // MemMapVersion
	5,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"Drop Counter",                     // SensorLongName
	"Drop",				                // SensorShortName
	0,							    	// Uncertainty
	0x22,							    // SigFigs
	40,							    	// CurrentRequirement
	-1,                                 // Averaging
	0.0001,                             // MinSamplePeriod
	1.00,                               // TypSamplePeriod
	1800,                               // TypNumberOfSamples
	0,                                  // WarmUpTime
	1,                                  // ExperimentType
	kProbeTypeDigitalCount,             // OperationType       (EProbeType)
	kEquationType_Linear,				// CalibrationEquation = none (EEquationType).
	0.0,                                // YminValue
	1000.0,                             // YmaxValue
	10,                                 // Yscale
	0,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 0.0, 1.0, 0.0, "" },            // CalibrationPage[0]
	  { 0.0, 0.0, 0.0, "" },            // CalibrationPage[1]
	  { 0.0, 0.0, 0.0, "" }             // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for digital AUTO-ID value 68k : Rotary Motion Sensor
GSensorDDSRec RotaryMotion_Sensor = 
{   // Auto-ID : 68k Rotary Motion sensor
	1,								    // MemMapVersion
	6,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"Rotary Motion",                    // SensorLongName
	"Pos",								// SensorShortName
	0,							    	// Uncertainty
	0x22,							    // SigFigs
	21,							    	// CurrentRequirement
	-1,                                 // Averaging
	0.01,								// MinSamplePeriod
	0.05,								// TypSamplePeriod
	200,                                // TypNumberOfSamples
	0,                                  // WarmUpTime
	1,                                  // ExperimentType
	kProbeTypeRotary,                   // OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType).
	-1000.0,                            // YminValue
	1000.0,                             // YmaxValue
	10,                                 // Yscale
	1,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 0.0, 1.0, 0.0, "(deg)" },       // CalibrationPage[0]
	  { 0.0, 0.25, 0.0, "(deg)" },      // CalibrationPage[1]
	  { 0.0, 0.0, 0.0, "" }             // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

GSensorDDSRec Radiation_Sensor = 
{   //
	1,								    // MemMapVersion
	7,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"Radiation",                        // SensorLongName
	"Rad",				                // SensorShortName
	0,							    	// Uncertainty
	0x22,							    // SigFigs
	0,							    	// CurrentRequirement
	-1,                                 // Averaging
	0.01,								// MinSamplePeriod
	10.0,								// TypSamplePeriod
	60,									// TypNumberOfSamples
	0,                                  // WarmUpTime
	1,                                  // ExperimentType
	kProbeTypeDigitalCount,             // OperationType       (EProbeType)
	kEquationType_Linear,                 // CalibrationEquation = none (EEquationType).
	0.0,                                // YminValue
	1000.0,                             // YmaxValue
	10,                                 // Yscale
	0,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 0.0, 1.0, 0.0, "" },            // CalibrationPage[0]
	  { 0.0, 0.0, 0.0, "" },            // CalibrationPage[1]
	  { 0.0, 0.0, 0.0, "" }             // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for digital AUTO-ID value 6.8k : Linear Position Sensor
GSensorDDSRec LinearPosition_Sensor = 
{  
	1,								    // MemMapVersion
	8,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"Linear Position",                    // SensorLongName
	"Pos",								// SensorShortName
	0,							    	// Uncertainty
	0x22,							    // SigFigs
	21,							    	// CurrentRequirement
	-1,                                 // Averaging
	0.01,								// MinSamplePeriod
	0.05,								// TypSamplePeriod
	200,                                // TypNumberOfSamples
	0,                                  // WarmUpTime
	1,                                  // ExperimentType
	kProbeTypeRotary,                   // OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType).
	-1000.0,                            // YminValue
	1000.0,                             // YmaxValue
	10,                                 // Yscale
	1,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 0.0, (float) (1.0/150.0), 0.0, "(in)" },      // CalibrationPage[0]
	  { 0.0, (float) (1.0/600.0), 0.0, "(in)" },      // CalibrationPage[1]
	  { 0.0, 0.0, 0.0, "" }             // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

// Sensor Record for digital AUTO-ID value 4.7k : Projectile Launcher
GSensorDDSRec Projectile_Launcher = 
{
	1,								    // MemMapVersion
	9,									// SensorNumber
	{ 0, 0, 0 },                        // SensorSerialNumber[3]				
	{ 0, 0 },						    // SensorLotCode[2]
	0,				       				// ManufacturerID
	"Projectile Launcher",				// SensorLongName
	"Launcher",							// SensorShortName
	0,							    	// Uncertainty
	0x22,							    // SigFigs
	30,							    	// CurrentRequirement
	-1,                                 // Averaging
	0.0001,                             // MinSamplePeriod
	0.1,								// TypSamplePeriod
	180,                                // TypNumberOfSamples
	0,                                  // WarmUpTime
	1,                                  // ExperimentType
	kProbeTypePhotoGate,                // OperationType       (EProbeType)
	kEquationType_Linear,               // CalibrationEquation (EEquationType).
	0.0,                                // YminValue
	1.0,								// YmaxValue
	25,                                 // Yscale
	0,                                  // HighestValidCalPageIndex
	0,                                  // ActiveCalPage
	{ { 0.0, 1.0, 0.0, "" },            // CalibrationPage[0]
	  { 0.0, 0.0, 0.0, "" },            // CalibrationPage[1]
	  { 0.0, 0.0, 0.0, "" }             // CalibrationPage[2]
	},
	0                                   // Checksum (0=not calculated.)
};

int InitSensorDefaultDDSRecs(void)
{
	memset(g_analogSensorDefaultDDSRecs, 0, sizeof(g_analogSensorDefaultDDSRecs));
	memset(g_digitalSensorDefaultDDSRecs, 0, sizeof(g_digitalSensorDefaultDDSRecs));

	g_analogSensorDefaultDDSRecs[1] = Thermocouple_Sensor;
	g_analogSensorDefaultDDSRecs[1].Checksum = GMBLSensor::CalculateDDSDataChecksum(Thermocouple_Sensor);

	g_analogSensorDefaultDDSRecs[2] = Voltage10_Sensor;
	g_analogSensorDefaultDDSRecs[2].Checksum = GMBLSensor::CalculateDDSDataChecksum(Voltage10_Sensor);

	g_analogSensorDefaultDDSRecs[3] = Current10_Sensor;
	g_analogSensorDefaultDDSRecs[3].Checksum = GMBLSensor::CalculateDDSDataChecksum(Current10_Sensor);

	g_analogSensorDefaultDDSRecs[8] = CV_Voltage_Sensor;
	g_analogSensorDefaultDDSRecs[8].Checksum = GMBLSensor::CalculateDDSDataChecksum(CV_Voltage_Sensor);

	g_analogSensorDefaultDDSRecs[9] = CV_Current_Sensor;
	g_analogSensorDefaultDDSRecs[9].Checksum = GMBLSensor::CalculateDDSDataChecksum(CV_Current_Sensor);

	g_analogSensorDefaultDDSRecs[10] = TemperatureC_Sensor;
	g_analogSensorDefaultDDSRecs[10].Checksum = GMBLSensor::CalculateDDSDataChecksum(TemperatureC_Sensor);

	g_analogSensorDefaultDDSRecs[11] = Voltage30_Sensor;
	g_analogSensorDefaultDDSRecs[11].Checksum = GMBLSensor::CalculateDDSDataChecksum(Voltage30_Sensor);

	g_analogSensorDefaultDDSRecs[12] = Light_Sensor;
	g_analogSensorDefaultDDSRecs[12].Checksum = GMBLSensor::CalculateDDSDataChecksum(Light_Sensor);

	g_analogSensorDefaultDDSRecs[13] = ExHeartRate_Sensor;
	g_analogSensorDefaultDDSRecs[13].Checksum = GMBLSensor::CalculateDDSDataChecksum(ExHeartRate_Sensor);

	g_analogSensorDefaultDDSRecs[14] = Voltage5_Sensor;
	g_analogSensorDefaultDDSRecs[14].Checksum = GMBLSensor::CalculateDDSDataChecksum(Voltage5_Sensor);

	g_analogSensorDefaultDDSRecs[15] = EKG_Sensor;
	g_analogSensorDefaultDDSRecs[15].Checksum = GMBLSensor::CalculateDDSDataChecksum(EKG_Sensor);

	g_analogSensorDefaultDDSRecs[16] = ExtraLong_Temp_Sensor;
	g_analogSensorDefaultDDSRecs[16].Checksum = GMBLSensor::CalculateDDSDataChecksum(ExtraLong_Temp_Sensor);

	g_analogSensorDefaultDDSRecs[17] = CO2_Gas_Sensor;
	g_analogSensorDefaultDDSRecs[17].Checksum = GMBLSensor::CalculateDDSDataChecksum(CO2_Gas_Sensor);

	g_analogSensorDefaultDDSRecs[18] = O2_Gas_Sensor;
	g_analogSensorDefaultDDSRecs[18].Checksum = GMBLSensor::CalculateDDSDataChecksum(O2_Gas_Sensor);

	g_digitalSensorDefaultDDSRecs[2] = Motion_Sensor;
	g_digitalSensorDefaultDDSRecs[2].Checksum = GMBLSensor::CalculateDDSDataChecksum(Motion_Sensor);

	g_digitalSensorDefaultDDSRecs[3] = DCU_Sensor;
	g_digitalSensorDefaultDDSRecs[3].Checksum = GMBLSensor::CalculateDDSDataChecksum(DCU_Sensor);

	g_digitalSensorDefaultDDSRecs[4] = Photogate_Sensor;
	g_digitalSensorDefaultDDSRecs[4].Checksum = GMBLSensor::CalculateDDSDataChecksum(Photogate_Sensor);

	g_digitalSensorDefaultDDSRecs[5] = Drop_Counter;
	g_digitalSensorDefaultDDSRecs[5].Checksum = GMBLSensor::CalculateDDSDataChecksum(Drop_Counter);

	g_digitalSensorDefaultDDSRecs[6] = RotaryMotion_Sensor;
	g_digitalSensorDefaultDDSRecs[6].Checksum = GMBLSensor::CalculateDDSDataChecksum(RotaryMotion_Sensor);

	g_digitalSensorDefaultDDSRecs[7] = Radiation_Sensor;
	g_digitalSensorDefaultDDSRecs[7].Checksum = GMBLSensor::CalculateDDSDataChecksum(Radiation_Sensor);

	g_digitalSensorDefaultDDSRecs[8] = LinearPosition_Sensor;
	g_digitalSensorDefaultDDSRecs[8].Checksum = GMBLSensor::CalculateDDSDataChecksum(LinearPosition_Sensor);

	g_digitalSensorDefaultDDSRecs[9] = Projectile_Launcher;
	g_digitalSensorDefaultDDSRecs[9].Checksum = GMBLSensor::CalculateDDSDataChecksum(Projectile_Launcher);

	return 0;
}

#ifdef LIB_NAMESPACE
}
#endif

