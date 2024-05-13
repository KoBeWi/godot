module Ashton
    class Shader
        def initialize(args = {})
            path = args[:fragment]
            code = File.readlines(path).join()
            godot_create_shader(self, code)
        end

        def enable(z)
            godot_set_shader(_gd_z(z), self)
        end
        
        def disable(z)
            godot_set_shader(_gd_z(z), nil)
        end

        def image=(image)
            # TODO
        end

        def color=(color)
            # TODO
        end
    end

    class Texture
        def initialize(width, height)
            # TODO: finalizer
            @id = godot_create_framebuffer(width.to_i, height.to_i)
        end

        def render
            godot_set_framebuffer(@id)
            yield
            godot_set_framebuffer(nil)
        end

        def [](x, y)
            Color.new(godot_get_pixel(@id, x.to_i, y.to_i))
        end
    end

    class WindowBuffer < Texture
        def initialize
            super $__window.width, $__window.height
        end
    end
end

module Gosu
    class Window
        alias_method :ashton_initialize, :initialize
        def initialize(*args, &block)
            $__window = self
            ashton_initialize(*args, &block)
        end

        def gl(z)
            yield
        end
    end

    class Image    
        DEFAULT_DRAW_COLOR = Gosu::Color::WHITE
        
        alias_method :draw_without_hash, :draw
        protected :draw_without_hash
        def draw(*args)
            args, shader = if args.last.is_a?(Hash)
                                [args[0..-2], args.last[:shader]]
                            else
                                [args, nil]
                            end

            z = args[2]

            if shader
                shader.enable z
                $__window.gl z do
                    shader.image = self
                    shader.color = args[5].is_a?(Color) ? args[5] : DEFAULT_DRAW_COLOR
                end
            end

            begin
                draw_without_hash(*args)
            ensure
                shader.disable z if shader
            end
        end
        
        alias_method :draw_rot_without_hash, :draw_rot
        protected :draw_rot_without_hash
        def draw_rot(*args)
            args, shader = if args.last.is_a?(Hash)
                            [args[0..-2], args.last[:shader]]
                            else
                            [args, nil]
                            end
            z = args[2]

            if shader
                shader.enable z
                $__window.gl z do
                    shader.image = self
                    shader.color = args[8].is_a?(Color) ? args[8] : DEFAULT_DRAW_COLOR
                end
            end

            begin
                draw_rot_without_hash(*args)
            ensure
                shader.disable z if shader
            end
        end
    end
end