#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

typedef unsigned int uint;

void LoadData(const std::string path, std::vector<std::string> &data);
void WriteData(const std::string path, std::vector<std::string> &data);
std::vector<std::string> ParseCategories(std::vector<std::string> &data);
std::vector<std::string> AppendStuff(std::vector<std::string> &data, const std::string ext, const uint step);


namespace {
    const size_t SUCCESS = 0;
    const size_t ERROR_COMMAND_LINE = 1;
    const size_t ERROR_UNHANDLED_EXCEPTION = 2;
}

int main(int argc, char **argv)
{
    try {
        namespace po = boost::program_options;
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
        namespace fs = boost::filesystem;
        const bool verbosity = vm.count("verbose") ? true : false;
        const uint rotation_step = vm["rot_step"].as<uint>();
        fs::path input_path(vm["filelist"].as<std::string>());

        // Ensures file list actually exists
        if (!fs::exists(input_path)) {
            throw std::invalid_argument(std::string("Invalid file path."));
        }


        const auto file_size = fs::file_size(input_path);
        const std::string abs_path = input_path.string();
        const std::string stem = input_path.stem().string();
        const std::string type = input_path.extension().string();
        // Next one is destructive, don't re-use input_path.
        const std::string dir_path = input_path.remove_filename().string();

        std::cout << "\nDebug info...\n"
                  << "File List Path: " << abs_path << std::endl
                  << "Rotation Step (degrees): " << rotation_step << std::endl;
        // Optional output when verbose flag used.
        if (verbosity) {
            std::cout << "\nOptional info...\n"
                      << "Directory Path: " << dir_path << "\n"
                      << "Filename without extension: " << stem << "\n"
                      << "File Extension: " << type << "\n"
                      << "File Size (bytes): " << file_size << std::endl;

        }

        std::vector<std::string> file_data;
        LoadData(abs_path, file_data);
        if (verbosity) {
            std::cout << "\nFile contents..." << std::endl;
            for (auto &s : file_data) {
                std::cout << "\t" << s << std::endl;
            }
        }
        std::cout << "Lines loaded: " << file_data.size() << std::endl;

        std::vector<std::string> output_data = AppendStuff(file_data, type, rotation_step);

        if (verbosity) {
            for (auto &s : output_data) {
                std::cout << s << std::endl;
            }
        }

        WriteData("output.txt", output_data);



        // Categories seem largely unnecessary in C++, wrote a parser before realizing this...
//        std::vector<std::string> categories = ParseCategories(file_data);
//        if (verbosity) {
//            std::cout << "\nCategories..." << std::endl;
//            for (auto &s : categories) {
//                std::cout << "\t" << s << std::endl;
//            }
//        }
//        std::cout << "Categories found: " << categories.size() << std::endl;

    }
    catch(std::exception &e) { // If getting this regularly, more exception handling should be added.
        std::cerr << "Unhandled exception reached top level: " << e.what() << ", exiting application." << std::endl;
        return ERROR_UNHANDLED_EXCEPTION;
    }

    return SUCCESS;
}


std::vector<std::string> ParseCategories(std::vector<std::string> &data)
{
    std::vector<std::string> cats;
    char delim = '/';
    std::string tok;

    for (auto &s : data) {
        tok = s.substr(0, s.find(delim));
        cats.push_back(tok);
    }

    std::sort(cats.begin(), cats.end());
    cats.erase(std::unique(cats.begin(), cats.end()), cats.end());
    return cats;
}

void LoadData(const std::string path, std::vector<std::string> &data)
{
    std::ifstream fin(path);
    std::copy(std::istream_iterator<std::string>(fin), std::istream_iterator<std::string>(), std::back_inserter(data));
    // TODO: Error handling
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


std::vector<std::string> AppendStuff(std::vector<std::string> &data, const std::string ext, const uint step)
{
    const std::string rot = "_rot_";
    const std::string flipv = "_flip_v";
    const std::string flipn = "_flip_n";
    std::vector<std::string> output;

    for (auto &s : data) {
        for (int i = 0; i < 360; i += step) {
            std::string temp1 = s;
            std::string temp2 = s;
            std::stringstream ss;
            ss << i;
            std::string angle = ss.str();

            if (angle.length() == 1) {
                angle.insert(angle.begin(), '0');
                angle.insert(angle.begin(), '0');
            }
            if (angle.length() == 2) {
                angle.insert(angle.begin(), '0');
            }

            std::stringstream ss1;
            ss1 << rot << angle << flipv;
            temp1.insert(temp1.length() - ext.length(), ss1.str());
            output.push_back(temp1);

            std::stringstream ss2;
            ss2 << rot << angle << flipn;
            temp2.insert(temp2.length() - ext.length(), ss2.str());
            output.push_back(temp2);
        }
    }
    return output;
}