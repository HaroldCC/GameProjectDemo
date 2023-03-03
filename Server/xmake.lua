add_requires("spdlog", "doctest")

target("Common")
    set_kind("binary")
    add_packages("spdlog", "doctest")
    add_files("src/Common/*.cpp", "src/Common/*.ixx")
