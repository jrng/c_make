// C_MAKE_COMPILER_FLAGS = "-std=c99 -Wall -Wextra -pedantic"
#define C_MAKE_IMPLEMENTATION
#include "c_make.h"

C_MAKE_ENTRY()
{
    switch (c_make_target)
    {
        case TargetSetup:
        {
        } break;

        case TargetBuild:
        {
            Command command = { 0 };

            const char *target_c_compiler = get_target_c_compiler();

            command_append(&command, target_c_compiler);
            command_append_command_line(&command, get_target_c_flags());
            command_append_default_compiler_flags(&command, get_build_type());

            if (!compiler_is_msvc(target_c_compiler))
            {
                command_append(&command, "-std=c99", "-Wall", "-Wextra", "-pedantic");
            }

            command_append_output_executable(&command, c_string_path_concat(get_build_path(), "c_make"), get_target_platform());
            command_append(&command, c_string_path_concat(get_source_path(), "c_make_wrapper.c"));
            command_append_default_linker_flags(&command, get_target_architecture());

            if ((get_target_platform() == PlatformWindows) && !compiler_is_msvc(target_c_compiler))
            {
                command_append(&command, "-lole32", "-loleaut32", "-ladvapi32");
            }

            c_make_log(LogLevelInfo, "compile 'c_make_wrapper'\n");
            command_run(command);
        } break;

        case TargetInstall:
        {
            copy_file(c_string_path_concat(get_build_path(), "c_make"), c_string_path_concat(get_install_prefix(), "bin", "c_make"));
        } break;
    }
}
