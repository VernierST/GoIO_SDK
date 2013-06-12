//
//  SetMeasurementPeriod.h
//  GoIO_SDK
//
//  Created by swalton on 2/5/05.
// Vernier Software & Technology, Inc.
//

#import <Cocoa/Cocoa.h>


@interface SetMeasurementPeriod : NSWindowController
{
	IBOutlet NSTextField *measurementPeriodField;
}

+ (double) showDialog:(double)currentPeriod;

- (id) init;
- (double) runDialog:(double)currentPeriod;
- (void) finishDialog:(id)sender;

@end
