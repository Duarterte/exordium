@tool
extends SettingsManager

const TOOL_PANEL = preload("res://addons/settings/settings_panel.tscn")
var panel: VBoxContainer

func _enter_tree() -> void:
	panel = TOOL_PANEL.instantiate()
	var confFileInputTxt: LineEdit = panel.find_children("ConfFileName", "LineEdit", true)[0]
	var createConfFileBTN: Button = panel.find_children("ConfFileBTN", "Button", true)[0]
	
	createConfFileBTN.pressed.connect(createConfFile)
	
	add_control_to_dock(DOCK_SLOT_RIGHT_BR, panel)

func _exit_tree() -> void:
	remove_control_from_docks(panel)
	panel.queue_free()
