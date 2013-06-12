/* AppController */

#import <Cocoa/Cocoa.h>

#import "GoIO_DLL_Interface.h"

@class GraphView;
@interface AppController : NSObject
{
	IBOutlet NSButton *startButton;
	IBOutlet NSButton *stopButton;
	IBOutlet NSTextField *measurementField;
	IBOutlet NSTextField *unitsField;
		
	IBOutlet NSMenu *devicesMenu;
	IBOutlet NSMenu *calibrationsMenu;
	
	IBOutlet GraphView *graphView;
	IBOutlet NSWindow *mainWindow;
	
	GOIO_SENSOR_HANDLE currentDevice;
	BOOL collecting;
	NSTimer *readTimer;
}

- (IBAction) startCollection:(id)sender;
- (IBAction) stopCollection:(id)sender;
- (void) startTimer;
- (void) stopTimer;
- (void) readTimer:(id)sender;

- (IBAction) getStatus:(id)sender;
- (IBAction) getSensorID:(id)sender;
- (IBAction) setLED:(id)sender;
- (IBAction) setMeasurementPeriod:(id)sender;

- (void) updateUI:(NSNotification *)notification;
- (void) updateButtons:(NSNotification *)notification;
- (void) updateDeviceList:(NSNotification *)notification;

- (NSString *) getOpenDeviceName;
- (void) changeDevice:(id)sender;
- (void) openDevice:(NSString *)portName vendor:(int)vendorID product:(int)productID;
- (void) closeDevice;
- (BOOL) isDeviceOpen;

- (void) updateCalibrationList;
- (void) changeCalibration:(id)sender;

@end
