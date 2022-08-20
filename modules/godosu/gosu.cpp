#include "gosu.h"

#include "core/object/class_db.h"

static Gosu *global_gosu;

mrb_value ruby_method(mrb_state *mrb, mrb_value self) {
    global_gosu->draw_sth();
    return mrb_nil_value();
}

void Gosu::draw_sth() {
    print_line("draw");
}

String Gosu::get_file_source(const String &p_path) {
    Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::READ);
    PackedStringArray lines = f->get_as_utf8_string().split("\n");
    for (int i = 0; i < lines.size(); i++) {
        const String &line = lines[i];
        if (line.begins_with("require_relative")) {
            int start = line.find("\"");
            int end = line.find("\"", start + 1);

            String next_file = line.substr(start + 1, end - start - 1);
            lines.write[i] = get_file_source(next_file);
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
            // mrb_value obj = mrb_load_string(mrb, "def dupa;ruby_method;end");

            String source = get_file_source(main_file_path);
            mrb_value obj = mrb_load_string(mrb, source.utf8().get_data());
            mrb_funcall(mrb, obj, "dupa", 0);
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

    auto gosu_module = mrb_define_module(mrb, "Gosu");
    // mrb_define_module_function(mrb, gosu_module, "ruby_method", &Gosu::draw_sth, MRB_ARGS_NONE());
    mrb_define_method(mrb, mrb->object_class, "ruby_method", ruby_method, MRB_ARGS_NONE());

    // mrb_funcall_id(
}

Gosu::~Gosu() {
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    mrb_close(mrb);
}