using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace GoIOdotNET
{
	/// <summary>
	/// GoIO_ParmBlk defines the GoIO.Sensor_SendCmdAndGetResponse() protocol used with GoTemp, GoLink, and GoMotion devices.
	/// </summary>
	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	public class GoIO_ParmBlk
	{
		/// <summary>
		/// Used with GoIO.Sensor_SendCmdAndGetResponseN().
		/// <para>
		/// parameters: none
		/// </para>
		/// <para>
		/// response: GoIOGetStatusCmdResponsePayload
		/// </para>
		/// </summary>
		public const byte CMD_ID_GET_STATUS = 0x10;
		/// <summary>
		/// Used with GoIO.Sensor_SendCmdAndGetResponseN().
		/// <para>
		/// parameters: none
		/// </para>
		/// <para>
		/// response: GoIODefaultCmdResponse
		/// </para>
		/// </summary>
		public const byte CMD_ID_START_MEASUREMENTS = 0x18;
		/// <summary>
		/// Used with GoIO.Sensor_SendCmdAndGetResponseN().
		/// <para>
		/// parameters: none
		/// </para>
		/// <para>
		/// response: GoIODefaultCmdResponse
		/// </para>
		/// </summary>
		public const byte CMD_ID_STOP_MEASUREMENTS = 0x19;
		/// <summary>
		/// Used with GoIO.Sensor_SendCmdAndGetResponseN().
		/// Direct use of this command by application programs is not recommended. This command gets sent automatically by the GoIO
		/// library by GoIO_Sensor_Open().
		/// <para>
		/// parameters: none
		/// </para>
		/// <para>
		/// response: GoIODefaultCmdResponse
		/// </para>
		/// </summary>
		public const byte CMD_ID_INIT = 0x1A;
		/// <summary>
		/// Used with GoIO.Sensor_SendCmdAndGetResponseN().
		/// Direct use of this command by application programs is not recommended. Instead, call
		/// GoIO_Sensor_SetMeasurementPeriod().
		/// <para>
		/// parameters: GoIOSetMeasurementPeriodParams
		/// </para>
		/// <para>
		/// response: GoIODefaultCmdResponse
		/// </para>
		/// </summary>
		public const byte CMD_ID_SET_MEASUREMENT_PERIOD = 0x1B;
		/// <summary>
		/// Used with GoIO.Sensor_SendCmdAndGetResponseN().
		/// Direct use of this command by application programs is not recommended. Instead, call
		/// GoIO_Sensor_GetMeasurementPeriod().
		/// <para>
		/// parameters: none
		/// </para>
		/// <para>
		/// response: GoIOSetMeasurementPeriodParams
		/// </para>
		/// </summary>
		public const byte CMD_ID_GET_MEASUREMENT_PERIOD = 0x1C;
		/// <summary>
		/// Used with GoIO.Sensor_SendCmdAndGetResponseN().
		/// <para>
		/// parameters: GoIOSetLedStateParams
		/// </para>
		/// <para>
		/// response: GoIODefaultCmdResponse
		/// </para>
		/// </summary>
		public const byte CMD_ID_SET_LED_STATE = 0x1D;
		/// <summary>
		/// Used with GoIO.Sensor_SendCmdAndGetResponseN().
		/// <para>
		/// parameters: none
		/// </para>
		/// <para>
		/// response: GoIOSetLedStateParams
		/// </para>
		/// </summary>
		public const byte CMD_ID_GET_LED_STATE = 0x1E;
		/// <summary>
		/// Used with GoIO.Sensor_SendCmdAndGetResponseN().
		/// This command is supported by GoTemp and GoLink, but not GoMotion.
		/// <para>
		/// parameters: none
		/// </para>
		/// <para>
		/// response: GoIOSerialNumber
		/// </para>
		/// </summary>
		public const byte CMD_ID_GET_SERIAL_NUMBER = 0x20;
		/// <summary>
		/// Used with GoIO.Sensor_SendCmdAndGetResponseN().
		/// <para>
		/// parameters: GoIOSetAnalogInputParams
		/// </para>
		/// <para>
		/// response: GoIODefaultCmdResponse
		/// </para>
		/// </summary>
		public const byte CMD_ID_SET_ANALOG_INPUT_CHANNEL = 0x29;
		/// <summary>
		/// Used with GoIO.Sensor_SendCmdAndGetResponseN().
		/// <para>
		/// parameters: none
		/// </para>
		/// <para>
		/// response: GoIOSetAnalogInputParams
		/// </para>
		/// </summary>
		public const byte CMD_ID_GET_ANALOG_INPUT_CHANNEL = 0x2A;
		/// <summary>
		/// Used with GoIO.Sensor_SendCmdAndGetResponseN().
		/// Direct use of this command by application programs is not recommended. 
		/// Instead, call GoIO_Sensor_DDSMem_GetSensorNumber(sendQueryToHardwareflag=1).
		/// <para>
		/// parameters: none
		/// </para>
		/// <para>
		/// response: GoIOGetSensorIdCmdResponsePayload
		/// </para>
		/// </summary>
		public const byte CMD_ID_GET_SENSOR_ID = 0x28;
	}

	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	public struct GoIODefaultCmdResponse
	{
		public const byte STATUS_SUCCESS = 0;
		public const byte STATUS_NOT_READY_FOR_NEW_CMD = 0x30;
		public const byte STATUS_CMD_NOT_SUPPORTED = 0x31;
		public const byte STATUS_INTERNAL_ERROR1 = 0x32;
		public const byte STATUS_INTERNAL_ERROR2 = 0x33;
		public const byte STATUS_ERROR_CANNOT_CHANGE_PERIOD_WHILE_COLLECTING = 0x34;
		public const byte STATUS_ERROR_CANNOT_READ_NV_MEM_BLK_WHILE_COLLECTING_FAST = 0x35;
		public const byte STATUS_ERROR_INVALID_PARAMETER = 0x36;
		public const byte STATUS_ERROR_CANNOT_WRITE_FLASH_WHILE_COLLECTING = 0x37;
		public const byte STATUS_ERROR_CANNOT_WRITE_FLASH_WHILE_HOST_FIFO_BUSY = 0x38;
		public const byte STATUS_ERROR_OP_BLOCKED_WHILE_COLLECTING = 0x39;
		public const byte STATUS_ERROR_CALCULATOR_CANNOT_MEASURE_WITH_NO_BATTERIES = 0x3A;
		/// <summary>
		/// GoIO.GetErrorSentOverTheWire() returns GoIODefaultCmdResponse.NO_ERROR_SENT_OVER_THE_WIRE if the
		/// most recent call to GoIO.SendCmdAndGetResponse() did not result in an error code being sent back from
		/// the device.
		/// </summary>
		public const byte NO_ERROR_SENT_OVER_THE_WIRE = 0xFE;

		/// <summary>
		/// GoIODefaultCmdResponse.STATUS_...
		/// </summary>
		public byte status;
	}

	/// <summary>
	/// Parameter block passed into SendCmdAndGetResponse() with CMD_ID_SET_MEASUREMENT_PERIOD.
	/// This is also the response payload returned by SendCmdAndGetResponse() for CMD_ID_GET_MEASUREMENT_PERIOD.
	/// Direct use of the GoIOSetMeasurementPeriodParams struct is not recommended. 
	/// Use GoIO.Sensor_SetMeasurementPeriod() instead.
	/// </summary>
	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	public struct GoIOSetMeasurementPeriodParams
	{
		/// <summary>
		/// Measurement period in device 'ticks', which are milliseconds for the GoLink.
		/// </summary>
		public byte lsbyteLswordMeasurementPeriod;
		public byte msbyteLswordMeasurementPeriod;
		public byte lsbyteMswordMeasurementPeriod;
		public byte msbyteMswordMeasurementPeriod;
	}

	/// <summary>
	/// Parameter block passed into SendCmdAndGetResponse() with CMD_ID_SET_LED_STATE.
	/// This is also the response payload returned by SendCmdAndGetResponse() for CMD_ID_GET_LED_STATE.
	/// </summary>
	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	public struct GoIOSetLedStateParams
	{
		public const byte LED_COLOR_BLACK = 0xC0;
		public const byte LED_COLOR_RED = 0x40;
		public const byte LED_COLOR_GREEN = 0x80;
		public const byte LED_COLOR_RED_GREEN = 0;
		public const byte LED_BRIGHTNESS_MIN = 0;
		public const byte LED_BRIGHTNESS_MAX = 0x10;
		public const byte LED_COLOR_ORANGE = 0;
		public const byte LED_BRIGHTNESS_ORANGE = 4;

		public byte color;
		public byte brightness;
	}

	/// <summary>
	/// This is the response payload returned by SendCmdAndGetResponse() for CMD_ID_GET_STATUS.
	/// </summary>
	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	public struct GoIOGetStatusCmdResponsePayload
	{
		public const byte MASK_STATUS_ERROR_CMD_NOT_RECOGNIZED = 1;
		public const byte MASK_STATUS_ERROR_CMD_IGNORED = 2;
		public const byte MASK_STATUS_ERROR_MASTER_FIFO_OVERFLOW = 0X40;

		public const byte GO_LINK_MASK_STATUS_ERROR_PACKET_OVERRUN = 4;
		public const byte GO_LINK_MASK_STATUS_ERROR_MEAS_PACKET_LOST = 8;
		public const byte GO_LINK_MASK_STATUS_ERROR_MASTER_INVALID_SPI_PACKET = 0X10;
		public const byte GO_LINK_MASK_STATUS_ERROR_MASTER_INVALID_MEAS_COUNTER = 0X20;
		public const byte GO_LINK_MASK_STATUS_ERROR_MEAS_CONVERSION_LOST = 0X80;

		public const byte GO_MOTION_MASK_STATUS_BATTERY_STATE = 0X0C;
		/// <summary>
		/// ((status AND GO_MOTION_MASK_STATUS_BATTERY_STATE) == GO_MOTION_MASK_STATUS_BATTERY_STATE_GOOD) => good batteries.
		/// </summary>
		public const byte GO_MOTION_MASK_STATUS_BATTERY_STATE_GOOD = 0;
		/// <summary>
		/// ((status AND GO_MOTION_MASK_STATUS_BATTERY_STATE) == GO_MOTION_MASK_STATUS_BATTERY_STATE_LOW_WHILE_SAMPLING) 
		/// => marginal batteries.
		/// </summary>
		public const byte GO_MOTION_MASK_STATUS_BATTERY_STATE_LOW_WHILE_SAMPLING = 4;
		/// <summary>
		/// ((status AND GO_MOTION_MASK_STATUS_BATTERY_STATE) == GO_MOTION_MASK_STATUS_BATTERY_STATE_LOW_ALWAYS) 
		/// => bad batteries.
		/// </summary>
		public const byte GO_MOTION_MASK_STATUS_BATTERY_STATE_LOW_ALWAYS = 8;
		/// <summary>
		/// ((status AND GO_MOTION_MASK_STATUS_BATTERY_STATE) == GO_MOTION_MASK_STATUS_BATTERY_STATE_MISSING) 
		/// => no batteries present.
		/// </summary>
		public const byte GO_MOTION_MASK_STATUS_BATTERY_STATE_MISSING = 0X0C;

		/// <summary>
		/// See MASK_STATUS_ .
		/// </summary>
		public byte status;
		/// <summary>
		/// Binary coded decimal.
		/// </summary>
		public byte minorVersionMasterCPU;
		/// <summary>
		/// Binary coded decimal.
		/// </summary>
		public byte majorVersionMasterCPU;
		/// <summary>
		/// Binary coded decimal - updated by GoLink and GoMotion, but not by GoTemp.
		/// </summary>
		public byte minorVersionSlaveCPU;
		/// <summary>
		/// Binary coded decimal - updated by GoLink and GoMotion, but not by GoTemp.
		/// </summary>
		public byte majorVersionSlaveCPU;
	}

	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	public struct GoIOSerialNumber
	{
		/// <summary>
		/// week in year(starting at 1) in BCD format
		/// </summary>
		public byte ww;
		/// <summary>
		/// last two digits of year in BCD format
		/// </summary>
		public byte yy;
		public byte lsbyteLswordSerialCounter;
		public byte msbyteLswordSerialCounter;
		public byte lsbyteMswordSerialCounter;
		public byte msbyteMswordSerialCounter;
	}

	/// <summary>
	/// This is the response payload returned by SendCmdAndGetResponse() for CMD_ID_GET_SENSOR_ID.
	/// </summary>
	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	public struct GoIOGetSensorIdCmdResponsePayload
	{
		public byte lsbyteLswordSensorId;
		public byte msbyteLswordSensorId;
		public byte lsbyteMswordSensorId;
		public byte msbyteMswordSensorId;
	}

	/// <summary>
	/// Parameter block passed into SendCmdAndGetResponse() with CMD_ID_SET_ANALOG_INPUT_CHANNEL.
	/// This is also the response returned by SendCmdAndGetResponse() for CMD_ID_GET_ANALOG_INPUT_CHANNEL.
	/// </summary>
	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	public struct GoIOSetAnalogInputParams
	{
		const byte ANALOG_INPUT_CHANNEL_VOFF = 0;
		/// <summary>
		/// Used for +/- 10 volt probes.
		/// </summary>
		public const byte ANALOG_INPUT_CHANNEL_VIN = 1;
		/// <summary>
		/// Used for 5 volt probes.
		/// </summary>
		public const byte ANALOG_INPUT_CHANNEL_VIN_LOW = 2;
		const byte ANALOG_INPUT_CHANNEL_VID = 3;
		/// <summary>
		/// ANALOG_INPUT_... 
		/// </summary>
		public byte analogInputChannel;
	}
}
