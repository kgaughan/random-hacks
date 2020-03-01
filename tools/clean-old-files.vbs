'
' Clears files older than a certain number of days from a set of directories.
'

Dim cleaner
Set cleaner = New DirectoryCleaner

' Clean out files more than 30 days old.
cleaner.MaxFileAge = 30

' Add as many calls to the AddDir method you need for each folder you want
' cleaned.
cleaner.AddDir("C:\Path\To\Some\Folder")

' Scrub the folders.
cleaner.Run()

''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

''
' Wraps the directory cleaning functionality.
'
Class DirectoryCleaner

	Private m_maxFileAge
	Private m_directories

	Private Sub Class_Initialize()
		m_maxFileAge = 30
		m_directories = Array()
	End Sub

	Public Property Set MaxFileAge(age)
		m_maxFileAge = age
	End Property

	Public Sub AddDir(directory)
		m_directories(UBound(m_directories) + 1) = directory
	End Sub

	Public Sub Run
		Dim i, nm, folder, fso, f

		Set fso = CreateObject("Scripting.FileSystemObject")
		For i = LBound(m_directories) To UBound(m_directories)
			nm = m_directories(i)
			If fso.FolderExists(nm) Then
				Set folder = fso.GetFolder(nm)
				For Each f In folder.Files
					If DateDiff("d", f.DateCreated, Now()) > m_maxFileAge Then
						' Force the delete
						f.Delete(True)
					End If
				Next
			End If
		Next
	End Sub
End Class
