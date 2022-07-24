#include "gosu.h"

#include "mruby/compile.h"

void Gosu::_notification(int p_what) {
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    switch (p_what) {
        case NOTIFICATION_READY: {
            print_line("print_here");

            mrb_state *mrb = mrb_open();

            if (!mrb) {
                return;
            }

            mrb_value obj = mrb_load_string(mrb, "puts \"hello world\"");
            // mrb_funcall_argv(mrb, obj, MRB_SYM(method_name), 1, &obj); // Calling ruby function from test.rb.
            mrb_close(mrb);

            print_line("but did it work");
        } break;
    }
}