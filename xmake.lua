set_version("0.0.1", {build = "%Y%m%d%H%m"})

set_warnings("all", "error")
set_languages("c++20")

set_targetdir("Bin/$(plat)-$(arch)-$(mode)")

add_requires("protobuf-cpp", "swig")

rule("CommonRule")
    on_load(function (target) 
        if is_mode("debug") then
            target:add("defines", "DEBUG", "PERFORMANCE_DECT")
            target:set("symbols", "debug")
            target:set("optimize", "none")
        end

        if target:is_plat("windows") then
            target:add("defines", "WIN32", "WIN32_LEAN_AND_MEAN")
            target:add("cxxflags", "cl::/wd4819")
            target:add("defines", "_CRT_SECURE_NO_WARNINGS")
        end

        if target:is_plat("windows") then
        end
    end)
rule_end()

add_rules("mode.release", "mode.debug")
add_rules("plugin.vsxmake.autoupdate")
add_rules("plugin.compile_commands.autoupdate", {outputdir="$(projectdir)/.vscode"})


includes("Src/Common")
includes("Src/Servers")
includes("3rdParty")
