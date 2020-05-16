#include "./../include/parsing.h"

void parse(const std::string &text, std::map<std::string, size_t>& output)
{
    std::string word;
    boost::locale::boundary::ssegment_index myMap(boost::locale::boundary::word, text.begin(), text.end());
    myMap.rule(boost::locale::boundary::word_letters);
    for (const auto &x: myMap)
    {
        word = boost::locale::normalize(std::string(x), boost::locale::norm_default);
        word = boost::locale::fold_case(word);
        output[word]++;
    }
}
