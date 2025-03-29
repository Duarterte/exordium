extends EditorPlugin

class_name SettingsManager

func createConfFile()-> void: 
	if FileAccess.file_exists("res://addons/settings/CONF.dat"):
		printerr("File Already exists!")
	else:
		var file = FileAccess.open("res://addons/settings/CONF.dat", FileAccess.WRITE_READ)
		file.store_string("")
		print_debug("File Created!")
