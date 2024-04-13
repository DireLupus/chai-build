#include <chrono>
#include <filesystem>
#include <map>
#include <string>

class hashstamp 
{
    private :
    public :
        static std::map<std::string, int> read_from_file();
        // TODO can unpack as pair in for_each
        static int write_to_file(std::map<std::string, int> writeable);
};