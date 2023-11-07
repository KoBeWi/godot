#include "godosu.h"
#include "godosu_functions.h"

#include "core/config/project_settings.h"
#include "core/io/file_access.h"
#include "scene/audio/audio_stream_player.h"
#include "scene/main/viewport.h"
#include "scene/resources/atlas_texture.h"
#include "scene/resources/texture.h"

Color convert_color(VALUE from) {
	Color c = Color::hex(rb_big2ll(from));
	Color ret;
	ret.r = c.g;
	ret.g = c.b;
	ret.b = c.a;
	ret.a = c.r;
	return ret;
}

VALUE gd_setup_window(VALUE self, VALUE window, VALUE width, VALUE height) {
	Godosu::singleton->setup_window(window);
	Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_HIDDEN);
	Vector2i size(FIX2INT(width), FIX2INT(height));
	DisplayServer::get_singleton()->window_set_size(size);
	return OK;
}

VALUE godot_retrofication(VALUE self) {
	Godosu::singleton->set_texture_filter(CanvasItem::TEXTURE_FILTER_NEAREST);
	return OK;
}

VALUE gd_load_image(VALUE self, VALUE instance, VALUE source) {
	String path = StringValueCStr(source);
	Ref<Texture2D> texture = ResourceLoader::load(path, "Texture2D");
	rb_funcall(instance, rb_intern("godot_set_size"), 2, INT2NUM(texture->get_width()), INT2NUM(texture->get_height()));

	Godosu::singleton->data.texture_cache[instance] = texture;
	return OK;
}

VALUE gd_load_atlas(VALUE self, VALUE instance, VALUE base, VALUE x, VALUE y, VALUE w, VALUE h) {
	Ref<Texture2D> texture = Godosu::singleton->data.texture_cache[base];
	Ref<AtlasTexture> atlas;
	atlas.instantiate();
	atlas->set_atlas(texture);
	atlas->set_region(Rect2(FIX2LONG(x), FIX2LONG(y), FIX2LONG(w), FIX2LONG(h)));
	Godosu::singleton->data.texture_cache[instance] = atlas;
	return OK;
}

VALUE gd_load_audio(VALUE self, VALUE instance, VALUE source) {
	String path = StringValueCStr(source);
	Ref<AudioStream> audio = ResourceLoader::load(path, "AudioStream");

	Godosu::singleton->data.audio_cache[instance] = audio;
	return OK;
}

VALUE gd_load_font(VALUE self, VALUE instance, VALUE source, VALUE size) {
	String path = StringValueCStr(source);
	// Ref<Font> font = ResourceLoader::load(path, "Font");
	Ref<SystemFont> font;
	font.instantiate();
	font->set_font_names({ "Monospace" }); // TODO: szukać pliku jak nie ma to fallback

	Godosu::singleton->data.font_cache[instance] = font;
	return OK;
}

VALUE gd_draw_rect(VALUE self, VALUE x, VALUE y, VALUE width, VALUE height, VALUE c, VALUE z, VALUE mode) {
	Godosu::DrawCommand draw_data;
	draw_data.type = Godosu::DrawCommand::DRAW_RECT;
	draw_data.arguments = varray(Rect2(RFLOAT_VALUE(x), RFLOAT_VALUE(y), RFLOAT_VALUE(width), RFLOAT_VALUE(height)));
	Godosu::singleton->add_to_queue(FIX2LONG(z), draw_data);
	return OK;
}

VALUE gd_draw_quad(VALUE self, VALUE x1, VALUE y1, VALUE c1, VALUE x2, VALUE y2, VALUE c2, VALUE x3, VALUE y3, VALUE c3, VALUE x4, VALUE y4, VALUE c4, VALUE z, VALUE mode) {
	Godosu::DrawCommand draw_data;
	draw_data.type = Godosu::DrawCommand::DRAW_QUAD;
	draw_data.arguments.append(Vector2(RFLOAT_VALUE(x1), RFLOAT_VALUE(y1)));
	draw_data.arguments.append(Vector2(RFLOAT_VALUE(x2), RFLOAT_VALUE(y2)));
	draw_data.arguments.append(Vector2(RFLOAT_VALUE(x3), RFLOAT_VALUE(y3)));
	draw_data.arguments.append(Vector2(RFLOAT_VALUE(x4), RFLOAT_VALUE(y4)));
	draw_data.arguments.append(convert_color(c1));
	draw_data.arguments.append(convert_color(c2));
	draw_data.arguments.append(convert_color(c3));
	draw_data.arguments.append(convert_color(c4));

	Godosu::singleton->add_to_queue(FIX2LONG(z), draw_data);
	return OK;
}

VALUE gd_draw_texture(VALUE self, VALUE texture, VALUE x, VALUE y, VALUE z, VALUE scale_x, VALUE scale_y, VALUE color) {
	Godosu::DrawCommand draw_data;
	draw_data.type = Godosu::DrawCommand::DRAW_TEXTURE;
	draw_data.arguments = varray(
			Godosu::singleton->data.texture_cache[texture],
			Vector2(RFLOAT_VALUE(x), RFLOAT_VALUE(y)),
			Vector2(RFLOAT_VALUE(scale_x), RFLOAT_VALUE(scale_y)),
			convert_color(color));

	Godosu::singleton->add_to_queue(FIX2LONG(z), draw_data);
	return OK;
}

VALUE gd_draw_texture_rotated(VALUE self, VALUE texture, VALUE x, VALUE y, VALUE z, VALUE angle, VALUE center_x, VALUE center_y, VALUE scale_x, VALUE scale_y, VALUE color) {
	Godosu::DrawCommand draw_data;
	draw_data.type = Godosu::DrawCommand::DRAW_TEXTURE_ROTATED;
	draw_data.arguments.append(Godosu::singleton->data.texture_cache[texture]);
	draw_data.arguments.append(Vector2(RFLOAT_VALUE(x), RFLOAT_VALUE(y)));
	draw_data.arguments.append(RFLOAT_VALUE(angle));
	draw_data.arguments.append(Vector2(RFLOAT_VALUE(center_x), RFLOAT_VALUE(center_y)));
	draw_data.arguments.append(Vector2(RFLOAT_VALUE(scale_x), RFLOAT_VALUE(scale_y)));
	draw_data.arguments.append(FIX2LONG(color));

	Godosu::singleton->add_to_queue(FIX2LONG(z), draw_data);
	return OK;
}

VALUE gd_draw_string(VALUE self, VALUE font, VALUE x, VALUE y, VALUE text, VALUE z) {
	Godosu::DrawCommand draw_data;
	draw_data.type = Godosu::DrawCommand::DRAW_STRING;
	draw_data.arguments = varray(
			Godosu::singleton->data.font_cache[font],
			Vector2(RFLOAT_VALUE(x), RFLOAT_VALUE(y)),
			StringValueCStr(text));

	Godosu::singleton->add_to_queue(FIX2LONG(z), draw_data);
	return OK;
}

VALUE gd_play_song(VALUE self, VALUE instance) {
	AudioStreamPlayer *song_player = Godosu::singleton->data.song_player;
	song_player->set_stream(Godosu::singleton->data.audio_cache[instance]);
	song_player->play();
	return OK;
}

VALUE gd_stop_song(VALUE self) {
	Godosu::singleton->data.song_player->stop();
	return OK;
}

VALUE gd_play_sample(VALUE self, VALUE instance) {
	AudioStreamPlayer *sample_player = Godosu::singleton->data.sample_player;
	sample_player->set_stream(Godosu::singleton->data.audio_cache[instance]);
	sample_player->play();
	return OK;
}

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
				p_item->draw_texture_rect(texture, Rect2(pos, texture->get_size() * draw_scale), false, draw_command.arguments[3]);
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
				PackedVector2Array points;
				points.append(draw_command.arguments[0]);
				points.append(draw_command.arguments[1]);
				points.append(draw_command.arguments[2]);
				points.append(draw_command.arguments[3]);

				PackedColorArray colors;
				colors.append(draw_command.arguments[4]);
				colors.append(draw_command.arguments[5]);
				colors.append(draw_command.arguments[6]);
				colors.append(draw_command.arguments[7]);

				p_item->draw_polygon(points, colors);
			} break;

			case DrawCommand::DRAW_STRING: {
				p_item->draw_string(draw_command.arguments[0], draw_command.arguments[1], draw_command.arguments[2]);
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

			data.song_player = memnew(AudioStreamPlayer);
			add_child(data.song_player);
			data.sample_player = memnew(AudioStreamPlayer);
			data.sample_player->set_max_polyphony(1024);
			add_child(data.sample_player);

			// Initialize Ruby.

#ifdef WINDOWS_ENABLED
			{
				int argc = 0;
				char *argv0 = "";
				char **argv1 = &argv0;
				rb_w32_sysinit(&argc, &argv1);
			}
#endif
			ruby_init();
			ruby_init_loadpath();

			data.callback_base = rb_intern("godot_callback");
			data.callback_update_mouse = rb_intern("godot_update_mouse");
			data.callback_update = ID2SYM(rb_intern("update"));
			data.callback_draw = ID2SYM(rb_intern("godot_draw"));
			data.callback_button_down = ID2SYM(rb_intern("godot_button_down"));
			data.callback_button_up = ID2SYM(rb_intern("godot_button_up"));

			rb_define_global_function("godot_setup_window", gd_setup_window, 3);
			rb_define_global_function("gd_print", gd_print, 1);
			rb_define_global_function("godot_retrofication", godot_retrofication, 0);
			rb_define_global_function("godot_load_image", gd_load_image, 2);
			rb_define_global_function("godot_load_atlas", gd_load_atlas, 6);
			rb_define_global_function("godot_load_audio", gd_load_audio, 2);
			rb_define_global_function("godot_load_font", gd_load_font, 3);

			rb_define_global_function("godot_draw_rect", gd_draw_rect, 7);
			rb_define_global_function("godot_draw_quad", gd_draw_quad, 14);
			rb_define_global_function("godot_draw_texture", gd_draw_texture, 7);
			rb_define_global_function("godot_draw_texture_rotated", gd_draw_texture_rotated, 10);
			rb_define_global_function("godot_draw_string", gd_draw_string, 5);
			rb_define_global_function("godot_play_song", gd_play_song, 1);
			rb_define_global_function("godot_stop_song", gd_stop_song, 0);
			rb_define_global_function("godot_play_sample", gd_play_sample, 1);

			// Input constants.

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

			// Run.

			const String main_script_path = ProjectSettings::get_singleton()->globalize_path(main_script);
			const String exec = vformat(R"(-e require "%s" )", main_script_path);

			char *options[] = { "-v", const_cast<char *>(exec.ascii().get_data()) };
			void *node = ruby_options(2, options);

			if (ruby_executable_node(node, &ruby_state)) {
				ruby_state = ruby_exec_node(node);
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

void Godosu::setup_window(VALUE p_window) {
	data.window = p_window;
	set_process_internal(true);
	set_physics_process_internal(true);
	set_process_input(true);
}

CanvasItem *Godosu::get_ci(int p_z_index) {
	if (!ci_map.has(p_z_index)) {
		CanvasItem *ci = memnew(Node2D);
		add_child(ci);
		RenderingServer::get_singleton()->canvas_item_set_z_index(ci->get_canvas_item(), p_z_index);
		ci->connect(SNAME("draw"), callable_mp(this, &Godosu::_draw_canvas_item).bind(ci));
		ci_map[p_z_index] = ci;
	}
	return ci_map[p_z_index];
}

void Godosu::add_to_queue(int p_z, const DrawCommand &p_data) {
	CanvasItem *ci = get_ci(p_z);
	draw_queue[ci].append(p_data);
	// p_item->queue_redraw();
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
