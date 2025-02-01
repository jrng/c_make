// C_MAKE_COMPILER_FLAGS = "-std=c99 -Wall -Wextra -pedantic"
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
            c_make_command_append_default_compiler_flags(&command, c_make_get_build_type());

            if (!c_make_compiler_is_msvc(target_c_compiler))
            {
                c_make_command_append(&command, "-std=c99", "-Wall", "-Wextra", "-pedantic");
            }

            c_make_command_append_output_executable(&command, c_make_c_string_path_concat(c_make_get_build_path(), "c_make"), c_make_get_target_platform());
            c_make_command_append(&command, c_make_c_string_path_concat(c_make_get_source_path(), "c_make_wrapper.c"));
            c_make_command_append_default_linker_flags(&command, c_make_get_target_architecture());

            if ((c_make_get_target_platform() == CMakePlatformWindows) && !c_make_compiler_is_msvc(target_c_compiler))
            {
                c_make_command_append(&command, "-lole32", "-loleaut32", "-ladvapi32");
            }

            c_make_log(CMakeLogLevelInfo, "compile 'c_make_wrapper'\n");
            c_make_command_run(command);
        } break;

        case CMakeTargetInstall:
        {
            c_make_copy_file(c_make_c_string_path_concat(c_make_get_build_path(), "c_make"), c_make_c_string_path_concat(c_make_get_install_prefix(), "bin", "c_make"));
        } break;
    }
}
