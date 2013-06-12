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
 *  VST_USB.h
 *  The VST_USB module is a user-land IOKit-USB wrapper.
 *
 *  It is designed to simplify access to a bulk USB device
 *  from user space.  This module is currently oriented toward
 *  use of bulk 1-in, 1-out USB devices.  This module does not
 *  attempt multi-client connections to a device (device must be	
 *  not-in-use).  This module -does- allow multiple devices of the
 *  same type to be accessed by the client.
 *
 * !!!IMPORTANT: the Read() call used by this module requires MacOS 10.0.4
 * or later.  Clients MUST ensure that this is the minimum system
 * before using this module, or BAD THINGS may happen.
 *
 *  This module can be used directly from a mach-o app.
 *  To use this module from a CFM Carbon app, see VST_USB_CFMAPI.h.
 *
 *  Created by Christopher Corbell on Tue Mar 19 2002.
 *  Copyright (c) 2002-2007 Vernier Software & Technology. All rights reserved.
 *
 */

#ifndef _VST_USB_H_
#define _VST_USB_H_

#ifndef _VST_USBTYPES_H_
#include "VST_USBTypes.h"
#endif

#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/hid/IOHIDLib.h>
#include <CoreFoundation/CoreFoundation.h>

#define kHIDUnspecifiedCookie	((IOHIDElementCookie) -1)

#ifdef __cplusplus
extern "C" {
#endif

//
// We now wish to support a bulk data callback.
//
typedef long (*VST_USB_Bulk_Data_Callback)(unsigned char* pBytes, unsigned long nNumBytes, void* pContext);

//
// We support different reading modes for bulk/interrupt transfers
//
typedef enum
{
	kBulkReadSmallPacketThreaded,		// from a worker thread continuously read 64 byte packets (LabPro)
	kBulkReadLengthPacketThreaded,		// from a worker thread continuously fetch a length header, and then fetch the number of bytes specified by the length header (LabQuest)
	kBulkReadDirect,					// call ReadPipe() synchronously from the caller's thread (TI Calculators)
	kBulkReadGarminGPSProtocol,			// from worker thread do async read on interrupt pipe waiting for special "Data Available" packet. Then begin bulk read til none available. Repeat.
	kBulkReadDefault = kBulkReadSmallPacketThreaded
} EBulkReadMode;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    VST_USBBulkDevice struct - IOKit data for a simple Bulk-mode USB device 
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef struct
{
    SInt32				nProductID;
    SInt32				nVendorID;
    
    Boolean				bDeviceIsInUse; /* found dev. intf., but it's in use (couldn't open) */
    
    Boolean				bUsesHID;
    Boolean				bUsesFTDISerial; // if true, enable special handling of this device
    Boolean				bTextMode; // if true, throw-away trailing 0-bytes in a packet
    
    OSStatus			nError; // error from last i/o operation - watch for kIOReturnNotResponding!
    
	// Use direct read or not: see comments on VST_SetBulkUSBDeviceDirectRead() below.
	EBulkReadMode		eBulkReadMode;
	Boolean				bWaitOnClearBeforeWrite;
	Boolean				bGarminGPSProtocolSessionStarted;

	
    /* IOUSB family device interface: */
    io_service_t			ioUSBDeviceRef; 
    IOUSBDeviceInterface182 ** hDeviceInterface;
    IOUSBConfigurationDescriptor * pConfigurationDesc; /* obtained from the device interface */
    
    // IOHID interfaces
    IOHIDDeviceInterface ** hHIDInterface;
    IOHIDQueueInterface **  hHIDInputInterface;
    IOHIDOutputTransactionInterface **  hHIDOutputInterface;
    CFRunLoopSourceRef		hHIDRunLoopSource;
    CFRunLoopRef			hHIDRunLoop;
    unsigned char			nLastNumMeasurementsInPacket;
    unsigned char			nLastMeasurementRollingCounter;
    
    /* IOUSB family interface interface: */
    io_service_t			ioUSBInterfaceRef;
    IOUSBInterfaceInterface182 ** hInterfaceInterface;
    UInt8					nNumPipes;
    
    /* interface pipes (endpoints) */
    UInt8				nControlPipeRef;

    UInt8				nInPipeRef;
    UInt8				nOutPipeRef;
	UInt8				nInInterruptPipe;	// Garmin GPS Only

    UInt8				nInPipeInterval;
    UInt8				nOutPipeInterval;
	UInt8				nInInterruptPipeInterval;

    UInt16				nMaxInPacketSize;
    UInt16				nMaxOutPacketSize;
	UInt16				nMaxInInterruptPipePacketSize;
    
    /* Sub-packets are a concept used by LabPro in some i/o modes.  A sub-packet is a */
    /* uniform chunk of a packet (e.g. 16 bytes out of 64) which contains an indivudual */
    /* word or batch of data.  Only some of the sub-packet bytes may be valid, based on */
    /* configuration or device mode; e.g., we may want to use 11 bytes of each 16-byte subpacket. */
    /* If this subpacket logic is used, invalid bytes of each subpacket are discarded by the */
    /* listening thread; they do not get copied to the listen buffer. */
    UInt16				nSubPacketSize;
    UInt16				nSubPacketValidBytes;
    Boolean				bUsingSubPackets;
    
    /* Async notification and callback support: */
    pthread_t			listeningThread;
    pthread_mutex_t		listeningMutex;
    UInt32				nListenSleep; // in -MICROSECONDS- 1000 is a ms, 1000000 is 1 sec
    
    /* lock the mutex before changing any of the following!!! */
    unsigned char *		pMeasurementBuffer;
    UInt32				nMeasurementBufferBytes;
    UInt32				nMeasurementBufferMaxBytes;
    
    unsigned char *		pCommandBuffer;
    UInt32				nCommandBufferBytes;
    UInt32				nCommandBufferMaxBytes;
    
    unsigned char *		pListenBuffer;
    UInt32				nListenBufferBytes; /* number of valid bytes */
    UInt32				nListenBufferMaxBytes; /* size of buffer */
	
	IOHIDElementCookie	nHIDInputCookie;			// This corresponds to our input element.
	IOHIDElementCookie	nHIDOutputCookie;			// This corresponds to our output element -- we write all 8 feature bytes in one burst.
    Boolean				bWantsHIDInputFiltering;
	
	/* Bulk Data Callback */
	VST_USB_Bulk_Data_Callback
						pBulkCallback;
	void*				pBulkCallbackContext;
	
} VST_USBBulkDevice;

typedef struct
{
    CFMutableArrayRef   pAddedDevices;
    CFMutableArrayRef   pRemovedDevices;

    pthread_t			notificationThread;
    CFRunLoopRef		notificationRunLoop;
    
    SInt32				nVendorID;
    SInt32				nProductID;
    Boolean				bActive;
    Boolean				bUsesHID;
} VST_USBNotificationThread;

/* * * * * * * * * * * * * * * * * * *
    VST_GetUSBSpecArrayRefForDevice() 
* * * * * * * * * * * * * * * * * * */
OSStatus VST_GetUSBSpecsForDevice(SInt32 nVendorID, SInt32 nProductID, bool bHID, VST_USBSpecArrayRef * pOutArrayRef);
                                  
/* * * * * * * * * * * * * * * * * * *
    VST_GetUSBSpecByLocation() 
* * * * * * * * * * * * * * * * * * */
OSStatus VST_GetUSBSpecByLocation(SInt32 nVendorID, SInt32 nProductID, UInt32 nLocationID, bool bHID, VST_USBSpec ** pOutSpec);

/* * * * * * * * * * * * * * * * * * *
    VST_GetUSBSpecLocation() 
* * * * * * * * * * * * * * * * * * */
OSStatus VST_GetUSBSpecLocation(VST_USBSpec * pUSBSpec, UInt32 * pOutLocation);

/* * * * * * * * * * * * * * * * * * *
    VST_ReleaseUSBSpecArray() 
* * * * * * * * * * * * * * * * * * */
/* Releases the array */
OSStatus VST_ReleaseUSBSpecArray(VST_USBSpecArrayRef pArrayRef);

/* * * * * * * * * * * * * * * * * * *
    VST_MakeUSBBulkDeviceFromSpec() 
* * * * * * * * * * * * * * * * * * */
/* Given a VST_USBSpec, get a VST_USBBulkDevice record. */
/* This method allocates memory if it succeeds. */
VST_USBBulkDevice * VST_MakeUSBBulkDeviceFromSpec(VST_USBSpec * pInSpec);

/* * * * * * * * * * * * * * * * * * *
    VST_ReleaseUSBBulkDevice() 
* * * * * * * * * * * * * * * * * * */
/* Release all fields of a VST_USBBulkDevice created with */
/* VST_MakeUSBBulkDeviceFromSpec(). */
OSStatus VST_ReleaseUSBBulkDevice(VST_USBBulkDevice * pUSBDevice);

/* * * * * * * * * * * * * * * * * * *
    VST_IsDeviceInUse()
* * * * * * * * * * * * * * * * * * * */
/* Just checks the bDeviceInUse flag - users of this module */
/* don't need this, just check the spec's flag directly.  This */
/* is for the CFMAPI, where the VST_USBBulkDevice is opaque. */
Boolean VST_IsDeviceInUse(VST_USBBulkDevice * pUSBDevice); 

/* * * * * * * * * * * * * * * * * * *
    VST_SetDeviceIsFTDISerialConverter() 
* * * * * * * * * * * * * * * * * * */
/* Checks whether a device is using the HID driver */
Boolean VST_IsDeviceHID(VST_USBBulkDevice * pUSBDevice);

/* * * * * * * * * * * * * * * * * * *
    VST_SetDeviceIsFTDISerialConverter() 
* * * * * * * * * * * * * * * * * * */
/* Enables special support for the FTDI USB <-> Serial Converter */
OSStatus VST_SetDeviceIsFTDISerialConverter(VST_USBBulkDevice * pUSBDevice, Boolean bIsFTDI);

/* * * * * * * * * * * * * * * * * * *
    VST_GetMaxPacketSize()
* * * * * * * * * * * * * * * * * * * */
/* Returns the max of bulk transfer pipe packet sizes - */
/* note that if you're using this module directly you */
/* really don't need this function, you can just look at */
/* the VST_USBSpec fields directly.  The function is mainly */
/* for the CFMPAI, where the VST_USBBulkDevice is opaque. */
UInt32 VST_GetMaxPacketSize(VST_USBBulkDevice * pUSBDevice); 

/* * * * * * * * * * * * * * * * * * *
    VST_GetNumberOfMeasurementsInLastPacket() 
* * * * * * * * * * * * * * * * * * */
/* Only valid for HID devices.  Returns the number of measurements detected in the last 
   measurement packet. 
*/
UInt32 VST_GetNumberOfMeasurementsInLastPacket(VST_USBBulkDevice * pUSBDevice);

/* * * * * * * * * * * * * * * * * * *
    VST_SetBulkUSBDeviceDirectRead()
	
	// Clients can now choose to read from the device directly, circumventing the read thread.
	// This is applicable when reading from TI calculators whose interfaces seem to hang when
	// you spam them with read pipe requests. This works out okay because the protocol is
	// so tightly handshaked that the device will never be sending unexpected or asynchronous
	// data.
	//
	// Call VST_SetBulkUSBDeviceDirectRead() before calling VST_OpenUSBPortForIO()
	//
	// This is now implemented as a call to VST_SetBulkReadMode(bUseDirectRead ? kBulkReadDirect : kBulkReadDefault).
	
* * * * * * * * * * * * * * * * * * * */
OSStatus VST_SetBulkUSBDeviceDirectRead(VST_USBBulkDevice * pUSBDevice, Boolean bUseDirectRead);

/* * * * * * * * * * * * * * * * * * *
    VST_SetBulkReadMode()
	
	// Set the bulk read mode. Choose from one of the constants defined by EBulkReadMode.
	//
	// Call VST_SetBulkUSBDeviceDirectRead() before calling VST_OpenUSBPortForIO()
	
* * * * * * * * * * * * * * * * * * * */
OSStatus VST_SetBulkReadMode(VST_USBBulkDevice * pUSBDevice, EBulkReadMode eBulkReadMode);


/* * * * * * * * * * * * * * * * * * *
    VST_SetBulkUSBDataCallback()

	// Clients may now specify a callback for receiving data when it becomes available on a pipe.
	// Call VST_SetBulkUSBDataCallback with a given bulk device, specifying the callback. pContext will be
	// stored with this device instance and will be passed to your callback.
	// Call VST_SetBulkUSBDataCallback() before calling VST_OpenUSBPortForIO().
	// This may not be used in conjunction with the Direct Read feature.
	// Callback looks like: 
	//			long MyCallback(unsigned char* pBytes, unsigned long nNumBytes, void* pContext)
	// It may return an error code, or 0 if no error. The error will most likely be logged or ignored.
	// Specify NULL callback to remove your callbacks.
	//
	// Notes: if you remove/change your callback before closing your device, there is no guarantee that your old callback
	// won't continue to receive data (I.e. the bulk read thread may still refer to a stale reference for a brief
	// period after you call VST_SetBulkUSBDataCallback). Take care not to tear down your consumer chain until
	// the device is closed or until the new callback begins receiving data.
	
* * * * * * * * * * * * * * * * * * * */
OSStatus VST_SetBulkUSBDataCallback(VST_USBBulkDevice * pUSBDevice, VST_USB_Bulk_Data_Callback pCallback, void* pContext);

/* * * * * * * * * * * * * * * * * * *
	VST_SetUSBHIDInputCookie()
	VST_SetUSBHIDOutputCookie()
* * * * * * * * * * * * * * * * * * * */
OSStatus VST_SetUSBHIDInputCookie(VST_USBBulkDevice * pUSBDevice, IOHIDElementCookie cookie);
OSStatus VST_SetUSBHIDOutputCookie(VST_USBBulkDevice * pUSBDevice, IOHIDElementCookie cookie);

/* * * * * * * * * * * * * * * * * * *
	VST_SetWantsHIDInputFiltering()
* * * * * * * * * * * * * * * * * * * */
OSStatus VST_SetWantsHIDInputFiltering(VST_USBBulkDevice * pUSBDevice, Boolean wantsFiltering);

/* * * * * * * * * * * * * * * * * * *
    VST_OpenUSBPortForIO()
* * * * * * * * * * * * * * * * * * * */
OSStatus VST_OpenUSBPortForIO(VST_USBBulkDevice * pUSBDevice);

/* * * * * * * * * * * * * * * * * * *
    VST_CloseUSBPort()
* * * * * * * * * * * * * * * * * * * */
OSStatus VST_CloseUSBPort(VST_USBBulkDevice * pUSBDevice);

/* * * * * * * * * * * * * * * * * * *
    VST_WriteBytes()
* * * * * * * * * * * * * * * * * * * */                             
OSStatus VST_WriteBytes(VST_USBBulkDevice * pUSBDevice, void * pData, UInt32 nSize);

/* * * * * * * * * * * * * * * * * * *
    VST_ReadBytes()
* * * * * * * * * * * * * * * * * * * */    
OSStatus VST_ReadBytes(VST_USBBulkDevice * pUSBDevice, void * pOutBuf, UInt32 * pnMaxWanted, UInt32 nMaxTimeoutMS, int nPipe);
                        
/* * * * * * * * * * * * * * * *
    VST_BytesAvailable()
* * * * * * * * * * * * * * * */
UInt32 VST_BytesAvailable(VST_USBBulkDevice * pUSBDevice, int nPipe);

/* * * * * * * * * * * * * * * *
    VST_ClearInput()
* * * * * * * * * * * * * * * */
OSStatus VST_ClearInput(VST_USBBulkDevice * pUSBDevice, int nPipe);

/* * * * * * * * * * * * * * * *
    VST_SetTextMode()
* * * * * * * * * * * * * * * */
OSStatus VST_SetTextMode(VST_USBBulkDevice * pUSBDevice, Boolean bOn);

/* * * * * * * * * * * * * * * *
    VST_ExpectSubPackets()
* * * * * * * * * * * * * * * */
OSStatus VST_ExpectSubPackets(VST_USBBulkDevice * pUSBDevice, UInt16 nPacketSize, UInt16 nValidBytes);

/* * * * * * * * * * * * * * * *
    VST_NoSubPackets()
* * * * * * * * * * * * * * * */
OSStatus VST_NoSubPackets(VST_USBBulkDevice * pUSBDevice);

/* * * * * * * * * * * * * * * * * * * *
    VST_RegisterForUSBNotification()
* * * * * * * * * * * * * * * * * * * */
/* Tell system to inform process when USB device with given */
/* product/vendor ID is connected or disconnected... */
OSStatus VST_RegisterForUSBNotification(SInt32 nVectorID, SInt32 nProductID, Boolean bHID, VST_USBNotificationThread ** pOutThreadRef);

/* * * * * * * * * * * * * * * * * * * *
    VST_GetDeviceAdded()
* * * * * * * * * * * * * * * * * * * */
UInt32 VST_GetDeviceAdded(VST_USBNotificationThread * pNotification);

/* * * * * * * * * * * * * * * * * * * *
    VST_GetDeviceRemoved()
* * * * * * * * * * * * * * * * * * * */
UInt32 VST_GetDeviceRemoved(VST_USBNotificationThread * pNotification);

/* * * * * * * * * * * * * * * * * * * *
VST_DestroyUSBNotificationThread()
* * * * * * * * * * * * * * * * * * * */
/* Kill the notification thread and free related memory */
OSStatus VST_DestroyUSBNotificationThread(VST_USBNotificationThread * pThreadStruct);

/* * * * * * * * * * * * * * * * * * * *
VST_GetManufactureString()
* * * * * * * * * * * * * * * * * * * */
OSStatus VST_GetManufactureString(VST_USBBulkDevice * pUSBDevice, char* outBuffer, int maxLen);

/* * * * * * * * * * * * * * * * * * * *
VST_GetSerialNumberString()
* * * * * * * * * * * * * * * * * * * */
OSStatus VST_GetSerialNumberString(VST_USBBulkDevice* pUSBDevice, char* outBuffer, int maxLen);

/* * * * * * * * * * * * * * * * * * * *
VST_GetProductString()
* * * * * * * * * * * * * * * * * * * */
OSStatus VST_GetProductString(VST_USBBulkDevice* pUSBDevice, char* outBuffer, int maxLen);

/* * * * * * * * * * * * * * * * * * * *
VST_GetVersionNumberString()	HID only
* * * * * * * * * * * * * * * * * * * */
OSStatus VST_GetVersionNumberString(VST_USBBulkDevice* pUSBDevice, char* outBuffer, int maxLen);

/* * * * * * * * * * * * * * * * * * * *
VST_IsDeviceStillValid()
* * * * * * * * * * * * * * * * * * * */
Boolean VST_IsDeviceStillValid(VST_USBBulkDevice * pUSBDevice);

// Misc
OSStatus VST_ShowSplashScreen(void);
OSStatus VST_HideSplashScreen(void);
OSStatus VST_GetMainMonitorDPI(double *prHorizontal, double *prVertical);

#ifdef __cplusplus
}
#endif

#endif // _VST_USB_H_
