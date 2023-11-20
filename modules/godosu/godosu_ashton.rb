module Gosu
    class Image
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
            $window.gl z do
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
            $window.gl z do
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