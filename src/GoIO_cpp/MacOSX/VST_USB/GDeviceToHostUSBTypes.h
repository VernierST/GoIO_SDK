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
