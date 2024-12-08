#define C_MAKE_IMPLEMENTATION
#define C_MAKE_NO_ENTRY_POINT
#include "c_make.h"

int main(int argument_count, char **arguments)
{
    for (int i = 3; i < argument_count; i += 1)
    {
        CMakeString argument = CMakeCString(arguments[i]);

        if (c_make_strings_are_equal(argument, CMakeStringLiteral("--verbose")))
        {
            _c_make_context.verbose = true;
        }
    }

#if C_MAKE_PLATFORM_WINDOWS
    const char *c_make_executable_file = "c_make.exe";
#else
    const char *c_make_executable_file = "c_make";
#endif

    char *c_make_include_path = 0;

    CMakeCommand command = { 0 };

    if (!c_make_file_exists(c_make_executable_file))
    {
        for (int retry = 0; retry < 2; retry += 1)
        {
            command.count = 0;

            if (c_make_file_exists("c_make.cpp") || c_make_file_exists("c_make.c"))
            {
                const char *compiler = 0;
                const char *c_make_source_file = 0;

                if (c_make_file_exists("c_make.cpp"))
                {
                    compiler = c_make_get_host_cpp_compiler();
                    c_make_source_file = "c_make.cpp";
                }
                else if (c_make_file_exists("c_make.c"))
                {
                    compiler = c_make_get_host_c_compiler();
                    c_make_source_file = "c_make.c";
                }
                else
                {
                    break;
                }

                c_make_command_append(&command, compiler);

                if (c_make_compiler_is_msvc(compiler))
                {
                    c_make_command_append_msvc_compiler_flags(&command);
                    c_make_command_append(&command, "-nologo");
                }

                if (c_make_include_path)
                {
                    c_make_command_append(&command, c_make_c_string_concat("-I", c_make_include_path));
                    c_make_command_append(&command, c_make_c_string_concat("-DC_MAKE_INCLUDE_PATH=", c_make_include_path));
                }

                size_t memory_saved = c_make_memory_save();

                CMakeString source_content = { 0 };

                if (c_make_read_entire_file(c_make_source_file, &source_content))
                {
                    bool had_msvc_flags = false;
                    bool had_compiler_flags = false;

                    for (int line_index = 0; (line_index < 3) && source_content.count; line_index += 1)
                    {
                        CMakeString line = c_make_string_split_left(&source_content, '\n');
                        size_t index = c_make_string_find(line, CMakeStringLiteral("C_MAKE_COMPILER_FLAGS"));

                        if (index < line.count)
                        {
                            CMakeString argument;
                            argument.count = line.count - index;
                            argument.data  = line.data + index;

                            CMakeString key = c_make_string_trim(c_make_string_split_left(&argument, '='));
                            CMakeString compiler_flags = c_make_string_trim(argument);

                            if (!had_compiler_flags && c_make_strings_are_equal(key, CMakeStringLiteral("C_MAKE_COMPILER_FLAGS")) &&
                                !c_make_compiler_is_msvc(compiler))
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

                                compiler_flags = c_make_string_trim(compiler_flags);

                                c_make_command_append_command_line(&command, c_make_string_to_c_string(&_c_make_context.public_memory, compiler_flags));
                                c_make_command_append(&command, c_make_c_string_concat("-DC_MAKE_COMPILER_FLAGS=", c_make_string_to_c_string(&_c_make_context.public_memory, compiler_flags)));

                                had_compiler_flags = true;
                            }

                            if (!had_msvc_flags && c_make_strings_are_equal(key, CMakeStringLiteral("C_MAKE_COMPILER_FLAGS.msvc")) &&
                                c_make_compiler_is_msvc(compiler))
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

                                compiler_flags = c_make_string_trim(compiler_flags);

                                c_make_command_append_command_line(&command, c_make_string_to_c_string(&_c_make_context.public_memory, compiler_flags));
                                c_make_command_append(&command, c_make_c_string_concat("-DC_MAKE_COMPILER_FLAGS=", c_make_string_to_c_string(&_c_make_context.public_memory, compiler_flags)));

                                had_msvc_flags = true;
                            }
                        }
                    }
                }

                c_make_memory_restore(memory_saved);

                c_make_command_append_output_executable(&command, "c_make", c_make_get_host_platform());
                c_make_command_append(&command, c_make_source_file);
                c_make_command_append_default_linker_flags(&command, c_make_get_host_architecture());

                c_make_log(CMakeLogLevelInfo, "bootstrap c_make from %s\n", c_make_source_file);

                if (c_make_command_run_and_wait(command))
                {
                    break;
                }
                else
                {
                    c_make_log(CMakeLogLevelError, "bootstrapping did not work. Maybe try setting the c_make include path.\n"
                                                   "          C_MAKE_INCLUDE_PATH = ");

                    c_make_include_path = c_make_allocate(200);
                    c_make_include_path = fgets(c_make_include_path, 200, stdin);

                    if (c_make_include_path)
                    {
                        size_t length = c_make_get_c_string_length(c_make_include_path);

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
                c_make_log(CMakeLogLevelError, "this project does not use c_make.\n");
                c_make_log(CMakeLogLevelRaw, "please provide a c_make.c or c_make.cpp file.\n");
                return 0;
            }
        }
    }

    if (c_make_file_exists(c_make_executable_file))
    {
        command.count = 0;

        c_make_command_append(&command, c_make_c_string_path_concat(".", c_make_executable_file));
        c_make_command_append_slice(&command, argument_count - 1, (const char **) (arguments + 1));
        c_make_command_run_and_wait(command);
    }

    return _c_make_context.did_fail ? 1 : 0;
}
