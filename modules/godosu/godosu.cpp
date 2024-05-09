#include "godosu.h"
#include "godosu_functions.h"

#include "core/config/project_settings.h"
#include "core/io/file_access.h"
#include "scene/audio/audio_stream_player.h"
#include "scene/main/window.h"
#include "scene/resources/texture.h"

void Godosu::_draw_canvas_item(CanvasItem *p_item) {
	const auto I = draw_queue.find(p_item);
	{
		const double current_time = OS::get_singleton()->get_unix_time();
		if (!I) {
			const double last_used = p_item->get_meta(SNAME("last_use"), current_time);
			if (last_used < current_time - 5.0) {
				if (p_item->get_meta(SNAME("clipped"), false)) {
					p_item->get_parent()->queue_free();
				} else {
					p_item->queue_free();
				}
				to_remove.push_back(p_item);
			}
			return;
		}
		p_item->set_meta(SNAME("last_use"), current_time);
	}

	for (const DrawCommand &draw_command : I->value) {
		switch (draw_command.type) {
			case DrawCommand::DRAW_RECT: {
				const Rect2 &rect = draw_command.arguments[0];
				p_item->draw_rect(rect, Color(1, 1, 1), true); // TODO kolor?
			} break;

			case DrawCommand::DRAW_TEXTURE: {
				const Ref<Texture2D> &texture = draw_command.arguments[0];
				const Vector2 &pos = draw_command.arguments[1];
				const Vector2 &draw_scale = draw_command.arguments[2];
				const Color &color = draw_command.arguments[3];
				p_item->draw_texture_rect(texture, Rect2(pos + texture->get_size() * draw_scale.min(Vector2()), texture->get_size() * draw_scale), false, color);
			} break;

			case DrawCommand::DRAW_TEXTURE_ROTATED: {
				Ref<Texture2D> texture = draw_command.arguments[0];
				const Vector2 &pos = draw_command.arguments[1];
				const float angle = draw_command.arguments[2];
				const Vector2 &center = draw_command.arguments[3];
				const Vector2 &draw_scale = draw_command.arguments[4];
				const Color &color = draw_command.arguments[5];

				Vector2 uv_begin = Vector2(0, 0);
				Vector2 uv_end = Vector2(1, 1);
				Vector2 size;

				const Ref<AtlasTexture> atlas = texture;
				if (atlas.is_valid()) {
					texture = atlas->get_atlas();
					const Vector2 atlas_size = texture->get_size();

					const Vector2 remap_min = atlas->get_region().position / atlas_size;
					const Vector2 remap_max = atlas->get_region().get_end() / atlas_size;
					size = atlas->get_region().get_size() * draw_scale;

					uv_begin.x = Math::remap(uv_begin.x, 0, 1, remap_min.x, remap_max.x);
					uv_end.x = Math::remap(uv_end.x, 0, 1, remap_min.x, remap_max.x);
					uv_begin.y = Math::remap(uv_begin.y, 0, 1, remap_min.y, remap_max.y);
					uv_end.y = Math::remap(uv_end.y, 0, 1, remap_min.y, remap_max.y);
				} else {
					size = texture->get_size() * draw_scale;
				}

				const Vector2 draw_offset(Math::sin(angle), -Math::cos(angle));

				const Vector2 dist_to_left(
						draw_offset.y * size.x * center.x,
						-draw_offset.x * size.x * center.x);
				const Vector2 dist_to_right(
						-draw_offset.y * size.x * (1 - center.x),
						draw_offset.x * size.x * (1 - center.x));
				const Vector2 dist_to_top(
						draw_offset.x * size.y * center.y,
						draw_offset.y * size.y * center.y);
				const Vector2 dist_to_bottom(
						-draw_offset.x * size.y * (1 - center.y),
						-draw_offset.y * size.y * (1 - center.y));

				PackedVector2Array points{
					Vector2(pos.x + dist_to_left.x + dist_to_top.x, pos.y + dist_to_left.y + dist_to_top.y),
					Vector2(pos.x + dist_to_right.x + dist_to_top.x, pos.y + dist_to_right.y + dist_to_top.y),
					Vector2(pos.x + dist_to_right.x + dist_to_bottom.x, pos.y + dist_to_right.y + dist_to_bottom.y),
					Vector2(pos.x + dist_to_left.x + dist_to_bottom.x, pos.y + dist_to_left.y + dist_to_bottom.y),
				};

				p_item->draw_colored_polygon(points, color, { uv_begin, Vector2(uv_end.x, uv_begin.y), uv_end, Vector2(uv_begin.x, uv_end.y) }, texture);
			} break;

			case DrawCommand::DRAW_POLYGON: {
				const PackedVector2Array &points = draw_command.arguments[0];
				const PackedColorArray &colors = draw_command.arguments[1];
				p_item->draw_polygon(points, colors);
			} break;

			case DrawCommand::DRAW_LINE: {
				const Vector2 &point1 = draw_command.arguments[0];
				const Vector2 &point2 = draw_command.arguments[1];
				const Color &color1 = draw_command.arguments[2];
				const Color &color2 = draw_command.arguments[3];
				p_item->draw_line(point1, point2, color1.lerp(color2, 0.5)); // FIXME kolory
			} break;

			case DrawCommand::DRAW_STRING: {
				const Ref<Font> &font = draw_command.arguments[0];
				Vector2 pos = draw_command.arguments[1];
				const String &text = draw_command.arguments[2];
				const int font_size = draw_command.arguments[3];
				const Color &color = draw_command.arguments[4];
				const Vector2 &skale = draw_command.arguments[5]; // TODO
				const Vector2 &rel = draw_command.arguments[6];

				const Vector2 text_size = font->get_string_size(text, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size);
				pos.y += text_size.y * 0.7; // FIXME
				p_item->draw_string(font, pos - text_size * rel, text, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, color);
			} break;
		}
	}
}

void Godosu::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			if (Engine::get_singleton()->is_editor_hint()) {
				return;
			}

			print_verbose("Starting...");
			data.song_player = memnew(AudioStreamPlayer);
			add_child(data.song_player);

			{
				Ref<CanvasItemMaterial> mat;
				mat.instantiate();
				mat->set_blend_mode(CanvasItemMaterial::BLEND_MODE_ADD);
				data.additive_material = mat;
			}

			RenderingServer::get_singleton()->set_default_clear_color(Color(0, 0, 0));

			// Initialize Ruby.

#ifdef WINDOWS_ENABLED
			{
				int argc = 0;
				char *argv0 = "";
				char **argv1 = &argv0;
				rb_w32_sysinit(&argc, &argv1);
			}
#endif
			print_verbose("Initializing Ruby");
			ruby_init();
			print_verbose("Initializing Ruby loadpath");
			ruby_init_loadpath();

			print_verbose("Creating callbacks");
			data.callback_base = rb_intern("godot_callback");
			data.callback_update_mouse = rb_intern("godot_update_mouse");
			data.callback_update = ID2SYM(rb_intern("update"));
			data.callback_draw = ID2SYM(rb_intern("godot_draw"));
			data.callback_button_down = ID2SYM(rb_intern("godot_button_down"));
			data.callback_button_up = ID2SYM(rb_intern("godot_button_up"));

#define DEFINE_FUNCTION(function, argc) rb_define_global_function("godot_" #function, godosu_##function, argc)

			print_verbose("Defining functions");

			DEFINE_FUNCTION(print, 1);
			DEFINE_FUNCTION(exit, 0);
			DEFINE_FUNCTION(retrofication, 0);
			DEFINE_FUNCTION(set_clip, 4);
			DEFINE_FUNCTION(hsv_to_rgb, 3);
			DEFINE_FUNCTION(button_id_to_char, 1);

			DEFINE_FUNCTION(setup_window, 4);
			DEFINE_FUNCTION(set_window_title, 1);
			DEFINE_FUNCTION(set_window_fullscreen, 1);

			DEFINE_FUNCTION(create_text_input, 0);
			DEFINE_FUNCTION(destroy_text_input, 1);
			DEFINE_FUNCTION(focus_text_input, 1);
			DEFINE_FUNCTION(unfocus_text_input, 0);
			DEFINE_FUNCTION(set_text_input_text, 2);
			DEFINE_FUNCTION(get_text_input_text, 1);
			DEFINE_FUNCTION(set_text_input_caret, 2);
			DEFINE_FUNCTION(get_text_input_caret, 1);
			DEFINE_FUNCTION(set_text_input_selection_start, 2);
			DEFINE_FUNCTION(get_text_input_selection_start, 1);

			DEFINE_FUNCTION(load_image, 2);
			DEFINE_FUNCTION(load_atlas, 6);
			DEFINE_FUNCTION(load_audio, 2);
			DEFINE_FUNCTION(load_font, 2);

			DEFINE_FUNCTION(draw_line, 8);
			DEFINE_FUNCTION(draw_quad, 14);
			DEFINE_FUNCTION(draw_rect, 7);
			DEFINE_FUNCTION(draw_triangle, 11);
			DEFINE_FUNCTION(draw_texture, 8);
			DEFINE_FUNCTION(draw_texture_rotated, 10);
			DEFINE_FUNCTION(draw_string, 11);
			DEFINE_FUNCTION(get_text_width, 2);

			DEFINE_FUNCTION(play_song, 2);
			DEFINE_FUNCTION(stop_song, 0);
			DEFINE_FUNCTION(play_sample, 3);
			DEFINE_FUNCTION(is_channel_playing, 1);
			DEFINE_FUNCTION(destroy_channel, 1);

			DEFINE_FUNCTION(create_shader, 2);
			DEFINE_FUNCTION(set_shader, 2);

#undef DEFINE_FUNCTION

			// Input constants.

			print_verbose("Defining constants");
			VALUE GosuModule = rb_define_module("Gosu");

			rb_define_const(GosuModule, "Kb0", LONG2NUM(int(Key::KEY_0)));
			rb_define_const(GosuModule, "Kb1", LONG2NUM(int(Key::KEY_1)));
			rb_define_const(GosuModule, "Kb2", LONG2NUM(int(Key::KEY_2)));
			rb_define_const(GosuModule, "Kb3", LONG2NUM(int(Key::KEY_3)));
			rb_define_const(GosuModule, "Kb4", LONG2NUM(int(Key::KEY_4)));
			rb_define_const(GosuModule, "Kb5", LONG2NUM(int(Key::KEY_5)));
			rb_define_const(GosuModule, "Kb6", LONG2NUM(int(Key::KEY_6)));
			rb_define_const(GosuModule, "Kb7", LONG2NUM(int(Key::KEY_7)));
			rb_define_const(GosuModule, "Kb8", LONG2NUM(int(Key::KEY_8)));
			rb_define_const(GosuModule, "Kb9", LONG2NUM(int(Key::KEY_9)));

			rb_define_const(GosuModule, "KbA", LONG2NUM(int(Key::A)));
			rb_define_const(GosuModule, "KbB", LONG2NUM(int(Key::B)));
			rb_define_const(GosuModule, "KbC", LONG2NUM(int(Key::C)));
			rb_define_const(GosuModule, "KbD", LONG2NUM(int(Key::D)));
			rb_define_const(GosuModule, "KbE", LONG2NUM(int(Key::E)));
			rb_define_const(GosuModule, "KbF", LONG2NUM(int(Key::F)));
			rb_define_const(GosuModule, "KbG", LONG2NUM(int(Key::G)));
			rb_define_const(GosuModule, "KbH", LONG2NUM(int(Key::H)));
			rb_define_const(GosuModule, "KbI", LONG2NUM(int(Key::I)));
			rb_define_const(GosuModule, "KbJ", LONG2NUM(int(Key::J)));
			rb_define_const(GosuModule, "KbK", LONG2NUM(int(Key::K)));
			rb_define_const(GosuModule, "KbL", LONG2NUM(int(Key::L)));
			rb_define_const(GosuModule, "KbM", LONG2NUM(int(Key::M)));
			rb_define_const(GosuModule, "KbN", LONG2NUM(int(Key::N)));
			rb_define_const(GosuModule, "KbO", LONG2NUM(int(Key::O)));
			rb_define_const(GosuModule, "KbP", LONG2NUM(int(Key::P)));
			rb_define_const(GosuModule, "KbQ", LONG2NUM(int(Key::Q)));
			rb_define_const(GosuModule, "KbR", LONG2NUM(int(Key::R)));
			rb_define_const(GosuModule, "KbS", LONG2NUM(int(Key::S)));
			rb_define_const(GosuModule, "KbT", LONG2NUM(int(Key::T)));
			rb_define_const(GosuModule, "KbU", LONG2NUM(int(Key::U)));
			rb_define_const(GosuModule, "KbV", LONG2NUM(int(Key::V)));
			rb_define_const(GosuModule, "KbW", LONG2NUM(int(Key::W)));
			rb_define_const(GosuModule, "KbX", LONG2NUM(int(Key::X)));
			rb_define_const(GosuModule, "KbY", LONG2NUM(int(Key::Y)));
			rb_define_const(GosuModule, "KbZ", LONG2NUM(int(Key::Z)));

			rb_define_const(GosuModule, "KbApostrophe", LONG2NUM(int(Key::APOSTROPHE)));
			rb_define_const(GosuModule, "KbBackslash", LONG2NUM(int(Key::BACKSLASH)));
			rb_define_const(GosuModule, "KbBackspace", LONG2NUM(int(Key::BACKSPACE)));
			rb_define_const(GosuModule, "KbBacktick", LONG2NUM(int(Key::QUOTELEFT)));
			rb_define_const(GosuModule, "KbComma", LONG2NUM(int(Key::COMMA)));
			rb_define_const(GosuModule, "KbDelete", LONG2NUM(int(Key::KEY_DELETE)));
			rb_define_const(GosuModule, "KbDown", LONG2NUM(int(Key::DOWN)));
			rb_define_const(GosuModule, "KbEnd", LONG2NUM(int(Key::END)));
			rb_define_const(GosuModule, "KbPrintScreen", LONG2NUM(int(Key::PRINT)));
			rb_define_const(GosuModule, "KbScrollLock", LONG2NUM(int(Key::SCROLLLOCK)));
			rb_define_const(GosuModule, "KbPause", LONG2NUM(int(Key::PAUSE)));
			rb_define_const(GosuModule, "KbEnter", LONG2NUM(int(Key::KP_ENTER)));
			rb_define_const(GosuModule, "KbEquals", LONG2NUM(int(Key::EQUAL)));
			rb_define_const(GosuModule, "KbEscape", LONG2NUM(int(Key::ESCAPE)));

			rb_define_const(GosuModule, "KbF1", LONG2NUM(int(Key::F1)));
			rb_define_const(GosuModule, "KbF2", LONG2NUM(int(Key::F2)));
			rb_define_const(GosuModule, "KbF3", LONG2NUM(int(Key::F3)));
			rb_define_const(GosuModule, "KbF4", LONG2NUM(int(Key::F4)));
			rb_define_const(GosuModule, "KbF5", LONG2NUM(int(Key::F5)));
			rb_define_const(GosuModule, "KbF6", LONG2NUM(int(Key::F6)));
			rb_define_const(GosuModule, "KbF7", LONG2NUM(int(Key::F7)));
			rb_define_const(GosuModule, "KbF8", LONG2NUM(int(Key::F8)));
			rb_define_const(GosuModule, "KbF9", LONG2NUM(int(Key::F9)));
			rb_define_const(GosuModule, "KbF10", LONG2NUM(int(Key::F10)));
			rb_define_const(GosuModule, "KbF11", LONG2NUM(int(Key::F11)));
			rb_define_const(GosuModule, "KbF12", LONG2NUM(int(Key::F12)));

			rb_define_const(GosuModule, "KbHome", LONG2NUM(int(Key::HOME)));
			rb_define_const(GosuModule, "KbInsert", LONG2NUM(int(Key::INSERT)));
			rb_define_const(GosuModule, "KbLeft", LONG2NUM(int(Key::LEFT)));
			rb_define_const(GosuModule, "KbLeftAlt", LONG2NUM(int(Key::ALT)));
			rb_define_const(GosuModule, "KbLeftBracket", LONG2NUM(int(Key::BRACKETLEFT)));
			rb_define_const(GosuModule, "KbLeftControl", LONG2NUM(int(Key::CTRL)));
			rb_define_const(GosuModule, "KbLeftMeta", LONG2NUM(int(Key::META)));
			rb_define_const(GosuModule, "KbLeftShift", LONG2NUM(int(Key::SHIFT)));
			rb_define_const(GosuModule, "KbMinus", LONG2NUM(int(Key::MINUS)));

			rb_define_const(GosuModule, "KbNumpad0", LONG2NUM(int(Key::KP_0)));
			rb_define_const(GosuModule, "KbNumpad1", LONG2NUM(int(Key::KP_1)));
			rb_define_const(GosuModule, "KbNumpad2", LONG2NUM(int(Key::KP_2)));
			rb_define_const(GosuModule, "KbNumpad3", LONG2NUM(int(Key::KP_3)));
			rb_define_const(GosuModule, "KbNumpad4", LONG2NUM(int(Key::KP_4)));
			rb_define_const(GosuModule, "KbNumpad5", LONG2NUM(int(Key::KP_5)));
			rb_define_const(GosuModule, "KbNumpad6", LONG2NUM(int(Key::KP_6)));
			rb_define_const(GosuModule, "KbNumpad7", LONG2NUM(int(Key::KP_7)));
			rb_define_const(GosuModule, "KbNumpad8", LONG2NUM(int(Key::KP_8)));
			rb_define_const(GosuModule, "KbNumpad9", LONG2NUM(int(Key::KP_9)));
			rb_define_const(GosuModule, "KbNumpadDelete", LONG2NUM(int(Key::KP_PERIOD)));
			rb_define_const(GosuModule, "KbNumpadDivide", LONG2NUM(int(Key::KP_DIVIDE)));
			rb_define_const(GosuModule, "KbNumpadMinus", LONG2NUM(int(Key::KP_SUBTRACT)));
			rb_define_const(GosuModule, "KbNumpadMultiply", LONG2NUM(int(Key::KP_MULTIPLY)));
			rb_define_const(GosuModule, "KbNumpadPlus", LONG2NUM(int(Key::KP_ADD)));

			rb_define_const(GosuModule, "KbPageDown", LONG2NUM(int(Key::PAGEDOWN)));
			rb_define_const(GosuModule, "KbPageUp", LONG2NUM(int(Key::PAGEUP)));
			rb_define_const(GosuModule, "KbPeriod", LONG2NUM(int(Key::PERIOD)));
			rb_define_const(GosuModule, "KbReturn", LONG2NUM(int(Key::ENTER)));
			rb_define_const(GosuModule, "KbRight", LONG2NUM(int(Key::RIGHT)));
			rb_define_const(GosuModule, "KbRightAlt", LONG2NUM(int(Key::ALT)));
			rb_define_const(GosuModule, "KbRightBracket", LONG2NUM(int(Key::BRACERIGHT)));
			rb_define_const(GosuModule, "KbRightControl", LONG2NUM(int(Key::CTRL)));
			rb_define_const(GosuModule, "KbRightMeta", LONG2NUM(int(Key::META)));
			rb_define_const(GosuModule, "KbRightShift", LONG2NUM(int(Key::SHIFT)));
			rb_define_const(GosuModule, "KbSemicolon", LONG2NUM(int(Key::SEMICOLON)));
			rb_define_const(GosuModule, "KbSlash", LONG2NUM(int(Key::SLASH)));
			rb_define_const(GosuModule, "KbCapsLock", LONG2NUM(int(Key::CAPSLOCK)));
			rb_define_const(GosuModule, "KbSpace", LONG2NUM(int(Key::SPACE)));
			rb_define_const(GosuModule, "KbTab", LONG2NUM(int(Key::TAB)));
			rb_define_const(GosuModule, "KbUp", LONG2NUM(int(Key::UP)));

			rb_define_const(GosuModule, "MsLeft", LONG2NUM(int(MouseButton::LEFT)));
			rb_define_const(GosuModule, "MsMiddle", LONG2NUM(int(MouseButton::MIDDLE)));
			rb_define_const(GosuModule, "MsRight", LONG2NUM(int(MouseButton::RIGHT)));
			rb_define_const(GosuModule, "MsWheelDown", LONG2NUM(int(MouseButton::WHEEL_DOWN)));
			rb_define_const(GosuModule, "MsWheelUp", LONG2NUM(int(MouseButton::WHEEL_UP)));

			rb_define_const(GosuModule, "GpButton0", LONG2NUM(int(JoyButton::A)));
			rb_define_const(GosuModule, "GpButton1", LONG2NUM(int(JoyButton::B)));
			rb_define_const(GosuModule, "GpButton2", LONG2NUM(int(JoyButton::X)));
			rb_define_const(GosuModule, "GpButton3", LONG2NUM(int(JoyButton::Y)));
			rb_define_const(GosuModule, "GpButton4", LONG2NUM(int(JoyButton::BACK)));
			rb_define_const(GosuModule, "GpButton5", LONG2NUM(int(JoyButton::GUIDE)));
			rb_define_const(GosuModule, "GpButton6", LONG2NUM(int(JoyButton::START)));
			rb_define_const(GosuModule, "GpButton7", LONG2NUM(int(JoyButton::LEFT_STICK)));
			rb_define_const(GosuModule, "GpButton8", LONG2NUM(int(JoyButton::RIGHT_STICK)));
			rb_define_const(GosuModule, "GpButton9", LONG2NUM(int(JoyButton::LEFT_SHOULDER)));
			rb_define_const(GosuModule, "GpButton10", LONG2NUM(int(JoyButton::RIGHT_SHOULDER)));
			rb_define_const(GosuModule, "GpButton11", LONG2NUM(int(JoyButton::DPAD_UP)));
			rb_define_const(GosuModule, "GpButton12", LONG2NUM(int(JoyButton::DPAD_DOWN)));
			rb_define_const(GosuModule, "GpButton13", LONG2NUM(int(JoyButton::DPAD_LEFT)));
			rb_define_const(GosuModule, "GpButton14", LONG2NUM(int(JoyButton::DPAD_RIGHT)));
			rb_define_const(GosuModule, "GpButton15", LONG2NUM(int(JoyButton::MISC1)));

			rb_define_const(GosuModule, "GpLeft", LONG2NUM(int(JoyAxis::LEFT_X))); // FIXME
			rb_define_const(GosuModule, "GpRight", LONG2NUM(int(JoyAxis::LEFT_X)));
			rb_define_const(GosuModule, "GpUp", LONG2NUM(int(JoyAxis::LEFT_Y)));
			rb_define_const(GosuModule, "GpDown", LONG2NUM(int(JoyAxis::LEFT_Y)));

			// Run.

			print_verbose("Configuring exec");
			const String main_script_path = ProjectSettings::get_singleton()->globalize_path(main_script);
			const String exec = vformat(R"(-e require "%s" )", main_script_path);

			char *options[] = { "-v", const_cast<char *>(exec.ascii().get_data()) };
			void *node = ruby_options(2, options);

			print_verbose("Starting application");
			if (ruby_executable_node(node, &ruby_state)) {
				print_verbose("Any moment now");
				ruby_state = ruby_exec_node(node);
				print_verbose("Started: " + itos(ruby_state));
			}
		} break;

		case NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
			rb_funcall(data.window, data.callback_update_mouse, 2,
					INT2NUM(int(get_viewport()->get_mouse_position().x)),
					INT2NUM(int(get_viewport()->get_mouse_position().y)));
			godosu_window_callback(1, data.callback_update);
		} break;

		case NOTIFICATION_INTERNAL_PROCESS: {
			draw_queue.clear();
			if (!to_remove.is_empty()) {
				for (const auto kv : ci_map) {
					if (to_remove.find(kv.value) > -1) {
						ci_map.erase(kv.key);
					}
				}
				to_remove.clear();
			}

			godosu_window_callback(1, data.callback_draw);

			for (const auto &kv : ci_map) {
				kv.value->queue_redraw();
			}
		} break;
	}
}

void Godosu::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_main_script"), &Godosu::set_main_script);
	ClassDB::bind_method(D_METHOD("get_main_script"), &Godosu::get_main_script);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "main_script", PROPERTY_HINT_FILE, "*.rb"), "set_main_script", "get_main_script");
}

void Godosu::input(const Ref<InputEvent> &p_event) {
	if (p_event->is_echo()) {
		return;
	}

	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid()) {
		godosu_window_callback(2, mb->is_pressed() ? data.callback_button_down : data.callback_button_up, LONG2NUM(long(mb->get_button_index())));
		return;
	}

	Ref<InputEventKey> k = p_event;
	if (k.is_valid()) {
		godosu_window_callback(2, k->is_pressed() ? data.callback_button_down : data.callback_button_up, LONG2NUM(long(k->get_keycode())));
		return;
	}
}

void Godosu::set_main_script(const String &p_main) {
	main_script = p_main;
}

String Godosu::get_main_script() const {
	return main_script;
}

void Godosu::setup_window(VALUE p_window, const Vector2i &p_size, bool p_fullscreen) {
	data.window = p_window;
	set_process_internal(true);
	set_physics_process_internal(true);
	set_process_input(true);

	float window_scale = ProjectSettings::get_singleton()->get_setting("display/window/size/window_scale", 1.0);

	ProjectSettings::get_singleton()->set_setting("display/window/size/viewport_width", p_size.x);
	ProjectSettings::get_singleton()->set_setting("display/window/size/viewport_height", p_size.x);
	get_window()->set_size(p_size * window_scale);
	get_window()->set_content_scale_size(p_size);
	if (p_fullscreen) {
		get_window()->set_mode(Window::MODE_FULLSCREEN);
	} else {
		get_window()->move_to_center();
	}
}

CanvasItem *Godosu::get_ci(int p_z_index, const Ref<Material> &p_material, const Rect2 &p_clip_rect) {
	uint32_t key;
	{
		Array arr;
		arr.append(p_z_index);
		arr.append(p_material);
		arr.append(p_clip_rect.size);
		key = arr.hash();
	}

	auto I = ci_map.find(key);
	if (!I) {
		CanvasItem *parent = this;
		if (p_clip_rect.has_area()) {
			Control *clipper = memnew(Control);
			clipper->set_clip_contents(true);
			clipper->set_rect(p_clip_rect);
			add_child(clipper);
			parent = clipper;
		}

		Node2D *ci = memnew(Node2D);
		parent->add_child(ci);
		if (p_clip_rect.has_area()) {
			ci->set_meta(SNAME("clipped"), true);
			ci->set_global_transform(Transform2D());
		}

		ci->set_z_index(p_z_index);
		if (p_material.is_valid()) {
			ci->set_material(p_material);
		}
		ci->connect(SNAME("draw"), callable_mp(this, &Godosu::_draw_canvas_item).bind(ci));
		ci_map[key] = ci;
		return ci;
	} else if (p_clip_rect.has_area()) {
		Node2D *ci = Object::cast_to<Node2D>(I->value);
		Control *clipper = Object::cast_to<Control>(ci->get_parent_item());
		clipper->set_position(p_clip_rect.position);
		ci->set_global_transform(Transform2D());
	}
	return I->value;
}

void Godosu::add_to_queue(const DrawCommand &p_data, int p_z, const Ref<Material> &p_material) {
	Ref<Material> target_material;
	{
		auto I = data.shader_map.find(p_z);
		if (I) {
			target_material = I->value;
		} else {
			target_material = p_material;
		}
	}

	CanvasItem *ci = get_ci(p_z, target_material, data.clip_rect);
	draw_queue[ci].append(p_data);
}

VALUE Godosu::create_line_edit() {
	LineEdit *edit = memnew(LineEdit);
	edit->set_modulate(Color(0, 0, 0, 0));
	edit->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
	add_child(edit);
	data.text_inputs.append(edit);
	return INT2NUM(data.text_inputs.size() - 1);
}

LineEdit *Godosu::get_line_edit(VALUE id) {
	int index = FIX2INT(id);
	return data.text_inputs[index];
}

Godosu::Godosu() {
	singleton = this;
}

Godosu::~Godosu() {
	if (ruby_state == -1) {
		return;
	}

	if (ruby_state) {
		/* handle exception, perhaps */
	}

	ruby_cleanup(ruby_state);
}
