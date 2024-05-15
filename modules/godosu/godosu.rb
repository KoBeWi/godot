module Gosu
    def default_font_name
        "Fonot"
    end

    def enable_undocumented_retrofication
        godot_retrofication
    end

    def draw_rect(x, y, width, height, c, z = 0, mode = :default)
        godot_draw_rect(_gd_x(x), _gd_y(y), width.to_f, height.to_f, _gd_color(c), _gd_z(z), mode == :additive)
    end

    def draw_line(x1, y1, c1, x2, y2, c2, z = 0, mode = :default)
        godot_draw_line(_gd_x(x1), _gd_y(y1), _gd_color(c1), _gd_x(x2), _gd_y(y2), _gd_color(c2), _gd_z(z), mode == :additive)
    end

    def draw_quad(x1, y1, c1, x2, y2, c2, x3, y3, c3, x4, y4, c4, z = 0, mode = :default)
        godot_draw_quad(_gd_x(x1), _gd_y(y1), _gd_color(c1), _gd_x(x2), _gd_y(y2), _gd_color(c2), _gd_x(x3), _gd_y(y3), _gd_color(c3), _gd_x(x4), _gd_y(y4), _gd_color(c4), _gd_z(z), mode == :additive)
    end

    def draw_triangle(x1, y1, c1, x2, y2, c2, x3, y3, c3, z = 0, mode = :default)
        godot_draw_triangle(_gd_x(x1), _gd_y(y1), _gd_color(c1), _gd_x(x2), _gd_y(y2), _gd_color(c2), _gd_x(x3), _gd_y(y3), _gd_color(c3), _gd_z(z), mode == :additive)
    end

    def button_down?(id)
        return $__gosu_window.__keys.include?(id)
    end

    def offset_x(theta, r)
        return Math::sin(theta * (Math::PI / 180.0)) * r
    end

    def offset_y(theta, r)
        return -Math::cos(theta * (Math::PI / 180.0)) * r
    end

    def distance(x1, y1, x2, y2)
        dist_x = (x2 - x1).to_f
        dist_y = (y2 - y1).to_f
        return Math.sqrt(dist_x * dist_x + dist_y * dist_y)
    end

    def wrap(value, min, max)
        result = (value - min).to_f % (max - min).to_f
        return result < 0 ? result + max : result + min
    end

    def normalize_angle(angle)
        return wrap(angle, 0.0, 360.0)
    end

    def angle(x1, y1, x2, y2)
        dist_x = (x2 - x1).to_f
        dist_y = (y2 - y1).to_f
        return normalize_angle(Math.atan2(dist_y, dist_x) * (180.0 / Math::PI) + 90)
    end

    def angle_diff(angle1, angle2)
        normalize_angle(angle2 - angle1 + 180) - 180
    end

    def button_id_to_char(id)
        godot_button_id_to_char(id.to_i)
    end

    def milliseconds
        godot_milliseconds
    end

    def flush
        $_base_z_index += 1000
    end

    def translate(x, y)
        $_translate_x = x.to_f
        $_translate_y = y.to_f
        yield
        $_translate_x = 0.0
        $_translate_y = 0.0
    end

    def scale(scale_x, scale_y, around_x, around_y)
        yield # TODO
    end

    def clip_to(x, y, w, h)
        godot_set_clip(_gd_x(x), _gd_y(y), w.to_f, h.to_f)
        yield
        godot_set_clip(0.0, 0.0, 0.0, 0.0)
    end

    def record(width, height)
        macro = Macro.new(width, height)
        yield
        godot_end_macro
        macro
    end

    class Window
        attr_reader :mouse_x, :mouse_y, :width, :height, :__keys

        def initialize(w, h, fullscreen)
            godot_retrofication # TODO usunąć
            # TODO fullscreen
            @width, @height, @fullscreen = w.to_i, h.to_i, fullscreen
            godot_setup_window(self, @width, @height, @fullscreen)
            @__keys = []
        end

        def show
            $__gosu_window = self
        end

        def update
        end

        def draw
        end
  
        def button_down(id)
        end
        
        def button_up(id)
        end

        def text_input=(input)
            if input
                input.__focus()
            else
                godot_unfocus_text_input
            end
            @__text_input = input
        end

        def text_input
            return @__text_input
        end

        def godot_callback(method, data = nil)
            begin
                if data
                    send(method, data)
                else
                    send(method)
                end
            rescue Exception => e
                puts e
                puts e.backtrace
            end
        end

        def godot_button_down(id)
            @__keys << id
            button_down(id)
        end
        
        def godot_button_up(id)
            @__keys.delete(id)
            button_up(id)
        end

        def godot_draw
            $_base_z_index = 0
            draw
        end

        def godot_update_mouse(x, y)
            @mouse_x = x
            @mouse_y = y
        end

        def caption=(text)
            @caption = text.to_s
            godot_set_window_title(@caption)
        end

        def caption
            @caption
        end

        def fullscreen=(full)
            @fullscreen = full
            godot_set_window_fullscreen(full)
        end

        def fullscreen?
            @fullscreen
        end
    end

    class Image
        attr_reader :width, :height

        def initialize(screen, source = nil, tileable = false, x = 0, y = 0, w = 0, h = 0)
            if screen.class == String or screen.class == Image
                initialize_without_window(screen, source, tileable, x, y, w)
            else
                initialize_without_window(source, tileable, x, y, w, h)
            end
        end

        def initialize_without_window(source, tileable = false, x = 0, y = 0, w = 0, h = 0)
            # TODO tileable
            if w * h == 0
                godot_load_image(self, source.to_s)
            else
                godot_load_atlas(self, source, x.to_i, y.to_i, w.to_i, h.to_i)
                @width = w
                @height = h
            end
        end

        def self.load_tiles(screen, source, tile_width, tile_height, options = {})
            if screen.class == String or screen.class == Image
                load_tiles_without_window(screen, source, tile_width, options)
            else
                load_tiles_without_window(source, tile_width, tile_height, options)
            end
        end

        def self.load_tiles_without_window(source, tile_width, tile_height, options = {})
            base = source
            if source.class != Image
                base = Image.new(source)
            end

            if tile_width < 0
                tile_width = base.width / -tile_width
            end

            if tile_height < 0
                tile_height = base.height / -tile_height
            end

            ret = []

            y = 0
            while y < base.height
                x = 0
                while x < base.width
                    ret << Image.new(base, true, x, y, tile_width, tile_height)
                    x += tile_width
                end
                y += tile_height
            end

            ret
        end

        def draw(x, y, z = 0, scale_x = 1, scale_y = 1, color = 0xff_ffffff, mode = :default)
            godot_draw_texture(self, _gd_x(x), _gd_y(y), _gd_z(z), scale_x.to_f, scale_y.to_f, _gd_color(color), mode == :additive);
        end

        def draw_rot(x, y, z, angle = 0, center_x = 0.5, center_y = 0.5, scale_x = 1, scale_y = 1, color = 0xff_ffffff, mode = :default)
            godot_draw_texture_rotated(self, _gd_x(x), _gd_y(y), _gd_z(z), angle.to_f * (Math::PI / 180.0), center_x.to_f, center_y.to_f, scale_x.to_f, scale_y.to_f, _gd_color(color))
        end

        def subimage(x, y, w, h)
            Image.new(self, true, x, y, w, h)
        end

        def godot_set_size(w, h)
            @width = w
            @height = h
        end
    end

    class Macro
        def initialize(width, height)
            # TODO: finalizer
            @id = godot_create_macro(width.to_i, height.to_i)
        end

        def draw(x, y, z)
            godot_draw_macro(@id, x.to_f, y.to_f, _gd_z(z))
        end
    end

    class Song
        def initialize(screen, filename = nil)
            if screen.class == String
                initialize_without_window(screen)
            else
                initialize_without_window(filename)
            end
        end
        
        def initialize_without_window(filename)
            godot_load_audio(self, filename)
        end

        def play(lp = false)
            return if Song.current_song == self
            godot_play_song(self, lp)
            @@current_song = self
        end

        def stop
            godot_stop_song
            @@curent_song = nil
        end

        def playing?
            return Song.current_song == self
        end

        def Song.current_song
            @@current_song if defined? @@current_song
        end
    end

    class Sample
        def initialize(screen, filename = nil)
            if screen.class == String
                initialize_without_window(screen)
            else
                initialize_without_window(filename)
            end
        end
        
        def initialize_without_window(filename)
            godot_load_audio(self, filename)
        end

        def play(volume = 1.0, speed = 1.0) # TODO
            id = godot_play_sample(self, volume.to_f, speed.to_f)
            Channel.new(id)
        end
    end

    class Channel
        def initialize(id)
            @id = id
        end

        def playing?
            godot_is_channel_playing(@id)
        end

        ObjectSpace.define_finalizer(self, proc { godot_destroy_channel(@id) })
    end

    class TextInput
        def initialize
            @text = ""
            @caret_pos = 0
            @text_id = godot_create_text_input()
            ObjectSpace.define_finalizer(self, proc { godot_destroy_text_input(@text_id) })
        end

        def text=(val)
            godot_set_text_input_text(@text_id, val.to_s)
        end
        
        def text
            godot_get_text_input_text(@text_id)
        end

        def caret_pos=(val)
            godot_set_text_input_caret(@text_id, val.to_i)
        end
        
        def caret_pos
            godot_get_text_input_caret(@text_id)
        end

        def selection_start=(val)
            godot_set_text_input_selection_start(@text_id, val.to_i)
        end

        def selection_start
            godot_get_text_input_selection_start(@text_id)
        end

        def __focus
            godot_focus_text_input(@text_id)
        end
    end

    class Font
        def initialize(screen, name, size)
            @size = size.to_i
            godot_load_font(self, name)
        end

        def draw(text, x, y, z, scale_x = 1.0, scale_y = 1.0, color = 0xff_ffffff, mode = :default) # TODO mode
            godot_draw_string(self, @size, text.to_s, _gd_x(x), _gd_y(y), _gd_z(z), scale_x.to_f, scale_y.to_f, 0.0, 0.0, _gd_color(color))
        end

        def draw_rel(text, x, y, z, rel_x, rel_y, scale_x = 1, scale_y = 1, color = 0xff_ffffff, mode = :default) # TODO mode
            godot_draw_string(self, @size, text.to_s, _gd_x(x), _gd_y(y), _gd_z(z), scale_x.to_f, scale_y.to_f, rel_x.to_f, rel_y.to_f, _gd_color(color))
        end

        def text_width(text)
            godot_get_text_width(self, text.to_s)
        end
    end

    class Color
        attr_accessor :a, :r, :g, :b

        alias_method :alpha, :a
        alias_method :alpha=, :a=
        alias_method :red, :r
        alias_method :red=, :r=
        alias_method :green, :g
        alias_method :green=, :g=
        alias_method :blue, :b
        alias_method :blue=, :b=

        def initialize(*args)
            if args.size == 4
                @a, @r, @g, @b = args
            elsif args.size == 3
                @a, @r, @g, @b = 255, *args
            elsif args.size == 1
                v = args[0]
                @r = v >> 16 & 255
                @g = v >> 8 & 255
                @b = v & 255
                @a = v >> 24 & 255
            end
        end

        def Color.argb(a, r, g, b)
            Color.new(a, r, g, b)
        end

        def Color.from_ahsv(a, h, s, v)
            h = h % 360.0 / 360.0
            s = s.clamp(0.0, 1.0).to_f
            v = v.clamp(0.0, 1.0).to_f

            rgb = godot_hsv_to_rgb(h, s, v)

            Color.argb(a, *rgb)
        end

        def Color.from_hsv(h, s, v)
            Color.from_ahsv(1.0, h, s, v)
        end

        def Color.rgba(r, g, b, a)
            Color.new(a, r, g, b)
        end

        BLACK = Color.argb(255, 0, 0, 0)
        GRAY = Color.argb(255, 128, 128, 128)
        WHITE = Color.argb(255, 255, 255, 255)
        AQUA = Color.argb(255, 0, 255, 255)
        RED = Color.argb(255, 255, 0, 0)
        GREEN = Color.argb(255, 0, 255, 0)
        BLUE = Color.argb(255, 0, 0, 255)
        YELLOW = Color.argb(255, 255, 255, 0)
        FUCHSIA = Color.argb(255, 255, 0, 255)
        CYAN = Color.argb(255, 0, 255, 255)

        def gl
            return @a << 24 | @r << 16 | @g << 8 | @b
        end

        def to_i
            gl
        end

        def ==(other)
            other.class == Color and other.r == r and other.g == g and other.b == b and other.a == a
        end
    end
end

def puts(string)
    godot_print(string.to_s)
end

def exit
    godot_exit
end

def _gd_x(x)
    $_translate_x + x.to_f
end

def _gd_y(y)
    $_translate_y + y.to_f
end

def _gd_z(z)
    $_base_z_index + (z * 100).to_i
end

def _gd_color(color)
    color.to_i.to_s(16).rjust(8, "0") # TODO: można wysyłać jako int
end

$_base_z_index = 0
$_translate_x = 0.0
$_translate_y = 0.0