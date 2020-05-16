#include "./../include/cfg.h"

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

configuration_t init(const int& argc, const char* argv[])
{
    if (argc > 2)
        throw std::runtime_error("Argument error");
    std::string cfg_filepath;
    cfg_filepath = argc == 2 ? argv[1] : "../config.dat";
    std::ifstream config_stream(cfg_filepath);
    if(!config_stream.is_open())
        throw std::runtime_error("Failed to open configuration file " + cfg_filepath);
    configuration_t config {};
    config = read_conf(cfg_filepath);
    config_stream.close();
    return config;
}

configuration_t read_conf(const std::string& cf_name)
{
    configuration_t res{};
    std::ifstream ifs(cf_name);
    Json::Reader reader;
    Json::Value obj;
    reader.parse(ifs, obj);

    std::vector<std::string> vector_names = {obj["inputFile"].asString(), obj["outputA"].asString(),
                                            obj["outputB"].asString(), obj["threads"].asString()};

    for (int j = 0; j < 3; j++)
    {
        if(vector_names[j].empty())
        {
            throw std::runtime_error(std::to_string(j) + " th argument is empty!");
        }
        if(!boost::filesystem::exists(vector_names[j]))
        {
            throw std::runtime_error("Failed to locate file: " + vector_names[j]);
        }
    }

    if (boost::filesystem::is_empty(vector_names[0]))
    {
        throw std::runtime_error("Input file " + vector_names[0] + " is empty!");
    }

    if (!is_number(vector_names[3])){
        throw std::runtime_error("Number of threads is incorrect");
    }

    if ((std::stoi(vector_names[3]) < 1) || (std::stoi(vector_names[3]) > 100))
    {
        throw std::runtime_error("Invalid number of threads");
    }

    res.in_file = vector_names[0];
    res.out_file1 = vector_names[1];
    res.out_file2 = vector_names[2];
    res.threads = std::stoi(vector_names[3]);

    return res;
}
