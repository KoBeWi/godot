#ifndef GODOSU_FUNCTIONS_H
#define GODOSU_FUNCTIONS_H

#include "godosu.h"

#include "core/config/project_settings.h"
#include "core/os/keyboard.h"
#include "scene/audio/audio_stream_player.h"
#include "scene/gui/line_edit.h"
#include "scene/resources/atlas_texture.h"

Color gd_convert_color(VALUE from) {
	const String color_string = StringValueCStr(from);
	const Color c = Color::html(color_string);
	Color ret;
	ret.r = c.g;
	ret.g = c.b;
	ret.b = c.a;
	ret.a = c.r;
	return ret;
}

Ref<Material> gd_is_additive(VALUE additive) {
	return RTEST(additive) ? Godosu::singleton->data.additive_material : Ref<Material>();
}

VALUE godosu_print(VALUE self, VALUE string) {
	String print_string = StringValueCStr(string);
	print_line(print_string);
	return OK;
}

VALUE godosu_exit(VALUE self) {
	Godosu::singleton->get_tree()->quit();
	return OK;
}

VALUE godosu_retrofication(VALUE self) {
	Godosu::singleton->set_texture_filter(CanvasItem::TEXTURE_FILTER_NEAREST);
	return OK;
}

VALUE godosu_set_clip(VALUE self, VALUE x, VALUE y, VALUE w, VALUE h) {
	Godosu::singleton->data.clip_rect = Rect2(RFLOAT_VALUE(x), RFLOAT_VALUE(y), RFLOAT_VALUE(w), RFLOAT_VALUE(h));
	return OK;
}

VALUE godosu_hsv_to_rgb(VALUE self, VALUE h, VALUE s, VALUE v) {
	const Color c = Color::from_hsv(RFLOAT_VALUE(h), RFLOAT_VALUE(s), RFLOAT_VALUE(v));
	VALUE *components = (VALUE *)alloca(sizeof(VALUE) * 3);
	components[0] = INT2NUM(int(c.r * 255));
	components[1] = INT2NUM(int(c.g * 255));
	components[2] = INT2NUM(int(c.b * 255));
	VALUE array = rb_ary_new4(3, components);
	return array;
}

VALUE godosu_button_id_to_char(VALUE self, VALUE id) {
	Key keycode = Key(FIX2INT(id));
	const String keycode_name = keycode_get_string(keycode);
	return rb_str_new_cstr(keycode_name.ascii().get_data());
}

VALUE godosu_setup_window(VALUE self, VALUE window, VALUE width, VALUE height, VALUE fullscreen) {
	Vector2i size(FIX2INT(width), FIX2INT(height));
	Godosu::singleton->setup_window(window, size, RTEST(fullscreen));
	Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_HIDDEN);
	return OK;
}

VALUE godosu_set_window_title(VALUE self, VALUE title) {
	Godosu::singleton->get_window()->set_title(StringValueCStr(title));
	return OK;
}

VALUE godosu_set_window_fullscreen(VALUE self, VALUE fullscreen) {
	if (RTEST(fullscreen)) {
		Godosu::singleton->get_window()->set_mode(Window::MODE_FULLSCREEN);
	} else {
		Godosu::singleton->get_window()->set_mode(Window::MODE_WINDOWED);
		Godosu::singleton->get_window()->move_to_center();
	}
	return OK;
}

VALUE godosu_create_text_input(VALUE self) {
	return Godosu::singleton->create_line_edit();
}

VALUE godosu_destroy_text_input(VALUE self, VALUE id) {
	LineEdit *edit = Godosu::singleton->get_line_edit(id);
	memdelete(edit);
	return OK;
}

VALUE godosu_focus_text_input(VALUE self, VALUE id) {
	LineEdit *edit = Godosu::singleton->get_line_edit(id);
	edit->grab_focus();
	return OK;
}

VALUE godosu_unfocus_text_input(VALUE self) {
	Godosu::singleton->get_viewport()->gui_release_focus();
	return OK;
}

VALUE godosu_set_text_input_text(VALUE self, VALUE id, VALUE text) {
	const String string = StringValueCStr(text);
	LineEdit *edit = Godosu::singleton->get_line_edit(id);
	edit->set_text(string);
	return OK;
}

VALUE godosu_get_text_input_text(VALUE self, VALUE id) {
	LineEdit *edit = Godosu::singleton->get_line_edit(id);
	return rb_str_new_cstr(edit->get_text().ascii().get_data());
}

VALUE godosu_set_text_input_caret(VALUE self, VALUE id, VALUE caret) {
	const int caret_pos = FIX2INT(caret);
	LineEdit *edit = Godosu::singleton->get_line_edit(id);
	edit->set_caret_column(caret_pos);
	return OK;
}

VALUE godosu_get_text_input_caret(VALUE self, VALUE id) {
	LineEdit *edit = Godosu::singleton->get_line_edit(id);
	return INT2NUM(edit->get_caret_column());
}

VALUE godosu_set_text_input_selection_start(VALUE self, VALUE id, VALUE start) {
	const int start_pos = FIX2INT(start);
	LineEdit *edit = Godosu::singleton->get_line_edit(id);
	edit->select(start, edit->get_caret_column());
	return OK;
}

VALUE godosu_get_text_input_selection_start(VALUE self, VALUE id) {
	LineEdit *edit = Godosu::singleton->get_line_edit(id);
	if (edit->has_selection()) {
		return INT2NUM(edit->get_selection_from_column());
	} else {
		return INT2NUM(edit->get_caret_column());
	}
}

VALUE godosu_load_image(VALUE self, VALUE instance, VALUE source) {
	String path = StringValueCStr(source);
	Ref<Texture2D> texture = ResourceLoader::load("res://" + path, "Texture2D");
	rb_funcall(instance, rb_intern("godot_set_size"), 2, INT2NUM(texture->get_width()), INT2NUM(texture->get_height()));

	Godosu::singleton->data.texture_cache[instance] = texture;
	return OK;
}

VALUE godosu_load_atlas(VALUE self, VALUE instance, VALUE base, VALUE x, VALUE y, VALUE w, VALUE h) {
	Ref<Texture2D> texture = Godosu::singleton->data.texture_cache[base];
	Ref<AtlasTexture> atlas;
	atlas.instantiate();
	atlas->set_atlas(texture);
	atlas->set_region(Rect2(FIX2LONG(x), FIX2LONG(y), FIX2LONG(w), FIX2LONG(h)));
	Godosu::singleton->data.texture_cache[instance] = atlas;
	return OK;
}

VALUE godosu_load_audio(VALUE self, VALUE instance, VALUE source) {
	String path = StringValueCStr(source);
	Ref<AudioStream> audio = ResourceLoader::load("res://" + path, "AudioStream");

	Godosu::singleton->data.audio_cache[instance] = audio;
	return OK;
}

VALUE godosu_load_font(VALUE self, VALUE instance, VALUE source) {
	String path = StringValueCStr(source);
	// Ref<Font> font = ResourceLoader::load(path, "Font");
	Ref<SystemFont> font;
	font.instantiate();
	font->set_font_names({ "Monospace" }); // TODO: szukać pliku jak nie ma to fallback

	Godosu::singleton->data.font_cache[instance] = font;
	return OK;
}

VALUE godosu_draw_line(VALUE self, VALUE x1, VALUE y1, VALUE c1, VALUE x2, VALUE y2, VALUE c2, VALUE z, VALUE additive) {
	Godosu::DrawCommand draw_data;
	draw_data.type = Godosu::DrawCommand::DRAW_LINE;
	draw_data.arguments = varray(
			Vector2(RFLOAT_VALUE(x1), RFLOAT_VALUE(y1)),
			Vector2(RFLOAT_VALUE(x2), RFLOAT_VALUE(y2)),
			gd_convert_color(c1),
			gd_convert_color(c2));

	Godosu::singleton->add_to_queue(draw_data, FIX2LONG(z), gd_is_additive(additive));
	return OK;
}

VALUE godosu_draw_quad(VALUE self, VALUE x1, VALUE y1, VALUE c1, VALUE x2, VALUE y2, VALUE c2, VALUE x3, VALUE y3, VALUE c3, VALUE x4, VALUE y4, VALUE c4, VALUE z, VALUE additive) {
	Godosu::DrawCommand draw_data;
	draw_data.type = Godosu::DrawCommand::DRAW_POLYGON;
	draw_data.arguments = varray(
			PackedVector2Array{ Vector2(RFLOAT_VALUE(x1), RFLOAT_VALUE(y1)), Vector2(RFLOAT_VALUE(x2), RFLOAT_VALUE(y2)), Vector2(RFLOAT_VALUE(x3), RFLOAT_VALUE(y3)), Vector2(RFLOAT_VALUE(x4), RFLOAT_VALUE(y4)) },
			PackedColorArray{ gd_convert_color(c1), gd_convert_color(c2), gd_convert_color(c3), gd_convert_color(c4) });

	Godosu::singleton->add_to_queue(draw_data, FIX2LONG(z), gd_is_additive(additive));
	return OK;
}

VALUE godosu_draw_rect(VALUE self, VALUE x, VALUE y, VALUE width, VALUE height, VALUE c, VALUE z, VALUE additive) {
	Godosu::DrawCommand draw_data;
	draw_data.type = Godosu::DrawCommand::DRAW_RECT;
	draw_data.arguments = varray(Rect2(RFLOAT_VALUE(x), RFLOAT_VALUE(y), RFLOAT_VALUE(width), RFLOAT_VALUE(height)));

	Godosu::singleton->add_to_queue(draw_data, FIX2LONG(z), gd_is_additive(additive));
	return OK;
}

VALUE godosu_draw_triangle(VALUE self, VALUE x1, VALUE y1, VALUE c1, VALUE x2, VALUE y2, VALUE c2, VALUE x3, VALUE y3, VALUE c3, VALUE z, VALUE additive) {
	Godosu::DrawCommand draw_data;
	draw_data.type = Godosu::DrawCommand::DRAW_POLYGON;
	draw_data.arguments = varray(
			PackedVector2Array{ Vector2(RFLOAT_VALUE(x1), RFLOAT_VALUE(y1)), Vector2(RFLOAT_VALUE(x2), RFLOAT_VALUE(y2)), Vector2(RFLOAT_VALUE(x3), RFLOAT_VALUE(y3)) },
			PackedColorArray{ gd_convert_color(c1), gd_convert_color(c2), gd_convert_color(c3) });

	Godosu::singleton->add_to_queue(draw_data, FIX2LONG(z), gd_is_additive(additive));
	return OK;
}

VALUE godosu_draw_texture(VALUE self, VALUE texture, VALUE x, VALUE y, VALUE z, VALUE scale_x, VALUE scale_y, VALUE color, VALUE additive) {
	Godosu::DrawCommand draw_data;
	draw_data.type = Godosu::DrawCommand::DRAW_TEXTURE;
	draw_data.arguments = varray(
			Godosu::singleton->data.texture_cache[texture],
			Vector2(RFLOAT_VALUE(x), RFLOAT_VALUE(y)),
			Vector2(RFLOAT_VALUE(scale_x), RFLOAT_VALUE(scale_y)),
			gd_convert_color(color));

	Godosu::singleton->add_to_queue(draw_data, FIX2LONG(z), gd_is_additive(additive));
	return OK;
}

VALUE godosu_draw_texture_rotated(VALUE self, VALUE texture, VALUE x, VALUE y, VALUE z, VALUE angle, VALUE center_x, VALUE center_y, VALUE scale_x, VALUE scale_y, VALUE color) {
	Godosu::DrawCommand draw_data;
	draw_data.type = Godosu::DrawCommand::DRAW_TEXTURE_ROTATED;
	draw_data.arguments = varray(Godosu::singleton->data.texture_cache[texture],
			Vector2(RFLOAT_VALUE(x), RFLOAT_VALUE(y)),
			RFLOAT_VALUE(angle),
			Vector2(RFLOAT_VALUE(center_x), RFLOAT_VALUE(center_y)),
			Vector2(RFLOAT_VALUE(scale_x), RFLOAT_VALUE(scale_y)),
			gd_convert_color(color));

	Godosu::singleton->add_to_queue(draw_data, FIX2LONG(z));
	return OK;
}

VALUE godosu_draw_string(VALUE self, VALUE font, VALUE size, VALUE text, VALUE x, VALUE y, VALUE z, VALUE scale_x, VALUE scale_y, VALUE rel_x, VALUE rel_y, VALUE color) {
	Godosu::DrawCommand draw_data;
	draw_data.type = Godosu::DrawCommand::DRAW_STRING;
	draw_data.arguments = varray(Godosu::singleton->data.font_cache[font],
			Vector2(RFLOAT_VALUE(x), RFLOAT_VALUE(y)),
			StringValueCStr(text),
			FIX2LONG(size),
			gd_convert_color(color),
			Vector2(RFLOAT_VALUE(scale_x), RFLOAT_VALUE(scale_y)),
			Vector2(RFLOAT_VALUE(rel_x), RFLOAT_VALUE(rel_y)));

	Godosu::singleton->add_to_queue(draw_data, FIX2LONG(z));
	return OK;
}

VALUE godosu_get_text_width(VALUE self, VALUE font, VALUE text) {
	const Ref<Font> &fnt = Godosu::singleton->data.font_cache[font];
	const String string = StringValueCStr(text);
	return DBL2NUM(fnt->get_string_size(string).x);
}

VALUE godosu_play_song(VALUE self, VALUE instance, VALUE loop) {
	AudioStreamPlayer *song_player = Godosu::singleton->data.song_player;
	Ref<AudioStream> stream = Godosu::singleton->data.audio_cache[instance];

	if (stream->is_class("AudioStreamOggVorbis") || stream->is_class("AudioStreamMP3") || stream->is_class("AudioStreamWAV")) {
		stream->set(SNAME("loop"), RTEST(loop));
	}

	song_player->set_stream(stream);
	song_player->play();
	return OK;
}

VALUE godosu_stop_song(VALUE self) {
	Godosu::singleton->data.song_player->stop();
	return OK;
}

static inline int channel_counter = 0;

VALUE godosu_play_sample(VALUE self, VALUE instance, VALUE volume, VALUE speed) {
	// TODO: tu można jakiś pool albo polifonia dla tego samego dźwięku itp.
	AudioStreamPlayer *sample_player = memnew(AudioStreamPlayer);
	sample_player->set_stream(Godosu::singleton->data.audio_cache[instance]);
	sample_player->set_volume_db(Math::linear_to_db(RFLOAT_VALUE(volume)));
	sample_player->set_pitch_scale(RFLOAT_VALUE(speed));
	sample_player->set_autoplay(true);
	sample_player->connect(SNAME("finished"), callable_mp((Node *)sample_player, &Node::queue_free));
	Godosu::singleton->add_child(sample_player);

	Ref<AudioStreamPlayback> channel = sample_player->get_stream_playback();
	VALUE id = INT2NUM(channel_counter);
	channel_counter++;

	Godosu::singleton->data.channels[id] = channel;
	return id;
}

VALUE godosu_is_channel_playing(VALUE self, VALUE id) {
	const Ref<AudioStreamPlayback> &channel = Godosu::singleton->data.channels[id];
	return channel->is_playing() ? Qtrue : Qfalse;
}

VALUE godosu_destroy_channel(VALUE self, VALUE id) {
	Godosu::singleton->data.channels.erase(id);
	return OK;
}

VALUE godosu_create_shader(VALUE self, VALUE object, VALUE code) {
	const String source = StringValueCStr(code);

	Ref<Shader> shader;
	shader.instantiate();
	shader->set_code(source);

	Ref<ShaderMaterial> mat;
	mat.instantiate();
	mat->set_shader(shader);
	Godosu::singleton->data.shader_cache[object] = mat;
	return OK;
}

VALUE godosu_set_shader(VALUE self, VALUE z, VALUE object) {
	if (NIL_P(object)) {
		Godosu::singleton->data.shader_map.erase(FIX2LONG(z));
	} else {
		Godosu::singleton->data.shader_map[FIX2LONG(z)] = Godosu::singleton->data.shader_cache[object];
	}
	return OK;
}

#endif