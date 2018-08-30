#include <hawktracer/parser/make_unique.hpp>

#include "anomalies.hpp"
#include "chrome_file_loader.hpp"
#include "config.hpp"
#include "graphs.hpp"
#include "json_trees_file_loader.hpp"
#include "pattern_matching.hpp"

#include <algorithm>
#include <cstring>
#include <map>
#include <iostream>

using namespace HawkTracer;

void init_file_loaders(std::map<std::string, std::shared_ptr<anomaly::FileLoader>>& file_loaders)
{
    file_loaders["chrome-tracing"] = std::make_shared<anomaly::ChromeFileLoader>();
    file_loaders["json"] = std::make_shared<anomaly::JsonTreesFileLoader>();
}

int main(int argc, char** argv)
{
    std::map<std::string, std::shared_ptr<anomaly::FileLoader>> file_loaders;

    init_file_loaders(file_loaders);
    std::string config_file;
    std::string pattern_file;
    std::string source_file;
    std::string source_file_format;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--config") == 0 && i < argc - 1)
        {
            config_file = argv[++i];
        }
        else if (strcmp(argv[i], "--pattern") == 0 && i < argc - 1)
        {
            pattern_file = argv[++i];
        }
        else if (strcmp(argv[i], "--source") == 0 && i < argc - 1)
        {
            source_file = argv[++i];
        }
        else if (strcmp(argv[i], "--format") == 0 && i < argc - 1)
        {
            source_file_format = argv[++i];
        }
    }

    auto config = std::make_shared<anomaly::Config>();
    if (!config->load_from_file(config_file))
    {
        std::cerr << "Could not open config file" << std::endl;
        return 1;
    }

    auto pattern = std::make_shared<anomaly::Graphs>(parser::make_unique<anomaly::JsonTreesFileLoader>());
    if (!pattern->load_from_file(pattern_file))
    {
        std::cerr << "Could not open patterns file" << std::endl;
        return 1;
    }

    auto file_loader = file_loaders.find(source_file_format);
    if (file_loader == file_loaders.end())
    {
        std::cerr << "Unknown format will use chrome-tracing" << std::endl;
        file_loader = file_loaders.find("chrome-tracing");
    }
    anomaly::Graphs source(file_loader->second);
    if (!source.load_from_file(source_file))
    {
        std::cerr << "Could not open source file" << std::endl;
        return 1;
    }

    anomaly::PatternMatching pattern_matcher(config, pattern);   
    anomaly::Anomalies anomaly_detector(config);
    const auto& sources = source.get_trees();
    const auto& patterns = pattern->get_trees();
    for (size_t i = 0; i < sources.size(); ++i)
    {
        std::vector<unsigned int> scores = pattern_matcher.get_matching_scores(sources[i].first);
        auto min_score = std::min_element(scores.begin(), scores.end());
        if (*min_score <= pattern_matcher.get_pattern_score_threshold())
        {
            int pattern_index = std::distance(scores.begin(), min_score);
            auto mapping = pattern_matcher.get_mapping(sources[i].first, pattern_index);
            
            std::shared_ptr<TreeNode> new_tree = anomaly::PatternMatching::transform_tree(sources[i].first, 
                                                                                          patterns[pattern_index].first,
                                                                                          mapping);
            double anomaly_score = anomaly_detector.get_score(patterns[pattern_index].first, new_tree);
            if (anomaly_score >= config->get_anomaly_score_threshold())
            {
                std::cout << "Source " << i << " matched with pattern number " << pattern_index 
                    << " it's an anomaly (score: " << anomaly_score << ")" << std::endl;
            }
            else
            {
                std::cout << "Source " << i << " matched with pattern number " << pattern_index 
                    << " it's NOT an anomaly (score: " << anomaly_score << ")" << std::endl;
            }
        }
        else
        {
            std::cout << "Source " << i << " not found in patterns (score: " << *min_score << ")" << std::endl;
        }
    }
    return 0;
}
