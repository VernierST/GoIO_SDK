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
#include <math.h>

#include "GCalibrateDataFuncs.h"

double CalibrateData_Linear(
	double fRawVolts,
	double coeffA,
	double coeffB)
{
	double fCalibratedMeasurement = coeffB*fRawVolts + coeffA;

	return fCalibratedMeasurement;
}

double CalibrateData_SteinhartHart(
	double fRawVolts,
	double coeffA,
	double coeffB,
	double coeffC,
	double resistance,
	double maxVolts,
	char unit)
{
	double fCalibratedMeasurement = 0.0;
	double adjMaxVolts = 0.999*maxVolts;
	double adjMinVolts = 0.001*maxVolts;

	if (fRawVolts > adjMaxVolts)
		fRawVolts = adjMaxVolts;
	else if (fRawVolts < adjMinVolts)
		fRawVolts = adjMinVolts;

	fCalibratedMeasurement = (fRawVolts*resistance)/(maxVolts - fRawVolts); // Convert from volts to resistance

	// Now do the Steinhart-Hart equation
	fCalibratedMeasurement = 1.0/(coeffA + (coeffB*log(fCalibratedMeasurement)) + (coeffC*pow(log(fCalibratedMeasurement), 3.0)));

	// fCalibratedMeasurement is now in degrees Kelvin.  Adjust for degrees centigrade &/or Fahrenheit

	if (('c' == unit) || ('C' == unit))
		fCalibratedMeasurement -= 273.15;
	else if (('f' == unit) || ('F' == unit))
	{
		fCalibratedMeasurement -= 273.15;
		fCalibratedMeasurement = (9.0/5.0)*fCalibratedMeasurement + 32.0;
	}

	return fCalibratedMeasurement;
}

double CalibrateData_Quadratic(
	double fRawVolts,
	double coeffA,
	double coeffB,
	double coeffC)
{
	double fCalibratedMeasurement = coeffC*fRawVolts*fRawVolts + coeffB*fRawVolts + coeffA;
	return fCalibratedMeasurement;
}

double CalibrateData_ModifiedPower(
	double fRawVolts,
	double coeffA,
	double coeffB)
{
	double fCalibratedMeasurement = coeffA*pow(coeffB, fRawVolts);
	return fCalibratedMeasurement;
}
