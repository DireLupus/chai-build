#include "../include/settings.hpp"

#include <fstream>
#include <iostream>

std::vector<std::string> settings::split(std::string splittable, std::string delimiter)
{
    std::vector<std::string> values;
    
    size_t start = 0;
    size_t end = splittable.find(delimiter);
    while(end != std::string::npos) {
        values.push_back(splittable.substr(start, end - start));
        start = end + delimiter.length();
        end = splittable.find(delimiter, start);
    }
    
    values.push_back(splittable.substr(start));
    
    return values;
}

std::map<std::string, std::vector<std::string>> settings::read_from_file(std::filesystem::path file_path)
{
    std::map<std::string, std::vector<std::string>> returnable;
    
    std::ifstream file(file_path);
    
    std::string line, key, value;
    while(std::getline(file, line))
    {
        key = line.substr(0, line.find("=\""));
        value = line.substr(line.find("=\"") + 2);
        value = value.substr(0, value.length() - 1);
        returnable.insert(std::make_pair(key, settings::split(value, " ")));
    }
    
    return returnable;
}

int settings::write_to_file(std::map<std::string, std::vector<std::string>> writeable, std::filesystem::path file_path)
{
    int count = 0;
    
    std::ofstream file(file_path);
    
    for(const auto& [key, value] : writeable)
    {
        file << key << "=";
        
        std::string formatted = "";
        for(std::string str : value)
        {
            formatted = formatted + str;
            formatted = formatted + " ";
        }
        
        file << std::quoted(formatted.substr(0, formatted.length() - 1)) << std::endl;
        
        count++;
    }
    
    return count;
}
