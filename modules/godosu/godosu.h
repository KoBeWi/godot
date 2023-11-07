#ifndef GOSU_H
#define GOSU_H

#include "scene/2d/node_2d.h"

#include <ruby.h>

class Texture2D;
class AudioStream;
class AudioStreamPlayer;

class Godosu : public Node2D {
	GDCLASS(Godosu, Node2D);

	int ruby_state = -1;

	HashMap<int, CanvasItem *> ci_map;
	HashMap<CanvasItem *, Vector<Vector<Variant>>> draw_queue;

	String main_script;

	void _draw_canvas_item(CanvasItem *p_item);

protected:
	void _notification(int p_what);
	static void _bind_methods();
	virtual void input(const Ref<InputEvent> &p_event) override;

public:
	inline static Godosu *singleton = nullptr;

	struct {
		VALUE window = 0;

		HashMap<VALUE, Ref<Texture2D>> texture_cache;
		HashMap<VALUE, Ref<AudioStream>> audio_cache;
		HashMap<VALUE, Ref<Font>> font_cache;

		AudioStreamPlayer *song_player = nullptr;
		AudioStreamPlayer *sample_player = nullptr;

		VALUE callback_base = 0;
		VALUE callback_update_mouse = 0;
		VALUE callback_update = 0;
		VALUE callback_draw = 0;
		VALUE callback_button_down = 0;
		VALUE callback_button_up = 0;
	} data;

	void set_main_script(const String &p_main);
	String get_main_script() const;

	void setup_window(VALUE p_window);
	CanvasItem *get_ci(int p_z_index);
	void add_to_queue(CanvasItem *p_item, const Vector<Variant> &p_data);

	Godosu();
	~Godosu();
};

#define godosu_window_callback(m_argc, ...) rb_funcall(data.window, data.callback_base, m_argc, __VA_ARGS__);

#endif // GOSU_H
