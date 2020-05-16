#include <iostream>
#include <string>
#include <filesystem>

#include <tbb/tbb.h>

#include "./include/cfg.h"
#include "./include/dcomp.h"
#include "./include/timer.h"
#include "./include/parsing.h"

namespace fs = std::filesystem;

int main() {
    const size_t tokens_n = 400;
    const size_t size_limit = 10'000'000;
    const std::string dir_path = "data";

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
                                       } while (fs::is_directory(cur_dir_path) || fs::file_size(cur_dir_path) > size_limit);
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

    std::cout << static_cast<double>(to_us(get_current_time_fenced() - start_time_stamp)) / 1'000'000 << std::endl;

    for (const auto& [key, value]: total_indexed)
        std::cout << key << " " << value << std::endl;

    return 0;
}
