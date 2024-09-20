// C_MAKE_COMPILER_FLAGS="-std=c99 -Wall -Wextra -pedantic"
#define C_MAKE_IMPLEMENTATION
#include "c_make.h"

C_MAKE_ENTRY()
{
    switch (c_make_target)
    {
        case CMakeTargetSetup:
        {
        } break;

        case CMakeTargetBuild:
        {
            CMakeCommand command = { 0 };

            const char *target_c_compiler = c_make_get_target_c_compiler();

            c_make_command_append(&command, target_c_compiler);
            c_make_command_append_command_line(&command, c_make_get_target_c_flags());

            if (c_make_compiler_is_msvc(target_c_compiler))
            {
                c_make_command_append(&command, "-nologo");
                c_make_command_append(&command, c_make_c_string_concat("-Fe", c_make_c_string_path_concat(c_make_get_build_path(), "c_make.exe")));
                c_make_command_append(&command, c_make_c_string_concat("-Fo", c_make_c_string_path_concat(c_make_get_build_path(), "c_make.obj")));
                c_make_command_append(&command, c_make_c_string_path_concat(c_make_get_source_path(), "c_make_wrapper.c"));
            }
            else
            {
                c_make_command_append(&command, "-std=c99", "-Wall", "-Wextra", "-pedantic");
                c_make_command_append(&command, "-o", c_make_c_string_path_concat(c_make_get_build_path(), "c_make"));
                c_make_command_append(&command, c_make_c_string_path_concat(c_make_get_source_path(), "c_make_wrapper.c"));
            }

            c_make_log(CMakeLogLevelInfo, "compile 'c_make_wrapper'\n");
            c_make_command_run_and_wait(command);
        } break;

        case CMakeTargetInstall:
        {
            c_make_copy_file(c_make_c_string_path_concat(c_make_get_build_path(), "c_make"), c_make_c_string_path_concat(c_make_get_install_prefix(), "bin", "c_make"));
        } break;
    }
}
