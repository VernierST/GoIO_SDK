//
//  GraphView.m
//  GoIO_SDK
//
//  Created by swalton on 2/2/05.
//  Vernier Software & Technology, Inc
//

#import "GraphView.h"

@implementation GraphView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
	{
		yMin = 1.0;
		yMax = -1.0;
		measurementPeriod = 1000000.0;
		currentDevice = NULL;
		pvMeasurements = new std::vector<double>;
    }
    return self;
}

- (void)drawRect:(NSRect)rect
{
	// Calculate rects
	NSRect clientRect = NSMakeRect(0, 0, [self frame].size.width, [self frame].size.height);
	NSRect graphRect = clientRect;
	graphRect.origin.x += 100;
	graphRect.origin.y += 25;
	graphRect.size.width -= 125;
	graphRect.size.height -= 75;
	
	// Draw a background
	[[NSColor whiteColor] set];
	NSRectFill(clientRect);
	[[NSColor darkGrayColor] set];
	NSFrameRect(clientRect);
	
	// Frame the graph area
	NSBezierPath *tempBezier = [NSBezierPath bezierPath];
	[tempBezier moveToPoint:NSMakePoint(graphRect.origin.x, clientRect.origin.y)];
	[tempBezier lineToPoint:NSMakePoint(graphRect.origin.x, clientRect.origin.y + clientRect.size.height)];
	[tempBezier moveToPoint:NSMakePoint(graphRect.origin.x + graphRect.size.width, clientRect.origin.y)];
	[tempBezier lineToPoint:NSMakePoint(graphRect.origin.x + graphRect.size.width, clientRect.origin.y + clientRect.size.height)];
	[[NSColor blackColor] set];
	[tempBezier stroke];
	
	tempBezier = [NSBezierPath bezierPath];
	[tempBezier moveToPoint:NSMakePoint(clientRect.origin.x, graphRect.origin.y + graphRect.size.height)];
	[tempBezier lineToPoint:NSMakePoint(clientRect.origin.x + clientRect.size.width, graphRect.origin.y + graphRect.size.height)];
	[tempBezier moveToPoint:NSMakePoint(clientRect.origin.x, graphRect.origin.y)];
	[tempBezier lineToPoint:NSMakePoint(clientRect.origin.x + clientRect.size.width, graphRect.origin.y)];
	[[NSColor blueColor] set];
	[tempBezier stroke];
	
	if ((currentDevice != NULL) && !pvMeasurements->empty())
	{
		int meas_count_x_range, i;
		double meas_x_range, meas_x_min, meas_x_max;
		double meas_y_range, meas_y_min, meas_y_max, testy;
		float x, y, deltaX, deltaY;

		//Calculate full graph time range.
		int numMeasurements = pvMeasurements->size();
		if (numMeasurements <= 51)
			meas_count_x_range = 51;
		else if (numMeasurements <= 101)
			meas_count_x_range = 101;
		else if (numMeasurements <= 201)
			meas_count_x_range = 201;
		else if (numMeasurements <= 301)
			meas_count_x_range = 301;
		else if (numMeasurements <= 401)
			meas_count_x_range = 401;
		else
			meas_count_x_range = 501;
		meas_x_min = 0.0;
		meas_x_max = measurementPeriod * (meas_count_x_range - 1);
		meas_x_range = meas_x_max - meas_x_min;

		//Calculate full graph y range.
		meas_y_min = (*pvMeasurements)[0];
		meas_y_max = meas_y_min;
		for (i = 1; i < numMeasurements; i++)
		{
			testy = (*pvMeasurements)[i];
			if (testy < meas_y_min)
				meas_y_min = testy;
			if (testy > meas_y_max)
				meas_y_max = testy;
		}
		meas_y_range = meas_y_max - meas_y_min;

		//Make sure that meas_y_range corresponds to a voltage delta of at least 0.1 volts.
		double y_0_volts, y_dot1_volts, deltay_dot1_volts;
		y_0_volts = GoIO_Sensor_CalibrateData(currentDevice, 0.0);
		y_dot1_volts = GoIO_Sensor_CalibrateData(currentDevice, 0.1);
		deltay_dot1_volts = y_dot1_volts - y_0_volts;
		if (deltay_dot1_volts < 0.0)
			deltay_dot1_volts = -deltay_dot1_volts;
		if (deltay_dot1_volts > meas_y_range)
		{
			meas_y_range = deltay_dot1_volts;
			meas_y_max = meas_y_min + meas_y_range;
		}

		if (yMax > yMin)
		{
			if (yMax > meas_y_max)
				meas_y_max = yMax;
			if (yMin < meas_y_min)
				meas_y_min = yMin;
			meas_y_range = meas_y_max - meas_y_min;
		}
		yMin = meas_y_min;
		yMax = meas_y_max;
		
		// Draw data range labels for x-axis and y-axis
		NSNumberFormatter *number = [[[NSNumberFormatter alloc] init] autorelease];
		[number setFormat:@"0.000"];
		
		NSString *tempString;
		tempString = [NSString stringWithFormat:@"%@ secs", 
			[number stringForObjectValue:[NSNumber numberWithDouble:meas_x_min]]];
		[tempString drawAtPoint:NSMakePoint(NSMinX(graphRect) + 10, NSMaxY(graphRect) + 10) withAttributes:nil];

		tempString = [NSString stringWithFormat:@"%@ secs", 
			[number stringForObjectValue:[NSNumber numberWithDouble:meas_x_max]]];
		NSSize tempSize = [tempString sizeWithAttributes:nil];
		[tempString drawAtPoint:NSMakePoint(NSMaxX(graphRect) - tempSize.width - 10, NSMaxY(graphRect) + 10)
				 withAttributes:nil];

		float ftemp[3];
		char units[40];
		unsigned char calPageIndex;
		GoIO_Sensor_DDSMem_GetActiveCalPage(currentDevice, &calPageIndex);
		GoIO_Sensor_DDSMem_GetCalPage(currentDevice, calPageIndex,
			&ftemp[0], &ftemp[1], &ftemp[2], units, sizeof(units));

		[[NSColor blueColor] set];
		
		tempString = [NSString stringWithFormat:@"%@ %@", 
			[number stringForObjectValue:[NSNumber numberWithDouble:meas_y_min]],
			[NSString stringWithCString:units]];
		tempSize = [tempString sizeWithAttributes:nil];
		[tempString drawAtPoint:NSMakePoint(NSMinX(clientRect) + 10, NSMaxY(graphRect) - tempSize.height - 10) 
				 withAttributes:nil];

		tempString = [NSString stringWithFormat:@"%@ %@", 
			[number stringForObjectValue:[NSNumber numberWithDouble:meas_y_max]],
			[NSString stringWithCString:units]];
		tempSize = [tempString sizeWithAttributes:nil];
		[tempString drawAtPoint:NSMakePoint(NSMinX(clientRect) + 10, NSMinY(graphRect) + 10) 
				 withAttributes:nil];

		// Start drawing some data!
		[[NSColor redColor] set];
		tempBezier = [NSBezierPath bezierPath];
		
		double meas_x_frac, meas_y_frac;
		meas_y_frac = ((*pvMeasurements)[0] - meas_y_min)/meas_y_range;
		deltaY = (int) floor(meas_y_frac*graphRect.size.height + 0.5);
		deltaX = 0;

		x = NSMinX(graphRect) + deltaX;
		y = NSMaxY(graphRect) - deltaY;
		[tempBezier moveToPoint:NSMakePoint(x, y)];
		
		for (i = 0; i < numMeasurements; i++)
		{
			meas_y_frac = ((*pvMeasurements)[i] - meas_y_min)/meas_y_range;
			deltaY = (int) floor(meas_y_frac*graphRect.size.height + 0.5);
			meas_x_frac = i;
			meas_x_frac = meas_x_frac/(meas_count_x_range - 1);
			deltaX = (int) floor(meas_x_frac*graphRect.size.width + 0.5);
			x = NSMinX(graphRect) + deltaX;
			y = NSMaxY(graphRect) - deltaY;
			[tempBezier lineToPoint:NSMakePoint(x, y)];
		}
		
		[tempBezier stroke];
	}
}

- (BOOL) isFlipped
{
	return YES;
}

- (void) setGraphHistoryMin:(double)newYMin andMax:(double)newYMax
{
	yMin = newYMin;
	yMax = newYMax;
}

- (void) setMeasurementPeriod:(double)periodInSeconds
{
	measurementPeriod = periodInSeconds;
}

- (void) setCurrentDevice:(GOIO_SENSOR_HANDLE)newDevice
{
	currentDevice = newDevice;
}

- (void) addMeasurement:(double)newMeasurement
{
	pvMeasurements->push_back(newMeasurement);
}

- (void) clearMeasurements
{
	pvMeasurements->clear();
}

@end
