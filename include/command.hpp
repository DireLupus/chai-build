#include <functional>
#include <string>
#include <map>
#include <optional>
#include <filesystem>
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>

class command 
{
    private :
        static std::map<std::string, std::function<void()>> null_arg_function_map;
        static std::map<std::string, std::function<void(std::string)>> one_arg_function_map;
        static std::map<std::string, std::function<void(std::string, std::string)>> two_arg_function_map; 
        
        static std::vector<std::string> find_all_files(std::vector<std::string> source_paths, std::vector<std::string> extensions);
        static std::vector<std::string> get_files_from_directory(std::filesystem::path source_path, std::vector<std::string>& extensions);
        static void append_path_vector(std::vector<std::string>& appendee, const std::vector<std::string>& appender);
        static std::string unpack_string_vector(const std::vector<std::string> unpackable, std::string decoration);
		
		template <class T>
		static bool thread_safe_vector_pop(T& assignable, std::vector<T>& popable, std::mutex& lock);

		static void thread_task_build_object(int thread_num, std::vector<std::string>& source_files, std::map<std::string, int>& file_hashstamps, std::mutex& queue_lock, std::mutex& timestamp_lock, std::string& object_build_format);
    public :
		static std::string format_build_command(const std::string& format)
		{
			return format;
		}
		
		template <typename T, typename... Types>
		static std::string format_build_command(const std::string& format, T first, Types... rest)
		{
			std::string edited = format.substr(0, format.find_first_of("{")) + std::string(first) + format.substr(format.find_first_of("}") + 1);
			return command::format_build_command(edited, rest...);
		}

        static std::optional<std::filesystem::path> find_build_folder();
        
        static void add_command_option(std::string command, std::function<void()> command_function);
        static void add_command_option(std::string command, std::function<void(std::string)> command_function);
        static void add_command_option(std::string command, std::function<void(std::string, std::string)> command_function);
        
        static void parse_commands(int argc, char* argv[]);
        
        static void handle_null_arg_command(std::string command);
        static void handle_help();
        
        static void handle_one_arg_command(std::string command, std::string arg);
        static void handle_init(std::string project_name);
        static void handle_info(std::string project_name);
        static void handle_reset(std::string project_name);
        static void handle_build(std::string project_name);
        
        static void handle_two_arg_command(std::string first_arg, std::string command, std::string second_arg);
        static void handle_run(std::string project_name, std::string args);
        static void handle_debug(std::string project_name, std::string args);
        static void handle_copy_to(std::string existing_project, std::string new_project);
        static void handle_copy_from(std::string new_project, std::string existing_project);
        static void handle_rename(std::string existing_project, std::string new_name);
        static void handle_add_library(std::string existing_project, std::string path);
        static void handle_add_header_directory(std::string existing_project, std::string path);
        static void handle_add_source_directory(std::string existing_project, std::string path);
        static void handle_add_compile_flag(std::string existing_project, std::string flag);
        static void handle_remove_library(std::string existing_project, std::string path);
        static void handle_remove_header_directory(std::string existing_project, std::string path);
        static void handle_remove_source_directory(std::string existing_project, std::string path);
        static void handle_remove_compile_flag(std::string existing_project, std::string flag);
};
