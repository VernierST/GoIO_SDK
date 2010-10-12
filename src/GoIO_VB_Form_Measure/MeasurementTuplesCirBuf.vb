Public Class MeasurementTuplesCirBuf
	Dim measurements As DoubleCirBuf
	Dim timestamps As DoubleCirBuf

	Public Sub New(ByVal maxNumElements As Integer)
		measurements = New DoubleCirBuf(maxNumElements)
		timestamps = New DoubleCirBuf(maxNumElements)
	End Sub

	Public Sub AddElement(ByVal measurement As Double, ByVal timestamp As Double)
		measurements.AddElement(measurement)
		timestamps.AddElement(timestamp)
	End Sub

	Public Function CopyElement(ByVal elementIndex As Integer, ByRef measurement As Double, ByRef timestamp As Double) As Boolean
		Dim bSuccess As Boolean
		bSuccess = measurements.CopyElement(measurement, elementIndex)
		If bSuccess Then
			bSuccess = timestamps.CopyElement(timestamp, elementIndex)
		End If
		Return (bSuccess)
	End Function

	Public Function NumElementsAvailable() As Integer
		Return (measurements.NumElementsAvailable())
	End Function

	Public Function MaxNumElementsAvailable() As Integer
		Return (measurements.MaxNumElementsAvailable())
	End Function

	Public Sub Clear()
		measurements.Clear()
		timestamps.Clear()
	End Sub

End Class
