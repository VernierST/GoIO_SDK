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

using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace VSTCoreDefsdotNET
{
	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	public unsafe struct VSTDDSCalibrationPage
	{
		public const byte MAX_CALIBRATION_UNITS_CHARS_ON_SENSOR = 7;
		public float CalibrationCoefficientA;
		public float CalibrationCoefficientB;
		public float CalibrationCoefficientC;
		public fixed sbyte Units[7];
	}

	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	public unsafe struct VSTSensorDDSMemRec
	{
		public const byte kSensorIdNumber_Unknown = 0;
		public const byte kSensorIdNumber_Voltage10 = 2;
		public const byte kSensorIdNumber_FirstSmartSensor = 20;
		public const byte kSensorIdNumber_GoTemp = 60;
		public const byte kSensorIdNumber_GoMotion = 69;
		public const sbyte kEquationType_None = 0;
		/// <summary>
		/// linear, y = bx + a
		/// </summary>
		public const sbyte kEquationType_Linear = 1;
		/// <summary>
		/// y = cx^2 + bx + a
		/// </summary>
		public const sbyte kEquationType_Quadratic = 2;
		/// <summary>
		/// y = a * x^b
		/// </summary>
		public const sbyte kEquationType_Power = 3;
		/// <summary>
		/// y = a * b^x
		/// </summary>
		public const sbyte kEquationType_ModifiedPower = 4;
		/// <summary>
		/// y = a + b * ln(x)
		/// </summary>
		public const sbyte kEquationType_Logarithmic = 5;
		/// <summary>
		/// y = a + b * ln(1/x
		/// </summary>
		public const sbyte kEquationType_ModifiedLogarithmic = 6;
		/// <summary>
		/// y = a * e^(b * x)
		/// </summary>
		public const sbyte kEquationType_Exponential = 7;
		/// <summary>
		/// y = a * e^(b / x)
		/// </summary>
		public const sbyte kEquationType_ModifiedExponential = 8;
		/// <summary>
		/// y = a * x^(b * x)
		/// </summary>
		public const sbyte kEquationType_Geometric = 9;
		/// <summary>
		/// y = a * x^(b / x)
		/// </summary>
		public const sbyte kEquationType_ModifiedGeometric = 10;
		/// <summary>
		/// y = 1 / (a + b * ln(c * x))
		/// </summary>
		public const sbyte kEquationType_ReciprocalLog = 11;
		/// <summary>
		/// y = 1 / (a + b * ln(1000*x) + c * (ln(1000*x)^3) )
		/// </summary>
		public const sbyte kEquationType_SteinhartHart = 12;
		public const sbyte kEquationType_Motion = 13;
		public const sbyte kEquationType_Rotary = 14;
		public const sbyte kEquationType_HeatPulser = 15;
		public const sbyte kEquationType_DropCounter = 16;

		public const byte kProbeTypeNoProbe = 0;
		public const byte kProbeTypeTime = 1;
		public const byte kProbeTypeAnalog5V = 2;
		public const byte kProbeTypeAnalog10V = 3;
		public const byte kProbeTypeHeatPulser = 4;
		public const byte kProbeTypeAnalogOut = 5;
		public const byte kProbeTypeMD = 6;
		public const byte kProbeTypePhotoGate = 7;
		public const byte kProbeTypeDigitalCount = 10;
		public const byte kProbeTypeRotary = 11;
		public const byte kProbeTypeDigitalOut = 12;
		public const byte kProbeTypeLabquestAudio = 13;
		public const byte kNumProbeTypes = 14;

		public byte MemMapVersion;
		/// <summary>
		/// Identifies type of sensor; (SensorNumber >= 20) generally implies that
		/// SensorDDSRec is stored on the sensor hardware. Such sensors are called 'smart'.
		/// </summary>
		public byte SensorNumber;
		/// <summary>
		/// [0][1][2] - serial number as 3-byte integer, Little-Endian (LSB first).
		/// </summary>
		public fixed byte SensorSerialNumber[3];
		/// <summary>
		/// Lot code as 2-byte BCD date, [0] = YY, [1] == WW.
		/// </summary>
		public fixed byte SensorLotCode[2];
		public byte ManufacturerID;
		public fixed sbyte SensorLongName[20];
		public fixed sbyte SensorShortName[12];
		public byte Uncertainty;
		/// <summary>
        /// Ignore this field - it is only properly understood by legacy TI calculators.
        /// <para>
        /// GSensorDDSRec.SignificantFigures is not a reliable indicator of how many significant digits 
        /// of a measurement to display.
        /// </para>
        /// <para>
        /// An alternative criterion for determining this is the fact that for kProbeTypeAnalog5V
        /// sensors, the least significant bit of the analog to digital converter used to take measurements
        /// corresponds to a voltage measurement of 5.0/4096 = 0.00122 volts. Thus voltage measurements for 
        /// kProbeTypeAnalog5V sensors are accurate to within about 0.00122 volts.
        /// </para>
        /// <para>
        /// Note that almost all analog sensors are of type kProbeTypeAnalog5V.
        /// </para>
		/// </summary>
		public byte SignificantFigures;
		/// <summary>
		/// Number of mA(average) required to power sensor.
		/// </summary>
		public byte CurrentRequirement;
		public byte Averaging;
		/// <summary>
		/// seconds
		/// </summary>
		public float MinSamplePeriod;
		/// <summary>
		/// seconds
		/// </summary>
		public float TypSamplePeriod;
		public UInt16 TypNumberofSamples;
		/// <summary>
		/// Time (in seconds) required for the sensor to have power before reaching equilibrium.
		/// </summary>
		public UInt16 WarmUpTime;
		public byte ExperimentType;
		/// <summary>
		/// OperationType is used to infer probe type(analog 5 volt or 10 volt).
		/// </summary>
		public byte OperationType;
		/// <summary>
		/// kEquationType_...
		/// </summary>
		public byte CalibrationEquation;
		public float YminValue;
		public float YmaxValue;
		public byte Yscale;
		/// <summary>
		/// First index is 0. May range from 0 to 2.
		/// </summary>
		public byte HighestValidCalPageIndex;
		public byte ActiveCalPage;
		public VSTDDSCalibrationPage CalibrationPage0;
		public VSTDDSCalibrationPage CalibrationPage1;
		public VSTDDSCalibrationPage CalibrationPage2;
		/// <summary>
		/// Result of XORing bytes 0-126.
		/// </summary>
		public byte Checksum;
	}

}
