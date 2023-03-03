set_version("0.0.1", {build = "%Y%m%d%H%m"})

set_warnings("all", "error")
set_languages("c++20")

add_rules("mode.release", "mode.debug")

includes("Server")