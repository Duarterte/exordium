#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")

env.Append(CPPPATH=["src/", "/usr/include/sodium"])
env.Append(LIBPATH=["/lib/x86_64-linux-gnu"])

env.Append(LIBS=["sodium"])

sources = Glob("src/*.cpp")

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "exo_chacha/bin/Exo_Chacha.{}.{}.framework/helloWorld.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "exo_chacha/bin/Exo_ChaCha{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)