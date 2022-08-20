#ifndef GOSU_H
#define GOSU_H

#include "scene/gui/control.h"
#include "mruby/compile.h"

class Gosu : public Control {
	GDCLASS(Gosu, Control);

	mrb_state *mrb = nullptr;
	
	String main_file_path;

	String get_file_source(const String &p_path);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_main_file_path(const String &p_path) { main_file_path = p_path; }
	String get_main_file_path() { return main_file_path; }
	
	void draw_sth();

	Gosu();
	~Gosu();
};

#endif // GOSU_H
