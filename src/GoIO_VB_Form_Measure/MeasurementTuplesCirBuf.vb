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
