extends EditorPlugin

class_name SettingsManager

func createConfFile()-> void: 
	if FileAccess.file_exists("res://addons/settings/CONF.cfg"):
		printerr("File Already exists!")
	else:
		var config = ConfigFile.new()
		config.set_value("Server", "name", "Test")
		config.set_value("Server", "address", "192.168.0.2")
		config.set_value("Server", "port", 49382)
		config.save("res://addons/settings/CONF.cfg")
		print("File Writed at settings/CONF.cfg")
		
