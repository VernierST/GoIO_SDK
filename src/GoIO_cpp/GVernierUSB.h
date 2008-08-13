#ifndef _GVERNIERUSB_H_
#define _GVERNIERUSB_H_

//Constants used by the USB protocol to identify our devices:
enum { VERNIER_DEFAULT_VENDOR_ID  = 0x08F7 };
enum { LABPRO_DEFAULT_PRODUCT_ID = 0x0001,
		USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID = 0x0002,	//aka GoTemp
		SKIP_DEFAULT_PRODUCT_ID = 0x0003,				//aka GoLink
		CYCLOPS_DEFAULT_PRODUCT_ID = 0x0004,			//aka GoMotion
		NGI_DEFAULT_PRODUCT_ID = 0X0005					//aka LabPro 2
};	

#endif // _GVERNIERUSB_H_

