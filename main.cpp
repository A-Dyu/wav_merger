#include <iostream>
#include "wav_file.h"
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    po::options_description description("Options");
    description.add_options()
            ("input", po::value<std::vector<std::string>>(), "input files")
            ("output", po::value<std::string>(), "output file");
    po::positional_options_description pos_op;
    pos_op.add("input", -1);
    po::variables_map var_map;
    po::store(po::command_line_parser(argc, argv).
                      options(description).positional(pos_op).run(),
              var_map);
    po::notify(var_map);
    if (!var_map.count("input")) {
        throw std::runtime_error("No input files found");
    }
    if (!var_map.count("output")) {
        throw std::runtime_error("No output file found");
    }
    std::vector<wav_file> mono_files;
    for (auto const& file_name :
            var_map["input"].as<std::vector<std::string>>()) {
        mono_files.emplace_back(file_name.data(), "r");
    }
    wav_file out = merge(var_map["output"].as<std::string>().data(), mono_files, 2);
    return 0;
}