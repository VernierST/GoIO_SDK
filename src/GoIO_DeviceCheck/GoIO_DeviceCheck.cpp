// GoIO_DeviceCheck.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <string.h>
#include <memory.h>

#ifdef TARGET_OS_WIN
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#endif
#ifdef TARGET_OS_LINUX
#include <sys/time.h>
#endif
#ifdef TARGET_OS_MAC
#include <Carbon/Carbon.h>
#endif

#define MAX_NUM_MEASUREMENTS 100

#include "GoIO_DLL_interface.h"

char *deviceDesc[8] = {"?", "?", "Go! Temp", "Go! Link", "Go! Motion", "?", "?", "Mini GC"};

bool GetAvailableDeviceName(char *deviceName, gtype_int32 nameLength, gtype_int32 *pVendorId, gtype_int32 *pProductId);
static void OSSleep(unsigned long msToSleep);

int main(int argc, char* argv[])
{
	char deviceName[GOIO_MAX_SIZE_DEVICE_NAME];
	gtype_int32 vendorId;		//USB vendor id
	gtype_int32 productId;		//USB product id
	char tmpstring[100];
	gtype_uint16 MajorVersion;
	gtype_uint16 MinorVersion;
	char units[20];
	char equationType = 0;

	gtype_int32 rawMeasurements[MAX_NUM_MEASUREMENTS];
	gtype_real64 volts[MAX_NUM_MEASUREMENTS];
	gtype_real64 calbMeasurements[MAX_NUM_MEASUREMENTS];
	gtype_int32 numMeasurements, i;
	gtype_real64 averageCalbMeasurement;

	printf("GoIO_DeviceCheck version 1.0\n");
	
	GoIO_Init();

	GoIO_GetDLLVersion(&MajorVersion, &MinorVersion);
	printf("This app is linked to GoIO lib version %d.%d .\n", MajorVersion, MinorVersion);

	bool bFoundDevice = GetAvailableDeviceName(deviceName, GOIO_MAX_SIZE_DEVICE_NAME, &vendorId, &productId);
	if (!bFoundDevice)
		printf("No Go devices found.\n");
	else
	{
		GOIO_SENSOR_HANDLE hDevice = GoIO_Sensor_Open(deviceName, vendorId, productId, 0);
		if (hDevice != NULL)
		{
			printf("Successfully opened %s device %s .\n", deviceDesc[productId], deviceName);

			unsigned char charId;
			GoIO_Sensor_DDSMem_GetSensorNumber(hDevice, &charId, 0, 0);
			printf("Sensor id = %d", charId);

			GoIO_Sensor_DDSMem_GetLongName(hDevice, tmpstring, sizeof(tmpstring));
			if (strlen(tmpstring) != 0)
				printf("(%s)", tmpstring);
			printf("\n");

			GoIO_Sensor_SetMeasurementPeriod(hDevice, 0.040, SKIP_TIMEOUT_MS_DEFAULT);//40 milliseconds measurement period.
			GoIO_Sensor_SendCmdAndGetResponse(hDevice, SKIP_CMD_ID_START_MEASUREMENTS, NULL, 0, NULL, NULL, SKIP_TIMEOUT_MS_DEFAULT);
			OSSleep(1000); //Wait 1 second.

			numMeasurements = GoIO_Sensor_ReadRawMeasurements(hDevice, rawMeasurements, MAX_NUM_MEASUREMENTS);
			printf("%d measurements received after about 1 second.\n", numMeasurements);
			averageCalbMeasurement = 0.0;
			for (i = 0; i < numMeasurements; i++)
			{
				volts[i] = GoIO_Sensor_ConvertToVoltage(hDevice, rawMeasurements[i]);
				calbMeasurements[i] = GoIO_Sensor_CalibrateData(hDevice, volts[i]);
				averageCalbMeasurement += calbMeasurements[i];
			}
			if (numMeasurements > 1)
				averageCalbMeasurement = averageCalbMeasurement/numMeasurements;

			GoIO_Sensor_DDSMem_GetCalibrationEquation(hDevice, &equationType);
			if (equationType != kEquationType_Linear)
				strcpy(units, "volts");
			else
			{
				gtype_real32 a, b, c;
				unsigned char activeCalPage = 0;
				GoIO_Sensor_DDSMem_GetActiveCalPage(hDevice, &activeCalPage);
				GoIO_Sensor_DDSMem_GetCalPage(hDevice, activeCalPage, &a, &b, &c, units, sizeof(units));
			}
			printf("Average measurement = %8.3f %s .\n", averageCalbMeasurement, units);

			GoIO_Sensor_Close(hDevice);
		}
	}
	GoIO_Uninit();
	return 0;
}

bool GetAvailableDeviceName(char *deviceName, gtype_int32 nameLength, gtype_int32 *pVendorId, gtype_int32 *pProductId)
{
	bool bFoundDevice = false;
	deviceName[0] = 0;
	int numSkips = GoIO_UpdateListOfAvailableDevices(VERNIER_DEFAULT_VENDOR_ID, SKIP_DEFAULT_PRODUCT_ID);
	int numJonahs = GoIO_UpdateListOfAvailableDevices(VERNIER_DEFAULT_VENDOR_ID, USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID);
	int numCyclopses = GoIO_UpdateListOfAvailableDevices(VERNIER_DEFAULT_VENDOR_ID, CYCLOPS_DEFAULT_PRODUCT_ID);
	int numMiniGCs = GoIO_UpdateListOfAvailableDevices(VERNIER_DEFAULT_VENDOR_ID, MINI_GC_DEFAULT_PRODUCT_ID);

	if (numSkips > 0)
	{
		GoIO_GetNthAvailableDeviceName(deviceName, nameLength, VERNIER_DEFAULT_VENDOR_ID, SKIP_DEFAULT_PRODUCT_ID, 0);
		*pVendorId = VERNIER_DEFAULT_VENDOR_ID;
		*pProductId = SKIP_DEFAULT_PRODUCT_ID;
		bFoundDevice = true;
	}
	else if (numJonahs > 0)
	{
		GoIO_GetNthAvailableDeviceName(deviceName, nameLength, VERNIER_DEFAULT_VENDOR_ID, USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID, 0);
		*pVendorId = VERNIER_DEFAULT_VENDOR_ID;
		*pProductId = USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID;
		bFoundDevice = true;
	}
	else if (numCyclopses > 0)
	{
		GoIO_GetNthAvailableDeviceName(deviceName, nameLength, VERNIER_DEFAULT_VENDOR_ID, CYCLOPS_DEFAULT_PRODUCT_ID, 0);
		*pVendorId = VERNIER_DEFAULT_VENDOR_ID;
		*pProductId = CYCLOPS_DEFAULT_PRODUCT_ID;
		bFoundDevice = true;
	}
	else if (numMiniGCs > 0)
	{
		GoIO_GetNthAvailableDeviceName(deviceName, nameLength, VERNIER_DEFAULT_VENDOR_ID, MINI_GC_DEFAULT_PRODUCT_ID, 0);
		*pVendorId = VERNIER_DEFAULT_VENDOR_ID;
		*pProductId = MINI_GC_DEFAULT_PRODUCT_ID;
		bFoundDevice = true;
	}

	return bFoundDevice;
}

void OSSleep(
	unsigned long msToSleep)//milliseconds
{
#ifdef TARGET_OS_WIN
	::Sleep(msToSleep);
#endif
#ifdef TARGET_OS_LINUX
  struct timeval tv;
  unsigned long usToSleep = msToSleep*1000;
  tv.tv_sec = usToSleep/1000000;
  tv.tv_usec = usToSleep % 1000000;
  select (0, NULL, NULL, NULL, &tv);
#endif
#ifdef TARGET_OS_MAC
	AbsoluteTime absTime = ::AddDurationToAbsolute(msToSleep * durationMillisecond, ::UpTime());
	::MPDelayUntil(&absTime);
#endif
}
