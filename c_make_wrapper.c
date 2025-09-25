#define C_MAKE_IMPLEMENTATION
#define C_MAKE_NO_ENTRY_POINT
#include "c_make.h"

int main(int argument_count, char **arguments)
{
    for (int i = 3; i < argument_count; i += 1)
    {
        String argument = CString(arguments[i]);

        if (strings_are_equal(argument, StringLiteral("--verbose")))
        {
            _c_make_context.verbose = true;
        }
    }

#if PLATFORM_WINDOWS
    const char *c_make_executable_file = "c_make.exe";
#else
    const char *c_make_executable_file = "c_make";
#endif

    char *c_make_include_path = 0;

    Command command = { 0 };

    if (!file_exists(c_make_executable_file))
    {
        for (int retry = 0; retry < 2; retry += 1)
        {
            command.count = 0;

            if (file_exists("c_make.cpp") || file_exists("c_make.c"))
            {
                const char *compiler = 0;
                const char *c_make_source_file = 0;

                if (file_exists("c_make.cpp"))
                {
                    compiler = get_host_cpp_compiler();
                    c_make_source_file = "c_make.cpp";
                }
                else if (file_exists("c_make.c"))
                {
                    compiler = get_host_c_compiler();
                    c_make_source_file = "c_make.c";
                }
                else
                {
                    break;
                }

                command_append(&command, compiler);

                if (compiler_is_msvc(compiler))
                {
                    command_append_msvc_compiler_flags(&command);
                    command_append(&command, "-nologo");
                }

                if (c_make_include_path)
                {
                    command_append(&command, c_string_concat("-I", c_make_include_path));
                    command_append(&command, c_string_concat("-DC_MAKE_INCLUDE_PATH=", c_make_include_path));
                }

                size_t memory_saved = memory_save();

                String source_content = { 0 };

                if (read_entire_file(c_make_source_file, &source_content))
                {
                    bool had_msvc_flags = false;
                    bool had_compiler_flags = false;

                    for (int line_index = 0; (line_index < 3) && source_content.count; line_index += 1)
                    {
                        String line = string_split_left(&source_content, '\n');
                        size_t index = string_find(line, StringLiteral("C_MAKE_COMPILER_FLAGS"));

                        if (index < line.count)
                        {
                            String argument;
                            argument.count = line.count - index;
                            argument.data  = line.data + index;

                            String key = string_trim(string_split_left(&argument, '='));
                            String compiler_flags = string_trim(argument);

                            if (!had_compiler_flags && strings_are_equal(key, StringLiteral("C_MAKE_COMPILER_FLAGS")) &&
                                !compiler_is_msvc(compiler))
                            {
                                if ((compiler_flags.count > 0) && (compiler_flags.data[0] == '"'))
                                {
                                    compiler_flags.count -= 1;
                                    compiler_flags.data += 1;
                                }

                                if ((compiler_flags.count > 0) && (compiler_flags.data[compiler_flags.count - 1] == '"'))
                                {
                                    compiler_flags.count -= 1;
                                }

                                compiler_flags = string_trim(compiler_flags);

                                command_append_command_line(&command, string_to_c_string(compiler_flags));
                                command_append(&command, c_string_concat("-DC_MAKE_COMPILER_FLAGS=", string_to_c_string(compiler_flags)));

                                had_compiler_flags = true;
                            }

                            if (!had_msvc_flags && strings_are_equal(key, StringLiteral("C_MAKE_COMPILER_FLAGS.msvc")) &&
                                compiler_is_msvc(compiler))
                            {
                                if ((compiler_flags.count > 0) && (compiler_flags.data[0] == '"'))
                                {
                                    compiler_flags.count -= 1;
                                    compiler_flags.data += 1;
                                }

                                if ((compiler_flags.count > 0) && (compiler_flags.data[compiler_flags.count - 1] == '"'))
                                {
                                    compiler_flags.count -= 1;
                                }

                                compiler_flags = string_trim(compiler_flags);

                                command_append_command_line(&command, string_to_c_string(compiler_flags));
                                command_append(&command, c_string_concat("-DC_MAKE_COMPILER_FLAGS=", string_to_c_string(compiler_flags)));

                                had_msvc_flags = true;
                            }
                        }
                    }
                }

                memory_restore(memory_saved);

                command_append_output_executable(&command, "c_make", get_host_platform());
                command_append(&command, c_make_source_file);
                command_append_default_linker_flags(&command, get_host_architecture());

                c_make_log(LogLevelInfo, "bootstrap c_make from %s\n", c_make_source_file);

                if (command_run_and_wait(command))
                {
                    break;
                }
                else
                {
                    c_make_log(LogLevelError, "bootstrapping did not work. Maybe try setting the c_make include path.\n"
                                              "          C_MAKE_INCLUDE_PATH = ");

                    c_make_include_path = allocate(200);
                    c_make_include_path = fgets(c_make_include_path, 200, stdin);

                    if (c_make_include_path)
                    {
                        size_t length = get_c_string_length(c_make_include_path);

                        if ((length > 0) && (c_make_include_path[length - 1] == '\n'))
                        {
                            c_make_include_path[length - 1] = 0;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else
            {
                c_make_log(LogLevelError, "this project does not use c_make.\n");
                c_make_log(LogLevelRaw, "please provide a c_make.c or c_make.cpp file.\n");
                return 0;
            }
        }
    }

    if (file_exists(c_make_executable_file))
    {
        command.count = 0;

        command_append(&command, c_string_path_concat(".", c_make_executable_file));
        command_append_slice(&command, argument_count - 1, (const char **) (arguments + 1));
        command_run_and_wait(command);
    }

    return _c_make_context.did_fail ? 1 : 0;
}
