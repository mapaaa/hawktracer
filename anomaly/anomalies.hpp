#ifndef HAWKTRACER_ANOMALY_ANOMALIES_HPP
#define HAWKTRACER_ANOMALY_ANOMALIES_HPP

#include "config.hpp"
#include <client/call_graph.hpp>

using TreeNode = HawkTracer::client::CallGraph::TreeNode;

namespace HawkTracer
{
namespace anomaly
{

class Anomalies
{
public:
    Anomalies(std::shared_ptr<Config> config);
    double get_score(std::shared_ptr<TreeNode> pattern,
                     std::shared_ptr<TreeNode> source);

private:
    std::shared_ptr<Config> _config;

    double _score(std::shared_ptr<TreeNode> source_node,
                  std::shared_ptr<TreeNode> pattern_node,
                  unsigned int& cnt_nodes);
    double _children_score(std::vector<std::pair<std::shared_ptr<TreeNode>, int>>& children1,
                           HT_DurationNs total_duration1,
                           std::vector<std::pair<std::shared_ptr<TreeNode>, int>>& children2,
                           HT_DurationNs total_duration2);
    double _children_start_times_scores(std::shared_ptr<TreeNode>& source_node,
                                        std::shared_ptr<TreeNode>& pattern_node);
    double _get_percentage(HT_DurationNs x, HT_DurationNs y);
};

} // namespace anomaly
} // namespace HawkTracer

#endif // HAWKTRACER_ANOMALY_ANOMALIES_HPP
