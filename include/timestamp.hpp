#include <chrono>
#include <filesystem>
#include <map>
#include <string>

class timestamp 
{
    private :
    public :
        static std::map<std::string, std::chrono::nanoseconds> read_from_file();
        // TODO can unpack as pair in for_each
        static int write_to_file(std::map<std::string, std::chrono::nanoseconds> writeable);
};
