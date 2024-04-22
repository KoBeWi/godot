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
				p_item->draw_rect(draw_command.arguments[0], Color(1, 1, 1), true);
			} break;

			case DrawCommand::DRAW_TEXTURE: {
				Ref<Texture2D> texture = draw_command.arguments[0];
				Vector2 pos = draw_command.arguments[1];
				Vector2 draw_scale = draw_command.arguments[2];
				p_item->draw_texture_rect(texture, Rect2(pos + texture->get_size() * draw_scale.min(Vector2()), texture->get_size() * draw_scale), false, draw_command.arguments[3]);
			} break;

			case DrawCommand::DRAW_TEXTURE_ROTATED: {
				Ref<Texture2D> texture = draw_command.arguments[0];
				Vector2 pos = draw_command.arguments[1];
				float angle = draw_command.arguments[2];
				Vector2 center = draw_command.arguments[3];
				Vector2 draw_scale = draw_command.arguments[4];
				p_item->draw_set_transform(pos, angle, Vector2(1, 1));
				p_item->draw_texture_rect(texture, Rect2(-texture->get_size() * center * draw_scale, texture->get_size() * draw_scale), false, draw_command.arguments[5]);
				p_item->draw_set_transform_matrix(Transform2D());
			} break;

			case DrawCommand::DRAW_QUAD: {
				p_item->draw_polygon(draw_command.arguments[0], draw_command.arguments[1]);
			} break;

			case DrawCommand::DRAW_STRING: {
				const Vector2 &rel = draw_command.arguments[6];

				if (rel.is_zero_approx()) {
					p_item->draw_string(draw_command.arguments[0], draw_command.arguments[1], draw_command.arguments[2], HORIZONTAL_ALIGNMENT_LEFT, -1, draw_command.arguments[3], draw_command.arguments[4]);
				} else {
					// FIXME: niedokładne to
					const Ref<Font> &font = draw_command.arguments[0];
					const String &text = draw_command.arguments[2];
					int text_size = draw_command.arguments[3];

					const Vector2 size(text.size() * 10 * (text_size / 16.0), font->get_height(text_size));
					Vector2 pos = draw_command.arguments[1];
					pos -= size * rel;

					p_item->draw_string(font, pos, text, HORIZONTAL_ALIGNMENT_LEFT, -1, text_size, draw_command.arguments[4]);
				}
			} break;
		}
	}
}

void Godosu::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			print_verbose("Starting...");
			if (Engine::get_singleton()->is_editor_hint()) {
				return;
			}

			data.song_player = memnew(AudioStreamPlayer);
			add_child(data.song_player);

			{
				Ref<CanvasItemMaterial> mat;
				mat.instantiate();
				mat->set_blend_mode(CanvasItemMaterial::BLEND_MODE_ADD);
				data.additive_material = mat;
			}

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
			DEFINE_FUNCTION(setup_window, 3);
			DEFINE_FUNCTION(retrofication, 0);
			DEFINE_FUNCTION(hsv_to_rgb, 3);

			DEFINE_FUNCTION(load_image, 2);
			DEFINE_FUNCTION(load_atlas, 6);
			DEFINE_FUNCTION(load_audio, 2);
			DEFINE_FUNCTION(load_font, 2);

			DEFINE_FUNCTION(draw_rect, 7);
			DEFINE_FUNCTION(draw_quad, 14);
			DEFINE_FUNCTION(draw_texture, 8);
			DEFINE_FUNCTION(draw_texture_rotated, 10);
			DEFINE_FUNCTION(draw_string, 11);

			DEFINE_FUNCTION(play_song, 1);
			DEFINE_FUNCTION(stop_song, 0);
			DEFINE_FUNCTION(play_sample, 1);

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

CanvasItem *Godosu::get_ci(int p_z_index, const Ref<Material> &p_material) {
	Pair<int, Ref<Material>> key(p_z_index, p_material);
	auto I = ci_map.find(key);
	if (!I) {
		CanvasItem *ci = memnew(Node2D);
		add_child(ci);
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

void Godosu::add_to_queue(int p_z, const DrawCommand &p_data) {
	add_to_queue(p_z, Ref<Material>(), p_data);
}

void Godosu::add_to_queue(int p_z, const Ref<Material> &p_material, const DrawCommand &p_data) {
	CanvasItem *ci = get_ci(p_z, p_material);
	draw_queue[ci].append(p_data);
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
