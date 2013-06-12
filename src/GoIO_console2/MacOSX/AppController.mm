#include <sstream>

#import "AppController.h"
#import "GraphView.h"
#import "SetMeasurementPeriod.h"

@implementation AppController

- (id) init
{
	if (self = [super init])
	{
		GoIO_Init();
		currentDevice = NULL;
		readTimer = nil;
	}
	return self;
}

- (void) applicationWillTerminate:(NSNotification *)aNotification
{
	GoIO_Uninit();
}

- (void) awakeFromNib
{
	[measurementField setStringValue:@""];
	[unitsField setStringValue:@""];
	[self updateUI:nil];
}

- (BOOL)validateMenuItem:(id <NSMenuItem>)menuItem
{
	BOOL enabled = YES;
	if ([menuItem tag] == 1000)
		enabled = (currentDevice != NULL);
	if ([menuItem action] == @selector(startCollection:))
		enabled &= !collecting;
	else if ([menuItem action] == @selector(stopCollection:))
		enabled &= collecting;
	else if ([menuItem action] == @selector(setMeasurementPeriod:))
		enabled &= !collecting;
	else if ([menuItem action] == @selector(getSensorID:))
	{
		if (enabled)
		{
			char openDeviceName[GOIO_MAX_SIZE_DEVICE_NAME];
			int vendorId, productId;
			GoIO_Sensor_GetOpenDeviceName(currentDevice, openDeviceName, GOIO_MAX_SIZE_DEVICE_NAME, &vendorId, &productId);
			enabled &= (productId == SKIP_DEFAULT_PRODUCT_ID);
		}
	}
	
	return enabled;
}

- (IBAction) startCollection:(id)sender
{
	if (currentDevice != NULL)
	{
		GoIO_Sensor_ClearIO(currentDevice);
		GoIO_Sensor_SendCmdAndGetResponse(currentDevice, SKIP_CMD_ID_START_MEASUREMENTS, NULL, 0, NULL, NULL, 
										  SKIP_TIMEOUT_MS_DEFAULT);
		[self startTimer];

		[graphView clearMeasurements];
		[graphView setCurrentDevice:currentDevice];
		[graphView setGraphHistoryMin:1.0 andMax:-1.0];
		[self updateButtons:nil];
	}
}

- (IBAction) stopCollection:(id)sender
{
	if (currentDevice != NULL)
	{
		GoIO_Sensor_SendCmdAndGetResponse(currentDevice, SKIP_CMD_ID_STOP_MEASUREMENTS, NULL, 0, NULL, NULL, 
										  SKIP_TIMEOUT_MS_DEFAULT);
		[self stopTimer];
		[self updateButtons:nil];
		[measurementField setStringValue:@""]; //Clear latest measurement value.
	}
}

- (IBAction) getStatus:(id)sender
{
	if (currentDevice != NULL)
	{
		//Normally, you would do the following:
		GSkipGetStatusCmdResponsePayload statusRec;
		memset(&statusRec, 0, sizeof(statusRec)); //Do this because Go! Temp does not set the slave CPU fields.
		int nBytesRead = sizeof(statusRec);
		int nResult = GoIO_Sensor_SendCmdAndGetResponse(currentDevice, SKIP_CMD_ID_GET_STATUS, NULL, 0, &statusRec, 
														&nBytesRead, SKIP_TIMEOUT_MS_DEFAULT);
		if (0 == nResult)
		{
			int statusSummary = statusRec.status;
			double version = (statusRec.majorVersionMasterCPU/0x10)*10.0 + (statusRec.majorVersionMasterCPU & 0xf)*1.0 + 
				(statusRec.minorVersionMasterCPU/0x10)*0.1 + (statusRec.minorVersionMasterCPU & 0xf)*0.01 +
				(statusRec.majorVersionSlaveCPU/0x10)*0.001 + (statusRec.majorVersionSlaveCPU & 0xf)*0.0001 + 
				(statusRec.minorVersionSlaveCPU/0x10)*0.00001 + (statusRec.minorVersionSlaveCPU & 0xf)*0.000001; 
			
			NSString *message = [NSString stringWithFormat:@"Status byte = %d ; version = %.6f", statusSummary, version];
			NSRunInformationalAlertPanel(@"SKIP_CMD_ID_GET_STATUS", message, @"OK", nil, nil);
		}
	}	
}

- (IBAction) getSensorID:(id)sender
{
	if (currentDevice != NULL)
	{
		unsigned char oldSensorNumber, newSensorNumber;
		GoIO_Sensor_DDSMem_GetSensorNumber(currentDevice, &oldSensorNumber, 0, 0);
		
		//Actually query the hardware.
		if (0 != GoIO_Sensor_DDSMem_GetSensorNumber(currentDevice, &newSensorNumber, 1, SKIP_TIMEOUT_MS_DEFAULT))
			newSensorNumber = 0;
		
		if (oldSensorNumber != newSensorNumber)
		{
			//The user must have switched sensors. The easiest way to handle this is to close and then reopen the device.
			char openDeviceName[GOIO_MAX_SIZE_DEVICE_NAME];
			int vendorId, productId;
			GoIO_Sensor_GetOpenDeviceName(currentDevice, openDeviceName, GOIO_MAX_SIZE_DEVICE_NAME, &vendorId, &productId);
			
			[self closeDevice];
			[self openDevice:[NSString stringWithCString:openDeviceName] vendor:vendorId product:productId];
		}
	}	
}

- (IBAction) setLED:(id)sender
{
	if (currentDevice != NULL)
	{
		GSkipSetLedStateParams params;
		params.color = 0;
		params.brightness = SKIP_LED_BRIGHTNESS_MAX;
		if ([[(NSMenuItem *)sender title] isEqualToString:@"Off"])
		{
			params.color = SKIP_LED_COLOR_BLACK;
			params.brightness = 0;
		}
		else if ([[(NSMenuItem *)sender title] isEqualToString:@"Red"])
			params.color = SKIP_LED_COLOR_RED;
		else if ([[(NSMenuItem *)sender title] isEqualToString:@"Green"])
			params.color = SKIP_LED_COLOR_GREEN;
		else
		{
			params.color = kLEDOrange;
			params.brightness = kSkipOrangeLedBrightness;
		}
		GoIO_Sensor_SendCmdAndGetResponse(currentDevice, SKIP_CMD_ID_SET_LED_STATE, &params, sizeof(params), NULL, NULL, 
										  SKIP_TIMEOUT_MS_DEFAULT);
	}
}

- (IBAction) setMeasurementPeriod:(id)sender
{
	double currentMeasurementPeriod = 
		(unsigned int) floor(1000.0 * GoIO_Sensor_GetMeasurementPeriod(currentDevice, SKIP_TIMEOUT_MS_DEFAULT) + 0.5);
	double newMeasurementPeriod = [SetMeasurementPeriod showDialog:currentMeasurementPeriod];
	if (newMeasurementPeriod != currentMeasurementPeriod)
	{
		GoIO_Sensor_SetMeasurementPeriod(currentDevice, newMeasurementPeriod/1000.0, SKIP_TIMEOUT_MS_DEFAULT);
		[graphView setMeasurementPeriod:GoIO_Sensor_GetMeasurementPeriod(currentDevice, SKIP_TIMEOUT_MS_DEFAULT)];
	}
	[mainWindow makeKeyAndOrderFront:nil];
}

- (void) updateUI:(NSNotification *)notification
{
	[self updateButtons:nil];
	[self updateDeviceList:nil];
}

- (void) updateButtons:(NSNotification *)notification
{
	BOOL connected = [self isDeviceOpen];
	[startButton setEnabled:(connected && !collecting)];
	[stopButton setEnabled:(connected && collecting)];
}

- (void) updateDeviceList:(NSNotification *)notification
{
	// Clear out the current menu
	while ([devicesMenu numberOfItems] > 0)
		[devicesMenu removeItemAtIndex:0];
	
	// Get information about any currently open device
	NSString *openDeviceName = [self getOpenDeviceName];
	
	// Get all Skips and Jonahs
	int numSkips = GoIO_UpdateListOfAvailableDevices(VERNIER_DEFAULT_VENDOR_ID, SKIP_DEFAULT_PRODUCT_ID);
	if (numSkips > 0)
	{
		NSMenuItem *headerMenuItem = [[NSMenuItem alloc] initWithTitle:@"Go! Link Devices" action:nil keyEquivalent:@""];
		[headerMenuItem setEnabled:NO];
		[devicesMenu addItem:headerMenuItem];
		[headerMenuItem release];
	}
	
	for (int i = 0; i < numSkips; i++)
	{
		char newDeviceName[GOIO_MAX_SIZE_DEVICE_NAME];
		if (0 == GoIO_GetNthAvailableDeviceName(newDeviceName, GOIO_MAX_SIZE_DEVICE_NAME, VERNIER_DEFAULT_VENDOR_ID, 
												SKIP_DEFAULT_PRODUCT_ID, i))
		{
			NSString *deviceName = [NSString stringWithCString:newDeviceName];
			NSMenuItem *newMenuItem = [[NSMenuItem alloc] initWithTitle:deviceName action:@selector(changeDevice:) keyEquivalent:@""];
			[newMenuItem setIndentationLevel:1];
			[newMenuItem setTarget:self];
			[devicesMenu addItem:newMenuItem];
			if ([openDeviceName isEqualToString:deviceName])
				[newMenuItem setState:NSOnState];
		}
	}

	int numJonahs = GoIO_UpdateListOfAvailableDevices(VERNIER_DEFAULT_VENDOR_ID, USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID);
	if (numJonahs > 0)
	{
		NSMenuItem *headerMenuItem = [[NSMenuItem alloc] initWithTitle:@"Go! Temp Devices" action:nil keyEquivalent:@""];
		[headerMenuItem setEnabled:NO];
		[devicesMenu addItem:headerMenuItem];
	}
	for (int i = 0; i < numJonahs; i++)
	{
		char newDeviceName[GOIO_MAX_SIZE_DEVICE_NAME];
		if (0 == GoIO_GetNthAvailableDeviceName(newDeviceName, GOIO_MAX_SIZE_DEVICE_NAME, VERNIER_DEFAULT_VENDOR_ID, 
												USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID, i))
		{
			NSString *deviceName = [NSString stringWithCString:newDeviceName];
			NSMenuItem *newMenuItem = [[NSMenuItem alloc] initWithTitle:deviceName action:@selector(changeDevice:) keyEquivalent:@""];
			[newMenuItem setIndentationLevel:1];
			[newMenuItem setTarget:self];
			[newMenuItem setTag:1];
			[devicesMenu addItem:newMenuItem];
			if ([openDeviceName isEqualToString:deviceName])
				[newMenuItem setState:NSOnState];
		}
	}
	
	int numCyclops = GoIO_UpdateListOfAvailableDevices(VERNIER_DEFAULT_VENDOR_ID, CYCLOPS_DEFAULT_PRODUCT_ID);
	if (numCyclops > 0)
	{
		NSMenuItem *headerMenuItem = [[NSMenuItem alloc] initWithTitle:@"Go! Motion Devices" action:nil keyEquivalent:@""];
		[headerMenuItem setEnabled:NO];
		[devicesMenu addItem:headerMenuItem];
	}
	for (int i = 0; i < numCyclops; i++)
	{
		char newDeviceName[GOIO_MAX_SIZE_DEVICE_NAME];
		if (0 == GoIO_GetNthAvailableDeviceName(newDeviceName, GOIO_MAX_SIZE_DEVICE_NAME, VERNIER_DEFAULT_VENDOR_ID, 
												CYCLOPS_DEFAULT_PRODUCT_ID, i))
		{
			NSString *deviceName = [NSString stringWithCString:newDeviceName];
			NSMenuItem *newMenuItem = [[NSMenuItem alloc] initWithTitle:deviceName action:@selector(changeDevice:) keyEquivalent:@""];
			[newMenuItem setIndentationLevel:1];
			[newMenuItem setTarget:self];
			[newMenuItem setTag:2];
			[devicesMenu addItem:newMenuItem];
			if ([openDeviceName isEqualToString:deviceName])
				[newMenuItem setState:NSOnState];
		}
	}
}

- (NSString *) getOpenDeviceName
{
	NSString *result = nil;
	if (currentDevice != NULL)
	{
		char openDeviceName[GOIO_MAX_SIZE_DEVICE_NAME];
		int openVendorId, openProductId;
		GoIO_Sensor_GetOpenDeviceName(currentDevice, openDeviceName, GOIO_MAX_SIZE_DEVICE_NAME, &openVendorId, &openProductId);
		result = [NSString stringWithCString:openDeviceName];
	}
	return result;
}

- (void) changeDevice:(id)sender
{
	if ([(NSMenuItem *)sender state] == NSOnState)
		[self closeDevice];
	else
	{
		int deviceType;
		switch ((int)(NSMenuItem *)[sender tag])
		{
			case 0:
				deviceType = SKIP_DEFAULT_PRODUCT_ID;
				break;
			
			case 1:
				deviceType = USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID;
				break;
				
			case 2:
				deviceType = CYCLOPS_DEFAULT_PRODUCT_ID;
				break;
		}
		[self openDevice:[(NSMenuItem *)sender title] vendor:VERNIER_DEFAULT_VENDOR_ID product:deviceType];
	}
	[self updateUI:nil];
}

- (void) openDevice:(NSString *)portName vendor:(int)vendorID product:(int)productID
{
	if (currentDevice != NULL)
	{
		char openDeviceName[GOIO_MAX_SIZE_DEVICE_NAME];
		int openVendorId, openProductId;
		GoIO_Sensor_GetOpenDeviceName(currentDevice, openDeviceName, GOIO_MAX_SIZE_DEVICE_NAME, &openVendorId, &openProductId);
		if (![portName isEqualToString:[NSString stringWithCString:openDeviceName]])
			[self closeDevice];
	}

	if (currentDevice == NULL)
		currentDevice = GoIO_Sensor_Open([portName cString], vendorID, productID, 0);
		
	if (currentDevice != NULL)
	{
		unsigned char charId;
		GoIO_Sensor_DDSMem_GetSensorNumber(currentDevice, &charId, 0, 0);
		int id = charId;
		
		std::stringstream ss;
		ss << "Sensor ID = " << id;
		
		char tmpstring[GOIO_MAX_SIZE_DEVICE_NAME];
		GoIO_Sensor_DDSMem_GetLongName(currentDevice, tmpstring, sizeof(tmpstring));
		if (strlen(tmpstring) != 0)
		{
			std::string sensorName = tmpstring;
			ss << " ( " << sensorName << " ) ";
		}
		[mainWindow setTitle:[NSString stringWithFormat:@"%@:%s", @"Go! Console", ss.str().c_str()]];

		float ftemp[3];
		unsigned char calPageIndex;
		GoIO_Sensor_DDSMem_GetActiveCalPage(currentDevice, &calPageIndex);
		GoIO_Sensor_DDSMem_GetCalPage(currentDevice, calPageIndex,
			&ftemp[0], &ftemp[1], &ftemp[2], tmpstring, sizeof(tmpstring));
		[unitsField setStringValue:[NSString stringWithCString:tmpstring]];
		[graphView setMeasurementPeriod:GoIO_Sensor_GetMeasurementPeriod(currentDevice, SKIP_TIMEOUT_MS_DEFAULT)];
		
		[self updateCalibrationList];
	}
}

- (void) closeDevice
{
	if (currentDevice != NULL)
	{
		if (collecting)
			[self stopCollection:nil];
		GoIO_Sensor_Close(currentDevice);
	}
	currentDevice = NULL;
	
	[graphView clearMeasurements];
	
	[measurementField setStringValue:@""];
	[unitsField setStringValue:@""];
	
	[mainWindow setTitle:[NSString stringWithString:@"Go! Console"]];
}

- (BOOL) isDeviceOpen
{
	return (currentDevice != NULL);
}

- (void) startTimer
{
	readTimer = [[NSTimer scheduledTimerWithTimeInterval:0.01
					target:self selector:@selector(readTimer:) userInfo:nil repeats:YES] retain];
	collecting = TRUE;
}

- (void) stopTimer
{
	[readTimer invalidate];
	[readTimer release];
	readTimer = nil;
	collecting = FALSE;
}

- (void) readTimer:(id)sender
{
	if (currentDevice != NULL)
	{
		int measurements[150];
		int numMeasurementsAvailable = GoIO_Sensor_GetNumMeasurementsAvailable(currentDevice);
		if (numMeasurementsAvailable > 0)
		{
			//Note that the maxCount parameter to GoIO_Sensor_ReadRawMeasurements() is a multiple of 6.
			numMeasurementsAvailable = GoIO_Sensor_ReadRawMeasurements(currentDevice, measurements, 150);
		}

		if (numMeasurementsAvailable > 0)
		{
			//Stuff the new measurements in a circular buffer.
			double rVolts;
			double rMeasurement;
			for (int k = 0; k < numMeasurementsAvailable; k++)
			{
				rVolts = GoIO_Sensor_ConvertToVoltage(currentDevice, measurements[k]);
				rMeasurement = GoIO_Sensor_CalibrateData(currentDevice, rVolts);
				[graphView addMeasurement:rMeasurement];
			}
			[graphView setNeedsDisplay:YES];

			//Display latest measurement value on the toolbar.
			[measurementField setDoubleValue:rMeasurement];
		}
	}
}

- (void) updateCalibrationList
{
	while ([calibrationsMenu numberOfItems] > 0)
		[calibrationsMenu removeItemAtIndex:0];
	
	if (currentDevice != NULL)
	{
		unsigned char nActiveIndex;
		GoIO_Sensor_DDSMem_GetActiveCalPage(currentDevice, &nActiveIndex);
		unsigned char numCalibrations;
		GoIO_Sensor_DDSMem_GetHighestValidCalPageIndex(currentDevice, &numCalibrations);
		numCalibrations++;
		char units[30];
		gtype_real32 coeffs[3];
		for (unsigned int k = 0; k < numCalibrations; k++)
		{
			GoIO_Sensor_DDSMem_GetCalPage(currentDevice, k, &coeffs[0], &coeffs[1], &coeffs[2], units, sizeof(units));
			NSString *calName = [NSString stringWithFormat:@"Calib %d %s", k, units];
			NSMenuItem *newMenuItem = [[NSMenuItem alloc] initWithTitle:calName action:@selector(changeCalibration:) keyEquivalent:@""];
			[newMenuItem setTarget:self];
			[newMenuItem setTag:k];
			[calibrationsMenu addItem:newMenuItem];
			if (k == nActiveIndex)
				[newMenuItem setState:NSOnState];
		}
	}
}

- (void) changeCalibration:(id)sender
{
	int N = [sender tag];
	if (currentDevice != NULL)
	{
		unsigned char numCalibrations;
		GoIO_Sensor_DDSMem_GetHighestValidCalPageIndex(currentDevice, &numCalibrations);
		numCalibrations++;
		unsigned char oldN;
		GoIO_Sensor_DDSMem_GetActiveCalPage(currentDevice, &oldN);
		if ((N < numCalibrations) && (oldN != N))
		{
			char tmpstring[50];
			float ftemp[3];
			GoIO_Sensor_DDSMem_SetActiveCalPage(currentDevice, N);
			GoIO_Sensor_DDSMem_GetCalPage(currentDevice, N, &ftemp[0], &ftemp[1], &ftemp[2], tmpstring, sizeof(tmpstring));
			[unitsField setStringValue:[NSString stringWithCString:tmpstring]];

			[graphView clearMeasurements];
			[graphView setGraphHistoryMin:1.0 andMax:-1.0];
			[graphView setNeedsDisplay:YES];
		}
	}
	[self updateCalibrationList];
}

@end
