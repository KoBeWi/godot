def fix(dir)
    for file in Dir.entries(dir) - [".", ".."]
        f = dir + "/" + file
        if Dir.exist?(f)
            fix(f)
        else
            if not f.end_with?(".c") and not f.end_with?(".h")
                next
            end

            lines = File.readlines(f)
            for line in lines
                if line.start_with?("#include <mruby")
                    line.gsub!("<", "\"").gsub!(">", "\"")
                end
            end

            ff = File.open(f, "w")
            ff.puts(lines.join)
        end
    end
end

fix("modules/godosu")