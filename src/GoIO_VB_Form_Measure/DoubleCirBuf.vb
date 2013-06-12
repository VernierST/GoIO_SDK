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

Public Class DoubleCirBuf
    Dim buffer() As Double
    Dim nElementsAllocated As Integer   'Max num elements in circular buffer = (nElementsAllocated-1)
    Dim nFirstElement As Integer
    Dim nNextElement As Integer

    Public Sub New(ByVal maxNumElements As Integer)
        nElementsAllocated = maxNumElements + 1
        ReDim buffer(nElementsAllocated)
        nFirstElement = 0
        nNextElement = 0
    End Sub

    Public Sub AddElement(ByVal element As Double)
        If NumElementsAvailable() = MaxNumElementsAvailable() Then
            nFirstElement = nFirstElement + 1
            If nFirstElement = nElementsAllocated Then
                nFirstElement = 0
            End If
        End If
        buffer(nNextElement) = element
        nNextElement = nNextElement + 1
        If nNextElement = nElementsAllocated Then
            nNextElement = 0
        End If
    End Sub

    Public Sub AddElements(ByVal elements As Double(), ByVal count As Integer)
        For N As Integer = 0 To (count - 1)
            AddElement(elements(N))
        Next N
    End Sub

    Public Function RetrieveElement(ByRef element As Double) As Boolean
        Dim bSuccess As Boolean
        If NumElementsAvailable() > 0 Then
            bSuccess = True
            element = buffer(nFirstElement)
            nFirstElement = nFirstElement + 1
            If nFirstElement = nElementsAllocated Then
                nFirstElement = 0
            End If
        Else
            bSuccess = False
        End If
        Return (bSuccess)
    End Function

    Public Function RetrieveElements(ByVal elements As Double(), ByVal count As Integer) As Integer
        Dim numElementsRetrieved As Integer = 0
        While numElementsRetrieved < count
            If RetrieveElement(elements(numElementsRetrieved)) Then
                numElementsRetrieved = numElementsRetrieved + 1
            Else
                Exit While
            End If
        End While

        Return (numElementsRetrieved)
    End Function

    Public Function CopyElement(ByRef element As Double, ByVal elementIndex As Integer) As Boolean
        Dim bSuccess As Boolean
        Dim absElementIndex As Integer
        If NumElementsAvailable() > elementIndex Then
            bSuccess = True
            absElementIndex = nFirstElement + elementIndex
            If absElementIndex >= nElementsAllocated Then
                absElementIndex = absElementIndex - nElementsAllocated
            End If
            element = buffer(absElementIndex)
        Else
            bSuccess = False
        End If
        Return (bSuccess)
    End Function

    Public Function CopyElements(ByVal elements As Double(), ByVal count As Integer) As Integer
        Dim numElementsCopied As Integer = 0
        While numElementsCopied < count
            If CopyElement(elements(numElementsCopied), numElementsCopied) Then
                numElementsCopied = numElementsCopied + 1
            Else
                Exit While
            End If
        End While

        Return (numElementsCopied)
    End Function

    Public Function NumElementsAvailable() As Integer
        Dim numElementsAvail As Integer = nNextElement - nFirstElement
        If (numElementsAvail < 0) Then
            numElementsAvail = numElementsAvail + nElementsAllocated
        End If

        Return (numElementsAvail)
    End Function

    Public Function MaxNumElementsAvailable() As Integer
        Return (nElementsAllocated - 1)
    End Function

    Public Sub Clear()
        nFirstElement = 0
        nNextElement = 0
    End Sub

End Class

