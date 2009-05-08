// GoIO_DLL_interface.cpp : Defines the exported methods for the GoIO_DLL .
//

#ifdef TARGET_OS_WIN
	#include "stdafx.h"
	#include "GoIO_DLL.h"
	#include "WinEnumDevices.h"
	
	extern CGoIO_DLLApp theApp;
#endif

#include "GSkipDevice.h"
#include "GCyclopsDevice.h"
#include "GMiniGCDevice.h"
#include "GUSBDirectTempDevice.h"
#include "GMBLSensor.h"
#include "GUtils.h"
#include "GoIO_DLL_interface.h"

#define SKIP_TIMEOUT_MS_READ_FLASH 2000
#define SKIP_LIB_MNG_MUTEX_TIMEOUT_MS 500

typedef std::vector<void *> GPtrVector;
typedef GPtrVector::iterator GPtrVectorIterator;
GPtrVector openSensorVector;//list of CGoIOSensors
OSMutex openSensorVectorMutex = NULL;
OSMutex multipleInstanceDeviceMutex = NULL;
bool bMultipleInstanceDeviceMutexLocked = false;

class CGoIOSensor
{
public:
	CGoIOSensor(GPortRef *pPortRef)
	{
		if (SKIP_DEFAULT_PRODUCT_ID == pPortRef->GetUSBProductID())
			m_pInterface = new GSkipDevice(pPortRef);
		else
		if (USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID == pPortRef->GetUSBProductID())
			m_pInterface = new GUSBDirectTempDevice(pPortRef);
		else
		if (CYCLOPS_DEFAULT_PRODUCT_ID == pPortRef->GetUSBProductID())
			m_pInterface = new GCyclopsDevice(pPortRef);
		else
		if (MINI_GC_DEFAULT_PRODUCT_ID == pPortRef->GetUSBProductID())
			m_pInterface = new GMiniGCDevice(pPortRef);
		else
			GSTD_ASSERT(false);
		m_pMBLSensor = new GMBLSensor;
	}
	~CGoIOSensor()
	{
		if (m_pMBLSensor)
			delete m_pMBLSensor;
		if (m_pInterface)
			delete m_pInterface;
	}

	GSkipBaseDevice *m_pInterface;
	GMBLSensor *m_pMBLSensor;
};

static void OpenSensorVector_Clear()
{
	if (openSensorVectorMutex)
	{
		if (GThread::OSTryLockMutex(openSensorVectorMutex, 1))
		{
			while (openSensorVector.size() != 0)
				GoIO_Sensor_Close(openSensorVector[0]);

			GThread::OSUnlockMutex(openSensorVectorMutex);
		}
	}
}

static bool OpenSensorVector_FindAndLockSensor(GOIO_SENSOR_HANDLE hSensor)
{
	bool bFound = false;
	if (openSensorVectorMutex)
	{
		if (GThread::OSTryLockMutex(openSensorVectorMutex, SKIP_LIB_MNG_MUTEX_TIMEOUT_MS))
		{
			GPtrVectorIterator iter = std::find(openSensorVector.begin(), openSensorVector.end(), hSensor);
			if (iter != openSensorVector.end())
			{
				CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
				bFound = pGoIOSensor->m_pInterface->LockDevice(1);
			}

			GThread::OSUnlockMutex(openSensorVectorMutex);
		}
	}

	return bFound;
}

static bool UnlockSensor(GOIO_SENSOR_HANDLE hSensor)
{
	CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
	return pGoIOSensor->m_pInterface->UnlockDevice();
}

static bool OpenSensorVector_FindSensorByName(
	const char *pDeviceName,			//[in] NULL terminated string that uniquely identifies the device.
	gtype_int32 vendorId,				//[in] USB vendor id
	gtype_int32 productId)				//[in] USB product id
{
	bool bFound = false;
	if (openSensorVectorMutex)
	{
		if (GThread::OSTryLockMutex(openSensorVectorMutex, SKIP_LIB_MNG_MUTEX_TIMEOUT_MS))
		{
			cppstring deviceName = pDeviceName;
			GPortRef oldPortRef;

			GPtrVectorIterator iter = openSensorVector.begin();
			while (iter != openSensorVector.end())
			{
				CGoIOSensor *pGoIOSensor = (CGoIOSensor *) (*iter);
				oldPortRef = pGoIOSensor->m_pInterface->GetPortRef();
				if (((gtype_int32)oldPortRef.GetUSBVendorID() == vendorId) && ((gtype_int32)oldPortRef.GetUSBProductID() == productId))
				{
					if (deviceName == oldPortRef.GetLocation())
					{
						bFound = true;
						break;
					}
				}
				iter++;
			}

			GThread::OSUnlockMutex(openSensorVectorMutex);
		}
	}

	return bFound;
}

static bool OpenSensorVector_AddSensor(GOIO_SENSOR_HANDLE hSensor)
{
	bool bSuccess = false;
	if (openSensorVectorMutex)
	{
		if (GThread::OSTryLockMutex(openSensorVectorMutex, SKIP_LIB_MNG_MUTEX_TIMEOUT_MS))
		{
			bSuccess = true;
			openSensorVector.push_back(hSensor);

			GThread::OSUnlockMutex(openSensorVectorMutex);
		}
	}

	return bSuccess;
}


static bool OpenSensorVector_RemoveSensor(GOIO_SENSOR_HANDLE hSensor)
{
	bool bSuccess = false;
	if (openSensorVectorMutex)
	{
		if (GThread::OSTryLockMutex(openSensorVectorMutex, SKIP_LIB_MNG_MUTEX_TIMEOUT_MS))
		{
			GPtrVectorIterator iter = std::find(openSensorVector.begin(), openSensorVector.end(), hSensor);
			if (iter != openSensorVector.end())
			{
				openSensorVector.erase(iter);
				bSuccess = true;
			}

			GThread::OSUnlockMutex(openSensorVectorMutex);
		}
	}

	return bSuccess;
}

/***************************************************************************************************************************
	Function Name: GoIO_GetDLLVersion()
		Added in version 2.00.
	
	Purpose:	This routine returns the major and minor version numbers for the instance of GoIO_DLL that is
				currently running.

				If a function is not guaranteed to be present in all supported versions of GoIO_DLL, then the line
				"Added in version 'major.minor'" will appear in the function description in this file.

				It is our intention that all versions of GoIO_DLL created subsequent to a given version, will be
				backwards compatible with the older version. You should be able to replace an old version of GoIO_DLL
				with a newer version and everything should still work without rebuilding your application.

				Note that version major2.minor2 is later than version major1.minor1 
				iff. ((major2 > major1) || ((major2 == major1) && (minor2 > minor1))).
				
				Backwards compatibility is definitely our intention, but we do not absolutely guarantee it. If you think
				that you have detected a backwards compatibility bug, then please report it to Vernier Software & Technology.
				Calling GoIO_GetVersion() from your application is a way to identify precisely which version of
				GoIO_DLL you are actually using.


	Return:		0 iff successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_GetDLLVersion(
	gtype_uint16 *pMajorVersion, //[o]
	gtype_uint16 *pMinorVersion) //[o]
{
	*pMajorVersion = 2;
	*pMinorVersion = 26;
	return 0;
}

/***************************************************************************************************************************
	Function Name: GoIO_Init()
	
	Purpose:	Call GoIO_Init() once before making any other GoIO function calls.
				GoIO_Init() and GoIO_Uninit() should be called from the same thread.

				This routine attempts to grab the mutex that controls access to Vernier
				sensors. If Logger Pro, Logger Lite, or another GoIO application is currently
				running, then this routine will fail to grab the device control mutex, so it
				will return -1. Subsequent calls to GoIO functions will also fail until
				the application is able to run GoIO_Init() successfully.

	Return:		0 iff successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Init()
{
	#ifdef TARGET_OS_WIN // If this is Windows...
		if (!hWinSetupApiLibrary)
			hWinSetupApiLibrary = WinLoadSetupApiLibrary();
		if (!hWinHidDLibrary)
			hWinHidDLibrary = WinLoadHidDLibrary();

		if (!openSensorVectorMutex)
		{
			if (!bMultipleInstanceDeviceMutexLocked)
			{
				if (!multipleInstanceDeviceMutex)
					multipleInstanceDeviceMutex = GThread::OSCreateMutex(GSTD_S("MultipleInstanceDeviceMutex"));
				if (multipleInstanceDeviceMutex)
					bMultipleInstanceDeviceMutexLocked = GThread::OSTryLockMutex(multipleInstanceDeviceMutex, 1);
			}
			if (bMultipleInstanceDeviceMutexLocked)
				openSensorVectorMutex = GThread::OSCreateMutex(GSTD_S("GoIO_DLL_DeviceListMutex"));
		}

		if ((!openSensorVectorMutex) || (!hWinSetupApiLibrary) || (!hWinHidDLibrary))
			GoIO_Uninit();
	#endif

	#if defined (TARGET_OS_MAC) || defined (TARGET_OS_LINUX)
		if (!openSensorVectorMutex)
			openSensorVectorMutex = GThread::OSCreateMutex(GSTD_S("GoIO_DLL_DeviceListMutex"));

		if (!openSensorVectorMutex)
			GoIO_Uninit();
	#endif

	return (openSensorVectorMutex != NULL) ? 0 : -1;
}

/***************************************************************************************************************************
	Function Name: GoIO_Uninit()
	
	Purpose:	Call GoIO_Uninit() once to 'undo' GoIO_Init().
				GoIO_Init() and GoIO_Uninit() should be called from the same thread.

	Return:		0 iff successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Uninit()
{
	gtype_int32 nResult = 0;
	
	OpenSensorVector_Clear();

	if (openSensorVectorMutex)
		GThread::OSDestroyMutex(openSensorVectorMutex);
	openSensorVectorMutex = NULL;

	#ifdef TARGET_OS_WIN
		if (bMultipleInstanceDeviceMutexLocked)
		{
			if (GThread::OSUnlockMutex(multipleInstanceDeviceMutex))
				bMultipleInstanceDeviceMutexLocked = false;
			else
				nResult = -1;
		}

		if ((!bMultipleInstanceDeviceMutexLocked) && multipleInstanceDeviceMutex)
		{
			GThread::OSDestroyMutex(multipleInstanceDeviceMutex);
			multipleInstanceDeviceMutex = NULL;
		}

		if (hWinHidDLibrary)
			FreeLibrary(hWinHidDLibrary);
		hWinHidDLibrary = NULL;

		if (hWinSetupApiLibrary)
			FreeLibrary(hWinSetupApiLibrary);
		hWinSetupApiLibrary = NULL;
	#endif

	return nResult;
}

/***************************************************************************************************************************
	Function Name: GoIO_UpdateListOfAvailableDevices()
	
	Purpose:	This routine queries the operating system to build a list of available devices
				that have the specified USB vendor id and product id. Only Go! Link, Go! Temp, and Go! Motion
				vendor and product id's are supported.

	Return:		number of devices found.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_UpdateListOfAvailableDevices(
	gtype_int32 vendorId,	//[in]
	gtype_int32 productId)	//[in]
{
	gtype_int32 numDevices = 0;
	if (VERNIER_DEFAULT_VENDOR_ID == vendorId)
	{
		StringVector deviceVec;
		if (SKIP_DEFAULT_PRODUCT_ID == productId)
		{
			deviceVec = GSkipDevice::GetAvailableDevices();
			GSkipDevice::StoreSnapshotOfAvailableDevices(deviceVec);
		}
		else
		if (USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID == productId)
		{
			deviceVec = GUSBDirectTempDevice::GetAvailableDevices();
			GUSBDirectTempDevice::StoreSnapshotOfAvailableDevices(deviceVec);
		}
		else
		if (CYCLOPS_DEFAULT_PRODUCT_ID == productId)
		{
			deviceVec = GCyclopsDevice::GetAvailableDevices();
			GCyclopsDevice::StoreSnapshotOfAvailableDevices(deviceVec);
		}
		else
		if (MINI_GC_DEFAULT_PRODUCT_ID == productId)
		{
			deviceVec = GMiniGCDevice::GetAvailableDevices();
			GMiniGCDevice::StoreSnapshotOfAvailableDevices(deviceVec);
		}
		numDevices = deviceVec.size();
	}

	return numDevices;
}

/***************************************************************************************************************************
	Function Name: GoIO_GetNthAvailableDeviceName()
	
	Purpose:	Get a unique device name string for the n'th device in the list of known devices with a specified
				USB vendor id and product id. This routine will only succeed if GoIO_UpdateListOfAvailableDevices()
				has been called previously.

	Return:		0 iff successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_GetNthAvailableDeviceName(
	char *pBuf,		//[out] ptr to buffer to store device name string.
	gtype_int32 bufSize,	//[in] number of bytes in buffer pointed to by pBuf. Strlen(pBuf) < bufSize, because the string is NULL terminated.
	gtype_int32 vendorId,	//[in]
	gtype_int32 productId,	//[in]
	gtype_int32 N)			//[in] index into list of known devices, 0 => first device in list.
{
	gtype_int32 nResult = -1;
	GSTD_ASSERT(pBuf != NULL);
	GSTD_ASSERT(bufSize > 0);
	cppstring deviceName;
	if (VERNIER_DEFAULT_VENDOR_ID == vendorId)
	{
		if (SKIP_DEFAULT_PRODUCT_ID == productId)
		{
			const StringVector &skipDeviceVec = GSkipDevice::GetSnapshotOfAvailableDevices();
			if ((gtype_int32)skipDeviceVec.size() > N)
				deviceName = skipDeviceVec[N];
		}
		else
		if (USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID == productId)
		{
			const StringVector &jonahDeviceVec = GUSBDirectTempDevice::GetSnapshotOfAvailableDevices();
			if ((gtype_int32)jonahDeviceVec.size() > N)
				deviceName = jonahDeviceVec[N];
		}
        else
		if (CYCLOPS_DEFAULT_PRODUCT_ID == productId)
		{
			const StringVector &cyclopsDeviceVec = GCyclopsDevice::GetSnapshotOfAvailableDevices();
			if ((gtype_int32)cyclopsDeviceVec.size() > N)
				deviceName = cyclopsDeviceVec[N];
		}
        else
		if (MINI_GC_DEFAULT_PRODUCT_ID == productId)
		{
			const StringVector &MiniGCDeviceVec = GMiniGCDevice::GetSnapshotOfAvailableDevices();
			if ((gtype_int32)MiniGCDeviceVec.size() > N)
				deviceName = MiniGCDeviceVec[N];
		}
		strncpy(pBuf, deviceName.c_str(), bufSize);
		pBuf[bufSize - 1] = 0;
		if ((strlen(pBuf) == deviceName.size()) && (strlen(pBuf) > 0))
			nResult = 0;
	}
	return nResult;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_Open()
	
	Purpose:	Open a specified Go! device and the attached sensor.. 
	
				If the device is already open, then this routine will fail.

				In addition to establishing basic communication with the device, this routine will initialize the
				device. If a smart sensor is attached to the Go! device, then this routine will query the device for
				the DDS sensor configuration parameters.

				The following commands are sent to Go! Temp devices:
					SKIP_CMD_ID_INIT,
					SKIP_CMD_ID_READ_LOCAL_NV_MEM. - read DDS record

				The following commands are sent to Go! Link devices:
					SKIP_CMD_ID_INIT,
					SKIP_CMD_ID_GET_SENSOR_ID,
					SKIP_CMD_ID_READ_REMOTE_NV_MEM, - read DDS record if this is a 'smart' sensor
					SKIP_CMD_ID_SET_ANALOG_INPUT_CHANNEL. - based on sensor EProbeType

				Since the device stops sending measurements in response to SKIP_CMD_ID_INIT, an application must send
				SKIP_CMD_ID_START_MEASUREMENTS to the device in order to receive measurements.

				At any given time, a sensor is 'owned' by only one thread. The thread that calls this routine is the
				initial owner of the sensor. If a GoIO() call is made from a thread that does not own the sensor object
				that is passed in, then the call will generally fail. To allow another thread to access a sensor,
				the owning thread should call GoIO_Sensor_Unlock(), and then the new thread must call GoIO_Sensor_Lock().
  
	Return:		handle to open sensor device if successful, else NULL.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL GOIO_SENSOR_HANDLE GoIO_Sensor_Open(
	const char *pDeviceName,			//[in] NULL terminated string that uniquely identifies the device.
	gtype_int32 vendorId,				//[in] USB vendor id
	gtype_int32 productId,				//[in] USB product id
	gtype_int32 strictDDSValidationFlag)//[in] insist on exactly valid checksum if 1, else use a more lax validation test.
{
	//First find out if this device is already open.
	CGoIOSensor *pNewSensor = NULL;
	GPortRef newPortRef(kPortType_USB, pDeviceName, pDeviceName, vendorId, productId);
	long nResult = 0;
	GSensorDDSRec DDSRec;
	long nBytesRead;

	bool bFound = OpenSensorVector_FindSensorByName(pDeviceName, vendorId, productId);

	if (bFound)
		nResult = -1;
	else
	if (!openSensorVectorMutex)
		nResult = -1;
	else
	if (VERNIER_DEFAULT_VENDOR_ID != vendorId)
		nResult = -1;
	else
	if ((USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID != productId) && (SKIP_DEFAULT_PRODUCT_ID != productId) &&
        (CYCLOPS_DEFAULT_PRODUCT_ID != productId) && (MINI_GC_DEFAULT_PRODUCT_ID != productId))
		nResult = -1;

	if (0 == nResult)
	{
		pNewSensor = new CGoIOSensor(&newPortRef);
		nResult = pNewSensor->m_pInterface->Open(&newPortRef);
	}

	if (0 == nResult)
    {
        GCyclopsInitParams initParams;
        long initParamsSize = 0;
        void *pInitParams = NULL;
        if (CYCLOPS_DEFAULT_PRODUCT_ID == productId)
        {
            memset(&initParams, 0, sizeof(initParams));
#ifdef TARGET_HANDHELD
	    initParams.hostType = SKIP_HOST_TYPE_CALCULATOR;
#else
            initParams.hostType = SKIP_HOST_TYPE_COMPUTER;
#endif
            pInitParams = &initParams;
            initParamsSize = sizeof(initParams);
        }


	nResult = pNewSensor->m_pInterface->SendCmdAndGetResponse(SKIP_CMD_ID_INIT, pInitParams, initParamsSize, 
								  NULL, NULL,((SKIP_DEFAULT_PRODUCT_ID == productId) ? 
								  SKIP_TIMEOUT_MS_CMD_ID_INIT_WO_BUSY_STATUS :  
								  SKIP_TIMEOUT_MS_DEFAULT));
    }

	if (0 == nResult)
	{
		pNewSensor->m_pInterface->OSClearMeasurementPacketQueue();

		if (USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID == productId)
		{
			//This is a Jonah.
			//Read the DDS memory.
			nResult = pNewSensor->m_pInterface->ReadSensorDDSMemory((unsigned char *) &DDSRec, 0, sizeof(DDSRec), 
				SKIP_TIMEOUT_MS_READ_DDSMEMBLOCK);
			if (0 == nResult)
			{
				if (!GMBLSensor::VerifyDDSChecksum(DDSRec, (strictDDSValidationFlag != 0)))
					nResult = -2;
			}
			if (0 == nResult)
				pNewSensor->m_pMBLSensor->SetDDSRec(DDSRec, true);
		}
        else
		if (CYCLOPS_DEFAULT_PRODUCT_ID == productId)
		{
			//This is Cyclops..
			//Read the DDS memory. This is a fake read - we generate a Cyclops specific DDS record here. spam
			pNewSensor->m_pInterface->ReadSensorDDSMemory((unsigned char *) &DDSRec, 0, sizeof(DDSRec), 
				SKIP_TIMEOUT_MS_READ_DDSMEMBLOCK);
			pNewSensor->m_pMBLSensor->SetDDSRec(DDSRec, true);
		}
		else
		{
			//This is a Skip or a Mini GC.
			GSkipFlashMemoryRecord flashRec;
			GSkipGetSensorIdCmdResponsePayload getSensorIdResponsePayload;

			nResult = ((GSkipDevice *) pNewSensor->m_pInterface)->ReadSkipFlashRecord(&flashRec, SKIP_TIMEOUT_MS_READ_FLASH);
			if (0 == nResult)
				((GSkipDevice *) pNewSensor->m_pInterface)->SetSkipFlashRecord(flashRec);//Do this so that measurements are properly calibrated in ConvertToVoltage().

			//Find out what sensor is connected.
			if (0 == nResult)
			{
				nBytesRead = sizeof(GSkipGetSensorIdCmdResponsePayload);
				nResult = pNewSensor->m_pInterface->SendCmdAndGetResponse(SKIP_CMD_ID_GET_SENSOR_ID, NULL, 0, 
					&getSensorIdResponsePayload, &nBytesRead);
			}
			if (0 == nResult)
			{
				//Parse out sensor id.
				int nSensorId;
				GUtils::OSConvertBytesToInt(getSensorIdResponsePayload.lsbyteLswordSensorId, 
					getSensorIdResponsePayload.msbyteLswordSensorId, getSensorIdResponsePayload.lsbyteMswordSensorId, 
					getSensorIdResponsePayload.msbyteMswordSensorId, &nSensorId);

				if (nSensorId < 0)
					nSensorId = 0;
				else
				if (nSensorId > 255)
					nSensorId = 0;

				pNewSensor->m_pMBLSensor->SetID(nSensorId);
				if (pNewSensor->m_pMBLSensor->IsSmartSensor())
				{
					//Read the DDS memory.
					nResult = pNewSensor->m_pInterface->ReadSensorDDSMemory((unsigned char *) &DDSRec, 0, sizeof(DDSRec), 
						SKIP_TIMEOUT_MS_READ_DDSMEMBLOCK);
					if (0 == nResult)
					{
						if (!GMBLSensor::VerifyDDSChecksum(DDSRec, (strictDDSValidationFlag != 0)))
							nResult = -2;
					}
					if (0 == nResult)
						pNewSensor->m_pMBLSensor->SetDDSRec(DDSRec, true);
				}
			}
			if (0 == nResult)
			{
				//Setup the analog input channel based on the sensor's probe type.
				GSkipSetAnalogInputChannelParams setAnalogInputChannelParams;
				setAnalogInputChannelParams.analogInputChannel = (pNewSensor->m_pMBLSensor->GetProbeType() == kProbeTypeAnalog10V) ? 
					SKIP_ANALOG_INPUT_CHANNEL_VIN : SKIP_ANALOG_INPUT_CHANNEL_VIN_LOW;

				nResult = pNewSensor->m_pInterface->SendCmdAndGetResponse(SKIP_CMD_ID_SET_ANALOG_INPUT_CHANNEL, 
					&setAnalogInputChannelParams, sizeof(GSkipSetAnalogInputChannelParams), NULL, NULL);
			}
		}
	}

	if (0 != nResult)
	{
		if (pNewSensor)
			delete pNewSensor;
		pNewSensor = NULL;
	}
	else
		//Add new sensor to list of open devices.
		OpenSensorVector_AddSensor(pNewSensor);

	return pNewSensor;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_Close()
	
	Purpose:	Close a specified sensor. After this routine succeeds the hSensor handle is no longer valid.
	
	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_Close(
	GOIO_SENSOR_HANDLE hSensor)	//[in] handle to open sensor.
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
        if (pGoIOSensor->m_pInterface->AreMeasurementsEnabled())
            pGoIOSensor->m_pInterface->SendCmd(SKIP_CMD_ID_STOP_MEASUREMENTS, NULL, 0);

		pGoIOSensor->m_pInterface->Close();

		OpenSensorVector_RemoveSensor(hSensor);

		UnlockSensor(hSensor);

		delete pGoIOSensor;
	}

	return nResult;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetOpenDeviceName()
	
	Purpose:	Get the unique device name string and USB id's for a specified open device.

	Return:		0 iff successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_GetOpenDeviceName(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	char *pBuf,				//[out] ptr to buffer to store device name string.
	gtype_int32 bufSize,	//[in] number of bytes in buffer pointed to by pBuf. Strlen(pBuf) < bufSize, because the string is NULL terminated.
	gtype_int32 *pVendorId,	//[out]
	gtype_int32 *pProductId)//[out]
{
	GSTD_ASSERT(pBuf != NULL);
	GSTD_ASSERT(bufSize > 0);
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		GPortRef portRef = pGoIOSensor->m_pInterface->GetPortRef();
		*pVendorId = portRef.GetUSBVendorID();
		*pProductId = portRef.GetUSBProductID();
		cppstring deviceName = portRef.GetLocation();
		strncpy(pBuf, deviceName.c_str(), bufSize);
		pBuf[bufSize - 1] = 0;
		if ((strlen(pBuf) != deviceName.size()) || (strlen(pBuf) == 0))
			nResult = -1;

		UnlockSensor(hSensor);
	}

	return nResult;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_Lock()
	
	Purpose:	Lock a specified sensor so that no other thread can access it. This routine will fail if the sensor
				is currently locked by another thread.

				GoIO_Sensor_Lock() increments the lock count for the specified sensor by 1. In order for a second
				thread to acquire access to the sensor, the first thread must call GoIO_Sensor_Unlock() until the lock
				count reaches 0. Note that the lock count is set to 1 by GoIO_Sensor_Open().
				
				It is ok for the lock count to be greater than 1 when GoIO_Sensor_Close() is called.

				This is an advanced routine that most users should not use. It is only necessary if an application
				needs to access a single sensor from multiple threads.
	
	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_Lock(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	gtype_int32 timeoutMs)		//[in] wait this long to acquire the lock before failing.
{
	unsigned int nStartTime = GUtils::OSGetTimeStamp();
	bool bLocked = false;
	do
	{
		bLocked = OpenSensorVector_FindAndLockSensor(hSensor);
		if (bLocked)
			break;
		else
		{
			int sleepTimeMs = nStartTime + timeoutMs - 10 - GUtils::OSGetTimeStamp();
			if (sleepTimeMs <= 0)
				break;
			else
			{
				if (sleepTimeMs > 50)
					sleepTimeMs = 50;
				GUtils::Sleep(sleepTimeMs);
			}
		}
	}
	while ((GUtils::OSGetTimeStamp() - nStartTime) <= (unsigned int)timeoutMs);

	return bLocked ? 0 : -1;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_Unlock()
	
	Purpose:	Decrement the lock count for the specified sensor by 1. This routine will fail if the sensor
				is not currently locked by the calling thread.

				In order for a second thread to acquire access to the sensor, the first thread must call 
				GoIO_Sensor_Unlock() until the lock count reaches 0. Note that the lock count is set to 1 by 
				GoIO_Sensor_Open().

				If the lock count reaches zero, it is important to make a call to GoIO_Sensor_Lock() to lock the
				device. This will prevent multiple threads from simultaneously accessing the device, which can cause
				unpredictable results.
				
				It is ok for the lock count to be greater than 1 when GoIO_Sensor_Close() is called.

				This is an advanced routine that most users should not use. It is only necessary if an application
				needs to access a single sensor from multiple threads.
	
	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_Unlock(
	GOIO_SENSOR_HANDLE hSensor)	//[in] handle to open sensor.
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		UnlockSensor(hSensor); //Undo the FindAndLock.

		if (!pGoIOSensor->m_pInterface->UnlockDevice())
			nResult = -1;
	}

	return nResult;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_ClearIO()
	
	Purpose:	Clear the input/output communication buffers for a specified sensor. 
				This also expties the GoIO Measurement Buffer, which holds measurements that have been retrieved from 
				the sensor.
	
	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_ClearIO(
	GOIO_SENSOR_HANDLE hSensor)	//[in] handle to open sensor.
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		nResult = pGoIOSensor->m_pInterface->ClearIO();

		UnlockSensor(hSensor);
	}

	return nResult;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_SendCmdAndGetResponse()
	
	Purpose:	Send a command to the specified Go! device and wait for a response. 
	
	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_SendCmdAndGetResponse(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	unsigned char cmd,			//[in] command code. See SKIP_CMD_ID_* in GSkipCommExt.h.
	void *pParams,				//[in] ptr to cmd specific parameter block, may be NULL. See GSkipCommExt.h.
	gtype_int32 nParamBytes,	//[in] # of bytes in (*pParams).
	void *pRespBuf,				//[out] ptr to destination buffer, may be NULL. See GSkipCommExt.h.
	gtype_int32 *pnRespBytes,	//[in, out] ptr to size of of pRespBuf buffer on input, size of response on output, may be NULL if pRespBuf is NULL.
	gtype_int32 timeoutMs)		//[in] # of milliseconds to wait for a reply before giving up. Go! devices should reply to almost all the 
								//currently defined commands within SKIP_DEFAULT_TIMEOUT_MS(1000) milliseconds. In fact, typical response
								//times are less than 50 milliseconds. See SKIP_TIMEOUT_MS_* definitions.
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		GSTD_ASSERT(sizeof(gtype_int32) == sizeof(long));
		nResult = pGoIOSensor->m_pInterface->SendCmdAndGetResponse(cmd, pParams, nParamBytes, pRespBuf, (long *) pnRespBytes, timeoutMs);

		UnlockSensor(hSensor);
	}

	return nResult;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_SendCmd()
	
	Purpose:	GoIO_Sensor_SendCmd() is an advanced function. You should usually use 
				GoIO_Sensor_SendCmdAndGetResponse() instead. After calling GoIO_Sensor_SendCmd(), you must call
				GoIO_Sensor_GetNextResponse() before sending any more commands to the device.

				The main reason that GoIO_Sensor_SendCmd() is made available to the user is to allow a program to send
				SKIP_CMD_ID_START_MEASUREMENTS commands to several different devices as close together as possible so that
				measurements start at about the same time on separate devices.
	
	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_SendCmd(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	unsigned char cmd,	//[in] command code
	void *pParams,			//[in] ptr to cmd specific parameter block, may be NULL. See GSkipCommExt.h.
	gtype_int32 nParamBytes)//[in] # of bytes in (*pParams).
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		GSTD_ASSERT(sizeof(gtype_int32) == sizeof(long));
		nResult = pGoIOSensor->m_pInterface->SendCmd(cmd, pParams, nParamBytes);

		UnlockSensor(hSensor);
	}

	return nResult;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetNextResponse()
	
	Purpose:	GoIO_Sensor_GetNextResponse() is an advanced function. You should usually use 
				GoIO_Sensor_SendCmdAndGetResponse() instead. After calling GoIO_Sensor_SendCmd(), you must call
				GoIO_Sensor_GetNextResponse() before sending any more commands to the device.


	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_GetNextResponse(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	void *pRespBuf,				//[out] ptr to destination buffer, may be NULL. See GSkipCommExt.h.
	gtype_int32 *pnRespBytes,	//[in, out] ptr to size of of pRespBuf buffer on input, size of response on output, may be NULL if pRespBuf is NULL.
	unsigned char *pCmd,		//[out] identifies which command this response is for. Ptr must NOT be NULL!
	gtype_int32 *pErrRespFlag,	//[out] flag(1 or 0) indicating that the response contains error info. Ptr must NOT be NULL!
	gtype_int32 nTimeoutMs)		//[in] # of milliseconds to wait before giving up.
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		GSTD_ASSERT(sizeof(gtype_int32) == sizeof(long));
		bool errRespFlag = false;
		nResult = pGoIOSensor->m_pInterface->GetNextResponse(pRespBuf, (long *) pnRespBytes, pCmd, &errRespFlag, nTimeoutMs);
		if (0 == nResult)
			(*pErrRespFlag) = errRespFlag ? 1 : 0;


		UnlockSensor(hSensor);
	}

	return nResult;
}


/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetMeasurementTickInSeconds()
	
	Purpose:	The measurement period for Go! devices is specified in discrete 'ticks', so the actual time between 
				measurements is an integer multiple of the tick time. The length of time between ticks is different 
				for Go! Link versus Go! Temp. 
	
	Return:		If hSensor is not valid, then this routine returns -1.0, else the return value = the length of time
				in seconds between ticks.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_real64 GoIO_Sensor_GetMeasurementTickInSeconds(
	GOIO_SENSOR_HANDLE hSensor)	//[in] handle to open sensor.
{
	gtype_real64 tickTime;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		tickTime = -1.0;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		tickTime = pGoIOSensor->m_pInterface->GetMeasurementTickInSeconds();

		UnlockSensor(hSensor);
	}

	return tickTime;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetMinimumMeasurementPeriod()
	
	Return:		If hSensor is not valid, then this routine returns -1.0, else the return value = minimum measurement
				period in seconds that is supported by the device.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_real64 GoIO_Sensor_GetMinimumMeasurementPeriod(
	GOIO_SENSOR_HANDLE hSensor)	//[in] handle to open sensor.
{
	gtype_real64 minPeriod;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		minPeriod = -1.0;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		minPeriod = pGoIOSensor->m_pInterface->GetMinimumMeasurementPeriodInSeconds();

		UnlockSensor(hSensor);
	}

	return minPeriod;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetMaximumMeasurementPeriod()
	
	Return:		If hSensor is not valid, then this routine returns -1.0, else the return value = maximum measurement
				period in seconds that is supported by the device.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_real64 GoIO_Sensor_GetMaximumMeasurementPeriod(
	GOIO_SENSOR_HANDLE hSensor)	//[in] handle to open sensor.
{
	gtype_real64 maxPeriod;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		maxPeriod = -1.0;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		maxPeriod = pGoIOSensor->m_pInterface->GetMaximumMeasurementPeriodInSeconds();

		UnlockSensor(hSensor);
	}

	return maxPeriod;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_SetMeasurementPeriod()
	
	Purpose:	Set the measurement period to a specified number of seconds. The Go! sensor will report measurements
				to the computer at the measurement period interval once measurements have been started. These 
				measurements are held in the GoIO Measurement Buffer. 
				See documentation for GoIO_Sensor_GetNumMeasurementsAvailable().

				This routine sends SKIP_CMD_ID_SET_MEASUREMENT_PERIOD to the sensor with the parameter block configured 
				for the desiredPeriod.

				Because the measurement period is constrained to be a multiple of the tick returned by
				GoIO_Sensor_GetMeasurementTickInSeconds(), and because it must be 
					>= GoIO_Sensor_GetMinimumMeasurementPeriod(), and
					<= GoIO_Sensor_GetMaximumMeasurementPeriod(), 
				the actual period is slightly different than the desiredPeriod.

				You can determine the actual period by calling GoIO_Sensor_GetMeasurementPeriod().

				This routine will fail if we are currently collecting measurements from the sensor. Note that collection
				is started by sending SKIP_CMD_ID_START_MEASUREMENTS to the sensor, and stopped by sending 
				SKIP_CMD_ID_STOP_MEASUREMENTS.
	
	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_SetMeasurementPeriod(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	gtype_real64 desiredPeriod,	//[in] desired measurement period in seconds.
	gtype_int32 timeoutMs)		//[in] # of milliseconds to wait for a reply before giving up. SKIP_TIMEOUT_MS_DEFAULT is recommended.
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		nResult = pGoIOSensor->m_pInterface->SetMeasurementPeriod(desiredPeriod, timeoutMs);

		UnlockSensor(hSensor);
	}

	return nResult;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetMeasurementPeriod()
	
	Return:		1000000.0 if not successful, else the measurement period in seconds.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_real64 GoIO_Sensor_GetMeasurementPeriod(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	gtype_int32 timeoutMs)		//[in] # of milliseconds to wait for a reply before giving up. SKIP_TIMEOUT_MS_DEFAULT is recommended.
{
	gtype_real64 period;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		period = -1.0;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		period = pGoIOSensor->m_pInterface->GetMeasurementPeriod(timeoutMs);

		UnlockSensor(hSensor);
	}

	return period;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetNumMeasurementsAvailable()
	
	Purpose:	Report the number of measurements currently stored in the GoIO Measurement Buffer. 

				After SKIP_CMD_ID_START_MEASUREMENTS has been sent to the sensor, the sensor starts
				sending measurements to the host computer. These measurements are stored in the 
				GoIO Measurement Buffer. A separate GoIO Measurement Buffer is maintained for each
				open sensor.

				The first measurement is sent almost immediatedly after SKIP_CMD_ID_START_MEASUREMENTS
				is issued. Subsequent measurements are sent at the currently configured measurement
				period interval. See GoIO_Sensor_SetMeasurementPeriod().

				Call GoIO_Sensor_ReadRawMeasurements() to retrieve measurements from the
				GoIO Measurement Buffer. The GoIO Measurement Buffer is guaranteed to hold up to 1000
				measurements. The buffer is circular, so if you do not service it often enough, the
				oldest measurements in the buffer are lost. If you wish to capture all the 
				measurements coming from the sensor, you must call GoIO_Sensor_ReadRawMeasurements()
				often enough so that the GoIO_Sensor_GetNumMeasurementsAvailable() does not reach 1000.

				Each of the following actions clears the GoIO Measurement Buffer:
					1) Call GoIO_Sensor_ReadRawMeasurements() with count set to GoIO_Sensor_GetNumMeasurementsAvailable(), or
					2) Call GoIO_Sensor_GetLatestRawMeasurement(), or
					3) Call GoIO_Sensor_ClearIO().

				The GoIO Measurement Buffer is empty after GoIO_Sensor_Open() is called. It does not
				start filling up until SKIP_CMD_ID_START_MEASUREMENTS is sent to the sensor.

				Note that the sensor stops sending measurements to the host computer after
				SKIP_CMD_ID_STOP_MEASUREMENTS or SKIP_CMD_ID_INIT is sent, but sending these commands
				does not clear the GoIO Measurement Buffer.

	Return:		number of measurements currently stored in the GoIO Measurement Buffer. 

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_GetNumMeasurementsAvailable(
GOIO_SENSOR_HANDLE hSensor)	//[in] handle to open sensor.
{
	gtype_int32 nResult = 0;
	if (OpenSensorVector_FindAndLockSensor(hSensor))
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		nResult = pGoIOSensor->m_pInterface->MeasurementsAvailable();

		UnlockSensor(hSensor);
	}

	return nResult;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_ReadRawMeasurements()
	
	Purpose:	Retrieve measurements from the GoIO Measurement Buffer. The measurements reported
				by this routine are actually removed from the GoIO Measurement Buffer.

				After SKIP_CMD_ID_START_MEASUREMENTS has been sent to the sensor, the sensor starts
				sending measurements to the host computer. These measurements are stored in the 
				GoIO Measurement Buffer. A separate GoIO Measurement Buffer is maintained for each
				open sensor. See the description of GoIO_Sensor_GetNumMeasurementsAvailable().

				Even though a raw measurement is reported as a long, it can be stored in a short 
				integer: it ranges in value from -32768 to 32767.

				To convert a raw measurement to a voltage use GoIO_Sensor_ConvertToVoltage().
				To convert a voltage to a sensor specific calibrated unit, use GoIO_Sensor_CalibrateData().

	Return:		number of measurements retrieved from the GoIO Measurement Buffer. This routine
				returns immediately, so the return value may be less than maxCount.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_ReadRawMeasurements(
	GOIO_SENSOR_HANDLE hSensor,		//[in] handle to open sensor.
	gtype_int32 *pMeasurementsBuf,	//[out] ptr to loc to store measurements.
	gtype_int32 maxCount)			//[in] maximum number of measurements to copy to pMeasurementsBuf.
{
	gtype_int32 nResult = 0;
	if (OpenSensorVector_FindAndLockSensor(hSensor))
	{
		intVector vec;
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		vec = pGoIOSensor->m_pInterface->ReadRawMeasurements(maxCount);
		for (nResult = 0; (nResult < (gtype_int32)vec.size()) && (nResult < maxCount); nResult++)
			pMeasurementsBuf[nResult] = vec[nResult];

		UnlockSensor(hSensor);
	}

	return nResult;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetLatestRawMeasurement()
	
	Purpose:	Report the most recent measurement put in the GoIO Measurement Buffer. If no 
				measurements have been placed in the GoIO Measurement Buffer since it was
				created byGoIO_Sensor_Open(), then report a value of 0. 
				
				This routine also empties the GoIO Measurement Buffer, so GoIO_Sensor_GetNumMeasurementsAvailable()
				will report 0 after calling GoIO_Sensor_GetLatestRawMeasurement().

				After SKIP_CMD_ID_START_MEASUREMENTS has been sent to the sensor, the sensor starts
				sending measurements to the host computer. These measurements are stored in the 
				GoIO Measurement Buffer. A separate GoIO Measurement Buffer is maintained for each
				open sensor. See the description of GoIO_Sensor_GetNumMeasurementsAvailable().

				Even though a raw measurement is reported as a long, it can be stored in a short 
				integer: it ranges in value from -32768 to 32767.

				To convert a raw measurement to a voltage use GoIO_Sensor_ConvertToVoltage().
				To convert a voltage to a sensor specific calibrated unit, use GoIO_Sensor_CalibrateData().

	Return:		most recent measurement put in the GoIO Measurement Buffer. If no 
				measurements have been placed in the GoIO Measurement Buffer since it was
				created byGoIO_Sensor_Open(), then report a value of 0.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_GetLatestRawMeasurement(
	GOIO_SENSOR_HANDLE hSensor)	//[in] handle to open sensor.
{
	gtype_int32 nResult = 0;
	if (OpenSensorVector_FindAndLockSensor(hSensor))
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		nResult = pGoIOSensor->m_pInterface->GetLatestRawMeasurement();

		UnlockSensor(hSensor);
	}
	
	return nResult;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_ConvertToVoltage()
	
	Purpose:	Convert a raw measurement in the range -32768 to 32767 into a voltage value.
				Depending on the type of sensor(see GoIO_Sensor_GetProbeType()), the voltage
				may range from 0.0 to 5.0, or from -10.0 to 10.0 . For Go! Motion, voltage returned is simply distance
				in meters.

	Return:		voltage corresponding to a specified raw measurement value.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_real64 GoIO_Sensor_ConvertToVoltage(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	gtype_int32 rawMeasurement)	//[in] raw measurement obtained from GoIO_Sensor_GetLatestRawMeasurement() or 
								//GoIO_Sensor_ReadRawMeasurements().
{
	gtype_real64 volts = 0.0;
	if (OpenSensorVector_FindAndLockSensor(hSensor))
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		volts = pGoIOSensor->m_pInterface->ConvertToVoltage(rawMeasurement, pGoIOSensor->m_pMBLSensor->GetProbeType());

		UnlockSensor(hSensor);
	}

	return volts;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_CalibrateData()
	
	Purpose:	Convert a voltage value into a sensor specific unit.
				What units this routine produces can be determined by calling
				GoIO_Sensor_DDSMem_GetCalPage(hSensor, GoIO_Sensor_DDSMem_GetActiveCalPage(),...) .

	Return:		value in sensor specific units corresponding to a specified voltage. Just return input volts
				unless GoIO_Sensor_DDSMem_GetCalibrationEquation() indicates kEquationType_Linear.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_real64 GoIO_Sensor_CalibrateData(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	gtype_real64 volts)			//[in] voltage value obtained from GoIO_Sensor_ConvertToVoltage();
{
	gtype_real64 measurement = 0.0;
	if (OpenSensorVector_FindAndLockSensor(hSensor))
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		measurement = pGoIOSensor->m_pMBLSensor->CalibrateData(volts);

		UnlockSensor(hSensor);
	}

	return measurement;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_GetProbeType()
	
	Purpose:	Find out if the probe type is kProbeTypeAnalog5V or kProbeTypeAnalog10V.

				This attribute is dependent on the OperationType in the SensorDDSRecord. 
				See GoIO_Sensor_DDSMem_GetOperationType().
				If (2 == OperationType) then the sensor is kProbeTypeAnalog10V, else kProbeTypeAnalog5V.

				Note that for Go! Link devices, SKIP_CMD_ID_SET_ANALOG_INPUT_CHANNEL must be sent to the device with
				the analogInputChannel parameter set to SKIP_ANALOG_INPUT_CHANNEL_VIN_LOW for kProbeTypeAnalog5V,
				or set to SKIP_ANALOG_INPUT_CHANNEL_VIN for kProbeTypeAnalog10V devices. Generally, application
				programs do not have to deal with this, because GoIO_Sensor_Open() automatically sends
				SKIP_CMD_ID_SET_ANALOG_INPUT_CHANNEL to the device with the appropriate parameters.

	Return:		kProbeTypeAnalog5V or kProbeTypeAnalog10V.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_GetProbeType(
	GOIO_SENSOR_HANDLE hSensor)	//[in] handle to open sensor.
{
	gtype_int32 nResult = kProbeTypeAnalog5V;
	if (OpenSensorVector_FindAndLockSensor(hSensor))
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		nResult = pGoIOSensor->m_pMBLSensor->GetProbeType();

		UnlockSensor(hSensor);
	}

	return nResult;
}


/***************************************************************************************************************************/
/***************************************************************************************************************************/
/***************************************************************************************************************************
GoIO_Sensor_DDSMem_* routines:

Each GoIO sensor object maintains a SensorDDSRecord of type GSensorDDSRec. The fields in this record describe the sensor.
The GoIO_Sensor_DDSMem_* routines allow an application program to access the SensorDDSRecord.
See GSensorDDSMem.h for the declaration of GSensorDDSRec.

Sensors come in 3 basic flavors:
1) Smart sensors,
2) AutoId capable sensors that are not smart, and
3) Legacy sensors.

Smart sensors store the entire 128 byte GSensorDDSRec record on the actual sensor hardware. This allows calibration
data that is specific to a given sensor to be stored on the sensor. Smart sensors report an AutoId number
in response to the SKIP_CMD_ID_GET_SENSOR_ID command that is >= kSensorIdNumber_FirstSmartSensor. 

If a sensor reports an AutoId >= kSensorIdNumber_FirstSmartSensor, then GoIO_Sensor_Open() will get the GSensorDDSRec 
info from the sensor hardware and store in the GoIO sensor object's SensorDDSRecord.

AutoId capable sensors that are not smart report an AutoId value > 0 and < kSensorIdNumber_FirstSmartSensor. 

Legacy (aka Dumb) sensors report an AutoId = 0.

Non smart sensors just get default values for most of the fields in the SensorDDSRecord. The default calibration 
parameters are configured so that GoIO_Sensor_CalibrateData() just reports volts.

For all 3 types of sensors, SensorDDSRecord.SensorNumber = AutoId after GoIO_Sensor_Open().

****************************************************************************************************************************/
/***************************************************************************************************************************/
/***************************************************************************************************************************/

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_DDSMem_WriteRecord()
	
	Purpose:	Write the SensorDDSRecord into a smart sensor's DDS memory hardware. 
				Before the record is written to the sensor, SensorDDSRecord.Checksum is recalculated.
				Before the record is written to the sensor, a copy of the record is marshalled into little endian format,
				and then this copy is actually written to the sensor.

				WARNING: Be careful about using this routine. Changing a smart sensor's DDS memory can cause the sensor
				to stop working with Logger Pro.
	
	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_WriteRecord(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	gtype_int32 timeoutMs)		//[in] # of milliseconds to wait for a reply before giving up. SKIP_TIMEOUT_MS_WRITE_DDSMEMBLOCK is recommended.
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;

		//Verify that we are dealing with a smart sensor.
		//Do not try to write the SensorDDSRecord unless this is a smart sensor because old versions of Go! Link(ver. 1.5426)
		//will hang if we try to write the SensorDDSRecord when no smart sensor is connected.
		GSkipGetSensorIdCmdResponsePayload getSensorIdResponsePayload;
		long nBytesRead = sizeof(GSkipGetSensorIdCmdResponsePayload);
		nResult = pGoIOSensor->m_pInterface->SendCmdAndGetResponse(SKIP_CMD_ID_GET_SENSOR_ID, NULL, 0, 
			&getSensorIdResponsePayload, &nBytesRead);
		if (0 == nResult)
		{
			//Parse out sensor id.
			int nSensorId;
			GUtils::OSConvertBytesToInt(getSensorIdResponsePayload.lsbyteLswordSensorId, 
				getSensorIdResponsePayload.msbyteLswordSensorId, getSensorIdResponsePayload.lsbyteMswordSensorId, 
				getSensorIdResponsePayload.msbyteMswordSensorId, &nSensorId);

			if (nSensorId < 0)
				nSensorId = 0;
			else
			if (nSensorId > 255)
				nSensorId = 0;

			if (nSensorId < kSensorIdNumber_FirstSmartSensor)
				nResult = -1;
		}

		if (0 == nResult)
		{
			//Update checksum.
			pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->Checksum = 
				GMBLSensor::CalculateDDSDataChecksum(*pGoIOSensor->m_pMBLSensor->GetDDSRecPtr());

			//Marshall the SensorDDSRecord data so that it is in little-endian format.
			GSensorDDSRec littleEndianRec;
			GMBLSensor::MarshallDDSRec(&littleEndianRec, *pGoIOSensor->m_pMBLSensor->GetDDSRecPtr());

			//Write the data to the sensor!
			nResult = pGoIOSensor->m_pInterface->WriteSensorDDSMemory((unsigned char *) &littleEndianRec, 0, 
				sizeof(littleEndianRec), timeoutMs);
		}

		UnlockSensor(hSensor);
	}

	return nResult;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_DDSMem_ReadRecord()
	
	Purpose:	Read the SensorDDSRecord from a smart sensor's DDS memory hardware. 

				After the record is read from the sensor, it is unmarshalled from little endian format into processor
				specific format.
	
	Return:		0 if successful, 
				else if data validation fails then -2,
				else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_ReadRecord(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	gtype_int32 strictDDSValidationFlag,//[in] insist on exactly valid checksum if 1, else use a more lax validation test.
	gtype_int32 timeoutMs)	//[in] # of milliseconds to wait for a reply before giving up. SKIP_TIMEOUT_MS_READ_DDSMEMBLOCK is recommended.
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		GSensorDDSRec littleEndianRec;

		//Verify that we are dealing with a smart sensor.
		//Do not try to read the SensorDDSRecord unless this is a smart sensor because old versions of Go! Link(ver. 1.5426)
		//will hang if we try to read the SensorDDSRecord when no smart sensor is connected.
		GSkipGetSensorIdCmdResponsePayload getSensorIdResponsePayload;
		long nBytesRead = sizeof(GSkipGetSensorIdCmdResponsePayload);
		nResult = pGoIOSensor->m_pInterface->SendCmdAndGetResponse(SKIP_CMD_ID_GET_SENSOR_ID, NULL, 0, 
			&getSensorIdResponsePayload, &nBytesRead);
		if (0 == nResult)
		{
			//Parse out sensor id.
			int nSensorId;
			GUtils::OSConvertBytesToInt(getSensorIdResponsePayload.lsbyteLswordSensorId, 
				getSensorIdResponsePayload.msbyteLswordSensorId, getSensorIdResponsePayload.lsbyteMswordSensorId, 
				getSensorIdResponsePayload.msbyteMswordSensorId, &nSensorId);

			if (nSensorId < 0)
				nSensorId = 0;
			else
			if (nSensorId > 255)
				nSensorId = 0;

			if (nSensorId < kSensorIdNumber_FirstSmartSensor)
				nResult = -1;
		}

		if (0 == nResult)
		{
			nResult = pGoIOSensor->m_pInterface->ReadSensorDDSMemory((unsigned char *) &littleEndianRec, 0, 
				sizeof(littleEndianRec), timeoutMs);
		}

		if (0 == nResult)
		{
			if (!GMBLSensor::VerifyDDSChecksum(littleEndianRec, (strictDDSValidationFlag != 0)))
				nResult = -2;
		}

		if (0 == nResult)
			pGoIOSensor->m_pMBLSensor->SetDDSRec(littleEndianRec, true);

		UnlockSensor(hSensor);
	}

	return nResult;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_DDSMem_SetRecord()
	
	Purpose:	Update the SensorDDSRecord. 
				This routine does not send data to the sensor's DDS memory hardware - use GoIO_Sensor_DDSMem_WriteRecord for that.

	Return:		0 if hSensor is valid, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetRecord(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	const GSensorDDSRec *pRec)	//[in] ptr to data to copy into the SensorDDSRecord.
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->SetDDSRec(*pRec, false);

		UnlockSensor(hSensor);
	}

	return nResult;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_DDSMem_GetRecord()
	
	Purpose:	Get a copy of the SensorDDSRecord. 
				This routine does not read data from the sensor's DDS memory hardware - use GoIO_Sensor_DDSMem_ReadRecord for that.

	Return:		0 if hSensor is valid, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetRecord(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	GSensorDDSRec *pRec)		//[out] ptr to dest buf to copy the SensorDDSRecord into.
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRec(pRec);
		UnlockSensor(hSensor);
	}

	return nResult;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_DDSMem_CalculateChecksum()
	
	Return:		0 if hSensor is valid, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_CalculateChecksum(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	unsigned char *pChecksum)	//[out] ptr to checksum calculated by XOR'ing the first 127 bytes of the SensorDDSRecord.
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pChecksum = GMBLSensor::CalculateDDSDataChecksum(*pGoIOSensor->m_pMBLSensor->GetDDSRecPtr());
		UnlockSensor(hSensor);
	}

	return nResult;
}

/***************************************************************************************************************************/
/***************************************************************************************************************************/
/***************************************************************************************************************************
Languages that can handle GSensorDDSRec can access the DDS fields using GoIO_Sensor_DDSMem_SetRecord and GetRecord.
Languages such as Basic are unlikely to be able to handle a complex record like GSensorDDSRec, especially because of
byte alignment issues. The individual field level API's defined below provide an alternate way to access the SensorDDSRecord.
****************************************************************************************************************************/
/***************************************************************************************************************************/
/***************************************************************************************************************************/

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetMemMapVersion(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char MemMapVersion)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->MemMapVersion = MemMapVersion;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetMemMapVersion(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pMemMapVersion)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pMemMapVersion = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->MemMapVersion;
		UnlockSensor(hSensor);
	}

	return nResult;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_DDSMem_SetSensorNumber()
	
	Purpose:	Normally SensorDDSRecord.SensorNumber is automatically set inside GoIO_Sensor_Open().
				However, if the current sensor is a legacy version, then SensorDDSRecord.SensorNumber = 0, so
				you might want to use GoIO_Sensor_DDSMem_SetSensorNumber() to change it.

				
	SIDE EFFECTS(Go! Link only):	
				If the new SensorDDSRecord.SensorNumber is set to kSensorIdNumber_Voltage10, then
					SensorDDSRecord.OperationType is set = 2 to imply a probeType of kProbeTypeAnalog10V,
				else
					if SensorDDSRecord.OperationType is == 2, then
						SensorDDSRecord.OperationType is set = 14 to imply a probeType of kProbeTypeAnalog5V.

				If the GoIO_Sensor_DDSMem_SetSensorNumber() causes the probeType to change, then you should
				send a SKIP_CMD_ID_SET_ANALOG_INPUT_CHANNEL command to the sensor. See GoIO_Sensor_GetProbeType().

	Return:		0 if hSensor is valid and sensor is connected to a Go! Link, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetSensorNumber(
	GOIO_SENSOR_HANDLE hSensor,	//[in] handle to open sensor.
	unsigned char SensorNumber)	//[in]
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		GSkipDevice *pSkip = dynamic_cast<GSkipDevice *>(pGoIOSensor->m_pInterface);
		if (!pSkip)
			nResult = -1;
		else
			pGoIOSensor->m_pMBLSensor->SetID(SensorNumber);//This may cause GetDDSRecPtr()->OperationType to change also!
		UnlockSensor(hSensor);
	}

	return nResult;
}

/***************************************************************************************************************************
	Function Name: GoIO_Sensor_DDSMem_GetSensorNumber()
	
	Purpose:	Retrieve SensorDDSRecord.SensorNumber. 
	
				If sendQueryToHardwareflag != 0, then send a SKIP_CMD_ID_GET_SENSOR_ID to the sensor hardware
				 - sendQueryToHardwareflag is ignored for Go! Temp.

				If the sensor hardware reports a new SensorNumber, then a user has probably changed what sensor is
				plugged into a Go! Link. The simplest course of action in this case is to call
				GoIO_Sensor_GetOpenDeviceName() to get the device id, call GoIO_Sensor_Close(), and then
				call GoIO_Sensor_Open() with the values obtained from GoIO_Sensor_GetOpenDeviceName() to reopen the
				device. This will cause the GoIO_Sensor object to be properly configured for the new sensor.

	Return:		0 if successful, else -1.

****************************************************************************************************************************/
GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetSensorNumber(
	GOIO_SENSOR_HANDLE hSensor,		//[in] handle to open sensor. 
	unsigned char *pSensorNumber,	//[out] ptr to SensorNumber.
	gtype_int32 sendQueryToHardwareflag,//[in] If sendQueryToHardwareflag != 0, then send a SKIP_CMD_ID_GET_SENSOR_ID to the sensor hardware. 
	gtype_int32 timeoutMs)//[in] # of milliseconds to wait for a reply before giving up. SKIP_TIMEOUT_MS_DEFAULT is recommended.
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		if ((0 == sendQueryToHardwareflag) 
			|| (USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID == pGoIOSensor->m_pInterface->GetPortRefPtr()->GetUSBProductID())
			|| (CYCLOPS_DEFAULT_PRODUCT_ID == pGoIOSensor->m_pInterface->GetPortRefPtr()->GetUSBProductID()))
			*pSensorNumber = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorNumber;
		else
		{
			//Send command to the sensor.
			GSkipGetSensorIdCmdResponsePayload getSensorIdResponsePayload;
			long nBytesRead = sizeof(GSkipGetSensorIdCmdResponsePayload);
			long nResult = pGoIOSensor->m_pInterface->SendCmdAndGetResponse(SKIP_CMD_ID_GET_SENSOR_ID, NULL, 0, 
				&getSensorIdResponsePayload, &nBytesRead, timeoutMs);
			if (0 == nResult)
			{
				//Parse out sensor id.
				int nSensorId;
				GUtils::OSConvertBytesToInt(getSensorIdResponsePayload.lsbyteLswordSensorId, 
					getSensorIdResponsePayload.msbyteLswordSensorId, getSensorIdResponsePayload.lsbyteMswordSensorId, 
					getSensorIdResponsePayload.msbyteMswordSensorId, &nSensorId);

				if (nSensorId < 0)
					nSensorId = 0;
				else
				if (nSensorId > 255)
					nSensorId = 0;
				*pSensorNumber = (unsigned char) nSensorId;
				pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorNumber = *pSensorNumber;
			}
		}
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetSerialNumber(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char leastSigByte_SerialNumber,
	unsigned char midSigByte_SerialNumber,
	unsigned char mostSigByte_SerialNumber)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorSerialNumber[0] = leastSigByte_SerialNumber;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorSerialNumber[1] = midSigByte_SerialNumber;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorSerialNumber[2] = mostSigByte_SerialNumber;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetSerialNumber(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pLeastSigByte_SerialNumber,
	unsigned char *pMidSigByte_SerialNumber,
	unsigned char *pMostSigByte_SerialNumber)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pLeastSigByte_SerialNumber = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorSerialNumber[0];
		*pMidSigByte_SerialNumber = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorSerialNumber[1];
		*pMostSigByte_SerialNumber = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorSerialNumber[2];
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetLotCode(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char YY_BCD,
	unsigned char WW_BCD)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorLotCode[0] = YY_BCD;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorLotCode[1] = WW_BCD;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetLotCode(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pYY_BCD,
	unsigned char *pWW_BCD)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pYY_BCD = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorLotCode[0];
		*pWW_BCD = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorLotCode[1];
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetManufacturerID(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char ManufacturerID)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->ManufacturerID = ManufacturerID;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetManufacturerID(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pManufacturerID)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pManufacturerID = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->ManufacturerID;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetLongName(
	GOIO_SENSOR_HANDLE hSensor,
	const char *pLongName)//[in] ptr to NULL terminated string.
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		strncpy(pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorLongName, pLongName, 
			sizeof(pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorLongName));
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetLongName(
	GOIO_SENSOR_HANDLE hSensor,		//[in] handle to open sensor. 
	char *pLongName,				//[out] ptr to buffer for NULL terminated output string.
	gtype_uint16 maxNumBytesToCopy)	//[in] size of pLongName buffer.
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		char tmpstring[30];
		GSTD_ASSERT(sizeof(tmpstring) > sizeof(pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorLongName));
		memset(tmpstring, 0, sizeof(tmpstring));
		memcpy(tmpstring, pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorLongName, 
			sizeof(pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorLongName));
		strncpy(pLongName, tmpstring, maxNumBytesToCopy);
		pLongName[maxNumBytesToCopy - 1] = 0;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetShortName(
	GOIO_SENSOR_HANDLE hSensor,
	const char *pShortName)//[in] ptr to NULL terminated string.
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		strncpy(pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorShortName, pShortName, 
			sizeof(pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorShortName));
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetShortName(
	GOIO_SENSOR_HANDLE hSensor,			//[in] handle to open sensor. 
	char *pShortName,					//[out] ptr to buffer for NULL terminated output string. 
	gtype_uint16 maxNumBytesToCopy)		//[in] size of pShortName buffer.
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		char tmpstring[30];
		GSTD_ASSERT(sizeof(tmpstring) > sizeof(pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorShortName));
		memset(tmpstring, 0, sizeof(tmpstring));
		memcpy(tmpstring, pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorShortName, 
			sizeof(pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SensorShortName));
		strncpy(pShortName, tmpstring, maxNumBytesToCopy);
		pShortName[maxNumBytesToCopy - 1] = 0;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetUncertainty(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char Uncertainty)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->Uncertainty = Uncertainty;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetUncertainty(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pUncertainty)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pUncertainty = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->Uncertainty;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetSignificantFigures(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char SignificantFigures)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SignificantFigures = SignificantFigures;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetSignificantFigures(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pSignificantFigures)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pSignificantFigures = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->SignificantFigures;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetCurrentRequirement(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char CurrentRequirement)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->CurrentRequirement = CurrentRequirement;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetCurrentRequirement(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pCurrentRequirement)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pCurrentRequirement = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->CurrentRequirement;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetAveraging(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char Averaging)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->Averaging = Averaging;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetAveraging(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pAveraging)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pAveraging = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->Averaging;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetMinSamplePeriod(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_real32 MinSamplePeriod)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->MinSamplePeriod = MinSamplePeriod;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetMinSamplePeriod(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_real32 *pMinSamplePeriod)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pMinSamplePeriod = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->MinSamplePeriod;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetTypSamplePeriod(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_real32 TypSamplePeriod)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->TypSamplePeriod = TypSamplePeriod;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetTypSamplePeriod(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_real32 *pTypSamplePeriod)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pTypSamplePeriod = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->TypSamplePeriod;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetTypNumberofSamples(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_uint16 TypNumberofSamples)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->TypNumberofSamples = TypNumberofSamples;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetTypNumberofSamples(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_uint16 *pTypNumberofSamples)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pTypNumberofSamples = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->TypNumberofSamples;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetWarmUpTime(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_uint16 WarmUpTime)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->WarmUpTime = WarmUpTime;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetWarmUpTime(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_uint16 *pWarmUpTime)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pWarmUpTime = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->WarmUpTime;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetExperimentType(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char ExperimentType)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->ExperimentType = ExperimentType;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetExperimentType(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pExperimentType)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pExperimentType = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->ExperimentType;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetOperationType(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char OperationType)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->OperationType = OperationType;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetOperationType(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pOperationType)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pOperationType = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->OperationType;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetCalibrationEquation(
	GOIO_SENSOR_HANDLE hSensor,
	char CalibrationEquation)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->CalibrationEquation = CalibrationEquation;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetCalibrationEquation(
	GOIO_SENSOR_HANDLE hSensor,
	char *pCalibrationEquation)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pCalibrationEquation = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->CalibrationEquation;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetYminValue(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_real32 YminValue)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->YminValue = YminValue;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetYminValue(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_real32 *pYminValue)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pYminValue = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->YminValue;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetYmaxValue(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_real32 YmaxValue)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->YmaxValue = YmaxValue;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetYmaxValue(
	GOIO_SENSOR_HANDLE hSensor,
	gtype_real32 *pYmaxValue)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pYmaxValue = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->YmaxValue;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetYscale(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char Yscale)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->Yscale = Yscale;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetYscale(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pYscale)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pYscale = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->Yscale;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetHighestValidCalPageIndex(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char HighestValidCalPageIndex)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->HighestValidCalPageIndex = HighestValidCalPageIndex;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetHighestValidCalPageIndex(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pHighestValidCalPageIndex)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pHighestValidCalPageIndex = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->HighestValidCalPageIndex;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetActiveCalPage(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char ActiveCalPage)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->ActiveCalPage = ActiveCalPage;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetActiveCalPage(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pActiveCalPage)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pActiveCalPage = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->ActiveCalPage;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetCalPage(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char CalPageIndex,
	gtype_real32 CalibrationCoefficientA,
	gtype_real32 CalibrationCoefficientB,
	gtype_real32 CalibrationCoefficientC,
	const char *pUnits)//[in] ptr to NULL terminated string.
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		if (CalPageIndex < 3)
		{
			GCalibrationPage *pCalPage = &(pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->CalibrationPage[CalPageIndex]);
			pCalPage->CalibrationCoefficientA = CalibrationCoefficientA;
			pCalPage->CalibrationCoefficientB = CalibrationCoefficientB;
			pCalPage->CalibrationCoefficientC = CalibrationCoefficientC;
			strncpy(pCalPage->Units, pUnits, sizeof(pCalPage->Units));
		}
		else
			nResult = -1;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetCalPage(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char CalPageIndex,
	gtype_real32 *pCalibrationCoefficientA,
	gtype_real32 *pCalibrationCoefficientB,
	gtype_real32 *pCalibrationCoefficientC,
	char *pUnits,						//[out] ptr to buffer for NULL terminated output string. 
	gtype_uint16 maxNumBytesToCopy)	//[in] size of pUnits buffer.
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		if (CalPageIndex < 3)
		{
			GCalibrationPage *pCalPage = &(pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->CalibrationPage[CalPageIndex]);
			char tmpstring[30];
			GSTD_ASSERT(sizeof(tmpstring) > sizeof(pCalPage->Units));
			memset(tmpstring, 0, sizeof(tmpstring));
			memcpy(tmpstring, pCalPage->Units, sizeof(pCalPage->Units));
			strncpy(pUnits, tmpstring, maxNumBytesToCopy);
			pUnits[maxNumBytesToCopy - 1] = 0;

			*pCalibrationCoefficientA = pCalPage->CalibrationCoefficientA;
			*pCalibrationCoefficientB = pCalPage->CalibrationCoefficientB;
			*pCalibrationCoefficientC = pCalPage->CalibrationCoefficientC;
		}
		else
			nResult = -1;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_SetChecksum(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char Checksum)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->Checksum = Checksum;
		UnlockSensor(hSensor);
	}

	return nResult;
}

GOIO_DLL_INTERFACE_DECL gtype_int32 GoIO_Sensor_DDSMem_GetChecksum(
	GOIO_SENSOR_HANDLE hSensor,
	unsigned char *pChecksum)
{
	gtype_int32 nResult = 0;
	if (!OpenSensorVector_FindAndLockSensor(hSensor))
		nResult = -1;
	else
	{
		CGoIOSensor *pGoIOSensor = (CGoIOSensor *) hSensor;
		*pChecksum = pGoIOSensor->m_pMBLSensor->GetDDSRecPtr()->Checksum;
		UnlockSensor(hSensor);
	}

	return nResult;
}
