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
#ifndef _GVERNIERUSB_H_
#define _GVERNIERUSB_H_

//Constants used by the USB protocol to identify our devices:
enum { VERNIER_DEFAULT_VENDOR_ID  = 0x08F7 };

enum { LABPRO_DEFAULT_PRODUCT_ID = 0x0001,
		USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID = 0x0002,	//aka GoTemp
		SKIP_DEFAULT_PRODUCT_ID = 0x0003,				//aka GoLink
		CYCLOPS_DEFAULT_PRODUCT_ID = 0x0004,			//aka GoMotion
		NGI_DEFAULT_PRODUCT_ID = 0x0005,				//aka LabQuest
		LOWCOST_SPEC_DEFAULT_PRODUCT_ID = 0x0006,		//aka SpectroVis
		MINI_GC_DEFAULT_PRODUCT_ID = 0x0007,			//aka Vernier Mini Gas Chromatograph
		STANDALONE_DAQ_DEFAULT_PRODUCT_ID = 0x0008,		//aka LabQuest Mini
		LOWCOST_SPEC2_DEFAULT_PRODUCT_ID = 0x0009		//aka SpectroVis Plus
};	

#define LABQUEST_DEFAULT_PRODUCT_ID NGI_DEFAULT_PRODUCT_ID
#define LABQUEST_MINI_PRODUCT_ID STANDALONE_DAQ_DEFAULT_PRODUCT_ID
#define MINIGC_DEFAULT_PRODUCT_ID MINI_GC_DEFAULT_PRODUCT_ID

/// @brief What we thought was the unique Ohaus Scout Pro VID/PID is actually the FTDI generic VID/PID, which is also used by Watt's Up devices. This throws a big doo-doo in how we go about identifying and enumerating new devices on the bus. 
/// @see "GFTDIDevice.h"
enum { FTDI_GENERIC_VENDOR_ID  = 0x0403 };
enum { FTDI_GENERIC_PRODUCT_ID = 0x6001 };

enum { OCEAN_OPTICS_DEFAULT_VENDOR_ID  = 0x2457 };
enum { OCEAN_OPTICS_DEFAULT_PRODUCT_ID = 0x1002 };	// USB2000
enum { OCEAN_OPTICS_USB325_PRODUCT_ID  = 0x1024 };
enum { OCEAN_OPTICS_USB650_PRODUCT_ID  = 0x1014 };
enum { OCEAN_OPTICS_USB2000_PRODUCT_ID = 0x1002 };
enum { OCEAN_OPTICS_HR4000_PRODUCT_ID  = 0x1012 };
enum { OCEAN_OPTICS_USB4000_PRODUCT_ID = 0x1022 };

// FIX THIS!
enum { NATIONAL_INSTRUMENTS_DEFAULT_VENDOR_ID  = 0x3923 };
enum { SENSORDAQ_DEFAULT_PRODUCT_ID = 0x72CC };

#endif 
