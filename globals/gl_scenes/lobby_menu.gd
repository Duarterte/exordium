extends Node
@onready var tex: TextureRect = $Container/TextureRect
@onready var serverInfo: Label = $Container/TextureRect/SplitContainer/SerStatLB
@onready var errorWindow: Popup = $ErrWin
@onready var errLabel: Label = $ErrWin/ErrViewPort/SubViewport/Control/ErrMsg
@onready var conf: ConfigFile = ConfigFile.new()

func _ready() -> void:
	cacheData()
	connectToServer()
	pingServer()
	
	
func cacheData() -> void:
	if !FileAccess.file_exists("res://addons/settings/CONF.cfg"):
		errorWindow.visible = true
		errLabel.text = "Config File Not Found!"
	else:
		conf.load("res://addons/settings/CONF.cfg")

		var servName = conf.get_value("Server", "name")
		var servAddress = conf.get_value("Server", "address")
		var servPort = conf.get_value("Server", "port")

		GLOBAL.server.set("name", servName) 
		GLOBAL.server.set("address", servAddress)
		GLOBAL.server.set("port", servPort)
		#serverInfo.text = "Server Name: " + servName + "\nServer Address: " + servAddress + "\nServer Port: " + str(servPort)

func connectToServer() -> void:
	var err:= NET.connectToServer()
	if err != OK:
		errorWindow.visible = true
		errLabel.text = "Can Not Connect To The Server!"
		return

func pingServer() -> void:
	await NET.sendCommands([NET.PING], true)
	if(!NET.lastCommand.is_empty()):
		if NET.lastCommand == "pong\n":
			serverInfo.text = "ONLINE"
		
	else:
		await NET.command_response
		if NET.lastCommand == "pong\n":
			serverInfo.text = "ONLINE"
