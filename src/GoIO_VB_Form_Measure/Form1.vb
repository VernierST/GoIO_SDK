'/*********************************************************************************
'
'Copyright (c) 2010, Vernier Software & Technology
'All rights reserved.
'
'Redistribution and use in source and binary forms, with or without
'modification, are permitted provided that the following conditions are met:
'    * Redistributions of source code must retain the above copyright
'      notice, this list of conditions and the following disclaimer.
'    * Redistributions in binary form must reproduce the above copyright
'      notice, this list of conditions and the following disclaimer in the
'      documentation and/or other materials provided with the distribution.
'    * Neither the name of Vernier Software & Technology nor the
'      names of its contributors may be used to endorse or promote products
'      derived from this software without specific prior written permission.
'
'THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
'ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
'WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
'DISCLAIMED. IN NO EVENT SHALL VERNIER SOFTWARE & TECHNOLOGY BE LIABLE FOR ANY
'DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
'(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
'LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
'ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
'(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
'SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
'
'**********************************************************************************/

Imports GoIOdotNET
Imports VSTCoreDefsdotNET
Imports System.Runtime.InteropServices
Imports System.Text


Public Class Form1
    Dim GoIO_sensor_handle As IntPtr
    Dim bIsCollectingMeasurements As Boolean
    Dim measCirBuf As MeasurementTuplesCirBuf
    Const NUM_MEASUREMENTS_IN_LOCAL_BIG_BUF As UInteger = 10000
    Dim raw_measurements(NUM_MEASUREMENTS_IN_LOCAL_BIG_BUF) As Integer
    Dim raw_timestamps(NUM_MEASUREMENTS_IN_LOCAL_BIG_BUF) As Long
    Dim graph_history_y_min As Double
    Dim graph_history_y_max As Double
    Dim measPeriodInSeconds As Double
    Dim lib_major_version As UShort
    Dim lib_minor_version As UShort

    Private Sub Form1_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load
        Dim trace_threshold As Integer = 0

        GoIO.Init()
        GoIO.GetDLLVersion(lib_major_version, lib_minor_version)
        GoIO_sensor_handle = IntPtr.Zero
        bIsCollectingMeasurements = False
        measCirBuf = New MeasurementTuplesCirBuf(501)
        measPeriodInSeconds = 0.04

        ClearGraph()
        UpdateControls()

        GoIO.Diags_SetDebugTraceThreshold(GoIO.TRACE_SEVERITY_LOWEST)
        NGIOLibVerboseToolStripMenuItem.Checked = True

        Timer1.Start()
    End Sub

    Private Sub Form1_Resize(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Resize
        'Debug.WriteLine("resize message from Elliot")
        Graph_Canvas.Width = Me.Size.Width - 33
        Graph_Canvas.Height = Me.Size.Height - 102
        Graph_Canvas.Invalidate()

    End Sub

    Private Sub Graph_Canvas_Paint(ByVal sender As System.Object, ByVal e As System.Windows.Forms.PaintEventArgs) Handles Graph_Canvas.Paint
        Dim g As Graphics = e.Graphics
        Dim clientRect As New Rectangle(0, 0, Graph_Canvas.Width, Graph_Canvas.Height)
        Dim graphRect As New Rectangle(0, 0, Graph_Canvas.Width, Graph_Canvas.Height)
        Dim i As Integer
        Dim meas_x_range As Double
        Dim meas_x_min As Double
        Dim meas_x_max As Double
        Dim meas_y_range As Double
        Dim meas_y_min As Double
        Dim meas_y_max As Double
        Dim testy As Double
        Dim testx As Double
        Dim x As Integer
        Dim y As Integer
        Dim deltaX As Integer
        Dim deltaY As Integer
        Dim old_x As Integer
        Dim old_y As Integer
        Dim numMeasurements As Integer
        Dim meas_x_frac As Double
        Dim meas_y_frac As Double

        If (IntPtr.Zero <> GoIO_sensor_handle) And (measCirBuf.NumElementsAvailable() > 0) Then
            'Calculate what portion of the client area will hold the graph.
            If (graphRect.Width > 200) And (graphRect.Height > 200) Then
                graphRect = New Rectangle(graphRect.Left + 100, graphRect.Top + 25, graphRect.Width - 125, graphRect.Height - 75)
            End If

            'Calculate full graph time range.
            numMeasurements = measCirBuf.NumElementsAvailable()
            meas_x_min = 0.0
            meas_x_max = 1.0
            If numMeasurements > 0 Then
                measCirBuf.CopyElement(0, testy, meas_x_min)
                measCirBuf.CopyElement(numMeasurements - 1, testy, meas_x_max)
            End If
            meas_x_range = meas_x_max - meas_x_min
            If meas_x_range < 0.001 Then
                meas_x_range = 0.001
                meas_x_max = meas_x_min + meas_x_range
            End If

            'Calculate full graph y range.
            measCirBuf.CopyElement(0, meas_y_min, testx)
            meas_y_max = meas_y_min
            For i = 1 To (numMeasurements - 1)
                measCirBuf.CopyElement(i, testy, testx)
                If testy < meas_y_min Then
                    meas_y_min = testy
                End If
                If testy > meas_y_max Then
                    meas_y_max = testy
                End If
            Next i
            meas_y_range = meas_y_max - meas_y_min

            'Make sure that meas_y_range corresponds to a delta of at least 0.1
            If meas_y_range < 0.1 Then
                meas_y_range = 0.1
                meas_y_max = meas_y_min + meas_y_range
            End If

            If graph_history_y_max > graph_history_y_min Then
                If graph_history_y_max > meas_y_max Then
                    meas_y_max = graph_history_y_max
                End If
                If graph_history_y_min < meas_y_min Then
                    meas_y_min = graph_history_y_min
                End If
                meas_y_range = meas_y_max - meas_y_min
            End If
            graph_history_y_max = meas_y_max
            graph_history_y_min = meas_y_min

            'Do some drawing now.
            g.DrawLine(System.Drawing.Pens.Black, graphRect.Left, clientRect.Top, graphRect.Left, clientRect.Bottom)
            g.DrawLine(System.Drawing.Pens.Black, graphRect.Right, clientRect.Top, graphRect.Right, clientRect.Bottom)

            g.DrawLine(System.Drawing.Pens.Blue, clientRect.Left, graphRect.Bottom, clientRect.Right, graphRect.Bottom)
            g.DrawLine(System.Drawing.Pens.Blue, clientRect.Left, graphRect.Top, clientRect.Right, graphRect.Top)

            Dim str As String = meas_x_min.ToString("0.000") & " secs"
            Dim font As New Font("Courier New", 10)
            x = graphRect.Left + 10
            y = graphRect.Bottom + 10
            g.DrawString(str, font, System.Drawing.Brushes.Black, x, y)

            str = meas_x_max.ToString("0.000") & " secs"
            Dim textSize As SizeF = g.MeasureString(str, font)
            x = graphRect.Right - textSize.Width() - 10
            y = graphRect.Bottom + 10
            g.DrawString(str, font, System.Drawing.Brushes.Black, x, y)

            Dim units As String = GetUnits()
            If 0 = String.Compare(units, "") Then
                units = "volts"
            End If

            Dim fonty As New Font("Times New Roman", 11)
            str = meas_y_min.ToString("G5") & " " & units
            textSize = g.MeasureString(str, fonty)
            x = clientRect.Left + 10
            y = graphRect.Bottom - textSize.Height() - 10
            g.DrawString(str, fonty, System.Drawing.Brushes.Blue, x, y)

            str = meas_y_max.ToString("G5") & " " & units
            x = clientRect.Left + 10
            y = graphRect.Top + 10
            g.DrawString(str, fonty, System.Drawing.Brushes.Blue, x, y)

            Dim redPen As New Pen(Color.Red, 3.0)

            For i = 0 To (numMeasurements - 1)
                measCirBuf.CopyElement(i, meas_y_frac, meas_x_frac)
                meas_y_frac = (meas_y_frac - meas_y_min) / meas_y_range
                deltaY = Math.Round(meas_y_frac * graphRect.Height())
                meas_x_frac = (meas_x_frac - meas_x_min) / meas_x_range
                deltaX = Math.Round(meas_x_frac * graphRect.Width())
                x = graphRect.Left + deltaX
                y = graphRect.Bottom - deltaY

                If 0 = i Then
                    g.DrawLine(redPen, x, y, x, y)
                ElseIf ((x <> old_x) Or (y <> old_y)) Then
                    g.DrawLine(redPen, old_x, old_y, x, y)
                End If
                old_x = x
                old_y = y
            Next i
        Else
            'g.DrawLine(System.Drawing.Pens.Black, 0, 0, Graph_Canvas.Width, Graph_Canvas.Height)
            'g.DrawLine(System.Drawing.Pens.Black, 0, Graph_Canvas.Height, Graph_Canvas.Width, 0)
        End If
    End Sub

    Private Sub Form1_FormClosed(ByVal sender As System.Object, ByVal e As System.Windows.Forms.FormClosedEventArgs) Handles MyBase.FormClosed
        Timer1.Stop()

        If IntPtr.Zero <> GoIO_sensor_handle Then
            CloseSensor()
        End If
        GoIO.Uninit()
    End Sub

    Private Sub DevicesToolStripMenuItemSensors_DropDownOpening(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles DevicesToolStripMenuItem.DropDownOpening
        Dim numSensors As UInteger
        Dim numSensorsAdded As UInteger
        Dim deviceName As StringBuilder = New StringBuilder(CInt(GoIO.MAX_SIZE_SENSOR_NAME))
        Dim openSensorName As StringBuilder = New StringBuilder(CInt(GoIO.MAX_SIZE_SENSOR_NAME))
        Dim deviceName2 As String
        Dim openSensorName2 As String = ""
        Dim status As Integer
        Dim mnu As ToolStripMenuItem
        Dim N As UInteger = 0
        Dim vendorId As Integer
        Dim productId As Integer

        If IntPtr.Zero <> GoIO_sensor_handle Then
            GoIO.Sensor_GetOpenDeviceName(GoIO_sensor_handle, openSensorName, GoIO.MAX_SIZE_SENSOR_NAME, vendorId, productId)
            openSensorName2 = openSensorName.ToString()
        End If

        Me.DevicesToolStripMenuItem.DropDownItems.Clear()
        numSensorsAdded = 0

        numSensors = GoIO.UpdateListOfAvailableDevices(VST_USB_defs.VENDOR_ID, VST_USB_defs.PRODUCT_ID_GO_LINK)
        N = 0
        Do While N < numSensors
            status = GoIO.GetNthAvailableDeviceName(deviceName, GoIO.MAX_SIZE_SENSOR_NAME, VST_USB_defs.VENDOR_ID, VST_USB_defs.PRODUCT_ID_GO_LINK, N)
            If 0 = status Then
                deviceName2 = deviceName.ToString()
                mnu = New ToolStripMenuItem()
                mnu.Text = deviceName2
                If 0 = String.Compare(openSensorName2, deviceName2) Then
                    mnu.Checked() = True
                End If
                AddSensorNHandler(mnu, numSensorsAdded)
                numSensorsAdded = numSensorsAdded + 1
                Me.DevicesToolStripMenuItem.DropDownItems.Add(mnu)
            Else
                Exit Do
            End If
            N = N + 1
        Loop

        numSensors = GoIO.UpdateListOfAvailableDevices(VST_USB_defs.VENDOR_ID, VST_USB_defs.PRODUCT_ID_GO_TEMP)
        N = 0
        Do While N < numSensors
            status = GoIO.GetNthAvailableDeviceName(deviceName, GoIO.MAX_SIZE_SENSOR_NAME, VST_USB_defs.VENDOR_ID, VST_USB_defs.PRODUCT_ID_GO_TEMP, N)
            If 0 = status Then
                deviceName2 = deviceName.ToString()
                mnu = New ToolStripMenuItem()
                mnu.Text = deviceName2
                If 0 = String.Compare(openSensorName2, deviceName2) Then
                    mnu.Checked() = True
                End If
                AddSensorNHandler(mnu, numSensorsAdded)
                numSensorsAdded = numSensorsAdded + 1
                Me.DevicesToolStripMenuItem.DropDownItems.Add(mnu)
            Else
                Exit Do
            End If
            N = N + 1
        Loop

        numSensors = GoIO.UpdateListOfAvailableDevices(VST_USB_defs.VENDOR_ID, VST_USB_defs.PRODUCT_ID_GO_MOTION)
        N = 0
        Do While N < numSensors
            status = GoIO.GetNthAvailableDeviceName(deviceName, GoIO.MAX_SIZE_SENSOR_NAME, VST_USB_defs.VENDOR_ID, VST_USB_defs.PRODUCT_ID_GO_MOTION, N)
            If 0 = status Then
                deviceName2 = deviceName.ToString()
                mnu = New ToolStripMenuItem()
                mnu.Text = deviceName2
                If 0 = String.Compare(openSensorName2, deviceName2) Then
                    mnu.Checked() = True
                End If
                AddSensorNHandler(mnu, numSensorsAdded)
                numSensorsAdded = numSensorsAdded + 1
                Me.DevicesToolStripMenuItem.DropDownItems.Add(mnu)
            Else
                Exit Do
            End If
            N = N + 1
        Loop

        numSensors = GoIO.UpdateListOfAvailableDevices(VST_USB_defs.VENDOR_ID, VST_USB_defs.PRODUCT_ID_MINI_GC)
        N = 0
        Do While N < numSensors
            status = GoIO.GetNthAvailableDeviceName(deviceName, GoIO.MAX_SIZE_SENSOR_NAME, VST_USB_defs.VENDOR_ID, VST_USB_defs.PRODUCT_ID_MINI_GC, N)
            If 0 = status Then
                deviceName2 = deviceName.ToString()
                mnu = New ToolStripMenuItem()
                mnu.Text = deviceName2
                If 0 = String.Compare(openSensorName2, deviceName2) Then
                    mnu.Checked() = True
                End If
                AddSensorNHandler(mnu, numSensorsAdded)
                numSensorsAdded = numSensorsAdded + 1
                Me.DevicesToolStripMenuItem.DropDownItems.Add(mnu)
            Else
                Exit Do
            End If
            N = N + 1
        Loop

    End Sub

    Private Sub Sensor0MenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs)
        OnSensorNMenuItem_Click(0)
    End Sub

    Private Sub Sensor1MenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs)
        OnSensorNMenuItem_Click(1)
    End Sub

    Private Sub Sensor2MenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs)
        OnSensorNMenuItem_Click(2)
    End Sub

    Private Sub Sensor3MenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs)
        OnSensorNMenuItem_Click(3)
    End Sub

    Private Sub Sensor4MenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs)
        OnSensorNMenuItem_Click(4)
    End Sub

    Private Sub AddSensorNHandler(ByVal deviceMenuItem As ToolStripMenuItem, ByVal N As UInteger)
        Select Case N
            Case 0
                AddHandler deviceMenuItem.Click, AddressOf Sensor0MenuItem_Click
            Case 1
                AddHandler deviceMenuItem.Click, AddressOf Sensor1MenuItem_Click
            Case 2
                AddHandler deviceMenuItem.Click, AddressOf Sensor2MenuItem_Click
            Case 3
                AddHandler deviceMenuItem.Click, AddressOf Sensor3MenuItem_Click
            Case 4
                AddHandler deviceMenuItem.Click, AddressOf Sensor4MenuItem_Click
            Case Else
                Debug.WriteLine("too many devices!")
        End Select
    End Sub

    Private Sub OnSensorNMenuItem_Click(ByVal N As Integer)
        Dim mnu As ToolStripMenuItem

        mnu = Me.DevicesToolStripMenuItem.DropDownItems.Item(N)
        If CheckState.Checked = mnu.CheckState Then
            CloseSensor()
            MeasBox.Text = ""
        Else
            Refresh()
            OpenSensor(mnu.Text)
        End If
        UpdateControls()
        UpdateTitle()
        ClearGraph()
        UpdateUnits()
    End Sub

    Private Sub OpenSensor(ByVal deviceName As String)
        Dim productId As Integer = VST_USB_defs.PRODUCT_ID_GO_LINK 'matches pid_0003

        If IntPtr.Zero <> GoIO_sensor_handle Then
            CloseSensor()
        End If

        If deviceName.Contains("pid_0002") Then
            productId = VST_USB_defs.PRODUCT_ID_GO_TEMP
        ElseIf deviceName.Contains("pid_0004") Then
            productId = VST_USB_defs.PRODUCT_ID_GO_MOTION
        ElseIf deviceName.Contains("pid_0007") Then
            productId = VST_USB_defs.PRODUCT_ID_MINI_GC
        End If

        GoIO_sensor_handle = GoIO.Sensor_Open(deviceName, VST_USB_defs.VENDOR_ID, productId, 0)
        If IntPtr.Zero <> GoIO_sensor_handle Then
            Dim ledParams As GoIOSetLedStateParams = New GoIOSetLedStateParams
            ledParams.color = GoIOSetLedStateParams.LED_COLOR_GREEN
            ledParams.brightness = GoIOSetLedStateParams.LED_BRIGHTNESS_MAX
            GoIO.Sensor_SendCmdAndGetResponse2(GoIO_sensor_handle, GoIO_ParmBlk.CMD_ID_SET_LED_STATE, ledParams, GoIO.TIMEOUT_MS_DEFAULT)
        End If
        If IntPtr.Zero <> GoIO_sensor_handle Then
            'Set desired sampling period.
            GoIO.Sensor_SetMeasurementPeriod(GoIO_sensor_handle, measPeriodInSeconds, GoIO.TIMEOUT_MS_DEFAULT)
            'Find out what actual sampling period is.
            measPeriodInSeconds = GoIO.Sensor_GetMeasurementPeriod(GoIO_sensor_handle, GoIO.TIMEOUT_MS_DEFAULT)
        End If

    End Sub

    Private Sub CloseSensor()
        If IntPtr.Zero <> GoIO_sensor_handle Then
            GoIO.Sensor_Close(GoIO_sensor_handle)
        End If
        GoIO_sensor_handle = IntPtr.Zero
        bIsCollectingMeasurements = False
    End Sub

    Private Sub ActionToolStripMenuItem_DropDownOpening(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ActionToolStripMenuItem.DropDownOpening
        Dim openSensorName As StringBuilder = New StringBuilder(CInt(GoIO.MAX_SIZE_SENSOR_NAME))
        Dim vendorId As Integer
        Dim productId As Integer

        If (IntPtr.Zero <> GoIO_sensor_handle) Then
            GoIO.Sensor_GetOpenDeviceName(GoIO_sensor_handle, openSensorName, GoIO.MAX_SIZE_SENSOR_NAME, vendorId, productId)
        End If

        ActionToolStripMenuItem.DropDownItems.Item(0).Enabled = (IntPtr.Zero <> GoIO_sensor_handle)  'Get Status
        ActionToolStripMenuItem.DropDownItems.Item(1).Enabled = (IntPtr.Zero <> GoIO_sensor_handle) And (VST_USB_defs.PRODUCT_ID_GO_LINK = productId) 'Get Sensor Id
        ActionToolStripMenuItem.DropDownItems.Item(2).Enabled = (IntPtr.Zero <> GoIO_sensor_handle) And (Not bIsCollectingMeasurements)  'Set measurement period
        ActionToolStripMenuItem.DropDownItems.Item(3).Enabled = True    'Set display depth
        ActionToolStripMenuItem.DropDownItems.Item(4).Enabled = (IntPtr.Zero <> GoIO_sensor_handle) And (Not bIsCollectingMeasurements)  'Start measurements
        ActionToolStripMenuItem.DropDownItems.Item(5).Enabled = bIsCollectingMeasurements   'Stop measurements
    End Sub

    Private Sub GetStatusToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles GetStatusToolStripMenuItem.Click
        Dim response As GoIOGetStatusCmdResponsePayload = New GoIOGetStatusCmdResponsePayload
        Dim nRespBytes As UInteger
        Dim status As Integer
        Dim openSensorName As StringBuilder = New StringBuilder(CInt(GoIO.MAX_SIZE_SENSOR_NAME))
        Dim vendorId As Integer
        Dim productId As Integer

        If (IntPtr.Zero <> GoIO_sensor_handle) Then
            status = GoIO.Sensor_SendCmdAndGetResponse3(GoIO_sensor_handle, GoIO_ParmBlk.CMD_ID_GET_STATUS, response, nRespBytes, GoIO.TIMEOUT_MS_DEFAULT)

            If (0 = status) Then
                Dim digit0 As Integer = response.majorVersionMasterCPU
                Dim digit1 As Integer = response.minorVersionMasterCPU >> 4
                Dim digit2 As Integer = response.minorVersionMasterCPU And 15
                Dim digit3 As Integer = response.majorVersionSlaveCPU >> 4
                Dim digit4 As Integer = response.majorVersionSlaveCPU And 15
                Dim digit5 As Integer = response.minorVersionSlaveCPU >> 4
                Dim digit6 As Integer = response.minorVersionSlaveCPU And 15
                Dim rVersion As Double = digit0 + digit1 * 0.1 + digit2 * 0.01 + digit3 * 0.001 + digit4 * 0.0001 _
                 + digit5 * 0.00001 + digit6 * 0.000001
                Dim sVersion As String = rVersion.ToString("F6")
                Dim deviceStatus As UInteger = response.status

                Dim msg As String = "Status = " & deviceStatus.ToString("X") & " ; version = " & sVersion
                Dim title As String
                GoIO.Sensor_GetOpenDeviceName(GoIO_sensor_handle, openSensorName, GoIO.MAX_SIZE_SENSOR_NAME, vendorId, productId)
                Select Case productId
                    Case VST_USB_defs.PRODUCT_ID_GO_LINK
                        title = "Go Link Status"
                    Case VST_USB_defs.PRODUCT_ID_GO_MOTION
                        title = "Go Motion Status"
                    Case VST_USB_defs.PRODUCT_ID_GO_TEMP
                        title = "Go Temp Status"
                    Case VST_USB_defs.PRODUCT_ID_MINI_GC
                        title = "Mini GC Status"
                    Case Else
                        title = "Device Status"
                End Select

                MessageBox.Show(msg, title)
            End If
        End If
    End Sub

    Private Sub StartMeasurements()
        If (IntPtr.Zero <> GoIO_sensor_handle) And (Not bIsCollectingMeasurements) Then
            GoIO.Sensor_SendCmdAndGetResponse4(GoIO_sensor_handle, GoIO_ParmBlk.CMD_ID_START_MEASUREMENTS, GoIO.TIMEOUT_MS_DEFAULT)
            bIsCollectingMeasurements = True
        End If
    End Sub

    Private Sub StopMeasurements()
        If (IntPtr.Zero <> GoIO_sensor_handle) And bIsCollectingMeasurements Then
            GoIO.Sensor_SendCmdAndGetResponse4(GoIO_sensor_handle, GoIO_ParmBlk.CMD_ID_STOP_MEASUREMENTS, GoIO.TIMEOUT_MS_DEFAULT)
            'GoIO.Sensor_SendCmdAndGetResponse(GoIO_sensor_handle, GoIO_ParmBlk.CMD_ID_STOP_MEASUREMENTS, IntPtr.Zero, 0, IntPtr.Zero, IntPtr.Zero, GoIO.TIMEOUT_MS_DEFAULT)
            bIsCollectingMeasurements = False
        End If
    End Sub

    Private Sub UpdateControls()
        If IntPtr.Zero = GoIO_sensor_handle Then
            Start_Button.Enabled = False
            Stop_Button.Enabled = False
        ElseIf bIsCollectingMeasurements Then
            Start_Button.Enabled = False
            Stop_Button.Enabled = True
        Else
            Start_Button.Enabled = True
            Stop_Button.Enabled = False
        End If
        Me.Refresh()
    End Sub

    Private Sub Start_Button_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Start_Button.Click
        ClearGraph()
        StartMeasurements()
        UpdateControls()
    End Sub

    Private Sub StartToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles StartToolStripMenuItem.Click
        ClearGraph()
        StartMeasurements()
        UpdateControls()
    End Sub

    Private Sub Stop_Button_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Stop_Button.Click
        StopMeasurements()
        UpdateControls()
    End Sub

    Private Sub StopMeasurementsToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles StopMeasurementsToolStripMenuItem.Click
        StopMeasurements()
        UpdateControls()
    End Sub

    Private Sub ClearGraph()
        If IntPtr.Zero <> GoIO_sensor_handle Then
            GoIO.Sensor_ClearIO(GoIO_sensor_handle)
        End If
        measCirBuf.Clear()
        graph_history_y_min = 1.0   'This clears the history.
        graph_history_y_max = -1.0
        Graph_Canvas.Invalidate()
    End Sub

    Private Sub Timer1_Tick(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Timer1.Tick
        If IntPtr.Zero <> GoIO_sensor_handle Then
            Dim numMeasurementsAvailable As Integer
            Dim volts As Double
            Dim rMeasurement As Double
            Dim rTime As Double

            numMeasurementsAvailable = GoIO.Sensor_ReadRawMeasurements(GoIO_sensor_handle, _
             raw_measurements, NUM_MEASUREMENTS_IN_LOCAL_BIG_BUF)
            If numMeasurementsAvailable > 0 Then
                If measCirBuf.NumElementsAvailable() > 0 Then
                    measCirBuf.CopyElement(measCirBuf.NumElementsAvailable() - 1, rMeasurement, rTime)
                    rTime += measPeriodInSeconds
                Else
                    rTime = 0.0
                End If
                For i As Integer = 0 To (numMeasurementsAvailable - 1)
                    volts = GoIO.Sensor_ConvertToVoltage(GoIO_sensor_handle, raw_measurements(i))
                    rMeasurement = GoIO.Sensor_CalibrateData(GoIO_sensor_handle, volts)

                    measCirBuf.AddElement(rMeasurement, rTime)
                    rTime += measPeriodInSeconds
                Next i
                Graph_Canvas.Invalidate()

                MeasBox.Text = rMeasurement.ToString("G5")
            Else
                If Not bIsCollectingMeasurements Then
                    MeasBox.Text = ""
                End If
            End If
        End If
    End Sub

    Private Sub UpdateTitle()
        Dim title As String = "GoIO_Measure"
        If (IntPtr.Zero <> GoIO_sensor_handle) Then
            Dim sensorId As Byte = 0
            Dim longName As StringBuilder = New StringBuilder(30)
            Dim longName2 As String

            title = title & " - Sensor id = "
            GoIO.Sensor_DDSMem_GetSensorNumber(GoIO_sensor_handle, sensorId, 0, 0)
            title = title & sensorId.ToString()

            GoIO.Sensor_DDSMem_GetLongName(GoIO_sensor_handle, longName, 30)
            longName2 = longName.ToString()
            If longName2.Length() > 0 Then
                title = title & " ( " & longName2 & " ) "
            End If
        End If
        Me.Text = title
    End Sub

    Private Function GetUnits() As String
        Dim units As StringBuilder = New StringBuilder(20)
        Dim units2 As String
        If (IntPtr.Zero <> GoIO_sensor_handle) Then
            Dim CalPageIndex As Byte = 0
            Dim A, B, C As Single
            GoIO.Sensor_DDSMem_GetActiveCalPage(GoIO_sensor_handle, CalPageIndex)
            GoIO.Sensor_DDSMem_GetCalPage(GoIO_sensor_handle, CalPageIndex, A, B, C, units, 20)
            units2 = units.ToString()
        Else
            units2 = ""
        End If
        Return units2
    End Function


    Private Sub UpdateUnits()
        Dim units As String = GetUnits()
        UnitsLabel.Text = units
    End Sub

    Private Sub SetMeasurementPeriodToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles SetMeasurementPeriodToolStripMenuItem.Click
        Dim DialogBox As New SetMeasurementPeriodDlg()
        Dim periodms As Int32 = CInt((measPeriodInSeconds + 0.0001) / 0.001)

        DialogBox.period_uS_textbox.Text = periodms.ToString()
        If DialogBox.ShowDialog() = Windows.Forms.DialogResult.OK Then
            periodms = CInt(DialogBox.period_uS_textbox.Text)
            If periodms >= 5 Then
                measPeriodInSeconds = periodms * 0.001
                If (IntPtr.Zero <> GoIO_sensor_handle) Then
                    'Set desired sampling period.
                    GoIO.Sensor_SetMeasurementPeriod(GoIO_sensor_handle, measPeriodInSeconds, GoIO.TIMEOUT_MS_DEFAULT)
                    'Find out what actual sampling period is.
                    measPeriodInSeconds = GoIO.Sensor_GetMeasurementPeriod(GoIO_sensor_handle, GoIO.TIMEOUT_MS_DEFAULT)
                End If
            End If
        End If
    End Sub

    Private Sub SetDisplayDepthToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles SetDisplayDepthToolStripMenuItem.Click
        Dim DialogBox As New SetDisplayDepthDlg()
        Dim depth As Int32 = measCirBuf.MaxNumElementsAvailable()

        DialogBox.displayDepthTextBox.Text = depth.ToString()
        If DialogBox.ShowDialog() = Windows.Forms.DialogResult.OK Then
            depth = CInt(DialogBox.displayDepthTextBox.Text)
            If depth <> measCirBuf.MaxNumElementsAvailable() Then
                If (depth >= 10) And (depth <= 100000) Then
                    measCirBuf = New MeasurementTuplesCirBuf(depth)
                    ClearGraph()
                End If
            End If
        End If
    End Sub

    Private Sub NGIOLibVerboseToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles NGIOLibVerboseToolStripMenuItem.Click
        Dim trace_threshold As Integer = 0
        GoIO.Diags_GetDebugTraceThreshold(trace_threshold)
        If trace_threshold <= GoIO.TRACE_SEVERITY_LOWEST Then
            GoIO.Diags_SetDebugTraceThreshold(GoIO.TRACE_SEVERITY_HIGH)
            NGIOLibVerboseToolStripMenuItem.Checked = False
        Else
            GoIO.Diags_SetDebugTraceThreshold(GoIO.TRACE_SEVERITY_LOWEST)
            NGIOLibVerboseToolStripMenuItem.Checked = True
        End If
    End Sub

    Private Sub AboutNGIOMeasureToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles AboutNGIOMeasureToolStripMenuItem.Click
        Dim DialogBox As New AboutDlg()
        Dim sVersionLabel As String = "This app is linked to GoIO library version "
        sVersionLabel = sVersionLabel & lib_major_version.ToString("D1") & "." & lib_minor_version.ToString("D2")
        DialogBox.NGIO_lib_version_label.Text = sVersionLabel
        DialogBox.ShowDialog()
    End Sub

    Private Sub NewToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles NewToolStripMenuItem.Click
        StopMeasurements()
        UpdateControls()
        UpdateTitle()
        ClearGraph()
        UpdateUnits()
    End Sub

    Private Sub ExitToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ExitToolStripMenuItem.Click
        Me.Close()
    End Sub

    Private Sub Calib0ToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Calib0ToolStripMenuItem.Click
        Dim activeCalPageIndex As Byte = 0
        If (IntPtr.Zero <> GoIO_sensor_handle) Then
            GoIO.Sensor_DDSMem_GetActiveCalPage(GoIO_sensor_handle, activeCalPageIndex)
            If activeCalPageIndex <> 0 Then
                GoIO.Sensor_DDSMem_SetActiveCalPage(GoIO_sensor_handle, 0)
                ClearGraph()
                UpdateUnits()
            End If
        End If
    End Sub

    Private Sub Calib1ToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Calib1ToolStripMenuItem.Click
        If (IntPtr.Zero <> GoIO_sensor_handle) Then
            Dim highestValidCalPageIndex As Byte = 0
            Dim activeCalPageIndex As Byte = 0
            GoIO.Sensor_DDSMem_GetHighestValidCalPageIndex(GoIO_sensor_handle, highestValidCalPageIndex)
            If highestValidCalPageIndex > 0 Then
                GoIO.Sensor_DDSMem_GetActiveCalPage(GoIO_sensor_handle, activeCalPageIndex)
                If activeCalPageIndex <> 1 Then
                    GoIO.Sensor_DDSMem_SetActiveCalPage(GoIO_sensor_handle, 1)
                    ClearGraph()
                    UpdateUnits()
                End If
            End If
        End If
    End Sub

    Private Sub Calib2ToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Calib2ToolStripMenuItem.Click
        If (IntPtr.Zero <> GoIO_sensor_handle) Then
            Dim highestValidCalPageIndex As Byte = 0
            Dim activeCalPageIndex As Byte = 0
            GoIO.Sensor_DDSMem_GetHighestValidCalPageIndex(GoIO_sensor_handle, highestValidCalPageIndex)
            If highestValidCalPageIndex > 1 Then
                GoIO.Sensor_DDSMem_GetActiveCalPage(GoIO_sensor_handle, activeCalPageIndex)
                If activeCalPageIndex <> 2 Then
                    GoIO.Sensor_DDSMem_SetActiveCalPage(GoIO_sensor_handle, 2)
                    ClearGraph()
                    UpdateUnits()
                End If
            End If
        End If
    End Sub

    Private Sub CalibrationsToolStripMenuItem_DropDownOpening(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CalibrationsToolStripMenuItem.DropDownOpening
        Me.CalibrationsToolStripMenuItem.DropDownItems.Clear()
        If (IntPtr.Zero <> GoIO_sensor_handle) Then
            Dim activeCalPageIndex As Byte = 0
            Dim highestValidCalPageIndex As Byte = 0
            Dim units As StringBuilder = New StringBuilder(20)
            Dim offset, gain, dummy As Single

            GoIO.Sensor_DDSMem_GetHighestValidCalPageIndex(GoIO_sensor_handle, highestValidCalPageIndex)
            GoIO.Sensor_DDSMem_GetActiveCalPage(GoIO_sensor_handle, activeCalPageIndex)

            GoIO.Sensor_DDSMem_GetCalPage(GoIO_sensor_handle, 0, offset, gain, dummy, units, 20)
            Me.Calib0ToolStripMenuItem.Text = "Calib 0 " & units.ToString()
            If 0 <> activeCalPageIndex Then
                Me.Calib0ToolStripMenuItem.Checked = False
            Else
                Me.Calib0ToolStripMenuItem.Checked = True
            End If
            Me.CalibrationsToolStripMenuItem.DropDownItems.Add(Me.Calib0ToolStripMenuItem)

            If highestValidCalPageIndex > 0 Then
                GoIO.Sensor_DDSMem_GetCalPage(GoIO_sensor_handle, 1, offset, gain, dummy, units, 20)
                Me.Calib1ToolStripMenuItem.Text = "Calib 1 " & units.ToString()
                If 1 <> activeCalPageIndex Then
                    Me.Calib1ToolStripMenuItem.Checked = False
                Else
                    Me.Calib1ToolStripMenuItem.Checked = True
                End If
                Me.CalibrationsToolStripMenuItem.DropDownItems.Add(Me.Calib1ToolStripMenuItem)
            End If

            If highestValidCalPageIndex > 1 Then
                GoIO.Sensor_DDSMem_GetCalPage(GoIO_sensor_handle, 2, offset, gain, dummy, units, 20)
                Me.Calib2ToolStripMenuItem.Text = "Calib 2 " & units.ToString()
                If 2 <> activeCalPageIndex Then
                    Me.Calib2ToolStripMenuItem.Checked = False
                Else
                    Me.Calib2ToolStripMenuItem.Checked = True
                End If
                Me.CalibrationsToolStripMenuItem.DropDownItems.Add(Me.Calib2ToolStripMenuItem)
            End If
        End If
    End Sub

    Private Sub SetLEDToolStripMenuItem_DropDownOpening(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles SetLEDToolStripMenuItem.DropDownOpening
        SetLEDToolStripMenuItem.DropDownItems.Item(0).Enabled = (IntPtr.Zero <> GoIO_sensor_handle)
        SetLEDToolStripMenuItem.DropDownItems.Item(1).Enabled = (IntPtr.Zero <> GoIO_sensor_handle)
        SetLEDToolStripMenuItem.DropDownItems.Item(2).Enabled = (IntPtr.Zero <> GoIO_sensor_handle)
        SetLEDToolStripMenuItem.DropDownItems.Item(3).Enabled = (IntPtr.Zero <> GoIO_sensor_handle)
    End Sub

    Private Sub GreenToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles GreenToolStripMenuItem.Click
        Dim parms As GoIOSetLedStateParams = New GoIOSetLedStateParams

        If (IntPtr.Zero <> GoIO_sensor_handle) Then
            parms.color = GoIOSetLedStateParams.LED_COLOR_GREEN
            parms.brightness = GoIOSetLedStateParams.LED_BRIGHTNESS_MAX
            GoIO.Sensor_SendCmdAndGetResponse2(GoIO_sensor_handle, GoIO_ParmBlk.CMD_ID_SET_LED_STATE, parms, GoIO.TIMEOUT_MS_DEFAULT)
        End If
    End Sub

    Private Sub RedToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles RedToolStripMenuItem.Click
        Dim parms As GoIOSetLedStateParams = New GoIOSetLedStateParams

        If (IntPtr.Zero <> GoIO_sensor_handle) Then
            parms.color = GoIOSetLedStateParams.LED_COLOR_RED
            parms.brightness = GoIOSetLedStateParams.LED_BRIGHTNESS_MAX
            GoIO.Sensor_SendCmdAndGetResponse2(GoIO_sensor_handle, GoIO_ParmBlk.CMD_ID_SET_LED_STATE, parms, GoIO.TIMEOUT_MS_DEFAULT)
        End If
    End Sub

    Private Sub OrangeToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles OrangeToolStripMenuItem.Click
        Dim parms As GoIOSetLedStateParams = New GoIOSetLedStateParams

        If (IntPtr.Zero <> GoIO_sensor_handle) Then
            parms.color = GoIOSetLedStateParams.LED_COLOR_ORANGE
            parms.brightness = GoIOSetLedStateParams.LED_BRIGHTNESS_ORANGE
            GoIO.Sensor_SendCmdAndGetResponse2(GoIO_sensor_handle, GoIO_ParmBlk.CMD_ID_SET_LED_STATE, parms, GoIO.TIMEOUT_MS_DEFAULT)
        End If
    End Sub

    Private Sub OffToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles OffToolStripMenuItem.Click
        Dim parms As GoIOSetLedStateParams = New GoIOSetLedStateParams

        If (IntPtr.Zero <> GoIO_sensor_handle) Then
            parms.color = GoIOSetLedStateParams.LED_COLOR_BLACK
            parms.brightness = GoIOSetLedStateParams.LED_BRIGHTNESS_MIN
            GoIO.Sensor_SendCmdAndGetResponse2(GoIO_sensor_handle, GoIO_ParmBlk.CMD_ID_SET_LED_STATE, parms, GoIO.TIMEOUT_MS_DEFAULT)
        End If
    End Sub

    Private Sub GetSensorIdToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles GetSensorIdToolStripMenuItem.Click
        Dim oldSensorNumber As Byte
        Dim newSensorNumber As Byte
        Dim status As Integer
        Dim openSensorName As StringBuilder = New StringBuilder(CInt(GoIO.MAX_SIZE_SENSOR_NAME))
        Dim openSensorName2 As String = ""
        Dim vendorId As Integer
        Dim productId As Integer

        If (IntPtr.Zero <> GoIO_sensor_handle) Then
            GoIO.Sensor_DDSMem_GetSensorNumber(GoIO_sensor_handle, oldSensorNumber, 0, 0)

            'Actually query the hardware.
            status = GoIO.Sensor_DDSMem_GetSensorNumber(GoIO_sensor_handle, newSensorNumber, 1, GoIO.TIMEOUT_MS_DEFAULT)
            If (0 <> status) Then
                newSensorNumber = 0
            End If

            If (oldSensorNumber <> newSensorNumber) Then
                'The user must have switched sensors. The easiest way to handle this is to close and then reopen the device.
                GoIO.Sensor_GetOpenDeviceName(GoIO_sensor_handle, openSensorName, GoIO.MAX_SIZE_SENSOR_NAME, vendorId, productId)
                openSensorName2 = openSensorName.ToString()
                CloseSensor()
                OpenSensor(openSensorName2)

                UpdateControls()
                UpdateTitle()
                ClearGraph()
                UpdateUnits()
            End If
        End If
    End Sub
End Class
