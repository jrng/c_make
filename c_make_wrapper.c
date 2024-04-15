#define C_MAKE_IMPLEMENTATION
#define C_MAKE_NO_ENTRY_POINT
#include "c_make.h"

int main(int argument_count, char **arguments)
{
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

                if (c_make_include_path)
                {
                    c_make_command_append(&command, c_make_c_string_concat("-I", c_make_include_path));
                    c_make_command_append(&command, c_make_c_string_concat("-DC_MAKE_INCLUDE_PATH=", c_make_include_path));
                }

                // TODO: parse bootstrapping flags from c_make.cpp

                if (c_make_compiler_is_msvc(compiler))
                {
                    c_make_command_append(&command, "-nologo");
                    c_make_command_append(&command, c_make_c_string_concat("-Fe", c_make_executable_file), c_make_source_file);
                }
                else
                {
                    c_make_command_append(&command, "-o", c_make_executable_file, c_make_source_file);
                }

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

    return 0;
}
