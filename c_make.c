// C_MAKE_COMPILER_FLAGS = "-std=c99 -Wall -Wextra -pedantic"
#define C_MAKE_IMPLEMENTATION
#include "c_make.h"

C_MAKE_INFO(commands_info, configs_info)
{
    add_info(commands_info, StringLiteral("install"), StringLiteral("Run the install target on the given build directory."));
    add_info(commands_info, StringLiteral("check-readme"), StringLiteral("This compiles the examples from the README.md\nto see if they are working."));

    add_default_info(commands_info, configs_info);
}

C_MAKE_ENTRY(command, argument_count, arguments)
{
    (void) argument_count;
    (void) arguments;

    if (strings_are_equal(command, COMMAND_BUILD))
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
    }
    else if (strings_are_equal(command, StringLiteral("check-readme")))
    {
        String readme;

        if (!read_entire_file(c_string_path_concat(get_source_path(), "README.md"), &readme))
        {
            set_failed(true);
            return;
        }

        size_t code_index = 0;

        const String code_start = StringLiteral("```c");
        const String code_end = StringLiteral("```");

        size_t index = string_find(readme, code_start);

        while (index < readme.count)
        {
            size_t advance = index + code_start.count;

            readme.count -= advance;
            readme.data += advance;

            string_split_left(&readme, '\n');

            index = string_find(readme, code_end);

            String code = readme;
            code.count = index;

            write_entire_file(c_string_path_concat(get_build_path(), c_string_formated("readme%zu.c", code_index)), code);

            Command command = { 0 };

            const char *target_c_compiler = get_target_c_compiler();

            command_append(&command, target_c_compiler);
            command_append_command_line(&command, get_target_c_flags());
            command_append_default_compiler_flags(&command, get_build_type());
            command_append(&command, c_string_concat("-I", get_source_path()));

            if (!compiler_is_msvc(target_c_compiler))
            {
                command_append(&command, "-std=c99", "-Wall", "-Wextra", "-pedantic");
            }

            command_append_output_executable(&command, c_string_path_concat(get_build_path(), c_string_formated("readme%zu", code_index)), get_target_platform());
            command_append(&command, c_string_path_concat(get_build_path(), c_string_formated("readme%zu.c", code_index)));
            command_append_default_linker_flags(&command, get_target_architecture());

            if ((get_target_platform() == PlatformWindows) && !compiler_is_msvc(target_c_compiler))
            {
                command_append(&command, "-lole32", "-loleaut32", "-ladvapi32");
            }

            c_make_log(LogLevelInfo, "compile 'readme%zu'\n", code_index);
            command_run_and_reset_and_wait(&command);

            code_index += 1;
            index = string_find(readme, code_start);
        }
    }
    else if (strings_are_equal(command, StringLiteral("install")))
    {
        copy_file(c_string_path_concat(get_build_path(), "c_make"), c_string_path_concat(get_install_prefix(), "bin", "c_make"));
    }
    else
    {
        handle_default_commands(command, argument_count, arguments);
    }
}
