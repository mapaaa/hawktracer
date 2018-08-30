#ifndef HAWKTRACER_ANOMALY_PATTERN_MATCHING_HPP
#define HAWKTRACER_ANOMALY_PATTERN_MATCHING_HPP

#include "config.hpp"
#include "graphs.hpp"

using TreeNode = HawkTracer::client::CallGraph::TreeNode;

namespace HawkTracer
{
namespace anomaly
{

class PatternMatching
{
public:
    PatternMatching(std::shared_ptr<Config> config,
                    std::shared_ptr<Graphs> patterns);
    std::vector<unsigned int> get_matching_scores(std::shared_ptr<TreeNode> tree);
    std::vector<std::pair<unsigned int, unsigned int>> get_mapping(std::shared_ptr<TreeNode> tree,
                                                                   int pattern_index);
    unsigned int get_pattern_score_threshold();
    static std::shared_ptr<TreeNode> transform_tree(std::shared_ptr<TreeNode> src_tree,
                                                    std::shared_ptr<TreeNode> dst_tree,
                                                    std::vector<std::pair<unsigned int, unsigned int>>& mapping);

private:
    std::shared_ptr<Config> _config;
    std::shared_ptr<Graphs> _patterns;
    std::vector<unsigned int> _edit_distance;
    unsigned int _pattern_score_threshold;

    void _compute_matching_scores(std::shared_ptr<TreeNode> tree);
    void _compute_pattern_score_threshold();
    void _compute_mapping(std::shared_ptr<TreeNode> tree,
                          std::vector<std::pair<unsigned int, unsigned int>>& mapping,
                          int pattern_index);
};

} // namespace anomaly
} // namespace HawkTracer

#endif // HAWKTRACER_ANOMALY_PATTERN_MATCHING_HPP
