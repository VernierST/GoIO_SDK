/*********************************************************************************

Copyright (c) 2010, Vernier Software & Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Vernier Software & Technology nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL VERNIER SOFTWARE & TECHNOLOGY BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**********************************************************************************/
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
