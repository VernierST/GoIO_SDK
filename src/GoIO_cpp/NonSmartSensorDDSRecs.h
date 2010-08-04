#ifndef _NON_SMART_SENSOR_DDS_RECS_H_
#define _NON_SMART_SENSOR_DDS_RECS_H_

#include "GSensorDDSMem.h"

#ifdef __cplusplus
extern "C" {
#endif

	extern GSensorDDSRec g_analogSensorDefaultDDSRecs[/* kSensorIdNumber_FirstSmartSensor */];
	extern GSensorDDSRec g_digitalSensorDefaultDDSRecs[/* kSensorIdNumber_FirstSmartSensor */];

	int InitSensorDefaultDDSRecs(void);

#ifdef __cplusplus
}
#endif

#endif

