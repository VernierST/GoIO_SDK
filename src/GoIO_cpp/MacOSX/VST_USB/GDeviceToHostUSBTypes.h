/*
 *  GDeviceToHostUSBTypes.h
 *  NGIO_SDK
 *
 *  Created by Joseph Kelly on 6/22/07.
 *  Copyright 2007 Vernier Software & Technology. All rights reserved.
 *
 */

#ifdef TARGET_OS_WIN
	#include "Jungo.h"
#endif

#ifdef TARGET_OS_WIN
	#pragma pack(push)
	#pragma pack(1)
#endif 

#ifdef TARGET_OS_MAC
	#pragma pack(1)
#endif

struct tagGDeviceToHostUSBBulkPipePacketHeader
{
	unsigned char lsbyteLswordPayloadLength;
	unsigned char msbyteLswordPayloadLength;
	unsigned char lsbyteMswordPayloadLength;
	unsigned char msbyteMswordPayloadLength;
	unsigned char spare1;
	unsigned char spare2;
	unsigned char spare3;
	unsigned char spare4;
};
typedef struct tagGDeviceToHostUSBBulkPipePacketHeader GDeviceToHostUSBBulkPipePacketHeader;

#ifdef TARGET_OS_WIN
	#pragma pack(pop)
#endif 

#ifdef TARGET_OS_MAC
	#pragma pack()
#endif

#define DEVICE_TO_HOST_USB_BULK_MAX_PACKET_PAYLOAD_LENGTH 30000
