//
//  SetMeasurementPeriod.m
//  GoIO_SDK
//
//  Created by swalton on 2/5/05.
//  Vernier Software & Technology, Inc.
//

#import "SetMeasurementPeriod.h"

@implementation SetMeasurementPeriod

+ (double) showDialog:(double)currentPeriod
{
	SetMeasurementPeriod *dialog = [[SetMeasurementPeriod alloc] init];
	currentPeriod = [dialog runDialog:(double)currentPeriod];
	[dialog release];
	return currentPeriod;
}

- (id) init
{
	return [super initWithWindowNibName:@"SetMeasurementPeriod"];
}

- (double) runDialog:(double)currentPeriod
{
	[self window]; // Force dialog to load
	[measurementPeriodField setDoubleValue:currentPeriod];
	int result = [NSApp runModalForWindow:[self window]];
	if (result == NSOKButton)
		currentPeriod = [measurementPeriodField doubleValue];
	return currentPeriod;
}

- (void) finishDialog:(id)sender
{
	[NSApp stopModalWithCode:[sender tag]];
}

@end
