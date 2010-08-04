#ifndef _CALIBRATE_DATA_FUNCS_H_
#define _CALIBRATE_DATA_FUNCS_H_

#ifdef __cplusplus
extern "C" {
#endif

	double CalibrateData_Linear(
		double fRawVolts,
		double coeffA,
		double coeffB);

	double CalibrateData_SteinhartHart(
		double fRawVolts,
		double coeffA,
		double coeffB,
		double coeffC,
		double resistance,
		double maxVolts,
		char unit);

	double CalibrateData_Quadratic(
		double fRawVolts,
		double coeffA,
		double coeffB,
		double coeffC);

	double CalibrateData_ModifiedPower(
		double fRawVolts,
		double coeffA,
		double coeffB);

#ifdef __cplusplus
}
#endif

#endif

