#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <algorithm>

#include <tbb/tbb.h>

#include "./include/cfg.h"
#include "./include/dcomp.h"
#include "./include/timer.h"
#include "./include/parsing.h"

namespace fs = std::filesystem;

template<typename K, typename V>
void print_map_ordered(const std::vector<std::pair<K, V>>& order,
                       tbb::concurrent_unordered_map<K, V>& output_map,
                       const std::string& output_dir);

int main(int argc, const char* argv[]) {
    const size_t tokens_n = 400;
    const size_t size_limit = 12'000'000;

    configuration_t config;
    size_t execution_time_mcs;
    std::string dir_path;

    try
    {
        config = init(argc, argv);
    } catch (std::runtime_error &e)
    {
        std::cout << "Runtime exception: " << e.what() << std::endl;
        return 1;
    }

    dir_path = config.in_file;

    boost::locale::generator gen;
    std::locale loc = gen("en_US.UTF-8");
    std::locale::global(loc);
    std::cout.imbue(loc);

    fs::recursive_directory_iterator dir_iter(dir_path), dir_iter_end = fs::end(dir_iter);
    tbb::concurrent_unordered_map<std::string, size_t> total_indexed;

    auto start_time_stamp = get_current_time_fenced();

    tbb::parallel_pipeline(tokens_n,
                           tbb::make_filter<void, std::string>(
                                   tbb::filter::serial_out_of_order,
                                   [&](tbb::flow_control &fc) -> std::string {
                                       std::string cur_dir_path;
                                       do
                                       {
                                           if (dir_iter == dir_iter_end)
                                           {
                                               fc.stop();
                                               return std::string();
                                           }
                                           cur_dir_path = dir_iter->path();
                                           dir_iter++;
                                       } while (fs::is_directory(cur_dir_path));
                                       return cur_dir_path;
                                   }) &
                                   tbb::make_filter<std::string, std::string>(
                                           tbb::filter::serial_out_of_order,
                                           [&](const std::string& file_dir) -> std::string
                                           {
                                               std::ifstream raw_file;
                                               std::string buffer;
                                               try
                                                {
                                                    raw_file = std::ifstream(file_dir, std::ios::binary);
                                                    buffer = [&raw_file] {
                                                        std::ostringstream ss{};
                                                        ss << raw_file.rdbuf();
                                                        return ss.str();
                                                    } ();
                                                } catch (std::exception& e)
                                                {
                                                    std::cout << e.what() << std::endl;
                                                    raw_file.close();
                                                }
                                                return buffer;
                                           }
                                           ) &
                                   tbb::make_filter<std::string, std::map<std::string, size_t>>(
                                           tbb::filter::parallel,
                                           [&](const std::string& raw_file) -> std::map<std::string, size_t>
                                           {
                                               std::string file_data;
                                               std::map<std::string, size_t> indexed_words;
                                               try
                                                {
                                                    decompress(raw_file, file_data, size_limit);
                                                    parse(file_data, indexed_words);
                                                } catch (std::exception& e)
                                                {
                                                    std::cout << e.what() << std::endl;
                                                }
                                                return indexed_words;
                                           }
                                           ) &
                                   tbb::make_filter<std::map<std::string, size_t>, void>(
                                           tbb::filter::parallel,
                                           [&] (const std::map<std::string, size_t>& indexed_words)
                                           {
                                               for (const auto& [key, value]: indexed_words)
                                                   total_indexed[key] += value;
                                           }
                                           )
                           );

    execution_time_mcs = to_us(get_current_time_fenced() - start_time_stamp);
    std::cout << "Execution time: " << static_cast<double>(execution_time_mcs) / 1'000'000 << "s" << std::endl;

    std::vector<std::pair<std::string, size_t>> key_val_pairs;
    for (const auto& [key, value]: total_indexed)
        key_val_pairs.emplace_back(key, value);

    std::sort(key_val_pairs.begin(), key_val_pairs.end(), [](const auto& p1, const auto& p2) {
        return p1.first.compare(p2.first) < 0;
    });
    print_map_ordered(key_val_pairs, total_indexed, config.out_file1);
    std::sort(key_val_pairs.begin(), key_val_pairs.end(), [](const auto& p1, const auto& p2) {
      return p1.second > p2.second;
    });
    print_map_ordered(key_val_pairs, total_indexed, config.out_file2);

    return 0;
}

template<typename K, typename V>
void print_map_ordered(const std::vector<std::pair<K, V>>& order,
                       tbb::concurrent_unordered_map<K, V>& output_map,
                       const std::string& output_dir)
{
    std::ofstream output(output_dir, std::fstream::out);
    for (const auto& [key, value]: order)
        output << std::setw(20) << std::left << key <<
               std::setw(20) << std::left << output_map[key] << std::endl;
    output.close();
}
