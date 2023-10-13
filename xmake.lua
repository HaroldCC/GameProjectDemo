set_version("0.0.1", {build = "%Y%m%d%H%m"})

set_warnings("all", "error")
set_languages("c++20")

set_targetdir("Bin/$(plat)-$(arch)-$(mode)")

----------------------------------------------------------
--- @brief debug模式设置
----------------------------------------------------------
rule("Debug")
    on_load(function (target) 
        if is_mode("debug") then
            target:add("defines", "DEBUG", "PERFORMANCE_DECT")
            target:set("suffixname", "_d")
            target:set("symbols", "debug")
            target:set("optimize", "none")
        end

        if target:is_plat("windows") then
            target:add("defines", "WIN32")
            target:add("cxxflags", "cl::/wd4819")
        end

        if target:is_plat("windows") then
            target:add("defines", "_CRT_SECURE_NO_WARNINGS")
        end
    end)
rule_end()

----------------------------------------------------------
--- @brief 添加Profiler
----------------------------------------------------------
rule("Profiler")
    on_load(function (target) 
        if is_mode("debug") then
            target:add("files", "$(projectdir)/tools/profiler/tracy/public/TracyClient.cpp")
            target:add("defines", "TRACY_ENABLE")
        end
    end)
rule_end()

add_rules("mode.release", "mode.debug")
add_rules("plugin.vsxmake.autoupdate")
add_rules("plugin.compile_commands.autoupdate", {outputdir="$(projectdir)/.vscode"})


includes("Server")
