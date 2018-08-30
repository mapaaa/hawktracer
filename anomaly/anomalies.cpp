#include "anomalies.hpp"

namespace HawkTracer
{
namespace anomaly
{

Anomalies::Anomalies(std::shared_ptr<Config> config) : 
    _config(std::move(config))
{
}

double Anomalies::get_score(std::shared_ptr<TreeNode> pattern,
                            std::shared_ptr<TreeNode> source)
{
    unsigned int cnt_nodes = 0;
    // anomaly_score is a number between 0 and 1. 
    // The more similar the durations in the trees are, the closer the score is to 0
    // It is computed as an average between all nodes
    double anomaly_score = _score(source, pattern, cnt_nodes);
    if (cnt_nodes)
    {
        anomaly_score = anomaly_score / cnt_nodes;
    }
    return anomaly_score;
}

double Anomalies::_score(std::shared_ptr<TreeNode> source_node,
                         std::shared_ptr<TreeNode> pattern_node,
                         unsigned int& cnt_nodes)
{
    ++cnt_nodes;

    double node_score = 0;
    int cnt_conditions = 0;

    if (_config->get_consider_dur())
    {
        ++cnt_conditions;
        double duration_score;
        if (std::max(source_node->total_duration, pattern_node->total_duration) == 0)
        {
            duration_score = 0;
        }
        else
        {
            duration_score = 1.0 -  1.0 * std::min(source_node->total_duration, pattern_node->total_duration) /
                                          std::max(source_node->total_duration, pattern_node->total_duration);
        }
        node_score += duration_score;
    }

    if (_config->get_consider_children_prop() && source_node->children.size())
    {
        ++cnt_conditions;
        node_score += _children_score(source_node->children,
                                      source_node->total_duration, 
                                      pattern_node->children,
                                      pattern_node->total_duration);
    }

    if (_config->get_consider_relative_start_time() && source_node->children.size())
    {
        ++cnt_conditions;
        node_score += _children_start_times_scores(source_node, pattern_node);
    }

    if (cnt_conditions)
    {
        node_score /= cnt_conditions;
    }

    double children_scores = 0;
    for (size_t i = 0; i < source_node->children.size(); ++i)
    {
        children_scores += _score(source_node->children[i].first,
                                  pattern_node->children[i].first,
                                  cnt_nodes);
    }

    node_score += children_scores;
    return node_score;
}

double Anomalies::_children_score(std::vector<std::pair<std::shared_ptr<TreeNode>, int>>& children1,
                                  HT_DurationNs total_duration1,
                                  std::vector<std::pair<std::shared_ptr<TreeNode>, int>>& children2,
                                  HT_DurationNs total_duration2)
{
    double children_score = 0;
    for (size_t i = 0; i < children1.size(); ++i)
    {
        double source_child_prop;
        double pattern_child_prop;
        if (children1[i].first->total_duration)
        {
            source_child_prop = _get_percentage(total_duration1, children1[i].first->total_duration);
        }
        else
        {
            source_child_prop = 100;
        }
        if (children2[i].first->total_duration)
        {
            pattern_child_prop = _get_percentage(total_duration2, children2[i].first->total_duration);
        }
        else
        {
            pattern_child_prop = 100;
        }
        if (std::max(source_child_prop, pattern_child_prop))
        {
            children_score += 1.0 - 1.0 * std::min(source_child_prop, pattern_child_prop) /
                                          std::max(source_child_prop, pattern_child_prop);
        }
    }
    if (!children1.empty())
    {
        children_score /= children1.size();
    }
    return children_score; 
}

double Anomalies::_children_start_times_scores(std::shared_ptr<TreeNode>& source_node,
                                               std::shared_ptr<TreeNode>& pattern_node)
{
    double score = 0;
    HT_TimestampNs prev_source_stop_ts = source_node->data.start_ts;
    HT_TimestampNs prev_pattern_stop_ts = pattern_node->data.start_ts;
    for (size_t i = 0; i < source_node->children.size(); ++i)
    {
        HT_DurationNs source_delta;
        if (source_node->children[i].first->data.start_ts > prev_source_stop_ts)
        {
            source_delta = source_node->children[i].first->data.start_ts - prev_source_stop_ts;
        }
        else
        {
            source_delta = prev_source_stop_ts - source_node->children[i].first->data.start_ts;
        }

        HT_DurationNs pattern_delta;
        if (pattern_node->children[i].first->data.start_ts > prev_pattern_stop_ts)
        {
            pattern_delta = pattern_node->children[i].first->data.start_ts - prev_pattern_stop_ts;
        }
        else
        {
            pattern_delta = prev_pattern_stop_ts - pattern_node->children[i].first->data.start_ts;
        }

        if (std::max(source_delta, pattern_delta))
        {
            score +=  1.0 - 1.0 * std::min(source_delta, pattern_delta) /
                                  std::max(source_delta, pattern_delta);
        }
        prev_source_stop_ts = source_node->children[i].first->data.stop_ts;
        prev_pattern_stop_ts = pattern_node->children[i].first->data.stop_ts;
    }
    if (source_node->children.size())
    {
        score /= source_node->children.size();
    }
    return score;
}

double Anomalies::_get_percentage(HT_DurationNs x, HT_DurationNs y)
{
    if (x == 0)
    {
        return 0;
    }
    return (double)y * 100 / x;
}

} // namespace anomaly
} // namespace HawkTracer
