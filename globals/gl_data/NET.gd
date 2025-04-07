extends Node

signal command_response


enum {
	NONE,
	PING
}

var udp := PacketPeerUDP.new()
var lastCommand = ""

func connectToServer() -> Error:
	var error: Error 
	error = udp.connect_to_host(GLOBAL.server.address, GLOBAL.server.port)
	if error != OK:
		printerr("Error connecting to UDP server:", error)
		return error
	return error

func sendCommands(commands: Array[int], awaitResponse: bool = false, deterministic: String = ""):
	"""
	var data = "1".to_utf8_buffer()
	var sent = udp.put_packet(data)
	if sent != OK:
		printerr("Error sending data:", sent)
	"""
	for command in commands:
		var data := str(command).to_utf8_buffer()
		var sent := udp.put_packet(data)
		if sent != OK:
			printerr("Error sending data:", sent)
		if awaitResponse:
			get_udp_resp()
	
func get_udp_resp():
	var attempts = 0
	var max_attempts = 10
	var retry_delay = 0.1
	while  attempts < max_attempts:
		var str = udp.get_packet().get_string_from_utf8()
		if !str.is_empty():
			lastCommand = str
			command_response.emit(str)
			return
		attempts += 1
		await  get_tree().create_timer(retry_delay).timeout
	lastCommand = "ping fail"
		
		

	
