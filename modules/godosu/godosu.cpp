#include "godosu.h"
#include "godosu_functions.h"

#include "core/config/project_settings.h"
#include "core/io/file_access.h"
#include "scene/audio/audio_stream_player.h"
#include "scene/main/window.h"
#include "scene/resources/texture.h"

void Godosu::_draw_canvas_item(CanvasItem *p_item) {
	if (!draw_queue.has(p_item)) {
		return; // TODO: czyścić nieużywane
	}

	const Vector<DrawCommand> &draw_data = draw_queue[p_item];

	for (const DrawCommand &draw_command : draw_data) {
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
				const Ref<Texture2D> &texture = draw_command.arguments[0];
				const Vector2 &pos = draw_command.arguments[1];
				const float angle = draw_command.arguments[2];
				const Vector2 &center = draw_command.arguments[3];
				const Vector2 &draw_scale = draw_command.arguments[4];
				const Color &color = draw_command.arguments[5];
				p_item->draw_set_transform(pos, angle, Vector2(1, 1));
				p_item->draw_texture_rect(texture, Rect2(-texture->get_size() * center, texture->get_size() * draw_scale), false, color);
				p_item->draw_set_transform_matrix(Transform2D());
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
			DEFINE_FUNCTION(setup_window, 3);
			DEFINE_FUNCTION(retrofication, 0);
			DEFINE_FUNCTION(set_clip, 4);
			DEFINE_FUNCTION(hsv_to_rgb, 3);
			DEFINE_FUNCTION(button_id_to_char, 1);

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
			DEFINE_FUNCTION(play_sample, 1);

			DEFINE_FUNCTION(create_shader, 2);
			DEFINE_FUNCTION(set_shader, 2);

			// Input constants.

			print_verbose("Defining constants");
			VALUE GosuModule = rb_define_module("Gosu");
			rb_define_const(GosuModule, "KbLeft", LONG2NUM(int(Key::LEFT)));
			rb_define_const(GosuModule, "KbRight", LONG2NUM(int(Key::RIGHT)));
			rb_define_const(GosuModule, "KbUp", LONG2NUM(int(Key::UP)));
			rb_define_const(GosuModule, "KbDown", LONG2NUM(int(Key::DOWN)));
			rb_define_const(GosuModule, "KbSpace", LONG2NUM(int(Key::SPACE)));
			rb_define_const(GosuModule, "KbEscape", LONG2NUM(int(Key::ESCAPE)));
			rb_define_const(GosuModule, "KbReturn", LONG2NUM(int(Key::ENTER)));

			rb_define_const(GosuModule, "KbF1", LONG2NUM(int(Key::F1)));

			rb_define_const(GosuModule, "MsLeft", LONG2NUM(int(MouseButton::LEFT)));
			rb_define_const(GosuModule, "MsMiddle", LONG2NUM(int(MouseButton::MIDDLE)));
			rb_define_const(GosuModule, "MsRight", LONG2NUM(int(MouseButton::RIGHT)));

#undef DEFINE_FUNCTION

			// Run.

			print_verbose("Configuring exec");
#ifdef TOOLS_ENABLED
			const String main_script_path = ProjectSettings::get_singleton()->globalize_path(main_script);
#else
			const String main_script_path = OS::get_singleton()->get_executable_path().get_base_dir().path_join(ProjectSettings::get_singleton()->globalize_path(main_script));
#endif
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

void Godosu::setup_window(VALUE p_window, const Vector2i &p_size) {
	data.window = p_window;
	set_process_internal(true);
	set_physics_process_internal(true);
	set_process_input(true);

	get_window()->set_size(p_size * GLOBAL_GET("display/window/stretch/scale").operator real_t());
	get_window()->move_to_center();
}

CanvasItem *Godosu::get_ci(int p_z_index, const Ref<Material> &p_material, const Rect2 &p_clip_rect) {
	uint32_t key;
	{
		Array arr;
		arr.append(p_z_index);
		arr.append(p_material);
		arr.append(p_clip_rect);
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

		CanvasItem *ci = memnew(Node2D);
		parent->add_child(ci);
		RenderingServer::get_singleton()->canvas_item_set_z_index(ci->get_canvas_item(), p_z_index);
		if (p_material.is_valid()) {
			RenderingServer::get_singleton()->canvas_item_set_material(ci->get_canvas_item(), p_material->get_rid());
		}
		ci->connect(SNAME("draw"), callable_mp(this, &Godosu::_draw_canvas_item).bind(ci));
		ci_map[key] = ci;
		return ci;
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
