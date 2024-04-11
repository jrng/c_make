# c_make

`c_make` is a single header build system that let's you specify your build process
as a c or c++ program.

### Why use c as a build system?

The advantages of using c as the foundation for a build systems are:

  - No extra dependencies. You only need a c/c++ compiler which you need anyway.
  - The ability to include your existing code base. That gives you access to all the
    code and systems you developed. With that you have a much tighter integration of your
    build system with the rest of your project.
  - Writing a your build code in the same language that your project is written in.
    It's the language you probably know better than any build generator language.
    So getting the build system to do the stuff you need should be way easier.

### How to build a c_make project?

To build a c_make base project you have to first bootstrap the c_make executable
by compiling the `c_make.c` or `c_make.cpp` file with the c/c++ compiler of your choice.
After that you have to setup up a build directory by running `c_make setup <build-dir>`.
With that you can build the project: `c_make build <build-dir>`.

#### Linux, macOS, Android

```shell
$ cc -o c_make c_make.c  # only needs to happen once
$ ./c_make setup build
$ ./c_make build build
```

#### Windows

```shell
$ cl -Fec_make.exe c_make.c  # only needs to happen once
$ c_make setup build
$ c_make build build
```

### The c_make wrapper

To help with bootstrapping and building c_make based projects there is a c_make wrapper executable
that you can install into your system. It can be build by running `c_make` on this repo.
You then need to put the resulting executable in some directory, that's in your `$PATH` environment variable.

#### Linux, macOS, Android

```shell
$ cc -o c_make c_make.c  # only needs to happen once
$ ./c_make setup build
$ ./c_make build build
$ (sudo) install -m 755 build/c_make /usr/local/bin/c_make
```

After that you can just start calling `c_make` in a c_make based project.
The executable will try to bootstrap the c_make in the project and pass through the parameters.

### How to use c_make?

To start using c_make you need to first copy the `c_make.h` file into your project.
Then create a `c_make.c` or `c_make.cpp` file in your project root folder. This will be
the entry file for your build program. A minimal build file that compiles
a `hello_world.c` file looks like this:

```c
#define C_MAKE_IMPLEMENTATION
#include "c_make.h" // this depends on where you put the header file

C_MAKE_SETUP_ENTRY()
{
}

C_MAKE_BUILD_ENTRY()
{
    CMakeCommand command = { 0 };

    const char *target_c_compiler = c_make_get_target_c_compiler();

    c_make_command_append(&command, target_c_compiler);
    c_make_command_append_command_line(&command, c_make_get_target_c_flags());

    if (c_make_compiler_is_msvc(target_c_compiler))
    {
        c_make_command_append(&command, "-nologo");
        c_make_command_append(&command, c_make_c_string_concat("-Fe\"", c_make_c_string_path_concat(c_make_get_build_path(), "hello_world.exe"), "\""));
        c_make_command_append(&command, c_make_c_string_path_concat(c_make_get_source_path(), "hello_world.c"));
    }
    else
    {
        c_make_command_append(&command, "-o", c_make_c_string_path_concat(c_make_get_build_path(), "hello_world"));
        c_make_command_append(&command, c_make_c_string_path_concat(c_make_get_source_path(), "hello_world.c"));
    }

    c_make_command_run_and_wait(command);
}
```
