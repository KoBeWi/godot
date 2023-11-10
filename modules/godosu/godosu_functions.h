#ifndef GODOSU_FUNCTIONS_H
#define GODOSU_FUNCTIONS_H

#include "godosu.h"

#include "core/config/project_settings.h"
#include "scene/audio/audio_stream_player.h"
#include "scene/resources/atlas_texture.h"

Color gd_convert_color(VALUE from) {
	const String color_string = StringValueCStr(from);
	Color c = Color::html(color_string);
	Color ret;
	ret.r = c.g;
	ret.g = c.b;
	ret.b = c.a;
	ret.a = c.r;
	return ret;
}

VALUE godosu_print(VALUE self, VALUE string) {
	String print_string = StringValueCStr(string);
	print_line(print_string);
	return OK;
}

VALUE godosu_setup_window(VALUE self, VALUE window, VALUE width, VALUE height) {
	Vector2i size(FIX2INT(width), FIX2INT(height));
	Godosu::singleton->setup_window(window, size);
	Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_HIDDEN);
	return OK;
}

VALUE godosu_retrofication(VALUE self) {
	Godosu::singleton->set_texture_filter(CanvasItem::TEXTURE_FILTER_NEAREST);
	return OK;
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

VALUE godosu_draw_rect(VALUE self, VALUE x, VALUE y, VALUE width, VALUE height, VALUE c, VALUE z, VALUE mode) {
	Godosu::DrawCommand draw_data;
	draw_data.type = Godosu::DrawCommand::DRAW_RECT;
	draw_data.arguments = varray(Rect2(RFLOAT_VALUE(x), RFLOAT_VALUE(y), RFLOAT_VALUE(width), RFLOAT_VALUE(height)));
	Godosu::singleton->add_to_queue(FIX2LONG(z), draw_data);
	return OK;
}

VALUE godosu_draw_quad(VALUE self, VALUE x1, VALUE y1, VALUE c1, VALUE x2, VALUE y2, VALUE c2, VALUE x3, VALUE y3, VALUE c3, VALUE x4, VALUE y4, VALUE c4, VALUE z, VALUE mode) {
	Godosu::DrawCommand draw_data;
	draw_data.type = Godosu::DrawCommand::DRAW_QUAD;
	draw_data.arguments = varray(
			PackedVector2Array{ Vector2(RFLOAT_VALUE(x1), RFLOAT_VALUE(y1)), Vector2(RFLOAT_VALUE(x2), RFLOAT_VALUE(y2)), Vector2(RFLOAT_VALUE(x3), RFLOAT_VALUE(y3)), Vector2(RFLOAT_VALUE(x4), RFLOAT_VALUE(y4)) },
			PackedColorArray{ gd_convert_color(c1), gd_convert_color(c2), gd_convert_color(c3), gd_convert_color(c4) });

	Godosu::singleton->add_to_queue(FIX2LONG(z), draw_data);
	return OK;
}

VALUE godosu_draw_texture(VALUE self, VALUE texture, VALUE x, VALUE y, VALUE z, VALUE scale_x, VALUE scale_y, VALUE color) {
	Godosu::DrawCommand draw_data;
	draw_data.type = Godosu::DrawCommand::DRAW_TEXTURE;
	draw_data.arguments = varray(
			Godosu::singleton->data.texture_cache[texture],
			Vector2(RFLOAT_VALUE(x), RFLOAT_VALUE(y)),
			Vector2(RFLOAT_VALUE(scale_x), RFLOAT_VALUE(scale_y)),
			gd_convert_color(color));

	Godosu::singleton->add_to_queue(FIX2LONG(z), draw_data);
	return OK;
}

VALUE godosu_draw_texture_rotated(VALUE self, VALUE texture, VALUE x, VALUE y, VALUE z, VALUE angle, VALUE center_x, VALUE center_y, VALUE scale_x, VALUE scale_y, VALUE color) {
	Godosu::DrawCommand draw_data;
	draw_data.type = Godosu::DrawCommand::DRAW_TEXTURE_ROTATED;
	draw_data.arguments.append(Godosu::singleton->data.texture_cache[texture]);
	draw_data.arguments.append(Vector2(RFLOAT_VALUE(x), RFLOAT_VALUE(y)));
	draw_data.arguments.append(RFLOAT_VALUE(angle));
	draw_data.arguments.append(Vector2(RFLOAT_VALUE(center_x), RFLOAT_VALUE(center_y)));
	draw_data.arguments.append(Vector2(RFLOAT_VALUE(scale_x), RFLOAT_VALUE(scale_y)));
	draw_data.arguments.append(gd_convert_color(color));

	Godosu::singleton->add_to_queue(FIX2LONG(z), draw_data);
	return OK;
}

VALUE godosu_draw_string(VALUE self, VALUE font, VALUE size, VALUE text, VALUE x, VALUE y, VALUE z, VALUE scale_x, VALUE scale_y, VALUE rel_x, VALUE rel_y, VALUE color) {
	Godosu::DrawCommand draw_data;
	draw_data.type = Godosu::DrawCommand::DRAW_STRING;
	draw_data.arguments.append(Godosu::singleton->data.font_cache[font]);
	draw_data.arguments.append(Vector2(RFLOAT_VALUE(x), RFLOAT_VALUE(y)));
	draw_data.arguments.append(StringValueCStr(text));
	draw_data.arguments.append(FIX2LONG(size));
	draw_data.arguments.append(gd_convert_color(color));
	draw_data.arguments.append(Vector2(RFLOAT_VALUE(scale_x), RFLOAT_VALUE(scale_y)));
	draw_data.arguments.append(Vector2(RFLOAT_VALUE(rel_x), RFLOAT_VALUE(rel_y)));

	Godosu::singleton->add_to_queue(FIX2LONG(z), draw_data);
	return OK;
}

VALUE godosu_play_song(VALUE self, VALUE instance) {
	AudioStreamPlayer *song_player = Godosu::singleton->data.song_player;
	song_player->set_stream(Godosu::singleton->data.audio_cache[instance]);
	song_player->play();
	return OK;
}

VALUE godosu_stop_song(VALUE self) {
	Godosu::singleton->data.song_player->stop();
	return OK;
}

VALUE godosu_play_sample(VALUE self, VALUE instance) {
	// TODO: tu można jakiś pool albo polifonia dla tego samego dźwięku itp.
	AudioStreamPlayer *sample_player = memnew(AudioStreamPlayer);
	sample_player->set_stream(Godosu::singleton->data.audio_cache[instance]);
	sample_player->set_autoplay(true);
	sample_player->connect(SNAME("finished"), callable_mp((Node *)sample_player, &Node::queue_free));
	Godosu::singleton->add_child(sample_player);
	return OK;
}

#endif