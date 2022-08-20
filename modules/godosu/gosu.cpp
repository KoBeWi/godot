#include "gosu.h"

#include "core/object/class_db.h"

static Gosu *global_gosu;

mrb_value f_print(mrb_state *mrb, mrb_value self) {
    char *string;
    mrb_get_args(mrb, "z", &string);
    print_line(string);
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

    mrb_define_method(mrb, mrb->object_class, "gd_draw_rect", f_draw_rect, MRB_ARGS_REQ(5));
}

Gosu::~Gosu() {
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    mrb_close(mrb);
}