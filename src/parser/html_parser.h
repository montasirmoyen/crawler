#include <string>
#include <vector>
#include <regex>

class Parser {
public:
    std::vector<std::string> extractLinks(const std::string& html) {
        std::vector<std::string> links;
        // regex to look for http/https links
        std::regex link_regex("<a\\s+(?:[^>]*?\\s+)?href=\"(https?://[^\"]+)\"", std::regex_constants::icase);

        auto words_begin = std::sregex_iterator(html.begin(), html.end(), link_regex);
        auto words_end = std::sregex_iterator();

        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            links.push_back((*i)[1].str());
        }
        return links;
    }
};