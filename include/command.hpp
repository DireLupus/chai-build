#include <functional>
#include <string>
#include <map>
#include <optional>
#include <filesystem>
#include <vector>
#include <mutex>

namespace command 
{
	std::optional<std::filesystem::path> find_build_folder();

	void add_command_option(std::string command, std::function<void()> command_function);
	void add_command_option(std::string command, std::function<void(std::string)> command_function);
	void add_command_option(std::string command, std::function<void(std::string, std::string)> command_function);

	void parse_commands(int argc, char* argv[]);

	void handle_null_arg_command(std::string command);
	void handle_help();

	void handle_one_arg_command(std::string command, std::string arg);
	void handle_init(std::string project_name);
	void handle_info(std::string project_name);
	void handle_reset(std::string project_name);
	void handle_build(std::string project_name);

	void handle_two_arg_command(std::string first_arg, std::string command, std::string second_arg);
	void handle_run(std::string project_name, std::string args);
	void handle_debug(std::string project_name, std::string args);
	void handle_copy_to(std::string existing_project, std::string new_project);
	void handle_copy_from(std::string new_project, std::string existing_project);
	void handle_rename(std::string existing_project, std::string new_name);
	void handle_add_library(std::string existing_project, std::string path);
	void handle_add_header_directory(std::string existing_project, std::string path);
	void handle_add_source_directory(std::string existing_project, std::string path);
	void handle_add_compile_flag(std::string existing_project, std::string flag);
	void handle_remove_library(std::string existing_project, std::string path);
	void handle_remove_header_directory(std::string existing_project, std::string path);
	void handle_remove_source_directory(std::string existing_project, std::string path);
	void handle_remove_compile_flag(std::string existing_project, std::string flag);
}
