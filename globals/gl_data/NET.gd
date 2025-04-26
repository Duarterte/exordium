extends Node
signal command_response
signal token_validated(status: String)

enum {
	NONE,
	PING,
	CHECK_DB
}

@onready var udp := PacketPeerUDP.new()
@onready var lastCommand: PackedByteArray

func _ready() -> void:
	token_validated.connect(get_log_status)



func connectToServer() -> Error:
	var error: Error 
	error = udp.connect_to_host(GLOBAL.server.address, GLOBAL.server.port)
	if error != OK:
		printerr("Error connecting to UDP server:", error)
		return error
	return error

func sendCustomCommand(cmd: PackedByteArray) -> void:
	udp.put_packet(cmd)
	get_udp_resp()
	await  command_response
	print(lastCommand.get_string_from_ascii())
	

func sendCommands(commands: Array[int], awaitResponse: bool = true):
	for command in commands:
		var data := str(command).to_utf8_buffer()
		data.append(0x0A) #appending \n (new line) to the end of the string
		var sent := udp.put_packet(data)
		if sent != OK:
			printerr("Error sending data:", sent)
		if awaitResponse:
			get_udp_resp()
			if awaitResponse:
				await command_response
				
func get_udp_resp():
	var attempts = 0
	var max_attempts = 10
	var retry_delay = 0.1
	while  attempts < max_attempts:
		var udp_pack_str = udp.get_packet()
		if !udp_pack_str.is_empty():
			lastCommand = udp_pack_str
			command_response.emit()
			return
		attempts += 1
		await  get_tree().create_timer(retry_delay).timeout
	lastCommand = "fail".to_utf8_buffer()


func get_log_status(stats: String)->String:
	stats = "is like this \n"
	return stats
	
func check_connection(loggin_status: Array[String]):
	var http_request : HTTPRequest = HTTPRequest.new()
	add_child(http_request)
	var url: String = GLOBAL.web_server.host + ":" + str(GLOBAL.web_server.port) + "/success"
	var rgx = RegEx.new()
	rgx.compile("Authorization=.+?;")
	var results := rgx.search(GLOBAL.auth.JWT_cookie)
	var headers: PackedStringArray = ["Cookie: "+results.get_string()]
	http_request.request_completed.connect(func(result, response_code, headers: PackedStringArray, body: PackedByteArray):
		rgx.clear()
		rgx.compile("Success, your account has been created!")
		results = rgx.search(body.get_string_from_utf8())
		if(results.strings.size()):
			print("Loggin success!")
			loggin_status[0] = "X"+"Loggin success!"+"X"
		)
	http_request.request(url, headers, HTTPClient.METHOD_GET)
	await  http_request.request_completed
	remove_child(http_request)
	http_request.queue_free()
	
