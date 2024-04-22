module Gosu
    def default_font_name
        "Fonot"
    end

    def enable_undocumented_retrofication
        godot_retrofication
    end

    def draw_rect(x, y, width, height, c, z = 0, mode = :default)
        godot_draw_rect($_translate_x + x.to_f, $_translate_y + y.to_f, width.to_f, height.to_f, _colorize(c), _sanitize_z(z), mode)
    end

    def draw_quad(x1, y1, c1, x2, y2, c2, x3, y3, c3, x4, y4, c4, z = 0, mode = :default)
        godot_draw_quad($_translate_x + x1.to_f, $_translate_y + y1.to_f, _colorize(c1), $_translate_x + x2.to_f, $_translate_y + y2.to_f, _colorize(c2), $_translate_x + x3.to_f, $_translate_y + y3.to_f, _colorize(c3), $_translate_x + x4.to_f, $_translate_y + y4.to_f, _colorize(c4), _sanitize_z(z), mode)
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

    def button_id_to_char(id)
        return "O" # TODO
    end

    class Window
        attr_accessor :caption, :text_input # TODO
        attr_reader :mouse_x, :mouse_y, :width, :height, :__keys

        def initialize(w, h, fullscreen)
            # TODO fullscreen
            @width, @height = w.to_i, h.to_i
            godot_setup_window(self, @width, @height)
            @__keys = []
        end

        def show
            $__gosu_window = self
            # TODO
        end

        def flush
            $_base_z_index += 1000
        end

        def translate(x, y)
            $_translate_x = x
            $_translate_y = y
            yield
            $_translate_x = 0
            $_translate_y = 0
        end

        def clip_to(x, y, w, h)
            # TODO
            yield
        end

        def update
        end

        def draw
        end
  
        def button_down(id)
        end
        
        def button_up(id)
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
    end

    class Image
        attr_reader :width, :height

        def initialize(screen, source, tileable = false, x = 0, y = 0, w = 0, h = 0)
            # TODO tileable
            if w * h == 0
                godot_load_image(self, source)
            else
                godot_load_atlas(self, source, x.to_i, y.to_i, w.to_i, h.to_i)
                @width = w
                @height = h
            end
        end

        def Image.load_tiles(screen, source, tile_width, tile_height, options = {})
            base = Image.new(screen, source)

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
                    ret << Image.new(screen, base, true, x, y, tile_width, tile_height)
                    x += tile_width
                end
                y += tile_height
            end

            ret
        end

        def draw(x, y, z = 0, scale_x = 1, scale_y = 1, color = 0xff_ffffff, mode = :default)
            godot_draw_texture(self, $_translate_x + x.to_f, $_translate_y + y.to_f, _sanitize_z(z), scale_x.to_f, scale_y.to_f, _colorize(color));
        end

        def draw_rot(x, y, z, angle = 0, center_x = 0.5, center_y = 0.5, scale_x = 1, scale_y = 1, color = 0xff_ffffff, mode = :default)
            godot_draw_texture_rotated(self, $_translate_x + x.to_f, $_translate_y + y.to_f, _sanitize_z(z), angle.to_f * (Math::PI / 180.0), center_x.to_f, center_y.to_f, scale_x.to_f, scale_y.to_f, _colorize(color))
        end

        def godot_set_size(w, h)
            @width = w
            @height = h
        end
    end

    class Song
        def initialize(screen, filename)
            godot_load_audio(self, filename)
        end

        def play(loop)
            return if Song.current_song == self
            # TODO loop
            godot_play_song(self)
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
        def initialize(screen, filename)
            godot_load_audio(self, filename)
        end

        def play(volume = 1.0, speed = 1.0) # TODO
            godot_play_sample(self)
        end
    end

    class TextInput
        # TODO
        attr_accessor :text, :caret_pos

        def initialize
            @text = ""
            @caret_pos = 0
        end

        def text=(val)
            @text = val.to_s
        end
    end

    class Font
        def initialize(screen, name, size)
            @size = size.to_i
            godot_load_font(self, name)
        end

        def draw_rel(text, x, y, z, rel_x, rel_y, scale_x = 1, scale_y = 1, color = 0xff_ffffff, mode = :default)
            godot_draw_string(self, @size, text.to_s, x.to_f, y.to_f, _sanitize_z(z), scale_x.to_f, scale_y.to_f, rel_x.to_f, rel_y.to_f, _colorize(color))
        end

        def draw(text, x, y, z, scale_x = 1.0, scale_y = 1.0, color = 0xff_ffffff, mode = :default)
            godot_draw_string(self, @size, text.to_s, x.to_f, y.to_f, _sanitize_z(z), scale_x.to_f, scale_y.to_f, 0.0, 0.0, _colorize(color))
        end

        def text_width(text)
            # TODO
            return text.to_s.length * 10
        end
    end

    class Color
        attr_accessor :a, :r, :g, :b

        def initialize(a, r, g, b)
            @a, @r, @g, @b = a, r, g, b
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
            return @r << 24 | @g << 16 | @b << 8 | @a
        end

        def to_i
            gl
        end
    end
end

def puts(string)
    godot_print(string.to_s)
end

def _colorize(color)
    color.to_i.to_s(16).rjust(8, "0")
end

def _sanitize_z(z)
    $_base_z_index + (z * 100).to_i
end

$_base_z_index = 0
$_translate_x = 0
$_translate_y = 0