Imports System.Windows.Forms

Public Class SetMeasurementPeriodDlg

	Private Sub OK_Button_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles OK_Button.Click
        Dim periodms As Int32 = 0
        Try
            periodms = CInt(period_uS_textbox.Text())
        Catch ex As Exception
            periodms = 0
        End Try
        If (periodms >= 5) And (periodms < &H10000) Then
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

	Private Sub period_uS_textbox_KeyPress(ByVal sender As System.Object, ByVal e As System.Windows.Forms.KeyPressEventArgs) Handles period_uS_textbox.KeyPress
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

	Private Sub SetMeasurementPeriodDlg_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load

	End Sub
End Class
