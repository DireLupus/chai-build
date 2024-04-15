#include "../include/command.hpp"
#include "../include/settings.hpp"
#include "../include/timestamp.hpp"
#include "../include/hashstamp.hpp"

#include <chrono>
#include <iostream>
#include <iterator>
#include <utility>
#include <set>
#include <fstream>
#include <sstream>
#include <queue>

std::map<std::string, std::function<void()>> command::null_arg_function_map;
std::map<std::string, std::function<void(std::string)>> command::one_arg_function_map;
std::map<std::string, std::function<void(std::string, std::string)>> command::two_arg_function_map;

std::optional<std::filesystem::path> command::find_build_folder()
{
    std::filesystem::path resulting_path = std::filesystem::absolute(std::filesystem::current_path());
    resulting_path.append(".chai");
    
    while(!std::filesystem::exists(resulting_path))
    {
        
        resulting_path = resulting_path.parent_path(); // Remove .chai()
        if(resulting_path == resulting_path.parent_path())
        {
            return std::nullopt;
        } else 
        {
            resulting_path = resulting_path.parent_path();
        }    
    
        resulting_path.append(".chai");
    }
    
    return std::optional<std::filesystem::path>(resulting_path);
}

std::vector<std::string> command::find_all_files(std::vector<std::string> source_paths, std::vector<std::string> extensions)
{
  std::vector<std::string> file_paths;
  
  std::filesystem::path temp_path;
  for(const std::string& string_path : source_paths)
  {    
	if(string_path != "")
	{
		temp_path = std::filesystem::absolute(string_path);
		if(std::filesystem::is_directory(temp_path))
		{
		command::append_path_vector(file_paths, command::get_files_from_directory(temp_path, extensions)); 
		} else if(std::filesystem::is_regular_file(temp_path))
		{
			if(temp_path.has_extension() && std::find(extensions.begin(), extensions.end(), temp_path.extension()) != extensions.end())
			{
				file_paths.push_back(temp_path.string());
			}
		}
	}
  }
  
  return file_paths;
}

std::vector<std::string> command::get_files_from_directory(std::filesystem::path source_path, std::vector<std::string>& extensions)
{
    std::vector<std::string> file_paths;
    
    for(const std::filesystem::path subpath : std::filesystem::directory_iterator(source_path))
    {
        if(std::filesystem::is_directory(subpath))
        {
            command::append_path_vector(file_paths, command::get_files_from_directory(subpath, extensions));
        } else if(std::filesystem::is_regular_file(subpath))
        {
            if(subpath.has_extension() && std::find(extensions.begin(), extensions.end(), subpath.extension()) != extensions.end())
            {
                file_paths.push_back(subpath.string());
            }
        }
    }
    
    return file_paths;
}

std::string command::unpack_string_vector(const std::vector<std::string> unpackable, std::string decoration = "")
{
    std::string returnable = "";
    
    for(const std::string& unpack : unpackable)
    {
        if(unpack != "")
        {
            returnable += decoration + unpack + " ";
        }
    }
    
    return returnable.substr(0, returnable.length() - 1);
}

void command::append_path_vector(std::vector<std::string>& appendee, const std::vector<std::string>& appender)
{
    appendee.insert(appendee.end(), std::make_move_iterator(appender.begin()), std::make_move_iterator(appender.end()));
}

void command::parse_commands(int argc, char* argv[])
{
    switch(argc) 
    {
        case 2 : 
            return handle_null_arg_command(std::string(argv[1]));
        case 3 :
            return handle_one_arg_command(std::string(argv[1]), std::string(argv[2]));
        case 4 :
            return handle_two_arg_command(std::string(argv[1]), std::string(argv[2]), std::string(argv[3]));
        default :
            std::cerr << "Incorrect number of arguments! Please use the 'chai help' command to view proper command formatting!" << std::endl;
            return;
    }
}

template <class T>
bool command::thread_safe_vector_pop(T& assignable, std::vector<T>& popable, std::mutex& lock)
{
	lock.lock();
	if(popable.empty())
	{
		lock.unlock();
		return false;
	} else 
	{
		assignable = std::string(popable.back());
		popable.pop_back();
	}

	lock.unlock();
	return true;
}

void command::thread_task_build_object(int thread_num, std::vector<std::string>& source_files, std::map<std::string, int>& file_hashstamps, std::mutex& queue_lock, std::mutex& timestamp_lock, std::string& object_build_format)
{
	std::string source_file;
	std::string hash_command;
	std::string object_command;
	std::string hashable;
	std::string file_name = "";
	std::ostringstream buffer;
	std::fstream hash_file;

	int hash = 0;
	std::filesystem::path temp_file = std::filesystem::absolute(std::filesystem::current_path()).append("temp").append("chai_temp_" + std::to_string(thread_num));
	while(command::thread_safe_vector_pop<std::string>(source_file, source_files, queue_lock))
	{						
		file_name = std::filesystem::absolute(source_file).filename().string();
					
		// Compile to temp file
		hash_command = "g++ -E " + std::filesystem::absolute(source_file).string() + " > " +  temp_file.string();
		system(hash_command.c_str());

		hash_file.open(temp_file);
		if(hash_file) 
		{
			buffer.clear();
			buffer << hash_file.rdbuf();
			hashable = buffer.str();
		}
		hash_file.close();

		hash = std::hash<std::string>{}(hashable);
		timestamp_lock.lock();
		if(!std::filesystem::exists(std::filesystem::absolute(std::filesystem::current_path()).append(file_name.substr(0, file_name.find(".")) + ".o")) 
			|| file_hashstamps.count(source_file) == 0 
			|| file_hashstamps.at(source_file) != hash)
		{
			timestamp_lock.unlock();
			
			object_command = command::format_build_command(object_build_format, source_file);

			system(object_command.c_str());

			timestamp_lock.lock();
			file_hashstamps.insert_or_assign(source_file, hash);
		}
		timestamp_lock.unlock();
	}
}

void command::add_command_option(std::string command, std::function<void()> command_function)
{
    command::null_arg_function_map.insert(std::pair(command, command_function));
}

void command::add_command_option(std::string command, std::function<void(std::string)> command_function)
{
    command::one_arg_function_map.insert(std::pair(command, command_function));
}

void command::add_command_option(std::string command, std::function<void(std::string, std::string)> command_function)
{
    command::two_arg_function_map.insert(std::pair(command, command_function));
}

void command::handle_null_arg_command(std::string command)
{
    std::function<void()> runnable = [&]() {
        std::cerr << "The command \'" << command << "\' is not a supported command. Please run \'chai help\' for a list of supported commands!" << std::endl;
        return;
    };
    
    if(command::null_arg_function_map.count(command))
    {
        runnable = command::null_arg_function_map.at(command);
    }
    
    return runnable();
}

void command::handle_help()
{
    std::cout << "Welcome to chai, a c++ project build tool!" << std::endl;
    std::cout << "Here are all of the available commands currently supported: " << std::endl;
    std::cout << "[x] help" << std::endl;
    std::cout << "[x] init project_name" << std::endl;
    std::cout << "[x] info project_name" << std::endl;
    std::cout << "[x] reset project_name" << std::endl;
    std::cout << "[x] build project_name" << std::endl;
    std::cout << "[x] run project_name" << std::endl;
    std::cout << "[ ] debug project_name" << std::endl;
    std::cout << "[ ] existing_project copy_to new_project" << std::endl;
    std::cout << "[ ] new_project copy_from existing_project" << std::endl;
    std::cout << "[ ] project_name rename new_name" << std::endl; 
    std::cout << "[x] project_name add_library path" << std::endl;
    std::cout << "[x] project_name add_source_directory path" << std::endl;
    std::cout << "[x] project_name add_header_directory path" << std::endl;
    std::cout << "[x] project_name add_compile_flags path" << std::endl;
    std::cout << "[x] project_name remove_library path" << std::endl;
    std::cout << "[x] project_name remove_source_directory path" << std::endl; 
    std::cout << "[x] project_name remove_header_directory path" << std::endl;
    std::cout << "[x] project_name remove_compile_flags path" << std::endl;
    
    return;
}

void command::handle_one_arg_command(std::string command, std::string arg) 
{
    std::function<void(std::string)> runnable = [&](std::string) {
        std::cerr << "The command \'" << command << " " << arg << "\' is not a supported command. Please run \'chai help\' for a list of supported commands!" << std::endl;
        return;
    };
    
    if(command::one_arg_function_map.count(command))
    {
        runnable = command::one_arg_function_map.at(command);
    }
    
    return runnable(arg);
}

void command::handle_init(std::string project_name) 
{
    std::optional<std::filesystem::path> chai_path = command::find_build_folder();
    std::filesystem::path working_path;
    
    if(chai_path.has_value())
    {
        // TODO create new project
        working_path = chai_path.value();
        working_path.append("projects/" + project_name);
        std::filesystem::create_directory(working_path);
        working_path.append("build");
        std::filesystem::create_directory(working_path);
        working_path.append("executable");
        std::filesystem::create_directory(working_path);
        working_path = working_path.parent_path();
        working_path.append("objects");
        std::filesystem::create_directory(working_path);
        
        command::handle_reset(project_name);
    } else 
    {
        // TODO create new build, then recall
        working_path = std::filesystem::absolute(std::filesystem::current_path());
        working_path.append(".chai");
        std::filesystem::create_directory(working_path);
        working_path.append("cache");
        std::filesystem::create_directory(working_path);
        working_path = working_path.parent_path();
        working_path.append("projects");
        std::filesystem::create_directory(working_path);
        return command::handle_init(project_name);
    }
}

void command::handle_info(std::string project_name) 
{
    std::optional<std::filesystem::path> chai_path = command::find_build_folder();
    std::filesystem::path project_layout_path = chai_path.value().append("projects/" + project_name + "/project_layout");
    
    std::map<std::string, std::vector<std::string>> project_layout = settings::read_from_file(project_layout_path);
    
    std::cout << "This is the current layout of project " << project_name << ": " << std::endl;
    for(const auto& [key, value] : project_layout)
    {
        std::cout << "  " << key << ":";
        for(std::string str : value) 
        {
            std::cout << " " << str ;
        }
        std::cout << std::endl;
    }
}

void command::handle_reset(std::string project_name) 
{
    std::optional<std::filesystem::path> chai_path = command::find_build_folder();
    // TODO add check for nullopt
    std::filesystem::path project_layout_path = chai_path.value().append("projects/" + project_name + "/project_layout");
    
    std::map<std::string, std::vector<std::string>> default_project_layout;
    
    // TODO make easier default;
    default_project_layout.insert(std::make_pair("compiler", std::vector<std::string>({"g++"})));
    default_project_layout.insert(std::make_pair("flags", std::vector<std::string>()));
    default_project_layout.insert(std::make_pair("libraries", std::vector<std::string>()));
    default_project_layout.insert(std::make_pair("headers", std::vector<std::string>()));
    default_project_layout.insert(std::make_pair("sources", std::vector<std::string>()));
    default_project_layout.insert(std::make_pair("standard", std::vector<std::string>()));
	default_project_layout.insert(std::make_pair("threads", std::vector<std::string>({"8"})));

    settings::write_to_file(default_project_layout, project_layout_path);
}

// TODO make objects folder mimic subdirectories of src folder
void command::handle_build(std::string project_name) 
{
    std::optional<std::filesystem::path> chai_path = command::find_build_folder();

    if(!chai_path.has_value())
    {
        std::cerr << "O h F u c k" << std::endl;
        return;
    }
    
    std::filesystem::path project_layout_path = chai_path.value().append("projects/" + project_name + "/project_layout");
    
    std::map<std::string, std::vector<std::string>> project_layout = settings::read_from_file(project_layout_path);
    std::map<std::string, std::chrono::nanoseconds> file_timestamps = timestamp::read_from_file();
	std::map<std::string, int> file_hashstamps = hashstamp::read_from_file();

    // TODO specify source files
    std::vector<std::string> source_files = command::find_all_files(project_layout.at("sources"), std::vector<std::string>({".cpp"}));
    
	std::string compiler_string = project_layout.at("compiler").at(0);
    std::string header_file_string = command::unpack_string_vector(project_layout.at("headers"), "-I");
    std::string compiler_flags_string = command::unpack_string_vector(project_layout.at("flags"));
    std::string library_string = command::unpack_string_vector(project_layout.at("libraries"), "-l");
    std::string standard_string = "-std=" + project_layout.at("standard").at(0);
    
    std::string object_command = "";
    
    std::filesystem::current_path(project_layout_path.parent_path().append("build/objects/"));
    
    std::set<std::string> duplicate_checker;

	std::mutex queutex;
	std::mutex hashtex;
	
	for(const std::string& file_name : source_files) 
	{
		if(!duplicate_checker.extract(std::filesystem::absolute(file_name).filename().string()))
		{
			duplicate_checker.insert(std::filesystem::absolute(file_name).filename().string());  
		} else 
		{
			std::cerr << "Duplicate file detected!" << std::endl;
			std::cerr << "This project has multiple files named \"" << std::filesystem::absolute(file_name).filename().string() << "\"" << std::endl; 
			std::cerr << "Please rename one (or all) problematic files before rebuild" << std::endl;
			return;
		}
	}

	std::filesystem::path temp_directory = std::filesystem::absolute(std::filesystem::current_path()).append("temp");
	std::filesystem::create_directory(temp_directory);
	std::vector<std::thread> active_threads;
	std::string build_command_format = compiler_string 
										+ " -c" +
										+ " "   + compiler_flags_string +
										+ " "   + library_string +
										+ " "   + header_file_string +
										+ " "   + "{}" +
										+ " "   + standard_string;
	int max_threads = std::stoi(project_layout.at("threads").at(0).c_str());
    for(int i = 0; i < max_threads; i++)
    {
		// TODO somehow avoid using map?
		active_threads.push_back(std::thread([&]() { command::thread_task_build_object(i, source_files, file_hashstamps, queutex, hashtex, build_command_format); }));
    }

	for(std::thread& thread : active_threads)
	{
		thread.join();
	}
	std::filesystem::remove_all(temp_directory);
    
    std::vector<std::string> object_files = command::find_all_files(std::vector<std::string>({std::filesystem::absolute(std::filesystem::current_path()).string()}), std::vector<std::string>({".o"}));
    
    std::string object_file_string = command::unpack_string_vector(object_files);
    std::string exe_path_string = "-o " + project_layout_path.parent_path().append("build/executable/" + project_name).string();
    
     std::string final_command_string = project_layout.at("compiler").at(0) +
                                        + " " + exe_path_string +
                                        + " " + library_string +
                                        + " " + header_file_string +
                                        + " " + object_file_string +
                                        + " " + standard_string;
                                        
    system(final_command_string.c_str());
    
    hashstamp::write_to_file(file_hashstamps);
}

void command::handle_two_arg_command(std::string first_arg, std::string command, std::string second_arg) 
{
    std::function<void(std::string, std::string)> runnable = [&](std::string, std::string) {
        std::cerr << "The command \'" << first_arg << " " << command << " " << second_arg << "\' is not a supported command. Please run \'chai help\' for a list of supported commands!" << std::endl;
        return;
    }; 
    
    if(command::two_arg_function_map.count(command))
    {
        runnable = command::two_arg_function_map.at(command);
    }
    
    return runnable(first_arg, second_arg);
}

void command::handle_run(std::string project_name, std::string args) 
{
    std::optional<std::filesystem::path> chai_path = command::find_build_folder();

    if(!chai_path.has_value())
    {
        std::cerr << "O h F u c k" << std::endl;
        return;
    }
    
    std::filesystem::path exe_path = chai_path.value().append("projects/" + project_name + "/build/executable/" + project_name);
        
    std::string final_command = exe_path.string() + " " + args;
    
    system(final_command.c_str());
}

// TODO this.
void command::handle_debug(std::string project_name, std::string args) {}
// TODO also this.
void command::handle_copy_to(std::string existing_project, std::string new_project) {}
// TODO this too.
void command::handle_copy_from(std::string new_project, std::string existing_project) {}
// TODO oh also this
void command::handle_rename(std::string existing_project, std::string new_name) {}

void command::handle_add_library(std::string existing_project, std::string path) 
{
    std::optional<std::filesystem::path> chai_path = command::find_build_folder();
    std::filesystem::path project_layout_path = chai_path.value().append("projects/" + existing_project + "/project_layout");
    std::map<std::string, std::vector<std::string>> settings = settings::read_from_file(project_layout_path);
    
    settings.at("libraries").push_back(path);
    
    settings::write_to_file(settings, project_layout_path);
}

void command::handle_add_header_directory(std::string existing_project, std::string path) 
{
    std::optional<std::filesystem::path> chai_path = command::find_build_folder();
    std::filesystem::path project_layout_path = chai_path.value().append("projects/" + existing_project + "/project_layout");
    std::map<std::string, std::vector<std::string>> settings = settings::read_from_file(project_layout_path);
    
    settings.at("headers").push_back(path);
    
    settings::write_to_file(settings, project_layout_path);
}
void command::handle_add_source_directory(std::string existing_project, std::string path) 
{
    std::optional<std::filesystem::path> chai_path = command::find_build_folder();
    std::filesystem::path project_layout_path = chai_path.value().append("projects/" + existing_project + "/project_layout");
    std::map<std::string, std::vector<std::string>> settings = settings::read_from_file(project_layout_path);
    
    settings.at("sources").push_back(std::filesystem::absolute(path).string());
    
    settings::write_to_file(settings, project_layout_path);
}

void command::handle_add_compile_flag(std::string existing_project, std::string flag) 
{
    std::optional<std::filesystem::path> chai_path = command::find_build_folder();
    std::filesystem::path project_layout_path = chai_path.value().append("projects/" + existing_project + "/project_layout");
    std::map<std::string, std::vector<std::string>> settings = settings::read_from_file(project_layout_path);
    
    settings.at("flags").push_back(flag);
    
    settings::write_to_file(settings, project_layout_path);
}

void command::handle_remove_library(std::string existing_project, std::string path) 
{
    std::optional<std::filesystem::path> chai_path = command::find_build_folder();
    std::filesystem::path project_layout_path = chai_path.value().append("projects/" + existing_project + "/project_layout");
    std::map<std::string, std::vector<std::string>> settings = settings::read_from_file(project_layout_path);
    
    std::vector<std::string>& reference = settings.at("libraries");
    reference.erase(std::find(reference.begin(), reference.end(), path));
    
    settings::write_to_file(settings, project_layout_path); 
}

void command::handle_remove_header_directory(std::string existing_project, std::string path) 
{
    std::optional<std::filesystem::path> chai_path = command::find_build_folder();
    std::filesystem::path project_layout_path = chai_path.value().append("projects/" + existing_project + "/project_layout");
    std::map<std::string, std::vector<std::string>> settings = settings::read_from_file(project_layout_path);
    
    std::vector<std::string>& reference = settings.at("headers");
    reference.erase(std::find(reference.begin(), reference.end(), path));
    
    settings::write_to_file(settings, project_layout_path);  
}
void command::handle_remove_source_directory(std::string existing_project, std::string path) 
{
    std::optional<std::filesystem::path> chai_path = command::find_build_folder();
    std::filesystem::path project_layout_path = chai_path.value().append("projects/" + existing_project + "/project_layout");
    std::map<std::string, std::vector<std::string>> settings = settings::read_from_file(project_layout_path);
    
    std::vector<std::string>& reference = settings.at("sources");
    reference.erase(std::find(reference.begin(), reference.end(), path));
    
    settings::write_to_file(settings, project_layout_path); 
}

void command::handle_remove_compile_flag(std::string existing_project, std::string flag) 
{
    std::optional<std::filesystem::path> chai_path = command::find_build_folder();
    std::filesystem::path project_layout_path = chai_path.value().append("projects/" + existing_project + "/project_layout");
    std::map<std::string, std::vector<std::string>> settings = settings::read_from_file(project_layout_path);
    
    std::vector<std::string>& reference = settings.at("flags");
    reference.erase(std::find(reference.begin(), reference.end(), flag));
    
    settings::write_to_file(settings, project_layout_path); 
}
