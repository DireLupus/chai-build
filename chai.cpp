#include "./include/command.hpp"
#include <iostream>

int main(int argc, char* argv[]) 
{ 
    command::add_command_option(std::string("help"), command::handle_help);
    command::add_command_option(std::string("init"), command::handle_init);
    command::add_command_option(std::string("info"), command::handle_info);
    command::add_command_option(std::string("reset"), command::handle_reset);
    command::add_command_option(std::string("build"), command::handle_build);
    command::add_command_option(std::string("run"), command::handle_run);
    
    command::add_command_option(std::string("add_library"), command::handle_add_library);
    command::add_command_option(std::string("add_header_directory"), command::handle_add_header_directory);
    command::add_command_option(std::string("add_source_directory"), command::handle_add_source_directory);
    command::add_command_option(std::string("add_compile_flag"), command::handle_add_compile_flag);
    
    command::add_command_option(std::string("remove_library"), command::handle_remove_library);
    command::add_command_option(std::string("remove_header_directory"), command::handle_remove_header_directory);
    command::add_command_option(std::string("remove_source_directory"), command::handle_remove_source_directory);
    command::add_command_option(std::string("remove_compile_flag"), command::handle_remove_compile_flag);

    command::parse_commands(argc, argv);
    exit(0);
}
