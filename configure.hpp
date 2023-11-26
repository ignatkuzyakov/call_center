#ifndef CONFIGURE_HPP
#define CONFIGURE_HPP

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <cstddef>
#include <string>

extern std::size_t Rmin;
extern std::size_t Rmax;

extern std::size_t NQueue;
extern std::size_t NCalls;

class Config
{
private:
    boost::property_tree::ptree root;
    std::stringstream json;

public:
    Config(const std::string& fileName);
    void parse();
};

#endif
