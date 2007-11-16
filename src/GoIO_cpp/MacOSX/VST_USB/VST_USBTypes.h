/*
 *  VST_USBTypes.h
 *  Common types for the VST_USB module.
 *
 *  Created by Christopher Corbell on Tue Mar 19 2002.
 *  Copyright (c) 2002 Vernier Software & Technology. All rights reserved.
 *
 */

#ifndef _VST_USBTYPES_H_
#define _VST_USBTYPES_H_

#ifdef __MWERKS__ /* assume flat headers for CodeWarrior */
#include <CFArray.h>
#else
#include <CoreFoundation/CFArray.h>
#endif

#ifndef _VST_ERRORS_H_
#include "VST_Errors.h"
#endif

#define kMeasurementPipe 1
#define kCommandPipe 2

typedef void * TUSBDeviceRef; /* this wraps an io_service_t device ref */
typedef struct
{
    SInt32		nProductID;
    SInt32		nVendorID;
    bool                bHID;
    
    TUSBDeviceRef	pDeviceRef;
} VST_USBSpec;

typedef CFMutableArrayRef VST_USBSpecArrayRef;

typedef void (*TFunction_VST_USB_callback)(SInt32 nProductID, SInt32 nVendorID, void * pUserParam);

typedef void * TUSBBulkDevice;
typedef void * TUSBDeviceNotificationThread;

#endif // _VST_USBTYPES_H_
