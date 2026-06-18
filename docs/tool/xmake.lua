-- Standalone build for the okinawa documentation-site generator.
--
-- Kept separate from the engine's root xmake.lua on purpose: the generator
-- only needs md4c (Markdown -> HTML), never the engine or its OpenGL / GLFW /
-- GLEW packages. Building it from here means the CI deploy job (and anyone who
-- just wants to build the docs) pulls md4c alone, with no GL/X11 system
-- dependencies.
--
-- Build:  xmake -P docs/tool build okinawa-docs
-- Run:    xmake -P docs/tool run okinawa-docs          (uses the defaults below)
-- Tests:  xmake -P docs/tool run okinawa-docs-tests

set_languages("cxx17")
set_warnings("all")

-- Keep a compile_commands.json next to this project for clangd on the tool's
-- own sources (the engine's lives at the repo root).
add_rules("plugin.compile_commands.autoupdate", {outputdir = "$(projectdir)", lsp = "clangd"})

add_rules("mode.debug", "mode.release")
set_defaultmode("release")

add_requires("md4c")                -- Markdown -> HTML
add_requires("catch2")              -- only used by the tests target

-- Run the generator from the repo root (docs/tool is two levels down) so its
-- default --in/--out/--template/--static paths under docs/ resolve.
local repo_root = path.join(os.scriptdir(), "..", "..")

target("okinawa-docs")
    set_kind("binary")
    add_files("main.cpp", "docsgen.cpp")
    add_packages("md4c")
    set_rundir(repo_root)

target("okinawa-docs-tests")
    set_kind("binary")
    set_default(false)
    add_files("docsgen-test.cpp", "docsgen.cpp")
    add_packages("catch2", "md4c")
    set_rundir(repo_root)
    add_tests("docs")
