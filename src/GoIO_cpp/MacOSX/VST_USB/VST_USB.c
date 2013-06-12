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
 *  VST_USB.c
 *  VST_USB
 *
 *  Created by Christopher Corbell on Tue Mar 19 2002.
 *  Copyright (c) 2002-2007 Vernier Software & Technology. All rights reserved.
 *
 */

#include "VST_USB.h" 
#include "GDeviceToHostUSBTypes.h"
#include <stdio.h>

#include <pthread.h>
#include <sched.h>

#include <unistd.h>	// for sleep(), usleep()

#include <IOKit/usb/IOUSBLib.h>
#include <mach/mach.h>
#include <IOKit/IOCFPlugIn.h>
#include <CoreFoundation/CFRunLoop.h>

#define DIAGNOSTICS_ON 0

#ifndef IO_OBJECT_NULL
	#define IO_OBJECT_NULL NULL
#endif

#define VST_USE_SET_REPORT_IMP	1		// Use setReport() instead of output transaction

void NSLog(CFStringRef format, ...);
extern void NSLog(CFStringRef format, ...);
void	local_ConvertBytesToInt(unsigned char chLSB,
				unsigned char chLMidB,
				unsigned char chMMidB,
				unsigned char chMSB,
				int * pOutInt);
void	local_ConvertByteToShort(unsigned char LSB, unsigned char MSB, short* pOutShort);
	


static IONotificationPortRef	gNotifyPort;
static io_iterator_t		gRawAddedIter;
static io_iterator_t		gRawRemovedIter;

/* * * * * * * * * * * * * * * */
/* Local function prototypes: */
/* * * * * * * * * * * * * * */

// Functions for building a USB Bulk or HID device
Boolean local_USBDeviceFromService(VST_USBBulkDevice * pOutDevice, io_service_t ioSvc_usbDevice);
Boolean local_HIDDeviceFromService(VST_USBBulkDevice * pOutDevice, io_service_t ioSvc_usbDevice);

Boolean local_GetInterfaceInfoForUSBBulkDevice(VST_USBBulkDevice * pOutDevice); 
    /* called by local_USBSpecFromService (for each device) */

Boolean local_GetPipesInfoForUSBBulkDevice(VST_USBBulkDevice * pOutDevice); 
    /* called by local_GetInterfaceInfoForUSBBulkDevice */

// Device interface lookup
IOUSBDeviceInterface182 ** local_GetUSBDeviceInterfaceFromService(io_service_t ioSvc_usbDevice);
IOHIDDeviceInterface ** local_GetHIDDeviceInterfaceFromService(io_service_t ioSvc_usbDevice);

io_service_t local_GetFirstInterfaceServiceFromDevice(IOUSBDeviceInterface182 ** hIOUSBDevice);
IOUSBInterfaceInterface182 ** local_GetInterfaceInterfaceFromService(io_service_t ioSvc_usbInterface);
    
/* These functions force the interface open for io, or closed: */
OSStatus local_OpenIO(VST_USBBulkDevice * pOutDevice);
OSStatus local_CloseIO(VST_USBBulkDevice * pOutDevice);
OSStatus local_ReadBytes(VST_USBBulkDevice * pUSBDevice, void * pBuffer, UInt32 * pnMaxWanted, UInt8 nReadPipe);
OSStatus local_ReadBytesDirect(VST_USBBulkDevice * pUSBDevice, void* pBuffer, UInt32 * pnMaxWanted, UInt32 nMaxTimeoutMS, int nPipe);
     
void local_input_listen_thread(VST_USBBulkDevice * pTheDevice);
void local_start_notification_thread(VST_USBNotificationThread * pThread);
void local_device_added_callback(void *refCon, io_iterator_t iterator);
void local_device_removed_callback(void *refCon, io_iterator_t iterator);

void local_hid_listen_thread(VST_USBBulkDevice * pTheDevice);
void local_input_hid_callback(void * target, IOReturn result, void * refcon, void * sender);

void local_garmin_protocol_thread(VST_USBBulkDevice * pTheDevice);	// This  thread is used by garmin protocol. Oooh. Special.

void local_EnumerateHIDCookies(IOHIDDeviceInterface122** handle, VST_USBBulkDevice* pDev);

/********************************************************************************/
/********************************************************************************/

OSStatus VST_GetUSBSpecsForDevice(SInt32 nVendorID, 
                                  SInt32 nProductID,
                                  bool bHID,
                                  VST_USBSpecArrayRef * pOutArrayRef)
{
    kern_return_t 		nKernErr = 0;
    mach_port_t 		machMasterPort;
    CFMutableDictionaryRef 	cfMutDict_serviceMatcher;
    CFNumberRef			cfNum_VendorID = 0;
    CFNumberRef			cfNum_ProductID = 0;
    io_iterator_t 		ioIter_usbService = 0;
    io_service_t		ioSvc_usbDevice = 0;
    IOReturn                    ioReturnValue;
    
    if (pOutArrayRef == NULL)
        return kVST_paramErr;
    
    /* Get the master port... */
    nKernErr = IOMasterPort(MACH_PORT_NULL, &machMasterPort);
    if (nKernErr)
    {
        if (machMasterPort)
            mach_port_deallocate(mach_task_self(), machMasterPort);
        return (OSStatus) nKernErr;
    }
    else
    if (!machMasterPort)
        return (OSStatus) kVST_NullMasterPortError;
    
    if (DIAGNOSTICS_ON)
        printf("In VST_GetUSBSpecsForDevice(0x%X, 0x%X)...\n", (unsigned int)nVendorID, (unsigned int)nProductID);
    
    // Create a matching dictionary...   
    if (!bHID) // ... for USB
        cfMutDict_serviceMatcher = IOServiceMatching(kIOUSBDeviceClassName);
    else // ... or HID
        cfMutDict_serviceMatcher = IOServiceMatching(kIOHIDDeviceKey);
    
    if (!cfMutDict_serviceMatcher)
    {
        mach_port_deallocate(mach_task_self(), machMasterPort);
        return (OSStatus) kVST_NullObjectError;
    }
    
    cfNum_VendorID = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &nVendorID);
    cfNum_ProductID = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &nProductID);
    if (!bHID)
    {
        CFDictionarySetValue(cfMutDict_serviceMatcher, CFSTR(kUSBVendorID), cfNum_VendorID);
        CFDictionarySetValue(cfMutDict_serviceMatcher, CFSTR(kUSBProductID), cfNum_ProductID);
    }
    else
    {
        CFDictionarySetValue(cfMutDict_serviceMatcher, CFSTR(kIOHIDVendorIDKey), cfNum_VendorID);
        CFDictionarySetValue(cfMutDict_serviceMatcher, CFSTR(kIOHIDProductIDKey), cfNum_ProductID);
    }
    CFRelease(cfNum_VendorID);
    CFRelease(cfNum_ProductID);

    /* Get an iterator to the first service that matches our USB vendor & product ID... */
    nKernErr = IOServiceGetMatchingServices(machMasterPort, cfMutDict_serviceMatcher, &ioIter_usbService);
    if (nKernErr != 0)
    {
        mach_port_deallocate(mach_task_self(), machMasterPort);
    	return (OSStatus) kVST_NullObjectError;
    }
    cfMutDict_serviceMatcher = NULL; /* this was consumed by the above call */
    
    /* If we've gotten this far we should go ahead and allocate our array... */
    *pOutArrayRef = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);
    if (!*pOutArrayRef)
    {
        mach_port_deallocate(mach_task_self(), machMasterPort);
        return (OSStatus) kVST_NullObjectError;
    }
    
    while (ioSvc_usbDevice = IOIteratorNext(ioIter_usbService))
    {
        VST_USBSpec * pNewUSBSpec = (VST_USBSpec *) malloc(sizeof(VST_USBSpec));
        if (!pNewUSBSpec)
            break;
            
        if (DIAGNOSTICS_ON) 
        {
            io_name_t className;
            ioReturnValue = IOObjectGetClass(ioSvc_usbDevice, className);
            if (ioReturnValue == 0)
                printf("Got a service of type %s at address 0x%X...\n", className, (unsigned int)ioSvc_usbDevice);
            else
                printf("Got a service of unknown type at address 0x%X...\n", (unsigned int)ioSvc_usbDevice);
        }
        
        pNewUSBSpec->nVendorID = nVendorID;
        pNewUSBSpec->nProductID = nProductID;
        pNewUSBSpec->bHID = bHID;
        pNewUSBSpec->pDeviceRef = (TUSBDeviceRef) ioSvc_usbDevice;

        CFArrayAppendValue(*pOutArrayRef, pNewUSBSpec);
    }
    
    mach_port_deallocate(mach_task_self(), machMasterPort);
    return (OSStatus) nKernErr;
}

OSStatus VST_ReleaseUSBSpecArray(VST_USBSpecArrayRef vstArrayRef)
{
    CFIndex nNumSpecs = 0;
    CFIndex nIndex = 0;
    if(!vstArrayRef)
        return kVST_paramErr;
        
    nNumSpecs = CFArrayGetCount(vstArrayRef);
    for(nIndex = 0; nIndex < nNumSpecs; nIndex++)
    {
        VST_USBSpec * pTheSpec = (VST_USBSpec *) CFArrayGetValueAtIndex(vstArrayRef, nIndex);
        if(pTheSpec != NULL)
        {
            io_service_t ioUSBDeviceRef = (io_service_t) pTheSpec->pDeviceRef;
            if(ioUSBDeviceRef)
            {
                IOObjectRelease(ioUSBDeviceRef);
                ioUSBDeviceRef = 0;
                pTheSpec->pDeviceRef = 0;
            }
            free(pTheSpec);
        }
    }
    CFArrayRemoveAllValues(vstArrayRef);
    CFRelease(vstArrayRef);
    
    return 0;
}

OSStatus VST_GetUSBSpecByLocation(SInt32 nVendorID, SInt32 nProductID, UInt32 nLocationID, bool bHID, VST_USBSpec ** pOutSpec)
{
    kern_return_t 		nKernErr = 0;
    mach_port_t 		machMasterPort;
    CFMutableDictionaryRef 	cfMutDict_serviceMatcher;
    CFNumberRef			cfNum_VendorID = NULL;
    CFNumberRef			cfNum_ProductID = NULL;
    io_iterator_t 		ioIter_usbService = 0;
    io_service_t		ioSvc_usbDevice = 0;
    
    if (pOutSpec == NULL)
        return kVST_paramErr;
    
    /* Get the master port... */
    nKernErr = IOMasterPort(MACH_PORT_NULL, &machMasterPort);
    if (nKernErr)
    {
        if (machMasterPort)
            mach_port_deallocate(mach_task_self(), machMasterPort);
        return (OSStatus) nKernErr;
    }
    else
    if (!machMasterPort)
        return (OSStatus) kVST_NullMasterPortError;
    
    if (DIAGNOSTICS_ON)
        printf("In VST_GetUSBSpecByLocation(0x%08x)...\n", (unsigned int)nLocationID);
    
    // Create a matching dictionary...   
    if (!bHID) // ... for USB
        cfMutDict_serviceMatcher = IOServiceMatching(kIOUSBDeviceClassName);
    else // ... or HID
        cfMutDict_serviceMatcher = IOServiceMatching(kIOHIDDeviceKey);
    
    if (!cfMutDict_serviceMatcher)
    {
        mach_port_deallocate(mach_task_self(), machMasterPort);
        return (OSStatus) kVST_NullObjectError;
    }
    
    cfNum_VendorID = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &nVendorID);
    cfNum_ProductID = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &nProductID);
    if (!bHID)
    {
        CFDictionarySetValue(cfMutDict_serviceMatcher, CFSTR(kUSBVendorID), cfNum_VendorID);
        CFDictionarySetValue(cfMutDict_serviceMatcher, CFSTR(kUSBProductID), cfNum_ProductID);
    }
    else
    {
        CFDictionarySetValue(cfMutDict_serviceMatcher, CFSTR(kIOHIDVendorIDKey), cfNum_VendorID);
        CFDictionarySetValue(cfMutDict_serviceMatcher, CFSTR(kIOHIDProductIDKey), cfNum_ProductID);
    }
    CFRelease(cfNum_VendorID);
    CFRelease(cfNum_ProductID);

    /* Get an iterator to the first service that matches our criteria */
    nKernErr = IOServiceGetMatchingServices(machMasterPort, cfMutDict_serviceMatcher, &ioIter_usbService);
    if (nKernErr != 0)
    {
        mach_port_deallocate(mach_task_self(), machMasterPort);
    	return (OSStatus) kVST_NullObjectError;
    }
    cfMutDict_serviceMatcher = 0; /* this was consumed by the above call */

    while (ioSvc_usbDevice = IOIteratorNext(ioIter_usbService))
    {
    	UInt32 nDevLocationID = 0;
    	CFNumberRef cfNumLocation;
    	CFDictionaryRef cfDictProperties;
    	nKernErr = IORegistryEntryCreateCFProperties(ioSvc_usbDevice, (CFMutableDictionaryRef *)&cfDictProperties, kCFAllocatorDefault, kNilOptions);
    	if (nKernErr == 0)
    	{
    	    if (DIAGNOSTICS_ON)
                printf("Got a properties dictionary for the device\n");
            if (!bHID)
                cfNumLocation = (CFNumberRef) CFDictionaryGetValue(cfDictProperties, CFSTR(kUSBDevicePropertyLocationID));
            else
                cfNumLocation = (CFNumberRef) CFDictionaryGetValue(cfDictProperties, CFSTR(kIOHIDLocationIDKey));
            if (cfNumLocation != NULL)
            {
                CFNumberGetValue(cfNumLocation, kCFNumberSInt32Type, &nDevLocationID);
                if (DIAGNOSTICS_ON)
                    printf("Device location: 0x%08x\n", (unsigned int)nDevLocationID);
            }
			CFRelease(cfDictProperties);
    	}
    
    	if (nDevLocationID == nLocationID)
    	{
            VST_USBSpec * pNewUSBSpec = (VST_USBSpec *) malloc(sizeof(VST_USBSpec));
            if (pNewUSBSpec != NULL)
            {            
                if (DIAGNOSTICS_ON)    
                    printf("Got a service at address 0x%X...\n", (unsigned int)ioSvc_usbDevice);
                
                pNewUSBSpec->nVendorID = nVendorID;
                pNewUSBSpec->nProductID = nProductID;
                pNewUSBSpec->bHID = bHID;
                pNewUSBSpec->pDeviceRef = (TUSBDeviceRef)ioSvc_usbDevice;
                *pOutSpec = pNewUSBSpec;
            }
            break;
        }
    }
    
    mach_port_deallocate(mach_task_self(), machMasterPort);
    return (OSStatus) nKernErr;
}

OSStatus VST_GetUSBSpecLocation(VST_USBSpec * pUSBSpec, UInt32 * pOutLocation)
{
    kern_return_t kr = 0;

    if ((pUSBSpec == NULL) || (pUSBSpec->pDeviceRef == NULL))
    {
        if (DIAGNOSTICS_ON)
            printf("VST_GetUSBSpecLocation: Invalid USB Spec\n");
        kr = kVST_paramErr;
    }
    else
    {
        UInt32 nDevLocationID = 0;
        CFNumberRef cfNumLocation;
        CFDictionaryRef cfDictProperties;
        kr = IORegistryEntryCreateCFProperties((io_service_t)pUSBSpec->pDeviceRef, (CFMutableDictionaryRef *)&cfDictProperties, kCFAllocatorDefault, kNilOptions);
        if (kr == 0)
        {
            if (!pUSBSpec->bHID)
                cfNumLocation = (CFNumberRef) CFDictionaryGetValue(cfDictProperties, CFSTR(kUSBDevicePropertyLocationID));
            else
                cfNumLocation = (CFNumberRef) CFDictionaryGetValue(cfDictProperties, CFSTR(kIOHIDLocationIDKey));
            if (cfNumLocation != NULL)
            {
                CFNumberGetValue(cfNumLocation, kCFNumberSInt32Type, &nDevLocationID);
                *pOutLocation = nDevLocationID;
            }
        }
		CFRelease(cfDictProperties);
    }

    return (OSStatus) kr;
}

VST_USBBulkDevice * VST_MakeUSBBulkDeviceFromSpec(VST_USBSpec * pInSpec)
{
    VST_USBBulkDevice * pUSBDevice = NULL;
    if (pInSpec)
    {
        Boolean bResult = false;
        pUSBDevice = (VST_USBBulkDevice *) malloc(sizeof(VST_USBBulkDevice));
		pUSBDevice->nProductID = pInSpec->nProductID;
		pUSBDevice->nVendorID = pInSpec->nVendorID;
        
        if (!pInSpec->bHID)
            bResult = local_USBDeviceFromService(pUSBDevice, (io_service_t) pInSpec->pDeviceRef);
        else
            bResult = local_HIDDeviceFromService(pUSBDevice, (io_service_t) pInSpec->pDeviceRef);
        
        if (!bResult)
        {
            /* could not get interface-interface from device interface, this */
            /* device-service ref may be stale... */
            if (DIAGNOSTICS_ON)
                printf("VST_MakeUSBBulkDeviceFromSpec(): Stale device ref\n");
            free(pUSBDevice);
            pUSBDevice = 0;
        }
    }
    if (pUSBDevice == NULL)
		NSLog(CFSTR("*** VST_MakeUSBBulkDeviceFromSpec failed."));
    return pUSBDevice;
}

OSStatus VST_ReleaseUSBBulkDevice(VST_USBBulkDevice * pUSBDevice)
{
    if (pUSBDevice == NULL)
        return kVST_paramErr;

    if (!pUSBDevice->bUsesHID)
    {
        if (pUSBDevice->hInterfaceInterface != NULL)
        {
            (*pUSBDevice->hInterfaceInterface)->USBInterfaceClose(pUSBDevice->hInterfaceInterface);
            (*pUSBDevice->hInterfaceInterface)->Release(pUSBDevice->hInterfaceInterface);
            pUSBDevice->hInterfaceInterface = NULL;
        }
        
        if (pUSBDevice->ioUSBInterfaceRef != IO_OBJECT_NULL)
        {
            IOObjectRelease(pUSBDevice->ioUSBInterfaceRef);
            pUSBDevice->ioUSBInterfaceRef = IO_OBJECT_NULL;
        }
        
        if (pUSBDevice->hDeviceInterface != NULL)
        {
            (*pUSBDevice->hDeviceInterface)->USBDeviceClose(pUSBDevice->hDeviceInterface);
            (*pUSBDevice->hDeviceInterface)->Release(pUSBDevice->hDeviceInterface);
            pUSBDevice->hDeviceInterface = NULL;
            pUSBDevice->pConfigurationDesc = NULL;
        }
        
        if (pUSBDevice->ioUSBDeviceRef != IO_OBJECT_NULL)
        {
            IOObjectRelease(pUSBDevice->ioUSBDeviceRef);
            pUSBDevice->ioUSBDeviceRef = IO_OBJECT_NULL;
        }
    }
    else
    {
        if (pUSBDevice->hHIDInputInterface != NULL)
        {
            (*pUSBDevice->hHIDInputInterface)->stop(pUSBDevice->hHIDInputInterface);
            (*pUSBDevice->hHIDInputInterface)->dispose(pUSBDevice->hHIDInputInterface);
            (*pUSBDevice->hHIDInputInterface)->Release(pUSBDevice->hHIDInputInterface);
            pUSBDevice->hHIDInputInterface = NULL;
        }
        
#if !VST_USE_SET_REPORT_IMP
        if (pUSBDevice->hHIDOutputInterface != NULL)
        {
            (*pUSBDevice->hHIDOutputInterface)->dispose(pUSBDevice->hHIDOutputInterface);
            (*pUSBDevice->hHIDOutputInterface)->Release(pUSBDevice->hHIDOutputInterface);
            pUSBDevice->hHIDOutputInterface = NULL;
        }
#endif
    
        if (pUSBDevice->hHIDInterface != NULL)
        {
            (*pUSBDevice->hHIDInterface)->close(pUSBDevice->hHIDInterface);
            (*pUSBDevice->hHIDInterface)->Release(pUSBDevice->hHIDInterface);
            pUSBDevice->hHIDInterface = NULL;
        }
    }
    
    free(pUSBDevice);
    return 0;
}

Boolean VST_IsDeviceInUse(VST_USBBulkDevice * pUSBDevice)
{
    if (!pUSBDevice)
        return false;
    return pUSBDevice->bDeviceIsInUse;
}

Boolean VST_IsDeviceHID(VST_USBBulkDevice * pUSBDevice)
{
    if (!pUSBDevice)
        return false;
    return pUSBDevice->bUsesHID;
}

UInt32 VST_GetMaxPacketSize(VST_USBBulkDevice * pUSBDevice)
{
    if (!pUSBDevice)
        return 0L;
    /* NOTE the assumption that nMaxInPacketSize will be */
    /* the same as nMaxoutPacketSize... */
    return (UInt32) pUSBDevice->nMaxInPacketSize;
}

UInt32 VST_GetNumberOfMeasurementsInLastPacket(VST_USBBulkDevice * pUSBDevice)
{
	UInt32 nResult = 0;
	if (pUSBDevice != NULL)
	{
		nResult = pUSBDevice->nLastNumMeasurementsInPacket;
	}
	return nResult;
}

OSStatus VST_SetDeviceIsFTDISerialConverter(VST_USBBulkDevice * pUSBDevice, Boolean bIsFTDI)
{
    if (DIAGNOSTICS_ON && bIsFTDI)        
        printf("VST_SetDeviceIsFTDISerialConverter(true)\n");
        
    if (pUSBDevice == NULL)
        return kVST_paramErr;
        
    pUSBDevice->bUsesFTDISerial = bIsFTDI;
    return 0;
}

OSStatus VST_SetBulkUSBDeviceDirectRead(VST_USBBulkDevice * pUSBDevice, Boolean bUseDirectRead)
{
	if (pUSBDevice)
	{
		if (DIAGNOSTICS_ON)
			printf("VST_SetBulkUSBDeviceDirectRead(0x%x, %s)\n", (unsigned) pUSBDevice, bUseDirectRead ? "TRUE" : "FALSE");

		VST_SetBulkReadMode(pUSBDevice, bUseDirectRead ? kBulkReadDirect : kBulkReadDefault);
	}
	
	return 0;
}

OSStatus VST_SetBulkReadMode(VST_USBBulkDevice * pUSBDevice, EBulkReadMode eBulkReadMode)
{
	if (pUSBDevice)
	{
		if (DIAGNOSTICS_ON)
			printf("VST_SetBulkReadMode(0x%x, 0x%x)\n", (unsigned) pUSBDevice, eBulkReadMode);

		// The read thread needs to perform some tasks before we are allowed to write.
		if (eBulkReadMode == kBulkReadLengthPacketThreaded)
			pUSBDevice->bWaitOnClearBeforeWrite = true; 

		pUSBDevice->eBulkReadMode = eBulkReadMode;
	}
	
	return 0;
}


OSStatus VST_SetBulkUSBDataCallback(VST_USBBulkDevice * pUSBDevice, VST_USB_Bulk_Data_Callback pCallback, void* pContext)
{
	if (pUSBDevice)
	{
		if (DIAGNOSTICS_ON)
			printf("VST_SetBulkUSBDataCallback(0x%x, 0x%x 0x%x)\n", (unsigned) pUSBDevice, pCallback, pContext);

		pUSBDevice->pBulkCallback = pCallback;
		pUSBDevice->pBulkCallbackContext = pContext;
	}
	
	return 0;
}


OSStatus VST_SetUSBHIDInputCookie(VST_USBBulkDevice * pUSBDevice, IOHIDElementCookie cookie)
{
	pUSBDevice->nHIDInputCookie = cookie;
	return 0;
}

OSStatus VST_SetUSBHIDOutputCookie(VST_USBBulkDevice * pUSBDevice, IOHIDElementCookie cookie)
{
	pUSBDevice->nHIDOutputCookie = cookie;
	return 0;
}

OSStatus VST_SetWantsHIDInputFiltering(VST_USBBulkDevice * pUSBDevice, Boolean wantsFiltering)
{
	pUSBDevice->bWantsHIDInputFiltering = wantsFiltering;
	return 0;
}

OSStatus VST_OpenUSBPortForIO(VST_USBBulkDevice * pUSBDevice)
{
	OSStatus status = local_OpenIO(pUSBDevice);
	if (status != 0)
		NSLog(CFSTR("*** VST_OpenUSBPortForIO failed, err %d (0x%x)."), status, status);
    return status;
}

OSStatus VST_CloseUSBPort(VST_USBBulkDevice * pUSBDevice)
{
    if (DIAGNOSTICS_ON)
        printf("VST_CloseUSBPort()\n");
    
    return local_CloseIO(pUSBDevice);
}

OSStatus VST_RegisterForUSBNotification(SInt32 nVendor, 
                                        SInt32 nProduct, 
                                        Boolean bHID,
                                        VST_USBNotificationThread ** ppOutThread)
{
    OSStatus err = 0;
    VST_USBNotificationThread * pOutThread = NULL;

    if(ppOutThread == NULL)
        return kVST_paramErr;
    
    pOutThread = (VST_USBNotificationThread *) malloc(sizeof(VST_USBNotificationThread));

    pOutThread->nVendorID = nVendor;
    pOutThread->nProductID = nProduct;
    pOutThread->bUsesHID = bHID;
        
    // Create the notification arrays
    if (DIAGNOSTICS_ON)
        printf("Creating the notification arrays...\n");
    pOutThread->pAddedDevices = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    if (pOutThread->pAddedDevices == NULL)
        if (DIAGNOSTICS_ON)
            printf("Error creating notification arrays\n");
    pOutThread->pRemovedDevices = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    if (pOutThread->pRemovedDevices == NULL)
        if (DIAGNOSTICS_ON)
            printf("Error creating notification arrays\n");
    
    if (DIAGNOSTICS_ON)
        printf("Registering notifications for vendor 0x%lX, product 0x%lX.\n", pOutThread->nVendorID, pOutThread->nProductID);

    pOutThread->bActive = false;
    
	if (pthread_main_np() == 0)
	{
		// start a notification thread which will call CFRunLoop to receive the notifications
		err = (OSStatus) pthread_create(&(pOutThread->notificationThread), 
											NULL, /* default attributes */
											(void* (*)(void*))local_start_notification_thread, /* function */
											pOutThread); /* param to function */
	}
	else
	{
		// OR add the notification source to the main thread (i.e. currently calling thread)
		local_start_notification_thread(pOutThread);
	}
	
    // A bit of a cheat - sleep a few microseconds before returning
    // to make sure notification thread has time to set CFRunLoopRef field of
    // pOutThread...
    usleep(100);
    *ppOutThread = pOutThread;
    if (DIAGNOSTICS_ON)
    {
        printf("the thread struct is at address 0x%lX\n", (unsigned long)pOutThread);
    }
    return (OSStatus) err;
}

UInt32 VST_GetDeviceAdded(VST_USBNotificationThread * pNotification)
{
    CFNumberRef cfNumLocation;
    UInt32 nResult = 0;
    if ((pNotification != NULL) && (pNotification->pAddedDevices != NULL))
    {
        int nCount = CFArrayGetCount(pNotification->pAddedDevices);
        if (nCount > 0)
        {
        	if (DIAGNOSTICS_ON)
            	printf("%d added devices found\n", nCount);
        
            cfNumLocation = (CFNumberRef) CFArrayGetValueAtIndex(pNotification->pAddedDevices, (CFIndex)0);
            if (cfNumLocation != NULL)
            {
            	CFNumberGetValue(cfNumLocation, kCFNumberSInt32Type, &nResult);
            	CFRetain(cfNumLocation);
	            CFArrayRemoveValueAtIndex(pNotification->pAddedDevices, (CFIndex)0);
	            CFRelease(cfNumLocation);
	            
	            if (DIAGNOSTICS_ON)
	            	printf("We got a + notification for device at 0x%08x\n", (unsigned int)nResult);
	        }
        }
    }
    else
    {
        if (DIAGNOSTICS_ON)
            printf("Notification thread is NULL!\n");
    }
    
    return nResult;
}

UInt32 VST_GetDeviceRemoved(VST_USBNotificationThread * pNotification)
{
    CFNumberRef cfNumLocation;
    UInt32 nResult = 0;
    if ((pNotification != NULL) && (pNotification->pRemovedDevices != NULL))
    {
        int nCount = CFArrayGetCount(pNotification->pRemovedDevices);
        if (nCount > 0)
        {
            cfNumLocation = (CFNumberRef) CFArrayGetValueAtIndex(pNotification->pRemovedDevices, (CFIndex)0);
            if (cfNumLocation != NULL)
            {
            	CFNumberGetValue(cfNumLocation, kCFNumberSInt32Type, &nResult);
            	CFRetain(cfNumLocation);
                CFArrayRemoveValueAtIndex(pNotification->pRemovedDevices, (CFIndex)0);
                CFRelease(cfNumLocation);
	            
            }
        }
    }
    else
    {
        if (DIAGNOSTICS_ON)
            printf("Notification thread is NULL!\n");
    }
    
    return nResult;
}

OSStatus VST_DestroyUSBNotificationThread(VST_USBNotificationThread * pThread)
{
    OSStatus err = 0;
    
    if (DIAGNOSTICS_ON)
        printf("VST_DestroyUSBNotificationThread()\n");
        
    if (pThread == NULL)
        return kVST_paramErr;
    pThread->bActive = false;
    if (pThread->notificationRunLoop)
    {
		if (pthread_main_np() == 0)
			CFRunLoopStop(pThread->notificationRunLoop);
        pThread->notificationRunLoop = 0;
        // stopping the run loop will also termintate the thread
        pThread->notificationThread = 0;
    }
    free(pThread);
    return err;
}

OSStatus VST_WriteBytes(VST_USBBulkDevice * pUSBDevice, void * pData, UInt32 nSize)
{
    OSStatus err = kVST_InterfaceNotReady;
    if (pUSBDevice != NULL)
    {
        if (!pUSBDevice->bUsesHID)
        {
            if ((pUSBDevice->hInterfaceInterface != NULL) && (pUSBDevice->nOutPipeRef != 0))
            {
                if (pUSBDevice->nError == kIOReturnNotResponding)
				{
                    err = pUSBDevice->nError;
					NSLog(CFSTR("*** VST_WriteBytes bulk -- uncleared nError: kIOReturnNotResponding. Exiting?"));
				}
                else
				{
					if (pUSBDevice->bWaitOnClearBeforeWrite)
					{
						NSLog(CFSTR("### ===WRITE=== waiting for flag."));
						while (pUSBDevice->bWaitOnClearBeforeWrite)
							usleep(10000);
					}
					
					if (DIAGNOSTICS_ON)
					{
						size_t ix;
						NSLog(CFSTR("===WRITE=== VST_WriteBytes: data sent (%u): "), (int)nSize);
						for (ix = 0; ix < nSize; ix++)
							printf("%02x ", ((unsigned char*)pData)[ix]);
						printf("\n");
					}
					
				
                    err = (OSStatus) (*pUSBDevice->hInterfaceInterface)->WritePipeTO(pUSBDevice->hInterfaceInterface,
																					 pUSBDevice->nOutPipeRef,
																					 pData,
																					 nSize,
																					 5000, // 5-second timeout to prevent hangs
																					 5000);
					if (err != 0)
						NSLog(CFSTR("*** VST_WriteBytes WritePipeTO error %d (0x%x)"), err, err);
				}	
            }
        }
        else
        {
#if VST_USE_SET_REPORT_IMP		
			if (pUSBDevice->hHIDInterface != NULL)
			{
				err = (*pUSBDevice->hHIDInterface)->setReport(
								pUSBDevice->hHIDInterface,
								kIOHIDReportTypeOutput,				// TODO: was kIOHIDReportTypeFeature - CK spec needs output report type
								0x0, // How to specify "no report ID" if at all possible?
								pData,
								nSize,
								500,
								NULL, 
								NULL,
								NULL);
				if (err)
					NSLog(CFSTR("*** IOHIDDeviceInterface121::setReport() return err 0x%x"), err);
			}
#else		
            if (pUSBDevice->hHIDOutputInterface != NULL)
            {
                // Fill out an event structure
                IOHIDEventStruct outEvent;
                outEvent.type = kIOHIDElementTypeOutput;
				outEvent.elementCookie = pUSBDevice->nHIDOutputCookie;
                
				// if the length of the data to be sent (which should correspond to the cookie report size?) is
				// four bytes or less it goes into the value field, otherwise the longValue and longValueSize fields are used
				outEvent.value = 0;
				if (nSize <= 4) {		// NOTE: the byte ordering below seems to work for both Intel and PPC	sjs
					int i;
					unsigned char* cp = pData;
					for (i=nSize-1; i >= 0 ; --i) 
					{
						outEvent.value <<= 8;
						outEvent.value |= cp[i];
					}
					//printf("outEvent.value: 0x%08x\n", outEvent.value);
					
					outEvent.longValueSize = 0;
					outEvent.longValue = NULL;
				} else {
					outEvent.longValueSize = nSize;
					outEvent.longValue = pData;
				}
                
                if (DIAGNOSTICS_ON)
                {
                	size_t ix;
                    printf("===WRITE=== VST_WriteBytes: data sent (%d): ", (int)nSize);
                    for (ix = 0; ix < nSize; ix++)
                        printf("%02x ", ((unsigned char*)pData)[ix]);
                    printf("\n");
                }
                err = (*pUSBDevice->hHIDOutputInterface)->setElementValue(pUSBDevice->hHIDOutputInterface, outEvent.elementCookie, &outEvent);
				if (err != kIOReturnSuccess)
				{
					NSLog(CFSTR("*** (*pUSBDevice->hHIDOutputInterface)->setElementValue failed! err=0x%x. Cookie=0x%x"), err, outEvent.elementCookie);
				}
				else 
					err = (OSStatus) (*pUSBDevice->hHIDOutputInterface)->commit(pUSBDevice->hHIDOutputInterface, -1, NULL, NULL, NULL);
				if (err != kIOReturnSuccess)
					NSLog(CFSTR("*** (*pUSBDevice->hHIDOutputInterface)->commit failed! err=0x%x."), err);
            }
#endif
        }
    }
    
    return err;
}

OSStatus VST_ReadBytes(VST_USBBulkDevice * pUSBDevice, void * pBuffer, UInt32 * pnMaxWanted, UInt32 nMaxTimeoutMS, int nPipe)
{
    unsigned char * pReadBuffer = NULL;
    UInt32 * pBytes;
    
    OSStatus err = kVST_InterfaceNotReady;
    if (pUSBDevice != NULL)
    {
		if (pUSBDevice->eBulkReadMode == kBulkReadDirect)
		{
			err = local_ReadBytesDirect(pUSBDevice, pBuffer, pnMaxWanted, nMaxTimeoutMS, nPipe);
		}
		else if (!pUSBDevice->bUsesHID && pUSBDevice->pBulkCallback != NULL)
		{
			NSLog(CFSTR("*** VST_ReadBytes called. This should not happen as we have a callback. Please fix."));
		}
		else
		{
			// Figure out which buffer to use
			if (!pUSBDevice->bUsesHID)
			{
				pReadBuffer = pUSBDevice->pListenBuffer;
				pBytes = &pUSBDevice->nListenBufferBytes;
			}
			else
			{
				if (nPipe == kMeasurementPipe)
				{
					pReadBuffer = pUSBDevice->pMeasurementBuffer;
					pBytes = &pUSBDevice->nMeasurementBufferBytes;
				}
				else
				{
					pReadBuffer = pUSBDevice->pCommandBuffer;
					pBytes = &pUSBDevice->nCommandBufferBytes;
				}
			}
			
			// Now read from the buffer!
			if ((*pnMaxWanted > 0) && (pReadBuffer != NULL)) 
			{
				int nLockErr = 0;
				UInt32 nSleepCt = 0;
				err = 0; // debug
				
				if (pUSBDevice->nError == kIOReturnNotResponding)
					return pUSBDevice->nError;
				
				while (VST_BytesAvailable(pUSBDevice, nPipe) < *pnMaxWanted)
				{
					usleep(1000);
					nSleepCt++;
					if (nSleepCt > nMaxTimeoutMS)
						break;
				}

				nSleepCt = 0;
				do
				{
					nLockErr = pthread_mutex_trylock(&(pUSBDevice->listeningMutex));
					if (nLockErr)
					{
						usleep(1000);
						nSleepCt++;
					}
				} while (nLockErr && (nSleepCt < nMaxTimeoutMS));
				
				if (!nLockErr)
				{
					UInt32 nBytesToCopy = *pBytes;
					err = 0L;
					
					if (nBytesToCopy > *pnMaxWanted)
						nBytesToCopy = *pnMaxWanted;
					
					if (DIAGNOSTICS_ON)
					{
						printf("VST_ReadBytes: nBytesToCopy is %ld\n", nBytesToCopy);
						printf("VST_ReadBytes: pUSBDevice->nBufferBytes is %ld\n", *pBytes);
					}
					
					*pnMaxWanted = nBytesToCopy;
					if (nBytesToCopy > 0)
					{
						memmove(pBuffer, pReadBuffer, nBytesToCopy);
					
						if (nBytesToCopy < *pBytes)
						{
							// Shift up unread bytes
							memmove(pReadBuffer, pReadBuffer + nBytesToCopy, *pBytes - nBytesToCopy);
							*pBytes -= nBytesToCopy;
						}
						else
							*pBytes = 0L;
					}
					
					if (DIAGNOSTICS_ON)
						printf("VST_ReadBytes: pUSBDevice->nBufferBytes is now %ld\n", *pBytes);
					pthread_mutex_unlock(&pUSBDevice->listeningMutex);
				}
				else
				{
					err = (OSStatus)nLockErr;
					*pnMaxWanted = 0L;
					if (DIAGNOSTICS_ON)
						printf("VST_ReadBytes - nLockErr is %d\n", nLockErr);
				}

				if (pUSBDevice->nError == kVST_InputBufferOverflow && err == 0)
					err = kVST_InputBufferOverflow;
			}
		}
    }
	if (err != 0)
		NSLog(CFSTR("*** VST_ReadBytes err %d, (0x%x)"), err, err);
    return err;
}

UInt32 VST_BytesAvailable(VST_USBBulkDevice * pUSBDevice, int nPipe)
{
    UInt32 nResult = 0L;
    if (pUSBDevice != NULL)
    {
        if (!pUSBDevice->bUsesHID)
            nResult = pUSBDevice->nListenBufferBytes;
        else
        {
            if (nPipe == kMeasurementPipe)
                nResult = pUSBDevice->nMeasurementBufferBytes;
            else
                nResult = pUSBDevice->nCommandBufferBytes;
        }

        if (pUSBDevice->nError == kIOReturnNotResponding)
            nResult = 0;
    }
    return nResult;
}

OSStatus VST_ClearInput(VST_USBBulkDevice * pUSBDevice, int nPipe)
{
    OSStatus err = kVST_InterfaceNotReady;
    if ((pUSBDevice != NULL) && (pUSBDevice->listeningThread != NULL))
    {
        int nLockErr = 0;
        UInt32 nTryCt = 0;
        
        if (pUSBDevice->nError == kIOReturnNotResponding)
            return pUSBDevice->nError;
        
        do
        {
            nLockErr = pthread_mutex_trylock(&pUSBDevice->listeningMutex);
            if (nLockErr)
            {
                usleep(1000);
                ++nTryCt;
            }
        } while (nLockErr && nTryCt < 1100);
        
        err = nLockErr;
        if (nLockErr == 0)
        {
            err = noErr;
            if (!pUSBDevice->bUsesHID)
            {
                if ((pUSBDevice->hInterfaceInterface != NULL) && (pUSBDevice->pListenBuffer != NULL))
                {
                    pUSBDevice->nListenBufferBytes = 0L;
                    pUSBDevice->pListenBuffer[0] = 0;
                    if (DIAGNOSTICS_ON)
                        printf("VST_ClearInput: pUSBDevice->nBufferBytes is now 0\n");
                }
            }
            else
            {
                if (nPipe == kMeasurementPipe)
                {
                    pUSBDevice->nMeasurementBufferBytes = 0L;
                    pUSBDevice->pMeasurementBuffer[0] = 0;
                }
                else
                {
                    pUSBDevice->nCommandBufferBytes = 0L;
                    pUSBDevice->pCommandBuffer[0] = 0;
                }
            }
            pthread_mutex_unlock(&pUSBDevice->listeningMutex);
        }
    }
    
    return err;
}

OSStatus VST_SetTextMode(VST_USBBulkDevice * pUSBDevice, Boolean bOn)
{
    OSStatus err = kVST_InterfaceNotReady;
    if(pUSBDevice)
    {
        pUSBDevice->bTextMode = bOn;
        err = noErr;
    }
    return err;
}

OSStatus VST_ExpectSubPackets(VST_USBBulkDevice * pUSBDevice, UInt16 nPacketSize, UInt16 nValidBytes)
{
    OSStatus err = kVST_InterfaceNotReady;
    if(pUSBDevice)
    {
        if( nPacketSize <= pUSBDevice->nMaxInPacketSize &&
            nValidBytes <= nPacketSize)
        {
            pUSBDevice->nSubPacketSize = nPacketSize;
            pUSBDevice->nSubPacketValidBytes = nValidBytes;
            pUSBDevice->bUsingSubPackets = TRUE;
            err = noErr;
        }
        else
            err = kVST_UnknownError;
    }
    return err;
}

OSStatus VST_NoSubPackets(VST_USBBulkDevice * pUSBDevice)
{
    OSStatus err = kVST_InterfaceNotReady;
    if(pUSBDevice)
    {
        pUSBDevice->bUsingSubPackets = FALSE;
        err = noErr;
    }
    return err;
}

static void UsbCharactersToHostCharacters(UniChar* buf, int cnt)
{
	int i;
	for (i=0; i < cnt; ++i) 
		buf[i] = USBToHostWord(buf[i]);
}

static OSStatus local_GetStringDescriptor(VST_USBBulkDevice* pUSBDevice, UInt8 stringIndex, char* outBuffer, int maxLen)
{
	UInt8 buffer[258];
	IOUSBDevRequest request;
	request.bmRequestType = USBmakebmRequestType(kUSBIn, kUSBStandard, kUSBDevice);
	request.bRequest = kUSBRqGetDescriptor;
	request.wValue = (kUSBStringDesc << 8) | stringIndex;
	request.wIndex = 0x409;		// language
	request.wLength = 255;
	request.pData = buffer;
	
	OSStatus err = (OSStatus) (*pUSBDevice->hDeviceInterface)->DeviceRequest(pUSBDevice->hDeviceInterface, &request);
	if (err == kIOReturnSuccess && request.wLenDone > 0 && request.wLenDone <= sizeof(buffer))
	{
		if (0 < request.wLenDone && request.wLenDone <= sizeof(buffer))
		{
			// USB strings are Little-Endian so convert to Host-Endian
			// don't swap the byte the descriptor type (should always be 3), or the length
			UsbCharactersToHostCharacters(((UniChar*) buffer)+1, (request.wLenDone-2)/2);
			
			CFStringRef cfs = CFStringCreateWithCharacters(kCFAllocatorDefault, ((UniChar*) buffer)+1, (request.wLenDone-2)/2);
			CFStringGetCString(cfs, outBuffer, maxLen, kCFStringEncodingUTF8);
			CFRelease(cfs);
		}
	}
	
	return err;
}

OSStatus VST_GetManufactureString(VST_USBBulkDevice* pUSBDevice, char* outBuffer, int maxLen)
{
	outBuffer[0] = 0;
	OSStatus err = kVST_InterfaceNotReady;
	
    if (!pUSBDevice->bUsesHID && pUSBDevice && pUSBDevice->hDeviceInterface) 
    {
		UInt8 msi;
		err = (OSStatus) (*pUSBDevice->hDeviceInterface)->USBGetManufacturerStringIndex(pUSBDevice->hDeviceInterface, &msi);
		if (msi == 0) 
			return err;
		
		err = local_GetStringDescriptor(pUSBDevice, msi, outBuffer, maxLen); 
    } else if (pUSBDevice->bUsesHID && pUSBDevice->ioUSBDeviceRef) {
		CFMutableDictionaryRef properties = nil;
		kern_return_t result = IORegistryEntryCreateCFProperties(pUSBDevice->ioUSBDeviceRef, &properties, kCFAllocatorDefault, kNilOptions);
		if (result != noErr || properties == nil) 
			return result;
		
		CFStringRef cfs = CFDictionaryGetValue(properties, CFSTR(kIOHIDManufacturerKey));
		CFStringGetCString(cfs, outBuffer, maxLen, kCFStringEncodingUTF8);
		CFRelease(properties);
		err = noErr;
	}
    return err;
}

OSStatus VST_GetProductString(VST_USBBulkDevice* pUSBDevice, char* outBuffer, int maxLen)
{
	outBuffer[0] = 0;
	OSStatus err = kVST_InterfaceNotReady;
	
    if (!pUSBDevice->bUsesHID && pUSBDevice && pUSBDevice->hDeviceInterface) 
    {
		UInt8 msi;
		err = (OSStatus) (*pUSBDevice->hDeviceInterface)->USBGetProductStringIndex(pUSBDevice->hDeviceInterface, &msi);
		if (msi == 0) 
			return err;
		
		err = local_GetStringDescriptor(pUSBDevice, msi, outBuffer, maxLen); 
    } else if (pUSBDevice->bUsesHID && pUSBDevice->ioUSBDeviceRef) 
	{
		CFMutableDictionaryRef properties = nil;
		kern_return_t result = IORegistryEntryCreateCFProperties(pUSBDevice->ioUSBDeviceRef, &properties, kCFAllocatorDefault, kNilOptions);
		if (result != noErr || properties == nil) 
			return result;
		
		CFStringRef cfs = CFDictionaryGetValue(properties, CFSTR(kIOHIDProductKey));
		CFStringGetCString(cfs, outBuffer, maxLen, kCFStringEncodingUTF8);
		CFRelease(properties);
		err = noErr;
	}
    return err;
}

OSStatus VST_GetSerialNumberString(VST_USBBulkDevice* pUSBDevice, char* outBuffer, int maxLen)
{
	outBuffer[0] = 0;
    OSStatus err = kVST_InterfaceNotReady;
	
    if (!pUSBDevice->bUsesHID && pUSBDevice && pUSBDevice->hDeviceInterface) 
    {
		UInt8 sni;
		err = (OSStatus) (*pUSBDevice->hDeviceInterface)->USBGetSerialNumberStringIndex(pUSBDevice->hDeviceInterface, &sni);
		if (sni == 0) 
			return err;
		
		err = local_GetStringDescriptor(pUSBDevice, sni, outBuffer, maxLen); 
    } else if (pUSBDevice->bUsesHID && pUSBDevice->ioUSBDeviceRef) 
	{
		CFMutableDictionaryRef properties = nil;
		kern_return_t result = IORegistryEntryCreateCFProperties(pUSBDevice->ioUSBDeviceRef, &properties, kCFAllocatorDefault, kNilOptions);
		if (result != noErr || properties == nil) 
			return result;
		
		CFStringRef cfs = CFDictionaryGetValue(properties, CFSTR(kIOHIDSerialNumberKey));
		CFStringGetCString(cfs, outBuffer, maxLen, kCFStringEncodingUTF8);
		CFRelease(properties);
		err = noErr;
	}
    return err;
}

OSStatus VST_GetVersionNumberString(VST_USBBulkDevice* pUSBDevice, char* outBuffer, int maxLen)
{
	outBuffer[0] = 0;
    OSStatus err = kVST_InterfaceNotReady;
	
	// NOTE: there is no equivalent fucntionality for non-HID devices to retrieve a version number
	
    if (pUSBDevice->bUsesHID && pUSBDevice->ioUSBDeviceRef) 
	{
		CFMutableDictionaryRef properties = nil;
		kern_return_t result = IORegistryEntryCreateCFProperties(pUSBDevice->ioUSBDeviceRef, &properties, kCFAllocatorDefault, kNilOptions);
		if (result != noErr || properties == nil) 
			return result;
		
		SInt32 vers;
		CFNumberRef cfn = CFDictionaryGetValue(properties, CFSTR(kIOHIDVersionNumberKey));
		CFNumberType nt = CFNumberGetType(cfn);
		CFNumberGetValue(cfn, nt, &vers);
		CFRelease(properties);
		
//		snprintf(outBuffer, maxLen, "%d", (int) vers);
		snprintf(outBuffer, maxLen, "%d%d.%d%d", vers >> 12 & 0x0F, vers >> 8 & 0x0F, vers >> 4 & 0x0F, vers & 0x0F);	// TODO: this is same as the current Win code
		err = noErr;
	}
    return err;
}

Boolean VST_IsDeviceStillValid(VST_USBBulkDevice* pUSBDevice)
{
	// TODO: this may not the best possible test, but it seems to work
	
	Boolean valid = false;
	if (!pUSBDevice->bUsesHID && pUSBDevice && pUSBDevice->hDeviceInterface) 
	{
		UInt8 buffer[4];
		IOUSBDevRequest request;
		request.bmRequestType = USBmakebmRequestType(kUSBIn, kUSBStandard, kUSBDevice);
		request.bRequest = kUSBRqGetStatus;
		request.wValue = 0;
		request.wIndex = 0;
		request.wLength = 4;
		request.pData = buffer;
		
		OSStatus err = (OSStatus) (*pUSBDevice->hDeviceInterface)->DeviceRequest(pUSBDevice->hDeviceInterface, &request);
		valid = (err == kIOReturnSuccess);
	} else if (pUSBDevice->bUsesHID && pUSBDevice->ioUSBDeviceRef) 
	{
		// TODO: this does not work for checking HID device presence - always returns true...
		int busy;
		kern_return_t result = IOServiceGetBusyState(pUSBDevice->ioUSBDeviceRef, &busy);
		valid = (result == noErr);
	}
	return valid;
}


/********************************************************************************/
/********************************************************************************/

Boolean local_USBDeviceFromService(VST_USBBulkDevice * pUSBDevice, io_service_t ioSvc_usbDevice)
{
    /* Given a USB device io_service_t, fill out a VST_USBSpec.  Note that */
    /* we do not assume that we can open the device - it may be in use, */
    /* which is OK - but in this case many fields of the struct may be NULL. */
    
    /* If we do open the device, we do so only to fill out/verify some fields; */
    /* we do not leave the device in an open state on exit. */
    
    IOReturn 				err = 0;
    IOUSBDeviceInterface182 ** 		hIOUSBDevice = NULL;
    UInt8 				nNumConfigurations = 0;
    IOUSBConfigurationDescriptorPtr 	pUSBConfig = NULL;
    io_service_t			ioSvc_interface;
    int                                 nTry;
    
    if (!pUSBDevice)
        return false;
    
    /* Init out-spec so caller doesn't mistakenly think */
    /* fields are valid... */
    pUSBDevice->bUsesHID = false;
    pUSBDevice->bDeviceIsInUse = false;
    pUSBDevice->bUsesFTDISerial = false;
    pUSBDevice->bTextMode = false;
    pUSBDevice->nError = 0;
    pUSBDevice->ioUSBDeviceRef = ioSvc_usbDevice;
    pUSBDevice->pConfigurationDesc = 0;
    pUSBDevice->ioUSBInterfaceRef = 0;
    pUSBDevice->nControlPipeRef = pUSBDevice->nInPipeRef = pUSBDevice->nOutPipeRef = pUSBDevice->nInInterruptPipe = 0;
    pUSBDevice->nMaxInPacketSize = pUSBDevice->nMaxOutPacketSize = pUSBDevice->nMaxInInterruptPipePacketSize = 0;
    pUSBDevice->bUsingSubPackets = FALSE;
    pUSBDevice->pListenBuffer = NULL;
    pUSBDevice->hHIDInterface = NULL;
    pUSBDevice->hHIDInputInterface = NULL;
	pUSBDevice->nHIDInputCookie = kHIDUnspecifiedCookie;
	pUSBDevice->nHIDOutputCookie = kHIDUnspecifiedCookie;
	pUSBDevice->bWantsHIDInputFiltering = true;
    pUSBDevice->listeningThread = NULL;
	pUSBDevice->eBulkReadMode = kBulkReadDefault;
	pUSBDevice->bWaitOnClearBeforeWrite = false;
	pUSBDevice->pBulkCallback = NULL;
	pUSBDevice->pBulkCallbackContext = NULL;
	pUSBDevice->bGarminGPSProtocolSessionStarted = false;
    
    // Try getting a USB device
    hIOUSBDevice = local_GetUSBDeviceInterfaceFromService(ioSvc_usbDevice);
    if (hIOUSBDevice == NULL)
        return false;
    pUSBDevice->hDeviceInterface = hIOUSBDevice;

    /* try open the IOUSBDeviceInterface */
    err = (*hIOUSBDevice)->USBDeviceOpen(hIOUSBDevice);
    if (err == kIOReturnExclusiveAccess) // Exclusive access error
    {				     // Keep trying a few times
		NSLog(CFSTR("*** USBDeviceOpen Exclusive access error (0x%x). Will retry."), err);
        nTry = 0;
        while ((err == kIOReturnExclusiveAccess) && (nTry < 2))
        {
            sleep(1);
            err = (*hIOUSBDevice)->USBDeviceOpen(hIOUSBDevice);
            nTry++;
        }
    }
    
    if (err)
    {
		NSLog(CFSTR("*** Unable to open device err 0x%x."), err);
        if (err == kIOReturnBusy) // another app has this device
        {
            pUSBDevice->bDeviceIsInUse = true;
			NSLog(CFSTR("*** (Device is opened by other process?)"), err);
            return true; /* the spec is still good, it's just busy */
        }
        
        /* REVISIT - handle other errors? */
        return false;
    }
    
    /* This fixes a bug where a crash in LoggerPro could leave the device in a bad state */
    err = (*hIOUSBDevice)->ResetDevice(hIOUSBDevice);
	if (err != 0)
		NSLog(CFSTR("*** Unable to reset device err 0x%x."), err);
    
    /* Get number of configurations, then get IOUSBConfigurationDescriptorPtr for */
    /* the first one.  Note that we're not really handling multiple configurations. */
    err = (*hIOUSBDevice)->GetNumberOfConfigurations(hIOUSBDevice, &nNumConfigurations);
    if (err || !nNumConfigurations)
    {
		NSLog(CFSTR("*** Unable to obtain the number of configurations. 0x%x."), err);
        (*hIOUSBDevice)->USBDeviceClose(hIOUSBDevice);
        (*hIOUSBDevice)->Release(hIOUSBDevice);
		return false;
    }

    err = (*hIOUSBDevice)->GetConfigurationDescriptorPtr(hIOUSBDevice, 0, &pUSBConfig);			// get the first config desc (index 0)
    if (err || !pUSBConfig)
    {
		NSLog(CFSTR("*** Unable to get config descriptor for index 0. Error 0x%x. Closing device."), err);
        (*hIOUSBDevice)->USBDeviceClose(hIOUSBDevice);
        (*hIOUSBDevice)->Release(hIOUSBDevice);
		pUSBDevice->hDeviceInterface = 0;
        return false;
    }
    
    if(DIAGNOSTICS_ON)
    {
        printf("pUSBConfig fields:\n");
        printf("- bLength: %d\n", (int) pUSBConfig->bLength);
        printf("- bDescriptorType: %d\n", (int) pUSBConfig->bDescriptorType);
        printf("- wTotalLength: %d\n", (int) pUSBConfig->wTotalLength);
        printf("- bNumInterfaces: %d\n", (int) pUSBConfig->bNumInterfaces);
        printf("- bConfigurationValue: %d\n", (int) pUSBConfig->bConfigurationValue);
        printf("- iConfiguration: %d\n", (int) pUSBConfig->iConfiguration);
        printf("- bmAttributes: %d\n", (int) pUSBConfig->bmAttributes);
        printf("- MaxPower: %d\n", (int) pUSBConfig->MaxPower);
    }
        
    pUSBDevice->pConfigurationDesc = pUSBConfig;
    
    err = (*hIOUSBDevice)->SetConfiguration(hIOUSBDevice, pUSBConfig->bConfigurationValue);
    if (err)
    {
		NSLog(CFSTR("*** Unable to set configuration on device err 0x%x."), err);
        /* REVISIT - should we close & release hIOUSBDevice, & nullify fields in the spec? */
		return false;
    }

    /* Get the first interface iterator (this module does not handle multiple-interface devices...) */
    ioSvc_interface = local_GetFirstInterfaceServiceFromDevice(hIOUSBDevice);
    pUSBDevice->ioUSBInterfaceRef = ioSvc_interface;
    
    /* Get interface & pipe info... */
    local_GetInterfaceInfoForUSBBulkDevice(pUSBDevice);
    
    /* Don't leave the device open... */
    err = (*hIOUSBDevice)->USBDeviceClose(hIOUSBDevice);
    
    return true;
}

Boolean local_HIDDeviceFromService(VST_USBBulkDevice * pUSBDevice, io_service_t ioSvc_usbDevice)
{
    /* Given a USB device io_service_t, fill out a VST_USBSpec.  Note that */
    /* we do not assume that we can open the device - it may be in use, */
    /* which is OK - but in this case many fields of the struct may be NULL. */
    
    /* If we do open the device, we do so only to fill out/verify some fields; */
    /* we do not leave the device in an open state on exit. */
    IOHIDDeviceInterface ** 		hIOHIDDevice = NULL;
    
    if (!pUSBDevice)
        return false;
    
    // Initialize our parameter fields
    pUSBDevice->bUsesHID = true;
    pUSBDevice->bDeviceIsInUse = false;
    pUSBDevice->bUsesFTDISerial = false;
    pUSBDevice->bTextMode = false;
    pUSBDevice->nError = 0;
    pUSBDevice->ioUSBDeviceRef = ioSvc_usbDevice;
    pUSBDevice->pConfigurationDesc = 0;
    pUSBDevice->ioUSBInterfaceRef = 0;
    pUSBDevice->nControlPipeRef = pUSBDevice->nInPipeRef = pUSBDevice->nOutPipeRef = pUSBDevice->nInInterruptPipe = 0;
    pUSBDevice->nMaxInPacketSize = pUSBDevice->nMaxOutPacketSize = pUSBDevice->nMaxInInterruptPipePacketSize = 0;
    pUSBDevice->bUsingSubPackets = FALSE;
    pUSBDevice->pListenBuffer = NULL;
    pUSBDevice->hDeviceInterface = NULL;
    pUSBDevice->hHIDInterface = NULL;
    pUSBDevice->hHIDInputInterface = NULL;
    pUSBDevice->hHIDOutputInterface = NULL;
	pUSBDevice->nHIDInputCookie = kHIDUnspecifiedCookie;
	pUSBDevice->nHIDOutputCookie = kHIDUnspecifiedCookie;
	pUSBDevice->bWantsHIDInputFiltering = true;
    pUSBDevice->listeningThread = NULL;
    pUSBDevice->nLastNumMeasurementsInPacket = 0;
    pUSBDevice->nLastMeasurementRollingCounter = 0;
	pUSBDevice->eBulkReadMode = kBulkReadDefault;	
	pUSBDevice->bWaitOnClearBeforeWrite = false;
	pUSBDevice->pBulkCallback = NULL;
	pUSBDevice->pBulkCallbackContext = NULL;
	pUSBDevice->bGarminGPSProtocolSessionStarted = false;	
	
    // Try getting a HID device
    hIOHIDDevice = local_GetHIDDeviceInterfaceFromService(ioSvc_usbDevice);
    if (hIOHIDDevice == NULL)
        return false;

	local_EnumerateHIDCookies((IOHIDDeviceInterface122**) hIOHIDDevice, pUSBDevice);	// Fetch I/O element cookies from IO registry dictionary.
    pUSBDevice->hHIDInterface = hIOHIDDevice;
    
    return true;
}


Boolean local_GetInterfaceInfoForUSBBulkDevice(VST_USBBulkDevice * pOutDevice)
{
    IOReturn				err = 0;
    IOUSBInterfaceInterface182 **       hIOUSBInterface = 0;
    UInt8				nNumPipes = 0;
    Boolean 				bResult = false;
    
    if (!pOutDevice || !pOutDevice->ioUSBInterfaceRef)
        return bResult;
    
    hIOUSBInterface = local_GetInterfaceInterfaceFromService(pOutDevice->ioUSBInterfaceRef);
    
    if (!hIOUSBInterface)
    {
		NSLog(CFSTR("*** Failed to create bulk USB interface."));
		return bResult;
    }
    
    pOutDevice->hInterfaceInterface = hIOUSBInterface;
    err = (*hIOUSBInterface)->USBInterfaceOpen(hIOUSBInterface);
    if (err)
    {
		NSLog(CFSTR("*** Failed to open bulk USB interface. Err 0x%x"), err);
        (*hIOUSBInterface)->Release(hIOUSBInterface);
        pOutDevice->hInterfaceInterface = 0;
        pOutDevice->bDeviceIsInUse = true; // can't open here == in use?
		return bResult;
    }
    err = (*hIOUSBInterface)->GetNumEndpoints(hIOUSBInterface, &nNumPipes);
    if (err)
    {
		NSLog(CFSTR("*** Failed to open get num bulk USB endpoints. Err 0x%x"), err);
        (*hIOUSBInterface)->USBInterfaceClose(hIOUSBInterface);
        (*hIOUSBInterface)->Release(hIOUSBInterface);
		return bResult;
    }
    
    if(DIAGNOSTICS_ON)
        printf("VST_USB: found %d pipes for this interface\n", nNumPipes);
    pOutDevice->nNumPipes = nNumPipes;
    if (nNumPipes == 0)
    {
	/* Note that we don't mess with alternate settigns like the */
        /* example code - do we need to? */
        return bResult;
    }
    
    bResult = local_GetPipesInfoForUSBBulkDevice(pOutDevice);
    (*hIOUSBInterface)->USBInterfaceClose(hIOUSBInterface);
    (*hIOUSBInterface)->Release(hIOUSBInterface);
    pOutDevice->hInterfaceInterface = 0;
    return bResult;
}

Boolean local_GetPipesInfoForUSBBulkDevice(VST_USBBulkDevice * pOutDevice)
{
    int					i;
    IOReturn				err;			
    UInt8				direction, number, transferType, interval;
    UInt16				maxPacketSize;
    
    if(!pOutDevice || !pOutDevice->hInterfaceInterface)
        return false; /* bad params */
        
    for(i = 0; i < pOutDevice->nNumPipes + 1; i++) /* add 1 so we get control pipe at 0 + nNumPipes... */
    {
        err = (*(pOutDevice->hInterfaceInterface))->GetPipeProperties(pOutDevice->hInterfaceInterface, 
																		i, 
																		&direction, 
																		&number, 
																		&transferType, 
																		&maxPacketSize, 
																		&interval);
        if(err && DIAGNOSTICS_ON)
            printf("VST_USB - failed getting pipe properties for pipe %d\n", i);
        
        if (i == 0 && transferType == kUSBControl)
        {
            if(DIAGNOSTICS_ON)
            {
                printf("Found control pipe at index %d...\n", i);
                printf("Control pipe max packet size is %d\n", interval);
            }
            pOutDevice->nControlPipeRef = i;
        }
        else
        if(transferType == kUSBBulk)
        {
            if(direction == kUSBIn)
            {
                if(pOutDevice->nInPipeRef != 0) {
					if(DIAGNOSTICS_ON) 
						printf("Duplicate IN pipes found at %d and %d.\n", i, pOutDevice->nInPipeRef);
				}
                else
                {
                    pOutDevice->nInPipeRef = i;
                    pOutDevice->nMaxInPacketSize = maxPacketSize;
                    pOutDevice->nInPipeInterval = interval;
                    if(DIAGNOSTICS_ON)
                        printf("Found IN pipe at %d; maxPacketSize: %d; interval: %d\n", i, maxPacketSize, interval);
                }
            }
            else
            if(direction == kUSBOut)
            {
                if(pOutDevice->nOutPipeRef != 0) {
					if(DIAGNOSTICS_ON) 
						printf("Duplicate OUT pipes found at %d and %d.\n", i, pOutDevice->nOutPipeRef);
				}
                else
                {
                    pOutDevice->nOutPipeRef = i;
                    pOutDevice->nMaxOutPacketSize = maxPacketSize;
                    pOutDevice->nOutPipeInterval = interval;
                    if(DIAGNOSTICS_ON)
                        printf("Found OUT pipe at %d; maxPacketSize: %d; interval: %d\n", i, maxPacketSize, interval);
                }
            }
        }
        else if (transferType == kUSBInterrupt)
		{
			if (direction == kUSBIn)
			{
				if(pOutDevice->nInInterruptPipe != 0) {
					if(DIAGNOSTICS_ON) 
						printf("Duplicate IN Interrupt pipes found at %d and %d.\n", i, pOutDevice->nInInterruptPipe);
				}
				else
				{
					pOutDevice->nInInterruptPipe = i;
					pOutDevice->nMaxInInterruptPipePacketSize = maxPacketSize;
					pOutDevice->nInInterruptPipeInterval = interval;
				}
			}
		}
		else
        if(DIAGNOSTICS_ON)
            printf("Found non-control, non-bulk pipe at %d\n", i);
    }
    return true;
}

IOUSBDeviceInterface182 ** local_GetUSBDeviceInterfaceFromService(io_service_t ioSvc_usbDevice)
{
    // Get an IOUSBDeviceInterface ** object from an io_service_t for the USB device.
    IOReturn 				err = 0;
    IOCFPlugInInterface ** 		hIOCFPlugin = NULL;
    IOUSBDeviceInterface182 ** 		hIOUSBDevice = NULL;
    SInt32 				nScore;
    
    if (ioSvc_usbDevice != IO_OBJECT_NULL)
    {
        // Create plugin for our USB device service
        err = IOCreatePlugInInterfaceForService(ioSvc_usbDevice, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &hIOCFPlugin, &nScore);
        if (hIOCFPlugin != NULL)
        {
            err = (*hIOCFPlugin)->QueryInterface((void*)hIOCFPlugin, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID182), (LPVOID*)&hIOUSBDevice);	// was kIOUSBDeviceInterfaceID		sjs
            IODestroyPlugInInterface(hIOCFPlugin);	// Replaces: (*hIOCFPlugin)->Release(hIOCFPlugin);
            
            if ((err != noErr) && (hIOUSBDevice != NULL))
            {
				NSLog(CFSTR("*** (QueryInterface failed err 0x%x, dev=0x%x)"), err, hIOUSBDevice);
                // REVISIT: see if error is trivial?
            }
        }
    }
    
    return hIOUSBDevice;
}

void local_EnumerateHIDCookies(IOHIDDeviceInterface122** handle, VST_USBBulkDevice* pDev)
{
	CFTypeRef                               object;
	long                                    number;         
	IOHIDElementCookie                      cookie;
	unsigned int							cookie_l;
	long                                    usage;
	long                                    usagePage;
	unsigned long							size, repSize, repCount;
	char									name[128];
	CFArrayRef                              elements; //
	CFDictionaryRef                         element;
	bool									foundInput = false;
	bool									foundOutput = false;

	if (!handle || !(*handle)) 
		return;
	if (pDev->nHIDInputCookie != kHIDUnspecifiedCookie && pDev->nHIDOutputCookie != kHIDUnspecifiedCookie) 
		return;
	
	if ((*handle)->copyMatchingElements(handle, NULL, &elements) == kIOReturnSuccess)
	{		
               CFIndex i;
                for (i=0; i<CFArrayGetCount(elements); i++)
                {       
                        element = CFArrayGetValueAtIndex(elements, i);
						
                        // Get cookie
                        object = (CFDictionaryGetValue(element, CFSTR(kIOHIDElementCookieKey)));
                        if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID())
                            continue;
                        if(!CFNumberGetValue((CFNumberRef) object, kCFNumberLongType, &number))
							continue;
						cookie_l = number;
                        cookie = (IOHIDElementCookie) number;
 
                        // Get usage
                        object = CFDictionaryGetValue(element, CFSTR(kIOHIDElementUsageKey));
                        if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID())
                            continue;
                        if (!CFNumberGetValue((CFNumberRef) object, kCFNumberLongType, &number))
							continue;
                        usage = number;
 
                        // Get usage page
                        object = CFDictionaryGetValue(element, CFSTR(kIOHIDElementUsagePageKey));
                        if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID())
                            continue;
                        if (!CFNumberGetValue((CFNumberRef) object, kCFNumberLongType, &number))
							continue;
                        usagePage = number;
						
						// Get size
                        object = CFDictionaryGetValue(element, CFSTR(kIOHIDElementSizeKey));
                        if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID())
                            continue;
                        if (!CFNumberGetValue((CFNumberRef) object, kCFNumberLongType, &number))
							continue;
                        size = (unsigned int) number;
						
						// Get report size
                        object = CFDictionaryGetValue(element, CFSTR(kIOHIDElementReportSizeKey));
                        if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID()) {
//							printf("no report size for cookie: 0x%x\n", cookie_l);
                            continue;
						}
                        if (!CFNumberGetValue((CFNumberRef) object, kCFNumberLongType, &number))
							continue;
                        repSize = (unsigned int) number;
						
						// Get report count
                        object = CFDictionaryGetValue(element, CFSTR(kIOHIDElementReportCountKey));
                        if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID()) {
//							printf("no report count for cookie: 0x%x\n", cookie_l);
							continue;
						}
						if (!CFNumberGetValue((CFNumberRef) object, kCFNumberLongType, &number))
							continue;
                        repCount = (unsigned int) number;
						
						// name
						name[0] = 0;
                        object = CFDictionaryGetValue(element, CFSTR(kIOHIDElementNameKey));
                        if (object != 0 && CFGetTypeID(object) != CFStringGetTypeID()) 
							CFStringGetCString(object, name, 128, kCFStringEncodingUTF8);
						
						if (DIAGNOSTICS_ON)
							printf("cookie: 0x%x  usage: 0x%x  usagePage: 0x%x  size: 0x%x (0x%x/0x%x)\n", cookie_l, usage, usagePage, size, repSize, repCount);
						
						
						// NOTE: the following cookie recognition code works for GoTemp (and others) - it is not used for the CK spec		sjs
						
						// The reported elements seem to go like this, for report count=8, report size=8:
						//	cookie 0: ALL elements (i.e. 0-7) element size=64 <-- this is what we want for input queues and output transactions.
						//	cookie 1: element 0, element size=8
						//	cookie 2: element 1, element size=8
						//	...
						//	cookie 8: element 7, element size=8
						if (usagePage == 0x01 && usage == 0x46 && size == 64)
						{	// Our HID configuration is: 8 bytes input, 8 bytes output, 8 bytes LED (unused), so we know that 
							// the input element comes first: 
							if (!foundInput)
							{
								pDev->nHIDInputCookie = cookie;
								foundInput = true;
							}
							else if (!foundOutput)
							{
								pDev->nHIDOutputCookie = cookie;
								foundOutput = true;
							}
						}
                 }
	}
	CFRelease(elements);
	
	if (DIAGNOSTICS_ON)
		NSLog(CFSTR("### Cookies for this device are: input=0x%x, output=0x%x"), pDev->nHIDInputCookie, pDev->nHIDOutputCookie);
}

IOHIDDeviceInterface ** local_GetHIDDeviceInterfaceFromService(io_service_t ioSvc_usbDevice)
{
    // Get an IOUSBDeviceInterface ** object from an io_service_t for the USB device.
    IOReturn 				err = 0;
    IOCFPlugInInterface ** 		hIOCFPlugin = NULL;
    IOHIDDeviceInterface ** 		hIOHIDDevice = NULL;
    SInt32 				nScore;
    
    if (ioSvc_usbDevice != IO_OBJECT_NULL)
    {
        if (DIAGNOSTICS_ON)
        {
            io_name_t className;
            IOObjectGetClass(ioSvc_usbDevice, className);
            printf("Found device type %s\n", className);
        }
    
        // Create plugin for our USB device service
        err = IOCreatePlugInInterfaceForService(ioSvc_usbDevice, kIOHIDDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &hIOCFPlugin, &nScore);
        if (hIOCFPlugin != NULL)
        {
            if (DIAGNOSTICS_ON)
                printf("Got the plugin\n");
            err = (*hIOCFPlugin)->QueryInterface((void*)hIOCFPlugin, CFUUIDGetUUIDBytes(kIOHIDDeviceInterfaceID122), (LPVOID)&hIOHIDDevice);
            IODestroyPlugInInterface(hIOCFPlugin);	// Replaces: (*hIOCFPlugin)->Release(hIOCFPlugin);
            
            if ((err != noErr) && (hIOHIDDevice != NULL))
            {
				NSLog(CFSTR("*** QueryInterface(hid interface 122) failed err 0x%x"), err);
                // REVISIT: see if error is trivial?
            }
        }
        else
        if (DIAGNOSTICS_ON)
            printf("Couldn't even get a plugin\n");
    }
    return hIOHIDDevice;
}

io_service_t local_GetFirstInterfaceServiceFromDevice(IOUSBDeviceInterface182 ** hIOUSBDevice)
{
    IOUSBFindInterfaceRequest 		ioInterfaceRequest;
    io_iterator_t 			ioIterator = 0;
    io_service_t			ioSvc_interface = 0;
    IOReturn				err = 0;
    
    if(hIOUSBDevice == 0)
        return ioSvc_interface;
        
    // Make an IOInterfaceRequest to get an interface iterator...
    ioInterfaceRequest.bInterfaceClass = kIOUSBFindInterfaceDontCare; /* requested class */
    ioInterfaceRequest.bInterfaceSubClass = kIOUSBFindInterfaceDontCare; /* requested subclass */
    ioInterfaceRequest.bInterfaceProtocol = kIOUSBFindInterfaceDontCare; /*requested protocol */
    ioInterfaceRequest.bAlternateSetting = kIOUSBFindInterfaceDontCare; /* requested alt setting */
    
    err = (*hIOUSBDevice)->CreateInterfaceIterator(hIOUSBDevice, &ioInterfaceRequest, &ioIterator);
    if (err)
    {
        if(DIAGNOSTICS_ON)
            printf("Unable to create interface iterator\n");
	return 0;
    }
    
    ioSvc_interface = IOIteratorNext(ioIterator);
    IOObjectRelease(ioIterator);
    return ioSvc_interface;
}

IOUSBInterfaceInterface182  **	
local_GetInterfaceInterfaceFromService(io_service_t ioSvc_usbInterface)
{
    /* Given an io_service_t for a USB interface, get the IOUSBInterfaceInterface ** object... */
    IOReturn				err = 0;
    IOCFPlugInInterface **		hIOCFPlugin = 0;
    IOUSBInterfaceInterface182 **       hIOUSBInterface = 0;
    SInt32 				nScore = 0;

    /* Get CFPlugin interface to USB interface service... */
    err = IOCreatePlugInInterfaceForService(ioSvc_usbInterface, 
                                            kIOUSBInterfaceUserClientTypeID, 
                                            kIOCFPlugInInterfaceID, 
                                            &hIOCFPlugin, 
                                            &nScore);
    if (err || !hIOCFPlugin)
    {
        if(DIAGNOSTICS_ON)
            printf("Unable to create plugin. ret = %08x, iodev = %p\n", err, hIOCFPlugin);
	return hIOUSBInterface;
    }
    err = (*hIOCFPlugin)->QueryInterface( hIOCFPlugin, 
                                    CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID182), 
                                    (LPVOID*)&hIOUSBInterface);
	IODestroyPlugInInterface(hIOCFPlugin);	// Replaces: (*hIOCFPlugin)->Release(hIOCFPlugin);

    if(err)
    {
        if(hIOUSBInterface)
        {
            (*hIOUSBInterface)->Release(hIOUSBInterface);
            hIOUSBInterface = 0;
             /* REVISIT - if we get an err here but hIOUSBInterface is good, perhaps */
            /* we should look at the err to see if it's trivial, and return hIOUSBInterface? */
        }
    }
    
    return hIOUSBInterface;
}

/* These functions force the interface open for io, or closed: */
OSStatus local_OpenIO(VST_USBBulkDevice * pUSBDevice)
{
    IOReturn result;
    OSStatus err = 0;
    IOHIDDeviceInterface ** hIOHIDDevice;
    int nTry;
    
    if (!pUSBDevice)
        return kVST_paramErr;
    
    /* We must at least have the io_service_t for our USB device to open it... */
    if (!pUSBDevice->ioUSBDeviceRef)
        return kVST_InterfaceNotReady;
    
    if (!pUSBDevice->bUsesHID)
    {
        /* If we have a valid IOUSBInterfaceInterface, it should mean we're already open, */
        /* so only proceed if it's NULL... */
        if (pUSBDevice->hInterfaceInterface == NULL)
        {
            IOUSBDeviceInterface182 ** hInterface182 = nil;			// TODO: we probabably don't need to do this anymore	sjs
            
            /* Do we at least have our hDeviceInterface? */
            if (pUSBDevice->hDeviceInterface == NULL)
            {
                pUSBDevice->hDeviceInterface = local_GetUSBDeviceInterfaceFromService(pUSBDevice->ioUSBDeviceRef);
                if (pUSBDevice->hDeviceInterface == NULL)
                    return kVST_NullObjectError;
            }
            hInterface182 = (IOUSBDeviceInterface182 **) pUSBDevice->hDeviceInterface;
            
            /* open the device */
            err = (*hInterface182)->USBDeviceOpenSeize(pUSBDevice->hDeviceInterface);
            
            if (err == kIOReturnExclusiveAccess)
            {
                // whoever is holding the device might release it - give them 1 second
                sleep(1);
                err = (*hInterface182)->USBDeviceOpenSeize(pUSBDevice->hDeviceInterface);
            }
            
            if (err != noErr)
			{
				NSLog(CFSTR("*** HID Open Seize err 0x%x"), err);
                return err;
			}
            
            /* We also need our interface service... */
            if (!pUSBDevice->ioUSBInterfaceRef)
                pUSBDevice->ioUSBInterfaceRef = local_GetFirstInterfaceServiceFromDevice(pUSBDevice->hDeviceInterface);
            if (!pUSBDevice->hDeviceInterface)
                return kVST_NullObjectError;
                
            /* Now get the InterfaceInterface from the interface service... */
            pUSBDevice->hInterfaceInterface = local_GetInterfaceInterfaceFromService(pUSBDevice->ioUSBInterfaceRef);
            if (!pUSBDevice->hInterfaceInterface)
                return kVST_NullObjectError;
                
            err = (*(pUSBDevice->hInterfaceInterface))->USBInterfaceOpen(pUSBDevice->hInterfaceInterface);
            if (err)
            {
				NSLog(CFSTR("*** HID Interface Open err 0x%x"), err);
                local_CloseIO(pUSBDevice);
                return err;
            }
            
            /* make sure our pipe info is up-to-date */
            local_GetPipesInfoForUSBBulkDevice(pUSBDevice);
            
            /* If the device uses the FTDI USB <-> Serial Converter, then we need to send
            a series of control requests to put the device into the appropriate mode.  These requests
            were basically figured out by using a USB Bus Analyzer to sniff a terminal session between
            the Ohaus Scout Pro and ZTerm on OS X 10.3 */
            if (pUSBDevice->bUsesFTDISerial)
            {
                IOUSBDevRequest controlRequest;
                char buf[] = { 0 };
                controlRequest.bmRequestType = 0x40;
                controlRequest.wIndex = 0;
                controlRequest.wLength = 0;
                controlRequest.pData = buf;
            
                // Reset TX Buffer Request
                controlRequest.bRequest = 0;
                controlRequest.wValue = 0x2;
                controlRequest.wLenDone = 0;
                err = (*(pUSBDevice->hInterfaceInterface))->ControlRequest(pUSBDevice->hInterfaceInterface, 0, &controlRequest);
    
                // Reset RX Buffer Request
                controlRequest.bRequest = 0;
                controlRequest.wValue = 0x1;
                controlRequest.wLenDone = 0;
                err = (*(pUSBDevice->hInterfaceInterface))->ControlRequest(pUSBDevice->hInterfaceInterface, 0, &controlRequest);
                
                // Set DTR and RTS Request (Both High)
                controlRequest.bRequest = 1;
                controlRequest.wValue = (0x3 | (0x3 << 8));
                controlRequest.wLenDone = 0;
                err = (*(pUSBDevice->hInterfaceInterface))->ControlRequest(pUSBDevice->hInterfaceInterface, 0, &controlRequest);
    
                // Reset TX Buffer Request
                controlRequest.bRequest = 0;
                controlRequest.wValue = 0x2;
                controlRequest.wLenDone = 0;
                err = (*(pUSBDevice->hInterfaceInterface))->ControlRequest(pUSBDevice->hInterfaceInterface, 0, &controlRequest);
        
                // Reset RX Buffer Request
                controlRequest.bRequest = 0;
                controlRequest.wValue = 0x1;
                controlRequest.wLenDone = 0;
                err = (*(pUSBDevice->hInterfaceInterface))->ControlRequest(pUSBDevice->hInterfaceInterface, 0, &controlRequest);
    
                // Set Flow Control Request (Off)
                controlRequest.bRequest = 2;
                controlRequest.wValue = 0;
                controlRequest.wLenDone = 0;
                err = (*(pUSBDevice->hInterfaceInterface))->ControlRequest(pUSBDevice->hInterfaceInterface, 0, &controlRequest);
                
                // Set Baud Rate Request
                controlRequest.bRequest = 3;
                controlRequest.wValue = 0x4138;
                controlRequest.wLenDone = 0;
                err = (*(pUSBDevice->hInterfaceInterface))->ControlRequest(pUSBDevice->hInterfaceInterface, 0, &controlRequest);
                
                // Set Data Request
                controlRequest.bRequest = 4;
                controlRequest.wValue = 0x8;
                controlRequest.wLenDone = 0;
                err = (*(pUSBDevice->hInterfaceInterface))->ControlRequest(pUSBDevice->hInterfaceInterface, 0, &controlRequest);
                
                // Set Baud Rate Request
                controlRequest.bRequest = 3;
                controlRequest.wValue = 0xC04E;
                controlRequest.wLenDone = 0;
                err = (*(pUSBDevice->hInterfaceInterface))->ControlRequest(pUSBDevice->hInterfaceInterface, 0, &controlRequest);
                
                // Set Baud Rate Request
                controlRequest.bRequest = 3;
                controlRequest.wValue = 0x04E2;
                controlRequest.wLenDone = 0;
                err = (*(pUSBDevice->hInterfaceInterface))->ControlRequest(pUSBDevice->hInterfaceInterface, 0, &controlRequest);
        
                // Set Data Request
                controlRequest.bRequest = 4;
                controlRequest.wValue = 0x7;
                controlRequest.wLenDone = 0;
                err = (*(pUSBDevice->hInterfaceInterface))->ControlRequest(pUSBDevice->hInterfaceInterface, 0, &controlRequest);
            }
    
            /* start listening! */
			if (pUSBDevice->eBulkReadMode != kBulkReadDirect)
			{
				pUSBDevice->nListenBufferBytes = 0L;
				pUSBDevice->nListenBufferMaxBytes = 32768L;
				pUSBDevice->pListenBuffer = (unsigned char *) malloc(32768);
				if (!pUSBDevice->bUsesFTDISerial)
					pUSBDevice->nListenSleep = 1;
				else
					pUSBDevice->nListenSleep = 1000;
				err = (OSStatus) pthread_mutex_init(&pUSBDevice->listeningMutex, NULL);
				
				err = (OSStatus) pthread_create(&pUSBDevice->listeningThread, 
											NULL, /* default attributes */
											(void* (*)(void*)) pUSBDevice->eBulkReadMode == kBulkReadGarminGPSProtocol 
																? local_garmin_protocol_thread : local_input_listen_thread, /* function */
											(void*)pUSBDevice); /* param to function */
				if (DIAGNOSTICS_ON)
					printf("local_OpenIO created listen thread for 0x%x\n", (unsigned) pUSBDevice);
											
			}
			else if (DIAGNOSTICS_ON)
				printf("local_OpenIO did NOT create a thread (bUsesDirectRead=true), 0x%x\n", (unsigned)pUSBDevice);

        }
    }
    else
    {
        // Make sure we have a valid HID device
        if (pUSBDevice->hHIDInterface == NULL)
        {
            pUSBDevice->hHIDInterface = local_GetHIDDeviceInterfaceFromService(pUSBDevice->ioUSBDeviceRef);
            if (pUSBDevice->hHIDInterface == NULL)
                return kVST_NullObjectError;
			local_EnumerateHIDCookies((IOHIDDeviceInterface122**) pUSBDevice->hHIDInterface, pUSBDevice);
        }
        
        hIOHIDDevice = pUSBDevice->hHIDInterface;
        err = (*hIOHIDDevice)->open(hIOHIDDevice, 1);
		if (err != 0)
			err = (*hIOHIDDevice)->open(hIOHIDDevice, 0);
        if (err == kIOReturnExclusiveAccess) // Exclusive access error
        {                                    // Keep trying a few times
            nTry = 0;
            while ((err == kIOReturnExclusiveAccess) && (nTry < 2))
            {
				NSLog(CFSTR("*** HID Device Exclusive access err 0x%x. Will retry."), err);
				sleep(1);
                err = (*hIOHIDDevice)->open(hIOHIDDevice, 0);
                nTry++;
            }
			if (err != 0)
			{
				NSLog(CFSTR("*** HID Device Open err 0x%x. Exit."), err);
				return err;
			}
        }
        
        // **** Create input pipe
        // Allocate an element queue
        pUSBDevice->hHIDInputInterface = (*hIOHIDDevice)->allocQueue(hIOHIDDevice);
        if (pUSBDevice->hHIDInputInterface == NULL)
		{
			NSLog(CFSTR("*** HID allocQueue failed."));
            return kVST_NullObjectError;
		}
        
        // Initialize the element queue
        result = (*pUSBDevice->hHIDInputInterface)->create(pUSBDevice->hHIDInputInterface, 0, 100 /* Queue size */);
        if (result != kIOReturnSuccess)
        {
			NSLog(CFSTR("*** HID create failed, err 0x%x"), result);
            return kVST_NullObjectError;
        }
        
        // Stop the queue
        result = (*pUSBDevice->hHIDInputInterface)->stop(pUSBDevice->hHIDInputInterface);
        if ((result != kIOReturnSuccess) && (DIAGNOSTICS_ON))
            printf("local_OpenIO: Couldn't stop the HID queue\n");
        
        // Add the cookies we're interested in to the queue
        result = (*pUSBDevice->hHIDInputInterface)->addElement(pUSBDevice->hHIDInputInterface, pUSBDevice->nHIDInputCookie, 0);
        if (result != kIOReturnSuccess)
        {
			NSLog(CFSTR("*** local_OpenIO() (*pUSBDevice->hHIDInputInterface)->addElement failed w/ 0x%x."), result, pUSBDevice->nHIDInputCookie);
            return kVST_NullObjectError;
        } else if (DIAGNOSTICS_ON) 
			printf("local_OpenIO: addElement to input queue cookie: 0x%x\n", pUSBDevice->nHIDInputCookie);
			
#if !VST_USE_SET_REPORT_IMP        
        // **** Create output pipe
        // Allocate a transaction
        pUSBDevice->hHIDOutputInterface = (*hIOHIDDevice)->allocOutputTransaction(hIOHIDDevice);
        if (pUSBDevice->hHIDOutputInterface == NULL)
            return kVST_NullObjectError;
            
        // Initialize the transaction
        result = (*pUSBDevice->hHIDOutputInterface)->create(pUSBDevice->hHIDOutputInterface);
        if (result != kIOReturnSuccess)
        {
            if (DIAGNOSTICS_ON)
                printf("local_OpenIO: Couldn't create HID transaction\n");
            return kVST_NullObjectError;
        }
        
        // Add the cookies we're interested in to the transaction
        result = (*pUSBDevice->hHIDOutputInterface)->addElement(pUSBDevice->hHIDOutputInterface, pUSBDevice->nHIDOutputCookie);
        if (result != kIOReturnSuccess)
        {
            if (DIAGNOSTICS_ON)
				NSLog(CFSTR("*** local_OpenIO() (*pUSBDevice->hHIDOutputInterface)->addElement failed w/ 0x%x, cookie=0x%x. Barfage."), result, pUSBDevice->nHIDOutputCookie);
            return kVST_NullObjectError;
        }
        else
		{
			if (DIAGNOSTICS_ON) 
				printf("local_OpenIO: addElement to output transaction cookie: 0x%x\n", pUSBDevice->nHIDOutputCookie);
		}
#endif        
        result = (*pUSBDevice->hHIDInputInterface)->createAsyncEventSource(pUSBDevice->hHIDInputInterface, &pUSBDevice->hHIDRunLoopSource);
        if (result != kIOReturnSuccess)
        {
            if (DIAGNOSTICS_ON)
                printf("local_OpenIO: Couldn't create HID event source\n");
            return kVST_NullObjectError;
        }
        
        result = (*pUSBDevice->hHIDInputInterface)->setEventCallout(pUSBDevice->hHIDInputInterface, local_input_hid_callback, pUSBDevice, pUSBDevice);
        if (result != kIOReturnSuccess)
        {
            if (DIAGNOSTICS_ON)
                printf("local_OpenIO: Couldn't register callback source\n");
            return kVST_NullObjectError;
        }
                
        // Start the queue
        result = (*pUSBDevice->hHIDInputInterface)->start(pUSBDevice->hHIDInputInterface);
        if ((result != kIOReturnSuccess) && (DIAGNOSTICS_ON))
            printf("local_OpenIO: Couldn't start the HID queue\n");

        // Allocate message buffers
        pUSBDevice->nMeasurementBufferBytes = 0L;
        pUSBDevice->nMeasurementBufferMaxBytes = 8192L;
        pUSBDevice->pMeasurementBuffer = (unsigned char *) malloc(8192);
        
        pUSBDevice->nCommandBufferBytes = 0L;
        pUSBDevice->nCommandBufferMaxBytes = 4096L;
        pUSBDevice->pCommandBuffer = (unsigned char *) malloc(4096);
        
        // Start listening
        pUSBDevice->nListenSleep = 1;
        err = (OSStatus) pthread_mutex_init(&(pUSBDevice->listeningMutex), NULL);
        err = (OSStatus) pthread_create(&(pUSBDevice->listeningThread), NULL, // default attributes
                                        (void* (*)(void*))local_hid_listen_thread, // function
                                        (void*)pUSBDevice); // param to function
    }
    
	if (DIAGNOSTICS_ON) 
		NSLog(CFSTR("*** local_OpenIO() made it to end okay. err = 0x%x"), err);
    return err;
}

OSStatus local_CloseIO(VST_USBBulkDevice * pUSBDevice)
{
    int nLockErr = 0;
    int nCt = 0;
    int nThreadErr = 0;
    bool bCleanupThread = false;

    if (pUSBDevice == NULL)
        return kVST_paramErr;
    
    // Stop the listening thread
    if (pUSBDevice->listeningThread != NULL)
    {
		// HID listening threads need to have their run loop stopped.
		if ((pUSBDevice->bUsesHID) && (pUSBDevice->hHIDRunLoop != NULL))
			CFRunLoopStop(pUSBDevice->hHIDRunLoop);
	
        nThreadErr = pthread_cancel(pUSBDevice->listeningThread);
        if (nThreadErr != ESRCH) // thread is still running, lock mutex...
        {
            do
            {
                nLockErr = pthread_mutex_trylock(&(pUSBDevice->listeningMutex));
                if (nLockErr)
                {
                    nCt++;
                    usleep(10000);
                    if (nCt > 100)
                        break; /* cant lock! maybe we never init'd the mutex? */
                }
            } while(nLockErr);
        }
        
        bCleanupThread = (nLockErr == 0);
    }
    
    if (bCleanupThread)
    {
        pUSBDevice->listeningThread = NULL;
        pUSBDevice->nListenSleep = 0L;
    
        // Kill the mutex
        pthread_mutex_unlock(&(pUSBDevice->listeningMutex));
        pthread_mutex_destroy(&(pUSBDevice->listeningMutex));
        
        // Destroy the buffers
        if (!pUSBDevice->bUsesHID)
        {
            if (pUSBDevice->pListenBuffer != NULL)
            {
                free(pUSBDevice->pListenBuffer);
                pUSBDevice->pListenBuffer = NULL;
                pUSBDevice->nListenBufferMaxBytes = pUSBDevice->nListenBufferBytes = 0L;
            }
            else
            if (DIAGNOSTICS_ON)
                printf("pUSBDevice->pListenBuffer was already NULL\n");
        }
        else
        {
            if (pUSBDevice->pMeasurementBuffer != NULL)
            {
                free(pUSBDevice->pMeasurementBuffer);
                pUSBDevice->pMeasurementBuffer = NULL;
                pUSBDevice->nMeasurementBufferMaxBytes = pUSBDevice->nMeasurementBufferBytes = 0L;
            }
            else
            if (DIAGNOSTICS_ON)
                printf("pUSBDevice->pMeasurementBuffer was already NULL\n");
                
            if (pUSBDevice->pCommandBuffer != NULL)
            {
                free(pUSBDevice->pCommandBuffer);
                pUSBDevice->pCommandBuffer = NULL;
                pUSBDevice->nCommandBufferMaxBytes = pUSBDevice->nCommandBufferBytes = 0L;
            }
            else
            if (DIAGNOSTICS_ON)
                printf("pUSBDevice->pCommandBuffer was already NULL\n");
        }
    }
    
    if (!pUSBDevice->bUsesHID)
    {
        // Release and invalidate the interface object; close the device object
        if (pUSBDevice->hInterfaceInterface != NULL)
        {
            (*pUSBDevice->hInterfaceInterface)->USBInterfaceClose(pUSBDevice->hInterfaceInterface);
            (*pUSBDevice->hInterfaceInterface)->Release(pUSBDevice->hInterfaceInterface);
            pUSBDevice->hInterfaceInterface = NULL;
        }
        else
        if (DIAGNOSTICS_ON)
            printf("pUSBDevice->hInterfaceInterface was already NULL\n");
    
        if (pUSBDevice->ioUSBInterfaceRef != IO_OBJECT_NULL)
        {
            IOObjectRelease(pUSBDevice->ioUSBInterfaceRef);
            pUSBDevice->ioUSBInterfaceRef = IO_OBJECT_NULL;
        }
    
        if (pUSBDevice->hDeviceInterface != NULL)
            (*pUSBDevice->hDeviceInterface)->USBDeviceClose(pUSBDevice->hDeviceInterface);
        else
        if (DIAGNOSTICS_ON)
            printf("pUSBDevice->hDeviceInterface was already NULL\n");
    }
    else
    {
        // Destroy our queue object
        if (pUSBDevice->hHIDInputInterface != NULL)
        {
            (*pUSBDevice->hHIDInputInterface)->stop(pUSBDevice->hHIDInputInterface);
            (*pUSBDevice->hHIDInputInterface)->dispose(pUSBDevice->hHIDInputInterface);
            (*pUSBDevice->hHIDInputInterface)->Release(pUSBDevice->hHIDInputInterface);
            pUSBDevice->hHIDInputInterface = NULL;
        }
    
        // Release and close the HID device object
        if (pUSBDevice->hHIDInterface != NULL)
        {
            (*pUSBDevice->hHIDInterface)->close(pUSBDevice->hHIDInterface);
            (*pUSBDevice->hHIDInterface)->Release(pUSBDevice->hHIDInterface);
            pUSBDevice->hHIDInterface = NULL;
        }
    }
    
    return 0;
}

OSStatus local_ReadBytes(VST_USBBulkDevice * pUSBDevice,
                        void * pBuffer,
                        UInt32 * pnMaxWanted,
						UInt8 nReadPipe)
{
    OSStatus err = 0;
    
    if (!pBuffer || !pnMaxWanted)
        return kVST_paramErr;
        
    if (!pUSBDevice || !pUSBDevice->hInterfaceInterface || !pUSBDevice->nInPipeRef)
        return kVST_InterfaceNotReady;
        
    if (pUSBDevice->nError == kIOReturnNotResponding)
	{
		NSLog(CFSTR("===LISTEN=== Read Pipe is kIOReturnNotResponding"));
        return pUSBDevice->nError;
	}

    err = (OSStatus) (*pUSBDevice->hInterfaceInterface)->ReadPipe(pUSBDevice->hInterfaceInterface, nReadPipe, pBuffer, pnMaxWanted);                                            
    if (err == kIOUSBTransactionTimeout)
    {
        *pnMaxWanted = 0L;
        err = (*pUSBDevice->hInterfaceInterface)->ResetPipe(pUSBDevice->hInterfaceInterface, nReadPipe);
		NSLog(CFSTR("===LISTEN=== Read Pipe is kIOUSBTransactionTimeout"));                                                                        
    }
    else
    if (err == kIOUSBPipeStalled)
    {
        *pnMaxWanted = 0L;
        /* {
            IOUSBDevRequest	myRequest;
            
            myRequest.bRequest = kUSBRqClearFeature;
            myRequest.bmRequestType = (UInt8)(kUSBOut + kUSBStandard + kUSBEndpoint);
            myRequest.wValue = kUSBFeatureEndpointStall;
            myRequest.wIndex = pUSBDevice->nInPipeRef;
            myRequest.wLength = 0;
            myRequest.pData = nil;
            myRequest.wLenDone = 0;
            
            err = (*pUSBDevice->hDeviceInterface)->DeviceRequest(pUSBDevice->hDeviceInterface, &myRequest);
        } */
    
        err = (*pUSBDevice->hInterfaceInterface)->ClearPipeStall(pUSBDevice->hInterfaceInterface, nReadPipe);
		NSLog(CFSTR("===LISTEN=== Read Pipe is stalled. ClearPipeStall issued."));                                                                        
    }
    else
    {
        if (err == kIOReturnNotResponding) // DEVICE MAY HAVE BEEN UNPLUGGED!
        {
            pUSBDevice->nError = err;
			NSLog(CFSTR("===LISTEN=== device is not responding, thread will exit..."));
        }
    }
        
    return err;
}


OSStatus local_ReadBytesDirect(VST_USBBulkDevice * pUSBDevice, void* pBuffer, UInt32 * pnMaxWanted, UInt32 nMaxTimeoutMS, int nPipe)
{
	// TODO GSpam Revisit -- nMaxTimeoutMS is un-implemented at this level. 
    OSStatus err = 0;
	UInt32 retryCount = 0;
	
	if (nPipe == -1) 
		nPipe = pUSBDevice->nInPipeRef;
	
    if (!pBuffer || !pnMaxWanted)
        return kVST_paramErr;
        
    if (!pUSBDevice || !pUSBDevice->hInterfaceInterface || !nPipe)
        return kVST_InterfaceNotReady;
        
    if (pUSBDevice->nError == kIOReturnNotResponding)
        return pUSBDevice->nError;

	// Certain TI calculators are prone not to service read requests if they are busy, and hence
	// we receive kIOReturnNotResponding for plugged in devices, even when we are expecting data 
	// from them. So we'll sleep and try again a couple of times until we're sure the device has
	// actually gone away, i.e. was unplugged.
	while (true)
	{
		err = (OSStatus) (*pUSBDevice->hInterfaceInterface)->ReadPipe(pUSBDevice->hInterfaceInterface, nPipe, pBuffer, pnMaxWanted);                                            

		if (err != kIOReturnNotResponding)
			break;
			
		if (retryCount++ >= 10)
			break;

		usleep(2000);
	}
	
    if (err == kIOUSBTransactionTimeout)
    {
		NSLog(CFSTR("===LISTEN=== Read Pipe Direct is kIOUSBTransactionTimeout. Resetting."));  
        *pnMaxWanted = 0L;
        err = (*pUSBDevice->hInterfaceInterface)->ResetPipe(pUSBDevice->hInterfaceInterface, nPipe);
    }
    else
    if (err == kIOUSBPipeStalled)
    {
        *pnMaxWanted = 0L;

        err = (*pUSBDevice->hInterfaceInterface)->ClearPipeStall(pUSBDevice->hInterfaceInterface, nPipe);
		NSLog(CFSTR("===LISTEN=== Read Pipe Direct is kIOUSBPipeStalled."));  
    }
    else
    {
        if (err == kIOReturnNotResponding) // DEVICE MAY HAVE BEEN UNPLUGGED!
        {
            pUSBDevice->nError = err;
        }
		if (err != noErr) 
			NSLog(CFSTR("===LISTEN=== Read Pipe Direct failed err 0x%x"), err);  
    }
	
    return err;
}

void local_input_listen_thread(VST_USBBulkDevice * pTheDevice)
{
    OSStatus err = 0L;
    
    struct sched_param sp;

    unsigned char bytes[65]; /* make bigger if we ever support > 64-byte packet devices! */
    unsigned char * pBytes = NULL;
    UInt32 nNumBytes = 0L;
    size_t ix;
    Boolean bRun = true;
    
    // Sub packet variables
    unsigned char * pSrcChar;
    unsigned char * pWriteChar;
    int nSubPacketIndex;
    int nNumSubPackets;
    UInt32 nNewNumBytes;
    int nByte;
    Boolean bZeroPacket;

    if (pTheDevice == NULL)
        return;
    
	if (pTheDevice->eBulkReadMode == kBulkReadLengthPacketThreaded)
	{	// Before we do anything, reset read pipe -- if we're in this particular buld read mode that is.
//		NSLog(CFSTR("### --- about to read any pending data before we begin operations."));
//		char tBuf[64];
//		UInt32 len = 64;
//		while (local_ReadBytes(pTheDevice,tBuf, &len) != kIOReturnSuccess)
//		{
//			NSLog(CFSTR("### ---\tdumped %u bytes."), len);
//			len = 64;
//		}
//		NSLog(CFSTR("### --- dumped pending data."));
	}
	// Clear the flag -- this allows writes to go proceed.
	pTheDevice->bWaitOnClearBeforeWrite = false;
	
    // Initialize scheduler policy for this thread
    memset(&sp, 0, sizeof(struct sched_param));
    sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
    if ((pthread_setschedparam(pthread_self(), SCHED_RR, &sp) != 0) && DIAGNOSTICS_ON)
        printf("Failed to change priority.\n");
    
    while (bRun)
    {
        // Thread will exit if interface is closed, etc.
        if ((pTheDevice->hInterfaceInterface != NULL) && (pTheDevice->nListenBufferMaxBytes > 0))
        {
			if (pTheDevice->eBulkReadMode == kBulkReadSmallPacketThreaded)
			{
				// Try to read one packet from the device
				nNumBytes = (UInt32) pTheDevice->nMaxInPacketSize;
				bytes[0] = 0;
				bytes[64] = 0;
				err = local_ReadBytes(pTheDevice, bytes, &nNumBytes, pTheDevice->nInPipeRef);
				pBytes = &bytes[0];
			}
			else if (pTheDevice->eBulkReadMode == kBulkReadLengthPacketThreaded)
			{
				// Read a length packet first, then attempt to read the amount of data specified by the length packet.
				GDeviceToHostUSBBulkPipePacketHeader header;
				UInt32 headerSize = sizeof(GDeviceToHostUSBBulkPipePacketHeader);
				err = local_ReadBytes(pTheDevice, &header, &headerSize, pTheDevice->nInPipeRef);
				if (err == 0 && headerSize == sizeof(GDeviceToHostUSBBulkPipePacketHeader))
				{
					// Grab payload length from the packet header
					UInt32 payloadLength;
					local_ConvertBytesToInt(header.lsbyteLswordPayloadLength, header.msbyteLswordPayloadLength,
						header.lsbyteMswordPayloadLength, header.msbyteMswordPayloadLength, (int *) &payloadLength);
					
					// Init our data buffer to the max size.
					if (pBytes == NULL)
						pBytes = malloc(DEVICE_TO_HOST_USB_BULK_MAX_PACKET_PAYLOAD_LENGTH);
					
					nNumBytes = 0;
					
					// Now loop until we get a payloadLength's worth of data in the buffer:
					while (err == kIOReturnSuccess && nNumBytes < payloadLength)
					{
						UInt32 bytesRemaining = payloadLength - nNumBytes;
						UInt32 bytesRead = bytesRemaining;
						err = local_ReadBytes(pTheDevice, &pBytes[nNumBytes], &bytesRead, pTheDevice->nInPipeRef);
						if (err == kIOReturnSuccess)
							nNumBytes += bytesRead;
					}
				}
				else
				{
					NSLog(CFSTR("*** ===LISTEN=== length packet read: err 0x%x, %u of %u bytes."), err, headerSize, sizeof(GDeviceToHostUSBBulkPipePacketHeader));
					if (err == 0)
						err = 0xDEADA55E; // At least have result codes which are memorable.
				}
			}
			
			// We've read some data which is pointed to by pBytes and whose length is nNumBytes. We now process it somehow.
            if ((err == 0) || (err == kIOUSBTransactionTimeout))
            {
                if (pTheDevice->bUsesFTDISerial)
                {
                    // If the device uses FTDI's USB <-> Serial Converter, then the first two bytes
                    // of every message are status.  We can safely strip them off.
                    pBytes = NULL;
                    if (nNumBytes > 2)
                    {
                        nNumBytes -= 2;
                        pBytes = &bytes[2];	// Don't mix and match FTDI and LabQuest ;-)
                    }
                }
				
            	if ((pBytes != NULL) && (nNumBytes > 0))
            	{
            		if (DIAGNOSTICS_ON)
                    {
                        NSLog(CFSTR("===LISTEN=== bytes (%u): "), nNumBytes);
                        for (ix = 0; ix < nNumBytes; ix++)
                            printf("%02x ", pBytes[ix]);
                        printf("\n");
                    }    
        

                    // We read in a packet
					if (pTheDevice->pBulkCallback != NULL)
					{	// Client has specified a bulk callback, so we'll send the data to the callback instead of buffering it.
						long nErr = pTheDevice->pBulkCallback(pBytes, nNumBytes, pTheDevice->pBulkCallbackContext);
						if (nErr != 0)
							NSLog(CFSTR("*** ===LISTEN=== bulk callback returned err 0x%x. Logging and continuing. Please fix."), nErr);
					}
                    else if (pTheDevice->nListenBufferBytes < pTheDevice->nListenBufferMaxBytes - nNumBytes)
                    {	// See if we can put if in the buffer
                        if (pTheDevice->bUsesFTDISerial)
                            ;
                        else
                        if (pTheDevice->bTextMode)
                        {
                            // If we're in text mode, decrement nBytesToCopy as long as buffer
                            // contains trailing NULL characters...
                            while (nNumBytes && !(pBytes[nNumBytes-1]))
                                nNumBytes--;
                        }
                        else
                        if (pTheDevice->bUsingSubPackets && pTheDevice->nSubPacketSize && pTheDevice->nSubPacketValidBytes)
                        { // otherwise, if we're using subpackets, discared invalid bytes...
                            pSrcChar = pBytes;
                            pWriteChar = pBytes;
                            nSubPacketIndex = 0;
                            nNumSubPackets = (nNumBytes / pTheDevice->nSubPacketSize);
                            nNewNumBytes = 0L;
                            bZeroPacket = true;
                            
                            for (nSubPacketIndex = 0; nSubPacketIndex < nNumSubPackets; nSubPacketIndex++)
                            {
                                bZeroPacket = true;
                                for (nByte = 0; nByte < pTheDevice->nSubPacketValidBytes; nByte++)
                                {
                                    *pWriteChar = *pSrcChar;
                                    if (*pWriteChar)
                                        bZeroPacket = false;
                                    nNewNumBytes++;
                                    pWriteChar++;
                                    pSrcChar++;
                                }
                                nSubPacketIndex++;
                                
                                // Advance pSrcChar to next valid-packet start...
                                pSrcChar += (pTheDevice->nSubPacketSize - pTheDevice->nSubPacketValidBytes);
                                
                                // If this was a zero-packet, discard (decrement write-char and nNewNumBytes)
                                if (bZeroPacket)
                                {
                                    pWriteChar -= pTheDevice->nSubPacketValidBytes;
                                    nNewNumBytes -= pTheDevice->nSubPacketValidBytes;
                                }
                            }
                            
                            nNumBytes = nNewNumBytes;
                        }
                        
                        if (nNumBytes > 0)
                        {
		                    pthread_mutex_lock(&pTheDevice->listeningMutex);
                        
                            memmove(pTheDevice->pListenBuffer + pTheDevice->nListenBufferBytes, pBytes, nNumBytes);
                            pTheDevice->nListenBufferBytes += nNumBytes;
                            
                            pthread_mutex_unlock(&(pTheDevice->listeningMutex));
                            
                            if (DIAGNOSTICS_ON)
                            {
                                printf("===LISTEN=== copied bytes to listenBuffer\n");
                                printf("===LISTEN=== pTheDevice->nListenBufferBytes is now %ld\n", pTheDevice->nListenBufferBytes);
                            }
                        }
                    }
                    else
                    {
                        /* ERROR!  listen-buffer was allowed to overflow! */
						NSLog(CFSTR("*** ===LISTEN=== USB Read Thread: Error! buffer overflow"));
                        err = pTheDevice->nError = kVST_InputBufferOverflow;
                    }
                }
            }
			// LabPro surprise-unplugs from a hub seem to return kIOReturnNoDevice instead of
			// kIOReturnNotResponding which causes this thread basically to busy-poll off the 
			// broken endpoint. Not good. Instead, watch for both errors, and furthermore,
			// watch for the non-family specific generic error codes.
            else	
            if (	err == kIOReturnNotResponding   ||			// common IOKit "not responding"
					err == kIOReturnNoDevice		||			// common IOKit "no such device"
					err_get_code(err) == 0x2ed		||			// generic "not responding"
					err_get_code(err) == 0x2c0 )				// generic "no such device"
            {
				// exit listening thread - device is no longer responding
				NSLog(CFSTR("*** local_input_listen_thread got err 0x%x. Device is not responding. Has device been unplugged? Bailing."), err);
				pTheDevice->nError = kIOReturnNotResponding;
				pthread_exit(NULL);
            }
            else
                 NSLog(CFSTR("*** ===LISTEN=== local_input_listen_thread(), local_ReadBytes() error: 0x%x"), err);
                
            usleep(pTheDevice->nListenSleep);
            pthread_testcancel();
        }
        else
            bRun = false;
    }
	// Clean up our buffer.
	if (pTheDevice->eBulkReadMode == kBulkReadLengthPacketThreaded && pBytes != NULL)
		free(pBytes);
}

void local_garmin_protocol_thread(VST_USBBulkDevice * pTheDevice)	// This thread is used by garmin protocol. Oooh. Special.
{
	// For the first pass, I am going to make the assumption that data on both input pipes will be syncrhnonous in nature:
	// I.e. we get a Data Available packet on the interrupt pipe, and we do not get another one until after we read all the
	// bulk data. If they are asyncrhonous, then I'll have to re-think this.			
	#define Pid_Data_Available		2
	#define Pid_Session_Started		6
	// We assume that someone else will have issued a Start Session packet.
	unsigned char* pProtocolBuffer = malloc(pTheDevice->nMaxInInterruptPipePacketSize);
	OSStatus nErr = kIOReturnSuccess;
	while (true)
	{	// First block on reading from the interrupt pipe -- this will tell us if we've got data on the bulk pipe.
		UInt32 nBufferLen = pTheDevice->nMaxInInterruptPipePacketSize;
		nErr = local_ReadBytes(pTheDevice, pProtocolBuffer, &nBufferLen, pTheDevice->nInInterruptPipe);
		if (nErr == kIOReturnSuccess || nErr == kIOUSBTransactionTimeout)
		{
			if (pProtocolBuffer[0] == 0)	// Aka protocol layer -- the only thing which should arrive over the interrupt pipe, mind you.
			{	// Get the packet ID which tells us what to do with the packet.
				
				unsigned short packetID;
				local_ConvertByteToShort(pProtocolBuffer[4], pProtocolBuffer[5], (short*) &packetID);	// Make assumption that device is little endian.
	
				if (packetID == Pid_Session_Started)
				{
					NSLog(CFSTR("### Received GPS Garmin Session Started!"));
					pTheDevice->bGarminGPSProtocolSessionStarted = true;
				}
				else if (packetID == Pid_Data_Available)
				{	
					// Oooh, oooh. Data is available on the bulk pipe. Whoop-dee-doo.
					unsigned char* pDataBuffer = malloc(pTheDevice->nMaxInPacketSize);	// Create a buffer to hold our data.
					while (true)
					{	
						UInt32 nDataLength = pTheDevice->nMaxInPacketSize;
						nErr = local_ReadBytes(pTheDevice, pDataBuffer, &nDataLength, pTheDevice->nInPipeRef);
						if (nErr == kIOReturnSuccess || nErr == kIOUSBTransactionTimeout && nDataLength != 0)
						{
							// Stuff this data into the callback.
							if (pTheDevice->pBulkCallback != NULL)
								pTheDevice->pBulkCallback(pDataBuffer, nDataLength, pTheDevice->pBulkCallbackContext);

							if (DIAGNOSTICS_ON)
							{
								size_t ix;
								NSLog(CFSTR("===GARMIN-BULK-IN=== bytes (%u): "), nDataLength);
								for (ix = 0; ix < nDataLength; ix++)
									printf("%02x ", pDataBuffer[ix]);
								printf("\n");
							}    
						}
						else
						{
							NSLog(CFSTR("*** garmin data pipe read error: 0x%x (or 0 length packet [%u]). Continuing to monitor interrupt pipe."), nErr, nDataLength);
							break;
						}
					}
					free(pDataBuffer);
				}
			}
			else
			{	// Non-protocol data which we receive on the interrupt pipe should be forwarded to the callback.
				if (pTheDevice->pBulkCallback != NULL)
					pTheDevice->pBulkCallback(pProtocolBuffer, nBufferLen, pTheDevice->pBulkCallbackContext);
				if (DIAGNOSTICS_ON)
				{
					size_t ix;
					NSLog(CFSTR("===GARMIN-INTERRUPT-IN (non protocol layer) === bytes (%u): "), nBufferLen);
					for (ix = 0; ix < nBufferLen; ix++)
						printf("%02x ", pProtocolBuffer[ix]);
					printf("\n");
				}    
			}
		}
		else
		{
			NSLog(CFSTR("*** garmin protocol read error: 0x%x. Exiting."), nErr);
			break;
		}
	}
	free(pProtocolBuffer);
}

void local_hid_listen_thread(VST_USBBulkDevice * pTheDevice)
{
    if (DIAGNOSTICS_ON)
        printf("in local_hid_listen_thread\n");

    // Initialize scheduler policy for this thread
    struct sched_param sp;
    memset(&sp, 0, sizeof(struct sched_param));
    sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
    if ((pthread_setschedparam(pthread_self(), SCHED_RR, &sp) != 0) && DIAGNOSTICS_ON)
        printf("Failed to change priority.\n");

            
    if (pTheDevice != NULL)
    {
        CFRunLoopAddSource(CFRunLoopGetCurrent(), pTheDevice->hHIDRunLoopSource, kCFRunLoopCommonModes);
    
        if (DIAGNOSTICS_ON)
            printf("calling CFRunLoopRun()...\n");
        
        // Start the run loop.  Now we'll receive HID events.
        // Store a pointer to the run loop so we can shut it down later.
        pTheDevice->hHIDRunLoop = CFRunLoopGetCurrent();
        CFRunLoopRun();
    }
    
    if (DIAGNOSTICS_ON)
        printf("local_hid_listen_thread thread about to exit...\n");
}

void local_input_hid_callback(void * target, IOReturn result, void * refcon, void * sender)
{
    UInt32 nNumBytes = 0L;
    int nLockErr;
    size_t ix;
    AbsoluteTime zeroTime = {0, 0};
    IOHIDEventStruct event;
    unsigned char * pInputBuffer = NULL;
    UInt32 * pNumBytes, * pMaxBytes;
    unsigned char * data;
    VST_USBBulkDevice * pTheDevice = (VST_USBBulkDevice *)target;
	bool bWasMeas = false;
    
    if (pTheDevice != NULL)
    {
        if (pTheDevice->hHIDInputInterface != NULL)
        {
        	IOReturn nErr = 0;
        	while (nErr == 0)
        	{               
	            result = (*pTheDevice->hHIDInputInterface)->getNextEvent(pTheDevice->hHIDInputInterface, &event, zeroTime, 0);
	            if (result == kIOReturnSuccess)
	            {
	                data = (unsigned char *) event.longValue;
	                nNumBytes = event.longValueSize;
	            
	                // New event available
	                if (DIAGNOSTICS_ON)
	                {
	                    printf("===LISTEN=== local_input_hid_callback: data received (%d): ", (int) nNumBytes);
	                    for (ix = 0; ix < nNumBytes; ++ix)
	                        printf("%02x ", data[ix]);
	                    printf("\n");
	                }
	                
						
	                // Figure out which buffer to put it in
					if (pTheDevice->bWantsHIDInputFiltering) 
					{
						if ((((unsigned char *)event.longValue)[0] & 0xC0) == 0)
						{
							pNumBytes = &pTheDevice->nMeasurementBufferBytes;
							pMaxBytes = &pTheDevice->nMeasurementBufferMaxBytes;
							pInputBuffer = pTheDevice->pMeasurementBuffer;
							
							if (pTheDevice->nLastNumMeasurementsInPacket != 0)
							{
								unsigned char nTestCounter = pTheDevice->nLastMeasurementRollingCounter + pTheDevice->nLastNumMeasurementsInPacket;
								if ((nTestCounter != ((unsigned char *)event.longValue)[1]) && DIAGNOSTICS_ON)
									printf("Skip measurement rolling counter ERROR - expected %u, got %u\n", nTestCounter, ((unsigned char *)event.longValue)[1]);
							}
							bWasMeas = true;
							pTheDevice->nLastNumMeasurementsInPacket = ((unsigned char *)event.longValue)[0] & 0x07;	// (RM2846) JK 20090323.
							pTheDevice->nLastMeasurementRollingCounter = ((unsigned char *)event.longValue)[1];
							if (DIAGNOSTICS_ON)
							{
								printf("===LISTEN=== local_input_hid_callback: data received (%d): 0x%x\n", (int)nNumBytes, (unsigned int) *((UInt32*)&data[4]));
								printf("===LISTEN=== local_input_hid_callback: data received (%d): ", (int)nNumBytes);
								for (ix = 0; ix < nNumBytes; ix++)
									printf("%02x ", data[ix]);
								printf("\n");
							}
							
						}
						else 
						{
							pNumBytes = &pTheDevice->nCommandBufferBytes;
							pMaxBytes = &pTheDevice->nCommandBufferMaxBytes;
							pInputBuffer = pTheDevice->pCommandBuffer;
							
							// Do not perform any protocol specific action here -- i.e. don't clear the buffer, dammit. (RM2846) JK 20090323.
						}
					}
					else	// NOTE: the non-filtered mode puts all data in the command buffer
					{
						pNumBytes = &pTheDevice->nCommandBufferBytes;
						pMaxBytes = &pTheDevice->nCommandBufferMaxBytes;
						pInputBuffer = pTheDevice->pCommandBuffer;
					}
	                
	                if (*pNumBytes < *pMaxBytes - nNumBytes)
	                {
	                	nLockErr = 0;
		                do
		                {
		                    nLockErr = pthread_mutex_trylock(&(pTheDevice->listeningMutex));
		                    if (nLockErr)
		                        usleep(5);
		                } while (nLockErr);
	                
	                    memcpy(pInputBuffer + *pNumBytes, data, nNumBytes);
	                    *pNumBytes += nNumBytes;
	                    pthread_mutex_unlock(&(pTheDevice->listeningMutex));
	                }
					
					// According to the docs for getNextEvent "If a long value is present, it is up to the caller 
					// to deallocate it."
					if (event.longValue != NULL)
						free(event.longValue);
					else 
						printf("\n\n\n\nevent.longValue == NULL\n\n\n\n");		// NOTE: this should not happen?
	            }
	            else
	            if ((result != kIOReturnUnderrun) && DIAGNOSTICS_ON)
	                NSLog(CFSTR("===LISTEN=== local_input_hid_callback(), Read HID error: 0x%lx\n"), (long unsigned int)result);
	            nErr = result;
	        }
        }
    }
}

void local_start_notification_thread(VST_USBNotificationThread * pTheThread)
{
    mach_port_t 		masterPort;
    CFMutableDictionaryRef 	matchingDict;
    CFRunLoopSourceRef		runLoopSource;
    kern_return_t		kr = 0;

    if (DIAGNOSTICS_ON)
        printf("in local_start_notification_thread\n");
        
    pTheThread->notificationRunLoop = CFRunLoopGetCurrent();
    pTheThread->bActive = false;
    
    kr = IOMasterPort(MACH_PORT_NULL, &masterPort);
    if (kr || !masterPort)
    {
        if(DIAGNOSTICS_ON)
            printf("ISMasterPort() returned %d.\n", kr);
            
        pTheThread->notificationThread = 0;
        pTheThread->notificationRunLoop = 0;
        
        return;
    }
    
    if (!pTheThread->bUsesHID)  // Interested in instances of class IOUSBDevice and its subclasses
        matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
    else  // Interested in instances of class IOHIDDevice and its subclasses
        matchingDict = IOServiceMatching(kIOHIDDeviceKey);
    if (!matchingDict)
    {
        if(DIAGNOSTICS_ON)
            printf("Can't create a USB matching dictionary\n");
        mach_port_deallocate(mach_task_self(), masterPort);
        
        pTheThread->notificationThread = 0;
        pTheThread->notificationRunLoop = 0;
        
        return;
    }
    
    // Add our vendor and product IDs to the matching criteria
    if (!pTheThread->bUsesHID)
    {
        CFDictionarySetValue(matchingDict, CFSTR(kUSBVendorID), CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pTheThread->nVendorID));
        CFDictionarySetValue(matchingDict, CFSTR(kUSBProductID), CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pTheThread->nProductID));
    }
    else
    {
        CFDictionarySetValue(matchingDict, CFSTR(kIOHIDVendorIDKey), CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pTheThread->nVendorID)); 
        CFDictionarySetValue(matchingDict, CFSTR(kIOHIDProductIDKey), CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pTheThread->nProductID)); 
    }
	
    // Create a notification port and add its run loop event source to our run loop
    // This is how async notifications get set up.
    gNotifyPort = IONotificationPortCreate(masterPort);
    runLoopSource = IONotificationPortGetRunLoopSource(gNotifyPort);
    
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    
    // Retain additional references because we use this same dictionary with four calls to 
    // IOServiceAddMatchingNotification, each of which consumes one reference.
    matchingDict = (CFMutableDictionaryRef) CFRetain( matchingDict ); 
    matchingDict = (CFMutableDictionaryRef) CFRetain( matchingDict ); 
    matchingDict = (CFMutableDictionaryRef) CFRetain( matchingDict ); 
    
    // Now set up two notifications, one to be called when a raw device is first matched by I/O Kit, and the other to be
    // called when the device is terminated.
    kr = IOServiceAddMatchingNotification(gNotifyPort, kIOFirstMatchNotification, matchingDict, local_device_added_callback,
                                          pTheThread, &gRawAddedIter);
    
    // Iterate once to get already-present devices and arm the notification
    local_device_added_callback(pTheThread, gRawAddedIter);

    kr = IOServiceAddMatchingNotification(gNotifyPort, kIOTerminatedNotification, matchingDict, local_device_removed_callback,
                                          pTheThread, &gRawRemovedIter);
                                            
    // Iterate once to arm the notification
    local_device_removed_callback(pTheThread, gRawRemovedIter);
    
    // Now done with the master_port
    mach_port_deallocate(mach_task_self(), masterPort);
    masterPort = 0;

    pTheThread->bActive = true;
	
	if (pthread_main_np() == 0)	// if we're in a non-main thread, park ourselves in a run-loop
		CFRunLoopRun();
}

void local_device_added_callback(void *refCon, io_iterator_t iterator)
{
    kern_return_t		kr;
    io_service_t		usbDevice;
    UInt32              nLocationID;
    CFNumberRef         cfNumLocationID;
    CFDictionaryRef     cfDictProperties;
    
    VST_USBNotificationThread * pVSTThread = (VST_USBNotificationThread *) refCon;
    
    if (DIAGNOSTICS_ON)
        printf("in local_device_added_callback\n");
        
    if (!pVSTThread)
        return;
        
    while (usbDevice = IOIteratorNext(iterator))
    {
        kr = IORegistryEntryCreateCFProperties(usbDevice, (CFMutableDictionaryRef *)&cfDictProperties, kCFAllocatorDefault, kNilOptions);
        if (kr == 0)
        {
            if (!pVSTThread->bUsesHID)
                cfNumLocationID = (CFNumberRef) CFDictionaryGetValue(cfDictProperties, CFSTR(kUSBDevicePropertyLocationID));
            else
                cfNumLocationID = (CFNumberRef) CFDictionaryGetValue(cfDictProperties, CFSTR(kIOHIDLocationIDKey));
            if (cfNumLocationID != NULL)
            {
                if (DIAGNOSTICS_ON)
                {
                    CFNumberGetValue(cfNumLocationID, kCFNumberSInt32Type, &nLocationID);
                    printf("Location of removed device: 0x%08x\n", (unsigned int)nLocationID);
                }
                CFArrayAppendValue(pVSTThread->pAddedDevices, cfNumLocationID);
            }
        }
		CFRelease(cfDictProperties);
    }
}

void local_device_removed_callback(void *refCon, io_iterator_t iterator)
{
    kern_return_t		kr;
    io_service_t		usbDevice;
    UInt32                      nLocationID;
    CFNumberRef                 cfNumLocationID;
    CFDictionaryRef             cfDictProperties;
    
    VST_USBNotificationThread * pVSTThread = (VST_USBNotificationThread *) refCon;
    
    if (DIAGNOSTICS_ON)
        printf("in local_device_removed_callback\n");
        
    if (!pVSTThread)
        return;
    
    while (usbDevice = IOIteratorNext(iterator))
    {
    	if (pVSTThread->bActive && DIAGNOSTICS_ON)
            printf("A USB device was removed.\n");
    
    	if (pVSTThread->bActive)
        {
            kr = IORegistryEntryCreateCFProperties(usbDevice, (CFMutableDictionaryRef *)&cfDictProperties, kCFAllocatorDefault, kNilOptions);
            if (kr == 0)
            {
                if (!pVSTThread->bUsesHID)
                    cfNumLocationID = (CFNumberRef) CFDictionaryGetValue(cfDictProperties, CFSTR(kUSBDevicePropertyLocationID));
                else
                    cfNumLocationID = (CFNumberRef) CFDictionaryGetValue(cfDictProperties, CFSTR(kIOHIDLocationIDKey));
                if (cfNumLocationID != NULL)
                {
                    if (DIAGNOSTICS_ON)
                    {
                        CFNumberGetValue(cfNumLocationID, kCFNumberSInt32Type, &nLocationID);
                        printf("Location of removed device: 0x%08x\n", (unsigned int)nLocationID);
                    }
                    
                    CFArrayAppendValue(pVSTThread->pRemovedDevices, cfNumLocationID);
                }
				CFRelease(cfDictProperties);
            }
        }
    }
}

void local_ConvertBytesToInt(unsigned char chLSB,
				unsigned char chLMidB,
				unsigned char chMMidB,
				unsigned char chMSB,
				int * pOutInt)
{
#if TARGET_CPU_X86	
	char * pAssign = (char *) pOutInt;
	*pAssign = chLSB;
	pAssign++;
	*pAssign = chLMidB;
	pAssign++;
	*pAssign = chMMidB;
	pAssign++;
	*pAssign = chMSB;
#else
	unsigned char * pNumByte = (unsigned char *) (pOutInt);
	*pNumByte = chMSB;
	pNumByte++;
	*pNumByte = chMMidB;
	pNumByte++;
	*pNumByte = chLMidB;
	pNumByte++;
	*pNumByte = chLSB;
#endif
}


void local_ConvertByteToShort(unsigned char chLSB, unsigned char chMSB, short* pOutShort)
{
#if TARGET_CPU_X86
	char * pAssign = (char *) pOutShort;
	*pAssign = chLSB;
	pAssign++;
	*pAssign = chMSB;
#else
	char * pAssign = (char *) pOutShort;
	*pAssign = chMSB;
	pAssign++;
	*pAssign = chLSB;
#endif

}