extends Node

signal webserv_status(loggin: bool)
signal status_conn

@onready var t1: Thread = Thread.new()
@onready var s1: Semaphore = Semaphore.new()
@onready var tex: TextureRect = $Container/TextureRect
@onready var serverInfo: Label = $Container/TextureRect/SplitContainer/SerStatLB
@onready var errorWindow: Popup = $ErrWin
@onready var errLabel: Label = $ErrWin/ErrViewPort/SubViewport/Control/ErrMsg
@onready var l_user: LineEdit = $Container/TextureRect/MarginContainer/Panel/SplitContainer/HSplitContainer/LineEditUsr
@onready var l_password: LineEdit = $Container/TextureRect/MarginContainer/Panel/SplitContainer/HSplitContainer2/LineEditPaswd
@onready var conf: ConfigFile = ConfigFile.new()
@onready var exo: ExoChaCha = ExoChaCha.new()

@onready var conn_check: bool = false
@onready var secret_check = false
@onready var log_msg: Array[String] = ["", ""]
@onready var online_color: bool = false

func _ready() -> void:
	webserv_status.connect(func(logged_status): return logged_status)
	status_conn.connect(NET.check_connection)
	cacheData()
	connectToServer()
	pingServer()
	t1.start(func()->String:
		s1.wait()
		status_conn.emit.call_deferred(log_msg)
		return "thread finish"
		)
	await status_conn
	
	
	
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
		
		if NET.lastCommand.get_string_from_utf8() == "pong\n":
			serverInfo.text = "ONLINE"
		
	else:
		await NET.command_response
		if NET.lastCommand.get_string_from_utf8() == "pong\n":
			serverInfo.text = "ONLINE"

func webServerResponse(result, response_code, headers: PackedStringArray, body) -> bool:
	if(result && response_code && body):
		print(body)
	if body.get_string_from_utf8() == "Logged Successfully\n":
		webserv_status.emit(true)# get only the JWT token
		var header_str: String = headers[0]
		GLOBAL.auth.JWT_cookie = header_str;
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
	GLOBAL.auth.HMAC_key = HMAC_key;
	GLOBAL.auth.CHACHA_token = CHACHA_key
	GLOBAL.auth.CHACHA_nonce = CHACHA_nonce
	GLOBAL.player.user = user
	var url: String = GLOBAL.web_server.host + ":" + str(GLOBAL.web_server.port) + "/login-game?user="+user+"&password="+password
	#var url: String = GLOBAL.web_server.host + ":" + str(GLOBAL.web_server.port) + "/ping?test1=godot&test2=4%2E4"
	var headers = ["Content-Type: application/x-www-form-urlencoded"]
	http_request.request_completed.connect(webServerResponse)
	http_request.request(url, headers, HTTPClient.METHOD_POST)
	var is_logged = await webserv_status
	remove_child(http_request)
	http_request.queue_free()
	
	
	
func _on_button_pressed() -> void:
	initalizeUserSession(l_user.text, l_password.text)
	
	
func store_secrets(user_or_email: String, hmac_key: PackedByteArray,chacha_key: PackedByteArray, chacha_nonce: PackedByteArray, log_msg: Array[String]) -> void:
	var http_request: HTTPRequest = HTTPRequest.new()
	add_child(http_request)
	var url: String = GLOBAL.web_server.host + ":" + str(GLOBAL.web_server.port) + "/secretes?user=%s&hmac-key=%s&chacha-key=%s&chacha-nonce=%s" % [user_or_email, hmac_key.hex_encode(), chacha_key.hex_encode(), chacha_nonce.hex_encode()]
	var rgx = RegEx.new()
	rgx.compile("Authorization=.+?;")
	var results := rgx.search(GLOBAL.auth.JWT_cookie)
	var headers: PackedStringArray = ["Cookie: "+results.get_string(), "Content-Type: application/x-www-form-urlencoded"]
	http_request.request_completed.connect(func(result, response_code, headers, body: PackedByteArray): log_msg[1] = body.get_string_from_utf8())
	http_request.request(url, headers, HTTPClient.METHOD_POST)
	await http_request.request_completed
		

func _process(delta: float) -> void:
	if(!conn_check &&!GLOBAL.auth.JWT_cookie.is_empty()):
		s1.post()
		conn_check = true
		print(t1.wait_to_finish())
	if(!online_color):
		if(log_msg[0] == "XLoggin success!X"):
			serverInfo.add_theme_color_override("font_color", Color(0, 1.0, 0, 1.0))
			online_color = true
			var user_or_email: String = GLOBAL.player.user
			var hmac_key :PackedByteArray = GLOBAL.auth.HMAC_key
			var chacha_key: PackedByteArray = GLOBAL.auth.CHACHA_token 
			var chacha_nonce: PackedByteArray = GLOBAL.auth.CHACHA_nonce
			store_secrets(user_or_email, hmac_key, chacha_key, chacha_nonce, log_msg)
	if(!secret_check && online_color):
		if !log_msg[1].is_empty():
			print(log_msg[1])
			secret_check = true
			var msg := exo.encrypt_data(GLOBAL.auth.CHACHA_token, GLOBAL.auth.CHACHA_nonce, "exordium")
			msg.append(0x0A) #appending \n (new line) to the end of the string
			await NET.sendCustomCommand(msg)
			
			
