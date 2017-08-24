#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <cstring>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

typedef unsigned int uint;
namespace fs = boost::filesystem;
namespace po = boost::program_options;
namespace {
    const size_t SUCCESS = 0;
    const size_t ERROR_COMMAND_LINE = 1;
    const size_t ERROR_UNHANDLED_EXCEPTION = 2;
}

void LoadData(const std::string path, std::vector<std::string> &data);
void WriteData(const std::string path, std::vector<std::string> &data);
std::vector<std::pair<std::string, std::string>> ParseLine(std::vector<std::string> &data);
std::vector<std::string> Insertion(std::vector<std::pair<std::string, std::string>> data, const uint step);


int main(int argc, char **argv)
{
    try {
        po::options_description desc("Options");

        desc.add_options()
                ("help,h", "Display usage information")
                ("filelist,f", po::value<std::string>()->required(), "Required: Fold file list")
                ("rot_step,r", po::value<uint>()->required(), "Required: Rotation step amount in degrees")
                ("verbose,v", "Verbose output");

        // Positional arguments are required, and need to be in correct order.
        po::positional_options_description positional_options;
        positional_options.add("filelist", 1);
        positional_options.add("rot_step", 1);
        po::variables_map vm;

        try { // Help Section
            po::store(po::command_line_parser(argc, argv).options(desc).positional(positional_options).run(), vm);
            if (vm.count("help")) {
                std::cout << "Generate Augmented File List" << std::endl << std::endl
                          << "Usage: '" << argv[0] << " [OPTIONS] <fold_file_list> <rotation_step>'" << std::endl << std::endl
                          << desc << std::endl << std::endl;
                return SUCCESS;
            }
            po::notify(vm);

        }
        catch(po::error &e) {
            std::cerr << "ERROR: " << e.what() << std::endl;
            std::cout << "Try '" << argv[0] << " --help' for more information." << std::endl;
            return ERROR_COMMAND_LINE;
        }

        // Main Program
        const bool verbosity = vm.count("verbose") ? true : false;
        const uint rotation_step = vm["rot_step"].as<uint>();
        fs::path input_path(vm["filelist"].as<std::string>());

        // Ensures file list actually exists
        if (!fs::exists(input_path)) {
            throw std::invalid_argument(std::string("Invalid file path."));
        }

        const std::string abs_path = input_path.string();

        std::vector<std::string> file_data;
        LoadData(abs_path, file_data);

        if (verbosity) {
            std::cout << "\nDebug info...\n"
                      << "File List Path: " << abs_path << std::endl
                      << "Rotation Step (degrees): " << rotation_step << std::endl;
            std::cout << "\nFile contents..." << std::endl;
            for (auto &s : file_data) {
                std::cout << "\t" << s << std::endl;
            }
        }
        std::cout << "Augmenting " << file_data.size() << "filenames..." << std::endl << std::endl;

        std::vector<std::pair<std::string, std::string>> parsed_data = ParseLine(file_data);
        std::vector<std::string> output = Insertion(parsed_data, rotation_step);
        if (verbosity) {
            for (auto &s : output) {
                std::cout << s << std::endl;
            }
        }

        WriteData("augmented_" + abs_path, output);

    }
    catch(std::exception &e) { // If getting this regularly, more exception handling should be added.
        std::cerr << "Unhandled exception reached top level: " << e.what() << ", exiting application." << std::endl;
        return ERROR_UNHANDLED_EXCEPTION;
    }

    return SUCCESS;
}




void LoadData(const std::string path, std::vector<std::string> &data)
{
    std::ifstream fin(path);
    std::string line;
    while (std::getline(fin, line)) {
        data.emplace_back(line);
    }

}

void WriteData(const std::string path, std::vector<std::string> &data)
{
    std::ofstream outfile;
    outfile.open(path);
    for (auto &s : data) {
        outfile << s << std::endl;
    }
    outfile.close();
}


std::vector<std::pair<std::string, std::string>> ParseLine(std::vector<std::string> &data)
{

    std::vector<std::pair<std::string, std::string>> output;
    output.reserve(data.size());

    for (auto &s : data) {

        std::vector<std::string> lines;
        boost::split(lines, s, boost::is_any_of(" "));
        output.push_back(std::make_pair(lines[0], lines[1]));

    }

    return output;
}

std::vector<std::string> Insertion(std::vector<std::pair<std::string, std::string>> data, const uint step)
{
    const std::string rot = "_rot_";
    const std::string flip_v = "_flip_v";
    const std::string flip_n = "_flip_n";
    std::vector<std::string> output;
    output.reserve(data.size());

    for (auto &s : data) {
        fs::path path(s.first);
        const std::string class_id(s.second);

        const std::string ext = path.extension().string();
        const std::string stem = path.stem().string();
        std::string dir = path.remove_filename().string();
        const char slash = '/';
        const char space = ' ';

        for (int deg = 0; deg < 360; deg += step) {
            std::stringstream line_v;
            std::stringstream line_n;
            line_v << dir << slash << stem << rot << deg << flip_v << ext;
            line_n << dir << slash << stem << rot << deg << flip_n << ext;
            output.push_back(line_v.str() + space + class_id);
            output.push_back(line_n.str() + space + class_id);
        }
    }
    return output;
}
