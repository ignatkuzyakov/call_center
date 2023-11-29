#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <cstddef>
#include <string>

#include "configure.hpp"

std::size_t Rmin;
std::size_t Rmax;

std::size_t NQueue;
std::size_t NCalls;

Config::Config(const std::string& fileName) {
    std::ifstream f(fileName);
    json << f.rdbuf();
}

void Config::parse()
{
    boost::property_tree::read_json(json, root);

    Rmin = root.get<std::size_t>("Settings.CallsTimeRange..Rmin");
    Rmax = root.get<std::size_t>("Settings.CallsTimeRange..Rmax");

    NQueue = root.get<std::size_t>("Settings.NQueue");
    NCalls = root.get<std::size_t>("Settings.NCalls");

    root.clear();
}