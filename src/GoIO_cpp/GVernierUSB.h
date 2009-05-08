#ifndef _GVERNIERUSB_H_
#define _GVERNIERUSB_H_

//Constants used by the USB protocol to identify our devices:
enum { VERNIER_DEFAULT_VENDOR_ID  = 0x08F7 };

enum { LABPRO_DEFAULT_PRODUCT_ID = 0x0001,
		USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID = 0x0002,	//aka GoTemp
		SKIP_DEFAULT_PRODUCT_ID = 0x0003,				//aka GoLink
		CYCLOPS_DEFAULT_PRODUCT_ID = 0x0004,			//aka GoMotion
		NGI_DEFAULT_PRODUCT_ID = 0x0005,				//aka LabQuest
		LOWCOST_SPEC_DEFAULT_PRODUCT_ID = 0x0006,		//aka CK Spectrometer
		MINI_GC_DEFAULT_PRODUCT_ID = 0x0007,			//aka Vernier Mini Gas Chromatograph
		STANDALONE_DAQ_DEFAULT_PRODUCT_ID = 0x0008
};	

#define LABQUEST_DEFAULT_PRODUCT_ID NGI_DEFAULT_PRODUCT_ID

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
