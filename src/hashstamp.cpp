#include "../include/hashstamp.hpp"
#include "../include/command.hpp"

#include <fstream>
#include <string>
#include <chrono>
#include <iostream>

std::map<std::string, int> hashstamp::read_from_file()
{
    std::map<std::string, int> returnable;
    // TODO handle optional
    std::filesystem::path file_path = command::find_build_folder().value().append("cache/hashstamps");
    
    std::ifstream stream(file_path);
    
    std::string current_line = "";
    int splitter = 0;
    while(std::getline(stream, current_line))
    {
        splitter = current_line.find("=");
        returnable.insert(
            std::make_pair(
                current_line.substr(0, splitter),
                std::stoll(current_line.substr(splitter + 1))
            )
        );
    }
    
    stream.close();
    
    return returnable;
}

int hashstamp::write_to_file(std::map<std::string, int> writeable)
{
    int returnable = 0;
    std::optional<std::filesystem::path> chai_path = command::find_build_folder();
    if(!chai_path.has_value())
    {
        std::cerr << "O h F u c k" << std::endl;
        return -1;
    }
    
    std::filesystem::path file_path = chai_path.value().append("cache/hashstamps");
    
    std::ofstream stream(file_path);
    
    for(const std::pair<std::string, int>& pair : writeable)
    {
        stream << pair.first << "=" << pair.second << std::endl;
        returnable++;
    }
    
    stream.close();
    
    return returnable;
}
