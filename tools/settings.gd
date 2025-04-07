@tool
extends EditorScript

const VERSION = "0.0.1"

enum {
    PRINTVERSION
}

func _run() -> void:
    
	print(VERSION)
