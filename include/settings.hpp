#include <filesystem>
#include <map>
#include <vector>
#include <string>

class settings 
{
  private :
    static std::vector<std::string> split(std::string splittable, std::string delimiter);
  public :
    static std::map<std::string, std::vector<std::string>> read_from_file(std::filesystem::path file_path);
    static int write_to_file(std::map<std::string, std::vector<std::string>> writeable, std::filesystem::path file_path);
};
