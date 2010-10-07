using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using VSTCoreDefsdotNET;

namespace GoIOdotNET
{
	public class GoIO
	{
		public const Int32 MAX_SIZE_SENSOR_NAME = 220;

		/// <summary>
		/// TIMEOUT_MS_DEFAULT is the recommended timeout in milliseconds for most commands sent to the hardware 
		/// via GoIO_DeviceSendCmdAndGetResponse(). Note that this timeout is much longer than the expected execution time
		/// of GoIO_DeviceSendCmdAndGetResponse() for most commands, which is about 50 milliseconds.
		/// </summary>
		public const Int32 TIMEOUT_MS_DEFAULT = 2000;
		public const Int32 READ_DDSMEMBLOCK_TIMEOUT_MS = 2000;
		public const Int32 WRITE_DDSMEMBLOCK_TIMEOUT_MS = 4000;

        /// <summary>
        /// GoIO_Diags_SetDebugTraceThreshold() threshold parameter value. Setting debug trace threshold to 
        /// TRACE_SEVERITY_LOWEST causes the most messages to be sent to the debug console.
        /// </summary>
        public const byte TRACE_SEVERITY_LOWEST = 1;
        public const byte TRACE_SEVERITY_LOW = 10;
        public const byte TRACE_SEVERITY_MEDIUM = 50;
        public const byte TRACE_SEVERITY_HIGH = 100;

		/// <summary>
		/// Call GoIO_Init() once before any other GoIO_ calls are made.
		/// GoIO_Init() and GoIO_Uninit() should be called from the same thread.
		/// <para>
		/// Currently, only one application at a time may successfully communicate with LabQuests.
		/// If separate apps call GoIO_Init() before calling GoIO_Uninit(), generally only the first one to 
		/// invoke GoIO_Init() will find devices when it calls GoIO_UpdateListOfAvailableDevices() and GoIO_GetNthAvailableDeviceName().
		/// </para>
		/// </summary>
		/// <returns> 0 iff successful, else -1.</returns>
		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Init", CallingConvention = CallingConvention.Cdecl)]
		public static extern IntPtr Init();

		/// <summary>
		/// Call GoIO_Uninit() once to 'undo' GoIO_Init().
		/// GoIO_Init() and GoIO_Uninit() should be called from the same thread.
		/// </summary>
		/// <returns>0 iff successful, else -1.</returns>
		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Uninit", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Uninit();

/*
		/// <summary>
		/// GoIO_Diags_SetDebugTraceThreshold().
		/// </summary>
		/// <param name="hLIb"></param>
		/// <param name="threshold">Only trace messages marked with a severity >= threshold(GoIO.TRACE_SEVERITY_) are actually sent to the debug output.</param>
		/// <returns></returns>
		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Diags_SetDebugTraceThreshold", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Diags_SetDebugTraceThreshold(
			IntPtr hLIb,
			Int32 threshold);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Diags_GetDebugTraceThreshold", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Diags_GetDebugTraceThreshold(
			IntPtr hLIb,
			out Int32 threshold);
*/
		/// <summary>
		/// This routine returns the major and minor version numbers for the instance of the GoIO library that is
		/// currently running.
		/// 
		/// If a function is not guaranteed to be present in all supported versions of the GoIO library, then the line
		/// "Added in version 'major.minor'" will appear in the function description in this file.
		/// 
		/// It is our intention that all versions of the GoIO library created subsequent to a given version, will be
		/// backwards compatible with the older version. You should be able to replace an old version of the GoIO library
		/// with a newer version and everything should still work without rebuilding your application.
		/// 
		/// Note that version major2.minor2 is later than version major1.minor1 
		/// iff. ((major2 > major1) || ((major2 == major1) && (minor2 > minor1))).
		/// 
		/// Backwards compatibility is definitely our intention, but we do not absolutely guarantee it. If you think
		/// that you have detected a backwards compatibility bug, then please report it to Vernier Software & Technology.
		/// Calling GoIO_GetDLLVersion() from your application is a way to identify precisely which version of
		/// the GoIO library you are actually using.
		/// </summary>
		/// <param name="MajorVersion"></param>
		/// <param name="MinorVersion"></param>
		/// <returns></returns>
		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_GetDLLVersion", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 GetDLLVersion(
											out UInt16 MajorVersion,
											out UInt16 MinorVersion);

		/// <summary>
		/// The GoIO library maintains a separate list of available devices for each supported device type.
		/// GoIO_UpdateListOfAvailableDevices() updates the list for the specified device type.
		/// 
		/// </summary>
		/// <param name="vendorId">[in] USB vendor id</param>
		/// <param name="productId">[in] USB product id</param>
		/// <returns>0 iff successful, else -1.</returns>
		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_UpdateListOfAvailableDevices", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 UpdateListOfAvailableDevices(
											Int32 vendorId,
											Int32 productId);

		/// <summary>
		/// Return the Nth entry in the list created by UpdateListOfAvailableDevices().
		/// A device is placed in the list snapshot even if it has already been opened.
		/// 
		/// Pass the device name string placed in devnameBuf to GoIO_Sensor_Open() to open the device. Each
		/// device name string uniquely identifies the device, so you can determine if a device is already open
		/// by comparing devnameBuf with the string returned by GoIO_Sensor_GetOpenDeviceName() for each open
		/// device handle.
		/// </summary>
		/// <param name="devnameBuf">[out] buffer to store device name string. Allocate this with a capacity of GoIO.MAX_SIZE_Sensor_NAME.</param>
		/// <param name="bufSize">[in] Set this to the capacity of devnameBuf.</param>
		/// <param name="vendorId">[in] USB vendor id</param>
		/// <param name="productId">[in] USB product id</param>
		/// <param name="N">[in] index into list of known devices, 0 => first device in list.</param>
		/// <returns>0 iff successful, else -1.</returns>
		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_GetNthAvailableDeviceName", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
		public static extern Int32 GetNthAvailableDeviceName(
			StringBuilder devnameBuf,
			Int32 bufSize,
			Int32 vendorId,
			Int32 productId,
			Int32 N);

		/// <summary>
		/// <para>
		/// Open a device with the name returned by GoIO_GetNthAvailableDeviceName.
		/// </para>
		/// </summary>
		/// <param name="deviceName"></param>
		/// <param name="vendorId">[in] USB vendor id</param>
		/// <param name="productId">[in] USB product id</param>
		/// <param name="strictDDSValidationFlag">[in] insist on exactly valid checksum if 1, else use a more lax validation test.</param>
		/// <returns>handle to open sensor device if successful, else NULL.</returns>
		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_Open", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
		public static extern IntPtr Sensor_Open(
			String deviceName,
			Int32 vendorId,
			Int32 productId,
			Int32 strictDDSValidationFlag);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_Close", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_Close(
			IntPtr hSensor);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_GetOpenDeviceName", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
		public static extern Int32 Sensor_GetOpenDeviceName(
			IntPtr hSensor,
			StringBuilder devnameBuf,
			Int32 bufSize,
			out Int32 vendorId,
			out Int32 productId);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_Lock", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_Lock(
			IntPtr hSensor,
			Int32 timeoutMs);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_Unlock", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_Unlock(
			IntPtr hSensor);

		/// <summary>
		/// GoIO_Sensor_ClearIO()
		/// </summary>
		/// <param name="hSensor"></param>
		/// <param name="channel">-1 implies all channels</param>
		/// <returns></returns>
		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_ClearIO", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_ClearIO(
			IntPtr hSensor);

		/// <summary>
		/// GoIO_Sensor_SendCmdAndGetResponse() is a low level function that most .NET code should not use directly.
		/// Use one of the GoIO_Sensor_SendCmdAndGetResponseN() helper functions instead.
		/// </summary>
		/// <param name="hSensor"></param>
		/// <param name="cmd"></param>
		/// <param name="parameters"></param>
		/// <param name="nParamBytes"></param>
		/// <param name="response"></param>
		/// <param name="nRespBytes">size of of response buffer on input, # of bytes copied into response buffer on output</param>
		/// <param name="timeoutMs"></param>
		/// <returns>0 if successful, else -1.</returns>
		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_SendCmdAndGetResponse", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_SendCmdAndGetResponse(
			IntPtr hSensor,
			byte cmd,
			IntPtr parameters,
			UInt32 nParamBytes,
			IntPtr response,
			ref UInt32 nRespBytes,
			Int32 timeoutMs);

		/// <summary>
		/// Send a command to the specified device hardware and wait for a response. 
		/// <para>
		/// Each device type has a command protocol that is unique to that device type. The command protocol used by
		/// GoIO_DEVTYPE_LABQUEST_AUDIO devices is a subset of the protocol used by GoIO_DEVTYPE_LABQUEST devices. This
		/// protocol is documented in the GoIOSourceCmds class. Additional documentation can be found in GoIOSourceCmds.h.
		/// </para>
		/// <para>
		/// Note that GoIO_Sensor_SendCmdAndGetResponse() will fail if you send a CMD_ID_START_MEASUREMENTS
		/// command while GoIO_Sensor_GetNumMeasurementsAvailable() says measurements are available. 
		/// </para>
		/// <para>
		/// So...if you are restarting measurements, you should clear any old measurements in the GoIO Measurement 
		/// Buffer first by reading in the measurements until the Buffer is empty, or you should call GoIO_Sensor_ClearIO().
		/// </para>
		/// <para>
		/// Every command supported by GoIO_Sensor_SendCmdAndGetResponse() has an associated response. If no response
		/// specific to a command is defined, then the format of the response is GoIODefaultCmdResponse. Some commands
		/// have associated parameter blocks defined for them. 
		/// </para>
		/// <para>
		/// If GoIO_Sensor_SendCmdAndGetResponse() returns -1, additional information about a GoIO_Sensor_SendCmdAndGetResponse() 
		/// error may be obtained by calling GoIO_Sensor_GetLastCmdResponseStatus().
		/// </para>
		/// </summary>
		/// <param name="hSensor">[in] handle to open device.</param>
		/// <param name="cmd">[in] command code.</param>
		/// <param name="parameters">[in] cmd specific parameter block</param>
		/// <param name="response">[out] response block</param>
		/// <param name="nRespBytes">[out] num of bytes passed back in the response block. 
		/// Caller does not need to initialize this with the size of response block(as unmanaged code does) because the dotNET
		/// wrapper does so.
		/// </param>
		/// <param name="timeoutMs">[in] # of milliseconds to wait for a reply before giving up. Most devices should reply to 
		/// almost all the currently defined commands within GoIO.TIMEOUT_MS_DEFAULT milliseconds. In fact, typical response
		/// times are less than 50 milliseconds. </param>
		/// <returns>0 if successful, else -1.</returns>
		public static Int32 Sensor_SendCmdAndGetResponse1(
			IntPtr hSensor,
			byte cmd,
			object parameters,
			ref object response,
			ref UInt32 nRespBytes,
			Int32 timeoutMs)
		{
			IntPtr paramPtr = Marshal.AllocHGlobal(Marshal.SizeOf(parameters));
			Marshal.StructureToPtr(parameters, paramPtr, true);
			IntPtr responsePtr = Marshal.AllocHGlobal(Marshal.SizeOf(response));
			nRespBytes = (UInt32)Marshal.SizeOf(response);
			Int32 status = Sensor_SendCmdAndGetResponse(hSensor, cmd, paramPtr, (UInt32)Marshal.SizeOf(parameters),
				responsePtr, ref nRespBytes, timeoutMs);
			if (0 == status)
				response = Marshal.PtrToStructure(responsePtr, response.GetType());
			else if ((status < 0) && (1 == nRespBytes) && (1 == Marshal.SizeOf(response)))
				response = Marshal.PtrToStructure(responsePtr, response.GetType());
			Marshal.FreeHGlobal(responsePtr);
			Marshal.FreeHGlobal(paramPtr);
			return status;
		}

		/// <summary>
		/// Sensor_SendCmdAndGetResponse2() is the same as Sensor_SendCmdAndGetResponse1() except that no response block is
		/// returned by the function(even though some sort of response always comes back from the device unless there is a 
		/// communication failure). This is reasonable if the caller only cares about when the function succeeds or fails, 
		/// which can be determined from the return value.
		/// </summary>
		/// <param name="hSensor"></param>
		/// <param name="cmd"></param>
		/// <param name="parameters"></param>
		/// <param name="timeoutMs"></param>
		/// <returns>0 if successful, else -1.</returns>
		public static Int32 Sensor_SendCmdAndGetResponse2(
			IntPtr hSensor,
			byte cmd,
			object parameters,
			Int32 timeoutMs)
		{
			UInt32 respLen = 0;
			IntPtr paramPtr = Marshal.AllocHGlobal(Marshal.SizeOf(parameters));
			Marshal.StructureToPtr(parameters, paramPtr, true);
			Int32 status = Sensor_SendCmdAndGetResponse(hSensor, cmd, paramPtr, (UInt32)Marshal.SizeOf(parameters),
				IntPtr.Zero, ref respLen, timeoutMs);
			Marshal.FreeHGlobal(paramPtr);
			return status;
		}

		/// <summary>
		/// Sensor_SendCmdAndGetResponse3() is the same as Sensor_SendCmdAndGetResponse1() except that no parameter block is sent
		/// to the device. That works fine for commands that do not require parameters.
		/// </summary>
		/// <param name="hSensor"></param>
		/// <param name="cmd"></param>
		/// <param name="response"></param>
		/// <param name="nRespBytes"></param>
		/// <param name="timeoutMs"></param>
		/// <returns>0 if successful, else -1.</returns>
		public static Int32 Sensor_SendCmdAndGetResponse3(
			IntPtr hSensor,
			byte cmd,
			ref object response,
			ref UInt32 nRespBytes,
			Int32 timeoutMs)
		{
			IntPtr responsePtr = Marshal.AllocHGlobal(Marshal.SizeOf(response));
			nRespBytes = (UInt32)Marshal.SizeOf(response);
			Int32 status = Sensor_SendCmdAndGetResponse(hSensor, cmd, IntPtr.Zero, 0, responsePtr, ref nRespBytes, timeoutMs);
			if (0 == status)
				response = Marshal.PtrToStructure(responsePtr, response.GetType());
			else if ((status < 0) && (1 == nRespBytes) && (1 == Marshal.SizeOf(response)))
				response = Marshal.PtrToStructure(responsePtr, response.GetType());
			Marshal.FreeHGlobal(responsePtr);
			return status;
		}

		/// <summary>
		/// Sensor_SendCmdAndGetResponse4() is the same as Sensor_SendCmdAndGetResponse1() except that no parameter block is sent
		/// to the device and no response block is returned by the function. That works fine for commands that do not require 
		/// parameters when the caller is only interested in success or failure.
		/// 
		/// </summary>
		/// <param name="hSensor"></param>
		/// <param name="cmd"></param>
		/// <param name="timeoutMs"></param>
		/// <returns>0 if successful, else -1.</returns>
		public static Int32 Sensor_SendCmdAndGetResponse4(
			IntPtr hSensor,
			byte cmd,
			Int32 timeoutMs)
		{
			UInt32 respLen = 0;
			Int32 status = Sensor_SendCmdAndGetResponse(hSensor, cmd, IntPtr.Zero, 0, IntPtr.Zero, ref respLen, timeoutMs);
			return status;
		}

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_GetMeasurementTickInSeconds", CallingConvention = CallingConvention.Cdecl)]
		public static extern double Sensor_GetMeasurementTickInSeconds(
			IntPtr hSensor);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_GetMinimumMeasurementPeriod", CallingConvention = CallingConvention.Cdecl)]
		public static extern double Sensor_GetMinimumMeasurementPeriod(
			IntPtr hSensor);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_GetMaximumMeasurementPeriod", CallingConvention = CallingConvention.Cdecl)]
		public static extern double Sensor_GetMaximumMeasurementPeriod(
			IntPtr hSensor);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_SetMeasurementPeriod", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_SetMeasurementPeriod(
			IntPtr hSensor,
			Double desiredPeriod,
			Int32 timeoutMs);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_GetMeasurementPeriod", CallingConvention = CallingConvention.Cdecl)]
		public static extern double Sensor_GetMeasurementPeriod(
			IntPtr hSensor,
			Int32 timeoutMs);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_GetNumMeasurementsAvailable", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_GetNumMeasurementsAvailable(
			IntPtr hSensor);

		/// <summary>
		/// Retrieve measurements from the GoIO Measurement Buffer. The measurements reported
		/// by this routine are actually removed from the GoIO Measurement Buffer.
		/// <para>
		/// After SKIP_CMD_ID_START_MEASUREMENTS has been sent to the sensor, the sensor starts
		/// sending measurements to the host computer at the rate specified by GoIO_Sensor_SetMeasurementPeriod(). 
		/// These measurements are stored in the GoIO Measurement Buffer. 
		/// A separate GoIO Measurement Buffer is maintained for each open sensor. 
		/// See the description of GoIO_Sensor_GetNumMeasurementsAvailable().
		/// </para>
		/// <para>
		/// Note that for Go! Temp and Go! Link, raw measurements range from -32768 to 32767.
		/// Go! Motion raw measurements are in microns and can range into the millions.
		/// </para>
		/// <para>
		/// To convert a raw measurement to a voltage use GoIO_Sensor_ConvertToVoltage().
		/// To convert a voltage to a sensor specific calibrated unit, use GoIO_Sensor_CalibrateData().
		/// </para>
		/// <para>
		/// WARNING!!! IF YOU ARE COLLECTING MORE THAN 50 MEASUREMENTS A SECOND FROM GO! LINK,
		/// READ THIS: The GoIO Measurement Buffer is packet oriented. If you are collecting 50 or
		/// less measurements per second, then each packet contains only 1 measurement, and there is
		/// no problem.
		/// </para>
		/// <para>
		/// If you are collecting more than 50 measurements a second, then each packet may contain 2 
		/// or 3 measurements. Depending on the exact measurement period, all the packets will 
		/// contain 2, or all the packets will contain 3 measurements. IF THE LAST MEASUREMENT COPIED
		/// INTO THE measurements ARRAY ARGUMENT IS NOT THE LAST MEASUREMENT IN ITS PACKET, THEN MEASUREMENTS WILL
		/// BE LOST.
		/// </para>
		/// <para>
		/// There are a couple of safe workarounds to this problem:
		/// </para>
		/// <para>
		/// 1) Always set the maxCount parameter to a multiple of 6, or
		/// </para>
		/// <para>
		/// 2) Always set the maxCount parameter to GoIO_Sensor_GetNumMeasurementsAvailable().		
		/// </para>
		/// <para>
		/// </para>
		/// </summary>
		/// <param name="hSensor">[in] Handle to open device.</param>
		/// <param name="measurements">[out]Loc to store measurements.</param>
		/// trigger. For normal real time mode, measurements are triggered when CMD_ID_START_MEASUREMENTS is 
		/// received.</param>
		/// <param name="maxCount">Maximum number of measurements to copy to measurements array. The measurements and timestamps 
		/// arrays passed in as parameters must be allocated with a length of at least maxCount elements.
		/// If you are taking measurements faster than 50 hertz from GoLink, then you MUST set maxCount to either a multiple of
		/// 6, or to the value returned by GoIO_Sensor_GetNumMeasurementsAvailable().</param>
		/// <returns>Number of measurements retrieved from the GoIO Measurement Buffer. This routine returns 
		/// immediately, so the return value may be less than maxCount. Return value less than 0 implies error.</returns>
		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_ReadRawMeasurements", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_ReadRawMeasurements(
			IntPtr hSensor,
			Int32[] measurements,
			UInt32 maxCount);

		/// <summary>
		/// Report the most recent measurement put in the GoIO Measurement Buffer. If no 
		/// measurements have been placed in the GoIO Measurement Buffer since it was
		/// created byGoIO_Sensor_Open(), then report a value of 0. 
		/// <para>
		/// This routine also empties the GoIO Measurement Buffer, so GoIO_Sensor_GetNumMeasurementsAvailable()
		/// will report 0 after calling GoIO_Sensor_GetLatestRawMeasurement().
		/// </para>
		/// <para>
		/// After SKIP_CMD_ID_START_MEASUREMENTS has been sent to the sensor, the sensor starts
		/// sending measurements to the host computer. These measurements are stored in the 
		/// GoIO Measurement Buffer. A separate GoIO Measurement Buffer is maintained for each
		/// open sensor. See the description of GoIO_Sensor_GetNumMeasurementsAvailable().
		/// </para>
		/// <para>
		/// Note that for Go! Temp and Go! Link, raw measurements range from -32768 to 32767.
		/// Go! Motion raw measurements are in microns and can range into the millions.
		/// </para>
		/// <para>
		/// To convert a raw measurement to a voltage use GoIO_Sensor_ConvertToVoltage().
		/// To convert a voltage to a sensor specific calibrated unit, use GoIO_Sensor_CalibrateData().
		/// </para>
		/// </summary>
		/// <param name="hSensor"></param>
		/// <returns>most recent measurement put in the GoIO Measurement Buffer. If no 
		/// measurements have been placed in the GoIO Measurement Buffer since it was
		/// created byGoIO_Sensor_Open(), then report a value of 0.</returns>
		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_GetLatestRawMeasurement", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_GetLatestRawMeasurement(
			IntPtr hSensor);

		/// <summary>
		/// Convert a raw measurement integer value into a real voltage value.
		/// Depending on the type of sensor(see GoIO_Sensor_GetProbeType()), the voltage
		/// may range from 0.0 to 5.0, or from -10.0 to 10.0 . For Go! Motion, voltage returned is simply distance
		/// in meters.
		/// </summary>
		/// <param name="hSensor"></param>
		/// <param name="rawMeasurement"></param>
		/// <returns></returns>
		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_ConvertToVoltage", CallingConvention = CallingConvention.Cdecl)]
		public static extern Double Sensor_ConvertToVoltage(
			IntPtr hSensor,
			Int32 rawMeasurement);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_CalibrateData", CallingConvention = CallingConvention.Cdecl)]
		public static extern Double Sensor_CalibrateData(
			IntPtr hSensor,
			Double volts);

		/// <summary>
		/// GoIO_Sensor_GetProbeType()
		/// </summary>
		/// <param name="hSensor"></param>
		/// <param name="channel"></param>
		/// <returns>VSTSensorDDSMemRec.kProbeType...</returns>
		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_GetProbeType", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_GetProbeType(
			IntPtr hSensor);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_WriteRecord", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_WriteRecord(
			IntPtr hSensor,
			Int32 timeoutMs);

		/// <summary>
		/// GoIO_Sensor_DDSMem_ReadRecord().
		/// </summary>
		/// <param name="hSensor"></param>
		/// <param name="channel"></param>
		/// <param name="strictDDSValidationFlag">insist on exactly valid checksum if 1, else use a more lax validation test.</param>
		/// <param name="timeoutMs"></param>
		/// <returns></returns>
		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_ReadRecord", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_ReadRecord(
			IntPtr hSensor,
			byte strictDDSValidationFlag,
			Int32 timeoutMs);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetRecord", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetRecord(
			IntPtr hSensor,
			ref VSTSensorDDSMemRec rec);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetRecord", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetRecord(
			IntPtr hSensor,
			out VSTSensorDDSMemRec rec);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_ClearRecord", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_ClearRecord(
			IntPtr hSensor);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_CalculateChecksum", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_CalculateChecksum(
			IntPtr hSensor,
			out byte checksum);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetMemMapVersion", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetMemMapVersion(
			IntPtr hSensor,
			byte MemMapVersion);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetMemMapVersion", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetMemMapVersion(
			IntPtr hSensor,
			out byte MemMapVersion);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetSensorNumber", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetSensorNumber(
			IntPtr hSensor,
			byte SensorNumber);

		/// <summary>
		/// GoIO_Sensor_DDSMem_GetSensorNumber().
		/// </summary>
		/// <param name="hSensor"></param>
		/// <param name="channel"></param>
		/// <param name="SensorNumber"></param>
		/// <param name="sendQueryToHardwareflag">If sendQueryToHardwareflag != 0, then send a CMD_ID_GET_SENSOR_ID to the sensor hardware.</param>
		/// <param name="timeoutMs"># of milliseconds to wait for a reply before giving up. GoIO.TIMEOUT_MS_DEFAULT is recommended.</param>
		/// <returns></returns>
		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetSensorNumber", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetSensorNumber(
			IntPtr hSensor,
			out byte SensorNumber,
			byte sendQueryToHardwareflag,
			Int32 timeoutMs);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetSerialNumber", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetSerialNumber(
			IntPtr hSensor,
			byte leastSigByte_SerialNumber,
			byte midSigByte_SerialNumber,
			byte mostSigByte_SerialNumber);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetSerialNumber", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetSerialNumber(
			IntPtr hSensor,
			out byte leastSigByte_SerialNumber,
			out byte midSigByte_SerialNumber,
			out byte mostSigByte_SerialNumber);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetLotCode", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetLotCode(
			IntPtr hSensor,
			byte YY_BCD,
			byte WW_BCD);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetLotCode", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetLotCode(
			IntPtr hSensor,
			out byte YY_BCD,
			out byte WW_BCD);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetManufacturerID", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetManufacturerID(
			IntPtr hSensor,
			byte ManufacturerID);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetManufacturerID", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetManufacturerID(
			IntPtr hSensor,
			out byte ManufacturerID);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetLongName", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
		public static extern IntPtr Sensor_DDSMem_SetLongName(
			IntPtr hSensor,
			String longName);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetLongName", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
		public static extern IntPtr Sensor_DDSMem_GetLongName(
			IntPtr hSensor,
			StringBuilder longName,
			UInt16 maxNumBytesToCopy);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetShortName", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
		public static extern IntPtr Sensor_DDSMem_SetShortName(
			IntPtr hSensor,
			String shortName);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetShortName", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
		public static extern IntPtr Sensor_DDSMem_GetShortName(
			IntPtr hSensor,
			StringBuilder shortName,
			UInt16 maxNumBytesToCopy);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetUncertainty", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetUncertainty(
			IntPtr hSensor,
			byte Uncertainty);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetUncertainty", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetUncertainty(
			IntPtr hSensor,
			out byte Uncertainty);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetSignificantFigures", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetSignificantFigures(
			IntPtr hSensor,
			byte SignificantFigures);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetSignificantFigures", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetSignificantFigures(
			IntPtr hSensor,
			out byte SignificantFigures);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetCurrentRequirement", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetCurrentRequirement(
			IntPtr hSensor,
			byte CurrentRequirement);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetCurrentRequirement", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetCurrentRequirement(
			IntPtr hSensor,
			out byte CurrentRequirement);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetAveraging", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetAveraging(
			IntPtr hSensor,
			byte Averaging);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetAveraging", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetAveraging(
			IntPtr hSensor,
			out byte Averaging);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetMinSamplePeriod", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetMinSamplePeriod(
			IntPtr hSensor,
			Single MinSamplePeriod);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetMinSamplePeriod", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetMinSamplePeriod(
			IntPtr hSensor,
			out Single MinSamplePeriod);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetTypSamplePeriod", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetTypSamplePeriod(
			IntPtr hSensor,
			Single TypSamplePeriod);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetTypSamplePeriod", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetTypSamplePeriod(
			IntPtr hSensor,
			out Single TypSamplePeriod);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetTypNumberofSamples", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetTypNumberofSamples(
			IntPtr hSensor,
			UInt16 TypNumberofSamples);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetTypNumberofSamples", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetTypNumberofSamples(
			IntPtr hSensor,
			out UInt16 TypNumberofSamples);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetWarmUpTime", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetWarmUpTime(
			IntPtr hSensor,
			UInt16 WarmUpTime);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetWarmUpTime", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetWarmUpTime(
			IntPtr hSensor,
			out UInt16 WarmUpTime);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetExperimentType", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetExperimentType(
			IntPtr hSensor,
			byte ExperimentType);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetExperimentType", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetExperimentType(
			IntPtr hSensor,
			out byte ExperimentType);

		/// <summary>
		/// GoIO_Sensor_DDSMem_SetOperationType() can change the probe type. See GoIO_Sensor_GetProbeType().
		/// </summary>
		/// <param name="hSensor"></param>
		/// <param name="channel"></param>
		/// <param name="OperationType"></param>
		/// <returns></returns>
		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetOperationType", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetOperationType(
			IntPtr hSensor,
			byte OperationType);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetOperationType", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetOperationType(
			IntPtr hSensor,
			out byte OperationType);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetCalibrationEquation", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetCalibrationEquation(
			IntPtr hSensor,
			sbyte CalibrationEquation);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetCalibrationEquation", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetCalibrationEquation(
			IntPtr hSensor,
			out sbyte CalibrationEquation);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetYminValue", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetYminValue(
			IntPtr hSensor,
			Single YminValue);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetYminValue", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetYminValue(
			IntPtr hSensor,
			out Single YminValue);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetYmaxValue", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetYmaxValue(
			IntPtr hSensor,
			Single YmaxValue);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetYmaxValue", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetYmaxValue(
			IntPtr hSensor,
			out Single YmaxValue);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetYscale", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetYscale(
			IntPtr hSensor,
			byte Yscale);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetYscale", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetYscale(
			IntPtr hSensor,
			out byte Yscale);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetHighestValidCalPageIndex", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetHighestValidCalPageIndex(
			IntPtr hSensor,
			byte HighestValidCalPageIndex);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetHighestValidCalPageIndex", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetHighestValidCalPageIndex(
			IntPtr hSensor,
			out byte HighestValidCalPageIndex);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetActiveCalPage", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetActiveCalPage(
			IntPtr hSensor,
			byte ActiveCalPage);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetActiveCalPage", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetActiveCalPage(
			IntPtr hSensor,
			out byte ActiveCalPage);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetCalPage", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
		public static extern Int32 Sensor_DDSMem_SetCalPage(
			IntPtr hSensor,
			byte CalPageIndex,
			float CalibrationCoefficientA,
			float CalibrationCoefficientB,
			float CalibrationCoefficientC,
			String units);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetCalPage", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
		public static extern Int32 Sensor_DDSMem_GetCalPage(
			IntPtr hSensor,
			byte CalPageIndex,
			out float CalibrationCoefficientA,
			out float CalibrationCoefficientB,
			out float CalibrationCoefficientC,
			StringBuilder units,
			UInt16 maxNumBytesToCopy);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_SetChecksum", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_SetChecksum(
			IntPtr hSensor,
			byte Checksum);

		[DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Sensor_DDSMem_GetChecksum", CallingConvention = CallingConvention.Cdecl)]
		public static extern Int32 Sensor_DDSMem_GetChecksum(
			IntPtr hSensor,
			byte Checksum);

        /// <summary>
        /// GoIO_Diags_SetDebugTraceThreshold().
        /// </summary>
        /// <param name="threshold">Only trace messages marked with a severity >= threshold(GoIO.TRACE_SEVERITY_) are actually sent to the debug output.</param>
        /// <returns></returns>
        [DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Diags_SetDebugTraceThreshold", CallingConvention = CallingConvention.Cdecl)]
        public static extern Int32 Diags_SetDebugTraceThreshold(
            Int32 threshold);

        [DllImport("GoIO_DLL.dll", EntryPoint = "GoIO_Diags_GetDebugTraceThreshold", CallingConvention = CallingConvention.Cdecl)]
        public static extern Int32 Diags_GetDebugTraceThreshold(
            out Int32 threshold);
	}
}
