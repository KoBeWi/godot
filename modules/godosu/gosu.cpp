#include "gosu.h"

#include "core/object/class_db.h"
#include "scene/main/window.h"
#include "scene/resources/texture.h"

static Gosu *global_gosu;

mrb_value f_print(mrb_state *mrb, mrb_value self) {
	char *string;
	mrb_get_args(mrb, "z", &string);
	print_line(string);
	return mrb_nil_value();
}

mrb_value f_set_window_title(mrb_state *mrb, mrb_value self) {
	char *title;
	mrb_get_args(mrb, "z", &title);
	((Window *)global_gosu->get_viewport())->set_title(title);
	return mrb_nil_value();
}

mrb_value f_draw_rect(mrb_state *mrb, mrb_value self) {
	mrb_float x;
	mrb_float y;
	mrb_float w;
	mrb_float h;
	char *color;
	mrb_get_args(mrb, "ffffz", &x, &y, &w, &h, &color);
	global_gosu->draw_rect(Rect2(x, y, w, h), Color(color));
	return mrb_nil_value();
}

mrb_value f_draw_texture(mrb_state *mrb, mrb_value self) {
	char *texture;
	mrb_float x;
	mrb_float y;
	mrb_int z;
	mrb_float scale_x;
	mrb_float scale_y;
	char *color;
	char *mode;
	mrb_get_args(mrb, "zffiffzz", &texture, &x, &y, &z, &scale_x, &scale_y, &color, &mode);
	global_gosu->draw_texture(global_gosu->image_cache[texture], Vector2(x, y), Color(color));
	return mrb_nil_value();
}

mrb_value f_load_texture(mrb_state *mrb, mrb_value self) {
	char *path;
	mrb_get_args(mrb, "z", &path);

    Ref<Image> image;
    image.instantiate();
    image->load(global_gosu->get_main_file_path().get_base_dir().plus_file(path));
    global_gosu->image_cache[path] = ImageTexture::create_from_image(image);

	return mrb_nil_value();
}

String Gosu::get_file_source(const String &p_path, const String &p_base) {
	Ref<FileAccess> f = FileAccess::open(p_base.is_empty() ? p_path : p_base.plus_file(p_path), FileAccess::READ);
	PackedStringArray lines = f->get_as_utf8_string().split("\n");
	for (int i = 0; i < lines.size(); i++) {
		const String &line = lines[i];
		if (line.begins_with("require_relative")) {
			int start = line.find("\"");
			int end = line.find("\"", start + 1);

			String next_file = line.substr(start + 1, end - start - 1);
			lines.write[i] = get_file_source(p_base + next_file, p_base.is_empty() ? p_path.get_base_dir() : p_base);
		}
	}
	return String("\n").join(lines);
}

void Gosu::_notification(int p_what) {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	switch (p_what) {
		case NOTIFICATION_READY: {
			String source = get_file_source(main_file_path);
			main = mrb_load_string(mrb, source.utf8().get_data());
			set_process_internal(true);
			set_process_input(true);
		} break;

		case NOTIFICATION_INTERNAL_PROCESS: {
			mrb_funcall(mrb, main, "update", 0);
			update();

			if (mrb->exc) {
				mrb_print_error(mrb);
				mrb_print_backtrace(mrb);
				queue_delete();
			}
		} break;

		case NOTIFICATION_DRAW: {
			mrb_funcall(mrb, main, "draw", 0);
		} break;
	}
}

void Gosu::input(const Ref<InputEvent> &p_event) {
	Ref<InputEventKey> iek = p_event;
	if (iek.is_valid()) {
		if (iek->is_pressed()) {
			mrb_funcall(mrb, main, "button_down", 1, mrb_int_value(mrb, int(iek->get_keycode())));
		} else {
			mrb_funcall(mrb, main, "button_up", 1, mrb_int_value(mrb, int(iek->get_keycode())));
		}
		return;
	}

	Ref<InputEventMouseButton> iemb = p_event;
	if (iemb.is_valid()) {
		if (iemb->is_pressed()) {
			mrb_funcall(mrb, main, "button_down", 1, mrb_int_value(mrb, int(iemb->get_button_index())));
		} else {
			mrb_funcall(mrb, main, "button_up", 1, mrb_int_value(mrb, int(iemb->get_button_index())));
		}
		return;
	}
}

void Gosu::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_main_file_path"), &Gosu::set_main_file_path);
	ClassDB::bind_method(D_METHOD("get_main_file_path"), &Gosu::get_main_file_path);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "main_file_path"), "set_main_file_path", "get_main_file_path");
}

Gosu::Gosu() {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	global_gosu = this;
	mrb = mrb_open();

	mrb_define_method(mrb, mrb->object_class, "gd_print", f_print, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, mrb->object_class, "gd_set_window_title", f_set_window_title, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, mrb->object_class, "gd_draw_rect", f_draw_rect, MRB_ARGS_REQ(5));
	mrb_define_method(mrb, mrb->object_class, "gd_draw_texture", f_draw_texture, MRB_ARGS_REQ(8));
	mrb_define_method(mrb, mrb->object_class, "gd_load_texture", f_load_texture, MRB_ARGS_REQ(1));

	mrb_define_global_const(mrb, "KB_0", mrb_int_value(mrb, int(Key::KEY_0)));
	mrb_define_global_const(mrb, "KB_1", mrb_int_value(mrb, int(Key::KEY_1)));
	mrb_define_global_const(mrb, "KB_2", mrb_int_value(mrb, int(Key::KEY_2)));
	mrb_define_global_const(mrb, "KB_3", mrb_int_value(mrb, int(Key::KEY_3)));
	mrb_define_global_const(mrb, "KB_4", mrb_int_value(mrb, int(Key::KEY_4)));
	mrb_define_global_const(mrb, "KB_5", mrb_int_value(mrb, int(Key::KEY_5)));
	mrb_define_global_const(mrb, "KB_6", mrb_int_value(mrb, int(Key::KEY_6)));
	mrb_define_global_const(mrb, "KB_7", mrb_int_value(mrb, int(Key::KEY_7)));
	mrb_define_global_const(mrb, "KB_8", mrb_int_value(mrb, int(Key::KEY_8)));
	mrb_define_global_const(mrb, "KB_9", mrb_int_value(mrb, int(Key::KEY_9)));
	mrb_define_global_const(mrb, "KB_A", mrb_int_value(mrb, int(Key::A)));
	mrb_define_global_const(mrb, "KB_B", mrb_int_value(mrb, int(Key::B)));
	mrb_define_global_const(mrb, "KB_C", mrb_int_value(mrb, int(Key::C)));
	mrb_define_global_const(mrb, "KB_D", mrb_int_value(mrb, int(Key::D)));
	mrb_define_global_const(mrb, "KB_E", mrb_int_value(mrb, int(Key::E)));
	mrb_define_global_const(mrb, "KB_F", mrb_int_value(mrb, int(Key::F)));
	mrb_define_global_const(mrb, "KB_G", mrb_int_value(mrb, int(Key::G)));
	mrb_define_global_const(mrb, "KB_H", mrb_int_value(mrb, int(Key::H)));
	mrb_define_global_const(mrb, "KB_I", mrb_int_value(mrb, int(Key::I)));
	mrb_define_global_const(mrb, "KB_J", mrb_int_value(mrb, int(Key::J)));
	mrb_define_global_const(mrb, "KB_K", mrb_int_value(mrb, int(Key::K)));
	mrb_define_global_const(mrb, "KB_L", mrb_int_value(mrb, int(Key::L)));
	mrb_define_global_const(mrb, "KB_M", mrb_int_value(mrb, int(Key::M)));
	mrb_define_global_const(mrb, "KB_N", mrb_int_value(mrb, int(Key::N)));
	mrb_define_global_const(mrb, "KB_O", mrb_int_value(mrb, int(Key::O)));
	mrb_define_global_const(mrb, "KB_P", mrb_int_value(mrb, int(Key::P)));
	mrb_define_global_const(mrb, "KB_Q", mrb_int_value(mrb, int(Key::Q)));
	mrb_define_global_const(mrb, "KB_R", mrb_int_value(mrb, int(Key::R)));
	mrb_define_global_const(mrb, "KB_S", mrb_int_value(mrb, int(Key::S)));
	mrb_define_global_const(mrb, "KB_T", mrb_int_value(mrb, int(Key::T)));
	mrb_define_global_const(mrb, "KB_U", mrb_int_value(mrb, int(Key::U)));
	mrb_define_global_const(mrb, "KB_V", mrb_int_value(mrb, int(Key::V)));
	mrb_define_global_const(mrb, "KB_W", mrb_int_value(mrb, int(Key::W)));
	mrb_define_global_const(mrb, "KB_X", mrb_int_value(mrb, int(Key::X)));
	mrb_define_global_const(mrb, "KB_Y", mrb_int_value(mrb, int(Key::Y)));
	mrb_define_global_const(mrb, "KB_Z", mrb_int_value(mrb, int(Key::Z)));
	mrb_define_global_const(mrb, "KB_APOSTROPHE", mrb_int_value(mrb, int(Key::APOSTROPHE)));
	mrb_define_global_const(mrb, "KB_BACKSLASH", mrb_int_value(mrb, int(Key::BACKSLASH)));
	mrb_define_global_const(mrb, "KB_BACKSPACE", mrb_int_value(mrb, int(Key::BACKSPACE)));
	// mrb_define_global_const(mrb, "KB_BACKTICK", mrb_int_value(mrb, int(Key::)));
	mrb_define_global_const(mrb, "KB_COMMA", mrb_int_value(mrb, int(Key::COMMA)));
	mrb_define_global_const(mrb, "KB_DELETE", mrb_int_value(mrb, int(Key::KEY_DELETE)));
	mrb_define_global_const(mrb, "KB_DOWN", mrb_int_value(mrb, int(Key::DOWN)));
	mrb_define_global_const(mrb, "KB_END", mrb_int_value(mrb, int(Key::END)));
	// mrb_define_global_const(mrb, "KB_PRINT_SCREEN", mrb_int_value(mrb, int(Key::)));
	mrb_define_global_const(mrb, "KB_SCROLL_LOCK", mrb_int_value(mrb, int(Key::SCROLLLOCK)));
	mrb_define_global_const(mrb, "KB_PAUSE", mrb_int_value(mrb, int(Key::PAUSE)));
	mrb_define_global_const(mrb, "KB_ENTER", mrb_int_value(mrb, int(Key::KP_ENTER)));
	mrb_define_global_const(mrb, "KB_EQUALS", mrb_int_value(mrb, int(Key::EQUAL)));
	mrb_define_global_const(mrb, "KB_ESCAPE", mrb_int_value(mrb, int(Key::ESCAPE)));
	mrb_define_global_const(mrb, "KB_F1", mrb_int_value(mrb, int(Key::F1)));
	mrb_define_global_const(mrb, "KB_F2", mrb_int_value(mrb, int(Key::F2)));
	mrb_define_global_const(mrb, "KB_F3", mrb_int_value(mrb, int(Key::F3)));
	mrb_define_global_const(mrb, "KB_F4", mrb_int_value(mrb, int(Key::F4)));
	mrb_define_global_const(mrb, "KB_F5", mrb_int_value(mrb, int(Key::F5)));
	mrb_define_global_const(mrb, "KB_F6", mrb_int_value(mrb, int(Key::F6)));
	mrb_define_global_const(mrb, "KB_F7", mrb_int_value(mrb, int(Key::F7)));
	mrb_define_global_const(mrb, "KB_F8", mrb_int_value(mrb, int(Key::F8)));
	mrb_define_global_const(mrb, "KB_F9", mrb_int_value(mrb, int(Key::F9)));
	mrb_define_global_const(mrb, "KB_F10", mrb_int_value(mrb, int(Key::F10)));
	mrb_define_global_const(mrb, "KB_F11", mrb_int_value(mrb, int(Key::F11)));
	mrb_define_global_const(mrb, "KB_F12", mrb_int_value(mrb, int(Key::F12)));
	mrb_define_global_const(mrb, "KB_HOME", mrb_int_value(mrb, int(Key::HOME)));
	mrb_define_global_const(mrb, "KB_INSERT", mrb_int_value(mrb, int(Key::INSERT)));
	// mrb_define_global_const(mrb, "KB_ISO", mrb_int_value(mrb, int(Key::)));
	mrb_define_global_const(mrb, "KB_LEFT", mrb_int_value(mrb, int(Key::LEFT)));
	mrb_define_global_const(mrb, "KB_LEFT_ALT", mrb_int_value(mrb, int(Key::ALT)));
	mrb_define_global_const(mrb, "KB_LEFT_BRACKET", mrb_int_value(mrb, int(Key::BRACKETLEFT)));
	mrb_define_global_const(mrb, "KB_LEFT_CONTROL", mrb_int_value(mrb, int(Key::CTRL)));
	mrb_define_global_const(mrb, "KB_LEFT_META", mrb_int_value(mrb, int(Key::META)));
	mrb_define_global_const(mrb, "KB_LEFT_SHIFT", mrb_int_value(mrb, int(Key::SHIFT)));
	mrb_define_global_const(mrb, "KB_MINUS", mrb_int_value(mrb, int(Key::MINUS)));
	mrb_define_global_const(mrb, "KB_NUMPAD_0", mrb_int_value(mrb, int(Key::KP_0)));
	mrb_define_global_const(mrb, "KB_NUMPAD_1", mrb_int_value(mrb, int(Key::KP_1)));
	mrb_define_global_const(mrb, "KB_NUMPAD_2", mrb_int_value(mrb, int(Key::KP_2)));
	mrb_define_global_const(mrb, "KB_NUMPAD_3", mrb_int_value(mrb, int(Key::KP_3)));
	mrb_define_global_const(mrb, "KB_NUMPAD_4", mrb_int_value(mrb, int(Key::KP_4)));
	mrb_define_global_const(mrb, "KB_NUMPAD_5", mrb_int_value(mrb, int(Key::KP_5)));
	mrb_define_global_const(mrb, "KB_NUMPAD_6", mrb_int_value(mrb, int(Key::KP_6)));
	mrb_define_global_const(mrb, "KB_NUMPAD_7", mrb_int_value(mrb, int(Key::KP_7)));
	mrb_define_global_const(mrb, "KB_NUMPAD_8", mrb_int_value(mrb, int(Key::KP_8)));
	mrb_define_global_const(mrb, "KB_NUMPAD_9", mrb_int_value(mrb, int(Key::KP_9)));
	mrb_define_global_const(mrb, "KB_NUMPAD_DELETE", mrb_int_value(mrb, int(Key::KP_PERIOD)));
	mrb_define_global_const(mrb, "KB_NUMPAD_DIVIDE", mrb_int_value(mrb, int(Key::KP_DIVIDE)));
	mrb_define_global_const(mrb, "KB_NUMPAD_MINUS", mrb_int_value(mrb, int(Key::KP_SUBTRACT)));
	mrb_define_global_const(mrb, "KB_NUMPAD_MULTIPLY", mrb_int_value(mrb, int(Key::KP_MULTIPLY)));
	mrb_define_global_const(mrb, "KB_NUMPAD_PLUS", mrb_int_value(mrb, int(Key::KP_ADD)));
	mrb_define_global_const(mrb, "KB_PAGE_DOWN", mrb_int_value(mrb, int(Key::PAGEDOWN)));
	mrb_define_global_const(mrb, "KB_PAGE_UP", mrb_int_value(mrb, int(Key::PAGEUP)));
	mrb_define_global_const(mrb, "KB_PERIOD", mrb_int_value(mrb, int(Key::PERIOD)));
	mrb_define_global_const(mrb, "KB_RETURN", mrb_int_value(mrb, int(Key::ENTER)));
	mrb_define_global_const(mrb, "KB_RIGHT", mrb_int_value(mrb, int(Key::RIGHT)));
	mrb_define_global_const(mrb, "KB_RIGHT_ALT", mrb_int_value(mrb, int(Key::ALT)));
	mrb_define_global_const(mrb, "KB_RIGHT_BRACKET", mrb_int_value(mrb, int(Key::BRACKETRIGHT)));
	mrb_define_global_const(mrb, "KB_RIGHT_CONTROL", mrb_int_value(mrb, int(Key::CTRL)));
	mrb_define_global_const(mrb, "KB_RIGHT_META", mrb_int_value(mrb, int(Key::META)));
	mrb_define_global_const(mrb, "KB_RIGHT_SHIFT", mrb_int_value(mrb, int(Key::SHIFT)));
	mrb_define_global_const(mrb, "KB_SEMICOLON", mrb_int_value(mrb, int(Key::SEMICOLON)));
	mrb_define_global_const(mrb, "KB_SLASH", mrb_int_value(mrb, int(Key::SLASH)));
	mrb_define_global_const(mrb, "KB_CAPS_LOCK", mrb_int_value(mrb, int(Key::CAPSLOCK)));
	mrb_define_global_const(mrb, "KB_SPACE", mrb_int_value(mrb, int(Key::SPACE)));
	mrb_define_global_const(mrb, "KB_TAB", mrb_int_value(mrb, int(Key::TAB)));
	mrb_define_global_const(mrb, "KB_UP", mrb_int_value(mrb, int(Key::UP)));
	mrb_define_global_const(mrb, "MS_LEFT", mrb_int_value(mrb, int(MouseButton::LEFT)));
	mrb_define_global_const(mrb, "MS_MIDDLE", mrb_int_value(mrb, int(MouseButton::MIDDLE)));
	mrb_define_global_const(mrb, "MS_RIGHT", mrb_int_value(mrb, int(MouseButton::RIGHT)));
	mrb_define_global_const(mrb, "MS_WHEEL_DOWN", mrb_int_value(mrb, int(MouseButton::WHEEL_DOWN)));
	mrb_define_global_const(mrb, "MS_WHEEL_UP", mrb_int_value(mrb, int(MouseButton::WHEEL_UP)));
}

Gosu::~Gosu() {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	mrb_close(mrb);
}