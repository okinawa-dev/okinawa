-- okinawa.cpp - xmake build
--
-- Build:        xmake
-- Run tests:    xmake run okinawa_test   (or: xmake test)
-- Coverage:     xmake coverage           (custom task, llvm source-based)
--
-- Third-party dependencies are fetched and built by xrepo (xmake's package
-- manager); there is no separate "install dependencies" step.

set_project("okinawa")
set_version("0.1.0")

set_languages("cxx17")
set_warnings("all")                 -- -Wall
add_cxflags("-Wundef", "-Wmacro-redefined", "-Wextra-semi", {tools = {"clang", "gcc"}})
set_symbols("debug")                -- always emit debug info (-g)

-- Keep compile_commands.json in sync for clangd / tooling.
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".", lsp = "clangd"})

-- Build modes. Debug is the default; `xmake f -m release` for release.
add_rules("mode.debug", "mode.release")
set_defaultmode("debug")

-- Third-party dependencies (resolved from xrepo). Must live in the root scope.
add_requires("glm")
add_requires("glfw")
add_requires("stb")
add_requires("opengl")
add_requires("catch2")              -- only used by the test target

-- =========================================================================
-- Engine static library
-- =========================================================================
target("okinawa")
    set_kind("static")
    add_files("src/**.cpp")
    -- Public include root: the engine and its consumers use it as the base
    -- for header paths. (Consumer "okinawa/..." prefix is handled when the
    -- consumers are migrated; the engine itself uses relative includes.)
    add_includedirs("src", {public = true})
    add_packages("glm", "glfw", "stb", "opengl", {public = true})

    -- macOS windowing/runtime frameworks required by GLFW.
    if is_plat("macosx") then
        add_frameworks("Cocoa", "IOKit", "CoreVideo", "OpenGL")
    end

-- =========================================================================
-- Test suite (Catch2)
-- =========================================================================
target("okinawa_test")
    set_kind("binary")
    set_default(false)              -- not built by a bare `xmake`; build explicitly
    add_files("tests/*.cpp")
    add_includedirs("tests")
    add_deps("okinawa")
    add_packages("catch2")
    -- Tests read data files by project-relative paths (e.g. tests/test-file.txt).
    set_rundir("$(projectdir)")
    add_tests("default")

-- =========================================================================
-- Coverage task: llvm source-based coverage over the test run.
--   xmake coverage   ->  HTML report in ./coverage/index.html
-- =========================================================================
task("coverage")
    set_menu({usage = "xmake coverage", description = "Run tests and generate an llvm-cov HTML report"})
    on_run(function ()
        import("core.project.project")
        os.exec("xmake build okinawa_test")

        local profraw  = "build/okinawa.profraw"
        local profdata = "build/okinawa.profdata"
        local testbin  = project.target("okinawa_test"):targetfile()
        local libbin   = project.target("okinawa"):targetfile()

        os.tryrm(profraw)
        os.tryrm(profdata)
        os.mkdir("coverage")

        os.execv(testbin, {"--reporter=console"}, {envs = {LLVM_PROFILE_FILE = profraw}})
        os.exec("llvm-profdata merge -sparse %s -o %s", profraw, profdata)
        os.exec("llvm-cov show %s -instr-profile=%s -format=html " ..
                "-show-line-counts-or-regions -show-instantiations -show-expansions " ..
                "-use-color -output-dir=coverage -ignore-filename-regex=.*tests/.* ",
                libbin, profdata)
        print("Coverage report generated in coverage/index.html")
    end)
