#include "gosu.h"

#include "mruby/compile.h"

mrb_value ruby_method(mrb_state *mrb, mrb_value self)
{
    print_line("test");
    return mrb_nil_value();
}

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

            mrb_define_method(mrb, mrb->object_class, "ruby_method", ruby_method, MRB_ARGS_NONE());
            mrb_value obj = mrb_load_string(mrb, "def dupa;ruby_method;end");
            print_line("...");
            // mrb_value obj = mrb_load_string(mrb, "puts \"hello world\"");
            // mrb_value obj = mrb_load_string(mrb, "def test;File.new('test.txt', 'w');puts :meh;end");
            mrb_funcall(mrb, obj, "dupa", 0);
            // mrb_funcall_argv(mrb, obj, MRB_SYM(test), 1, &obj); // Calling ruby function from test.rb.
            mrb_close(mrb);

            print_line("but did it work");
        } break;
    }
}