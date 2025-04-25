extends Node

var server: Dictionary = {
	"name": "",
	"address": "",
	"port": 0
}

var web_server: Dictionary = {
	"host": "",
	"port": 0
}

var auth: Dictionary = {
	"JWT_cookie" : "",
	"HMAC_toke": PackedByteArray(),	
	"CHACHA_token": PackedByteArray(),
	"CHAHA_nonce": PackedByteArray()
}

var player: Dictionary = {
	"user": "" #can be username or email
}
