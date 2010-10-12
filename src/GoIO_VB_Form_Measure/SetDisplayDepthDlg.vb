Imports System.Windows.Forms

Public Class SetDisplayDepthDlg

    Private Sub OK_Button_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles OK_Button.Click
		Dim depth As Int32 = 0
		Try
			depth = CInt(displayDepthTextBox.Text())
		Catch ex As Exception
			depth = 0
		End Try
		If (depth >= 10) And (depth <= 100000) Then
			Me.DialogResult = System.Windows.Forms.DialogResult.OK
			Me.Close()
		Else
			Beep()
		End If
	End Sub

    Private Sub Cancel_Button_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Cancel_Button.Click
        Me.DialogResult = System.Windows.Forms.DialogResult.Cancel
        Me.Close()
    End Sub

	Private Sub displayDepthTextBox_KeyPress(ByVal sender As System.Object, ByVal e As System.Windows.Forms.KeyPressEventArgs) Handles displayDepthTextBox.KeyPress
		Dim bIgnore As Boolean = True

		If Char.IsDigit(e.KeyChar) Then
			bIgnore = False
		End If
		If Char.IsControl(e.KeyChar) Then
			bIgnore = False
		End If
		If bIgnore Then
			' Invalid Character
			e.Handled = True
		End If
	End Sub
End Class
