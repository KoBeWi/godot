#ifndef GOSU_H
#define GOSU_H

#include "scene/main/node.h"

#include <ruby.h>

#undef IGNORE

class AudioStream;
class AudioStreamPlayback;
class AudioStreamPlayer;
class CanvasItem;
class Control;
class Font;
class LineEdit;
class Node2D;
class SubViewport;
class Texture2D;

class Godosu : public Node {
	GDCLASS(Godosu, Node);

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
	LocalVector<CanvasItem *> to_remove;
	HashMap<CanvasItem *, Vector<DrawCommand>> draw_queue;

	String main_script;

	void _draw_canvas_item(CanvasItem *p_item);
	void _update_pending_framebuffers();

protected:
	void _notification(int p_what);
	static void _bind_methods();
	virtual void input(const Ref<InputEvent> &p_event) override;

public:
	inline static Godosu *singleton = nullptr;

	bool is_cleanup = false;

	struct {
		VALUE window = 0;

		HashMap<VALUE, Ref<Texture2D>> texture_cache;
		HashMap<VALUE, Ref<AudioStream>> audio_cache;
		HashMap<VALUE, Ref<Font>> font_cache;
		HashMap<VALUE, Ref<ShaderMaterial>> shader_cache;

		Vector<LineEdit *> text_inputs; // TODO tu też HashMapa?
		HashMap<VALUE, Ref<AudioStreamPlayback>> channels;
		HashMap<VALUE, SubViewport *> framebuffers;
		HashMap<VALUE, Control *> macros;

		AudioStreamPlayer *song_player = nullptr;
		Ref<Material> additive_material;
		int clip_id = 0;
		Rect2 clip_rect;
		HashMap<int, Ref<Material>> shader_map;
		SubViewport *active_framebuffer = nullptr;
		Control *active_macro = nullptr;
		Vector<SubViewport *> pending_frame_buffers;

		VALUE callback_base = 0;
		VALUE callback_update_mouse = 0;
		VALUE callback_update = 0;
		VALUE callback_draw = 0;
		VALUE callback_button_down = 0;
		VALUE callback_button_up = 0;
	} data;

	void set_main_script(const String &p_main);
	String get_main_script() const;

	void setup_window(VALUE p_window, const Vector2i &p_size, bool p_fullscreen);
	CanvasItem *get_ci(int p_z_index, const Ref<Material> &p_material, int p_clip_id, SubViewport *p_framebuffer);
	void add_to_queue(const DrawCommand &p_data, int p_z, const Ref<Material> &p_material = Ref<Material>());

	VALUE create_line_edit();
	LineEdit *get_line_edit(VALUE id);
	Control *create_macro(const Vector2 &p_size);
	void set_active_framebuffer(SubViewport *p_framebuffer);

	Godosu();
};

#define godosu_window_callback(m_argc, ...) rb_funcall(data.window, data.callback_base, m_argc, __VA_ARGS__);

#endif // GOSU_H
