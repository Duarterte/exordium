extends Node

signal webserv_status(loggin: bool)



@onready var tex: TextureRect = $Container/TextureRect
@onready var serverInfo: Label = $Container/TextureRect/SplitContainer/SerStatLB
@onready var errorWindow: Popup = $ErrWin
@onready var errLabel: Label = $ErrWin/ErrViewPort/SubViewport/Control/ErrMsg
@onready var l_user: LineEdit = $Container/TextureRect/MarginContainer/Panel/SplitContainer/HSplitContainer/LineEditUsr
@onready var l_password: LineEdit = $Container/TextureRect/MarginContainer/Panel/SplitContainer/HSplitContainer2/LineEditPaswd
@onready var conf: ConfigFile = ConfigFile.new()
@onready var exo: ExoChaCha = ExoChaCha.new()
func _ready() -> void:
	webserv_status.connect(func(logged_status): return logged_status)
	cacheData()
	connectToServer()
	pingServer()
	
	"""
	var key := exo.generate_key()
	var nonce := exo.generate_nonce()
	var enc : PackedByteArray = exo.encrypt_data(key, nonce, "test msg");
	var dec : PackedByteArray = exo.decrypt_data(key, nonce, enc)

	if dec.size() > 0:
		# Decryption was successful
		var decrypted_string : String = dec.get_string_from_utf8()
		print("Encrypted:", enc.hex_encode())
		print("Decrypted:", decrypted_string)
	else:
		# Decryption failed (returned an empty PackedByteArray)
		print("Decryption failed!")
	"""
	
	
func cacheData() -> void:
	if !FileAccess.file_exists("res://addons/settings/CONF.cfg"):
		errorWindow.visible = true
		errLabel.text = "Config File Not Found!"
	else:
		conf.load("res://addons/settings/CONF.cfg")

		var servName = conf.get_value("Server", "name")
		var servAddress = conf.get_value("Server", "address")
		var servPort = conf.get_value("Server", "port")
		
		var webHost = conf.get_value("Web", "host")
		var webPort = conf.get_value("Web", "port")

		GLOBAL.server.set("name", servName) 
		GLOBAL.server.set("address", servAddress)
		GLOBAL.server.set("port", servPort)
		
		GLOBAL.web_server.set("host", webHost)
		GLOBAL.web_server.set("port", webPort)
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

func webServerResponse(result, response_code, headers, body) -> bool:
	#print("--- Web Server Response ---")
	#print("Result:", result)
	#print("Response Code:", response_code)
	#print("Headers:", headers)
	#print("Body:", body.get_string_from_utf8())
	if body.get_string_from_utf8() == "Logged Successfully\n":
		webserv_status.emit(true)
		return true
	webserv_status.emit(false)
	return false
	

func initalizeUserSession (user: String , password: String) -> void:
	var crypto: Crypto = Crypto.new()
	var http_request: HTTPRequest = HTTPRequest.new()
	add_child(http_request)
	var HMAC_key: PackedByteArray = crypto.generate_random_bytes(32)
	var CHACHA_key: PackedByteArray = exo.generate_key()
	var CHACHA_nonce: PackedByteArray = exo.generate_nonce();
	var url: String = GLOBAL.web_server.host + ":" + str(GLOBAL.web_server.port) + "/login-game?user="+user+"&password="+password
	#var url: String = GLOBAL.web_server.host + ":" + str(GLOBAL.web_server.port) + "/ping?test1=godot&test2=4%2E4"
	var headers = ["Content-Type: application/x-www-form-urlencoded"]
	http_request.request_completed.connect(webServerResponse)
	var r:bool = await http_request.request(url, headers, HTTPClient.METHOD_POST)
	var is_logged = await webserv_status
	print(is_logged)




func _on_button_pressed() -> void:
	initalizeUserSession(l_user.text, l_password.text)
