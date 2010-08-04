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
