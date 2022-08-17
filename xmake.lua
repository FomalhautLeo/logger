add_rules("mode.debug", "mode.release")

set_languages("c++20")

target("logger")
    set_kind("static")
    set_targetdir("$(projectdir)/install/lib")
    add_includedirs("$(projectdir)")
    add_syslinks("pthread")
    add_linkdirs("dep/fmt/lib")
    add_links("fmt")
    add_files("src/log_server.cc")
    add_files("src/logger.cc")

includes("test")