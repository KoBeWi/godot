#ifndef GOSU_H
#define GOSU_H

#include "scene/2d/node_2d.h"
#include "servers/audio/audio_stream.h"

#include <ruby.h>

class AudioStreamPlayer;
class LineEdit;
class Texture2D;

class Godosu : public Node2D {
	GDCLASS(Godosu, Node2D);

public:
	struct DrawCommand {
		enum Type {
			DRAW_RECT,
			DRAW_TEXTURE,
			DRAW_TEXTURE_ROTATED,
			DRAW_LINE,
			DRAW_POLYGON,
			DRAW_STRING
		};

		Type type;
		Vector<Variant> arguments;
	};

private:
	int ruby_state = -1;

	HashMap<uint32_t, CanvasItem *> ci_map;
	HashMap<CanvasItem *, Vector<DrawCommand>> draw_queue;

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
		HashMap<VALUE, Ref<ShaderMaterial>> shader_cache;
		Vector<LineEdit *> text_inputs;

		AudioStreamPlayer *song_player = nullptr;
		Ref<Material> additive_material;
		Rect2 clip_rect;
		HashMap<int, Ref<Material>> shader_map;

		VALUE callback_base = 0;
		VALUE callback_update_mouse = 0;
		VALUE callback_update = 0;
		VALUE callback_draw = 0;
		VALUE callback_button_down = 0;
		VALUE callback_button_up = 0;
	} data;

	void set_main_script(const String &p_main);
	String get_main_script() const;

	void setup_window(VALUE p_window, const Vector2i &p_size);
	CanvasItem *get_ci(int p_z_index, const Ref<Material> &p_material, const Rect2 &p_clip_rect);
	void add_to_queue(const DrawCommand &p_data, int p_z, const Ref<Material> &p_material = Ref<Material>());

	VALUE create_line_edit();
	LineEdit *get_line_edit(VALUE id);

	Godosu();
	~Godosu();
};

#define godosu_window_callback(m_argc, ...) rb_funcall(data.window, data.callback_base, m_argc, __VA_ARGS__);

#endif // GOSU_H
