def fix(dir)
    for file in Dir.entries(dir) - [".", ".."]
        f = dir + "/" + file
        if Dir.exist?(f)
            fix(f)
        else
            if not f.end_with?(".c") and not f.end_with?(".h") and not f.end_with?(".y")
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

def fix2(root, dir = nil)
    if not dir
        dir = root
    end

    for file in Dir.entries(dir) - [".", ".."]
        f = dir + "/" + file
        if Dir.exist?(f)
            fix2(f)
        else
            if not f.end_with?(".c") and not f.end_with?(".h") and not f.end_with?(".y")
                next
            end

            lines = File.readlines(f)
            lines.each.with_index do |line, i|
                if line.start_with?("#include \"mruby")
                    lines[i] = +"../" * [dir.count("/") - 2, 0].max + line
                end
            end

            ff = File.open(f, "w")
            ff.puts(lines.join)
        end
    end
end

fix("modules/godosu")
fix2("modules/godosu")