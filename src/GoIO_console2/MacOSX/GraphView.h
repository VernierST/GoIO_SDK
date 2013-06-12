//
//  GraphView.h
//  GoIO_SDK
//
//  Created by swalton on 2/2/05.
//  Vernier Software & Technology, Inc.
//

#include <vector>
#include "GoIO_DLL_interface.h"

#import <Cocoa/Cocoa.h>

@interface GraphView : NSView
{
	double yMin, yMax;
	double measurementPeriod; // in seconds
	std::vector<double> *pvMeasurements;
	GOIO_SENSOR_HANDLE currentDevice;
}

- (void) setGraphHistoryMin:(double)newYMin andMax:(double)newYMax;
- (void) setMeasurementPeriod:(double)periodInSeconds;
- (void) setCurrentDevice:(GOIO_SENSOR_HANDLE)newDevice;

- (void) addMeasurement:(double)newMeasurement;
- (void) clearMeasurements;

@end
