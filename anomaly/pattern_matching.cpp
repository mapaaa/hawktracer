#include "ordered_tree_editing_distance.hpp"
#include "pattern_matching.hpp"

#include <algorithm>

using TreeNodeData = HawkTracer::client::CallGraph::TreeNodeData;

namespace HawkTracer
{
namespace anomaly
{

PatternMatching::PatternMatching(std::shared_ptr<Config> config,
                                 std::shared_ptr<Graphs> patterns) :
    _config(std::move(config)),
    _patterns(std::move(patterns))
{
    _edit_distance.resize(_patterns->get_trees().size());
    _compute_pattern_score_threshold();
}

std::vector<unsigned int> PatternMatching::get_matching_scores(std::shared_ptr<TreeNode> tree)
{
    _compute_matching_scores(tree);
    return _edit_distance;
}

unsigned int PatternMatching::get_pattern_score_threshold()
{
    return _pattern_score_threshold;
}

void PatternMatching::_compute_pattern_score_threshold()
{
   _pattern_score_threshold = _config->get_insert_cost() * _config->get_max_insertions() +
                              _config->get_delete_cost() * _config->get_max_deletions() + 
                              _config->get_relabel_cost() * _config->get_max_relabel();
}

void PatternMatching::_compute_matching_scores(std::shared_ptr<TreeNode> tree)
{
    const auto& patterns = _patterns->get_trees();
    for (size_t i = 0 ; i < patterns.size(); ++i)
    {
        OrderedTreeEditingDistance distance(_config, tree, patterns[i].first);
        _edit_distance[i] = distance.get_distance(); 
    }
}

std::vector<std::pair<unsigned int, unsigned int>> PatternMatching::get_mapping(std::shared_ptr<TreeNode> tree,
                                                                                int pattern_index)
{
    const auto& patterns = _patterns->get_trees();
    const auto& matched_pattern = patterns[pattern_index];
    OrderedTreeEditingDistance distance(_config, tree, matched_pattern.first);
    return distance.get_mapping();
}

// Returns if the node was inserted or not
static bool transform_node(std::shared_ptr<TreeNode> node,
                           std::shared_ptr<TreeNode> dst_node,
                           std::vector<std::pair<unsigned int, unsigned int>>& mapping,
                           std::vector<TreeNodeData>& post_order_data_src,
                           size_t& current_mapping,
                           size_t& id_node)
{
    HT_TimestampNs prev_ts;
    HT_TimestampNs prev_dst_ts = dst_node->data.start_ts;
    for (size_t i = 0; i < node->children.size(); ++i)
    {
        bool inserted = transform_node(node->children[i].first, 
                                      dst_node->children[i].first, 
                                      mapping, 
                                      post_order_data_src, 
                                      current_mapping, 
                                      id_node);
        if (inserted == true)
        {
            HT_DurationNs delta = dst_node->children[i].first->data.start_ts - prev_dst_ts;

            HT_DurationNs duration = node->children[i].first->data.get_duration();

            node->children[i].first->data.start_ts = prev_ts + delta;
            node->children[i].first->data.stop_ts = node->children[i].first->data.start_ts + duration;
        }
        prev_ts = node->children[i].first->data.stop_ts;
    }

    if (current_mapping != mapping.size() && mapping[current_mapping].second == id_node)
    {
        unsigned int src_index = mapping[current_mapping].first;
        node->data = post_order_data_src[src_index].data;
        node->total_duration = post_order_data_src[src_index].total_duration;
        node->total_children_duration = post_order_data_src[src_index].total_children_duration;
        ++current_mapping;
        ++id_node;
        return false;
    }
    ++id_node;
    return true;
}

static void build_copy_tree(std::shared_ptr<TreeNode> new_node,
                            std::shared_ptr<TreeNode> src_node)
{
    new_node->data = src_node->data;
    new_node->total_duration = src_node->total_duration;
    new_node->total_children_duration = src_node->total_children_duration;

    for (auto child : src_node->children)
    {
        auto new_child = std::make_shared<TreeNode>();
        new_child->parent = new_node;
        build_copy_tree(new_child, child.first);
        new_node->children.emplace_back(new_child, 1);
    }
}

std::shared_ptr<TreeNode> PatternMatching::transform_tree(std::shared_ptr<TreeNode> src_tree,
                                                          std::shared_ptr<TreeNode> dst_tree,
                                                          std::vector<std::pair<unsigned int, unsigned int>>& mapping)
{
    auto new_tree = std::make_shared<TreeNode>();
    build_copy_tree(new_tree, dst_tree);

    std::sort(mapping.begin(), mapping.end(), [](const std::pair<unsigned int, unsigned int>& m1,
                                                 const std::pair<unsigned int, unsigned int>& m2){
            return m1.second < m2.second;
            });

    std::vector<TreeNodeData> post_order_data_src;
    OrderedTreeEditingDistance::compute_post_order_data(src_tree, post_order_data_src);

    size_t id_node = 0;
    size_t current_mapping = 0;
    transform_node(new_tree, dst_tree, mapping, post_order_data_src, current_mapping, id_node);
    return new_tree;
}

} // namespace anomaly
} // namespace HawkTracer
