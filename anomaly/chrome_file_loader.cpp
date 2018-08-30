#include "chrome_file_loader.hpp"

#include <utils/json/jsonxx_utils.hpp>

#include <algorithm>

namespace HawkTracer
{
namespace anomaly
{

bool ChromeFileLoader::init(const std::string& file_name)
{
    _file.open(file_name);
    if (!_file.is_open())
    {
        return false;
    }
    return _parse_file();
}

std::vector<std::pair<std::shared_ptr<TreeNode>, int>> ChromeFileLoader::get_trees()
{
    return _trees;
}

bool ChromeFileLoader::_parse_file()
{
    _json_obj.parse(_file);
    jsonxx::Array events_array;
    bool events_read = HawkTracer::utils::get_value<jsonxx::Array, jsonxx::Array>(_json_obj, "traceEvents", events_array);
    if (!events_read)
    {
        return false;
    }
    for (const auto& event : events_array.values())
    {
        jsonxx::Object event_obj = *event->object_value_;
        std::string label;
        HT_ThreadId thread_id;
        HT_DurationNs dur;
        HT_TimestampNs ts;
        bool label_read = HawkTracer::utils::get_value<std::string, jsonxx::String>(event_obj, "name", label);
        bool thread_id_read = HawkTracer::utils::get_value<HT_ThreadId, jsonxx::Number>(event_obj, "tid", thread_id);
        bool dur_read = HawkTracer::utils::get_value<HT_DurationNs, jsonxx::Number>(event_obj, "dur", dur);
        bool ts_read = HawkTracer::utils::get_value<HT_TimestampNs, jsonxx::Number>(event_obj, "ts", ts);
        if (!label_read || !thread_id_read || !dur_read || !ts_read)
        {
            return false;
        }
        _events[thread_id].emplace_back(label, ts, dur);
    }
    for (auto& thread : _events)
    {
        _make_trees(thread.second);
    }
    return true;
}

void ChromeFileLoader::_make_trees(std::vector<NodeData>& events)
{
    _current_call = nullptr;
    std::sort(events.begin(), events.end(), 
            [](const NodeData& e1, const NodeData& e2){
                return e1.start_ts < e2.start_ts;
            });
    for (const auto& event : events)
    {
        _add_event(event);
    }

}

void ChromeFileLoader::_add_event(const NodeData& node_data)
{
    bool added = _try_add_event_to_existing_calltree(node_data);
    if (!added) 
    {
        _add_new_calltree(node_data);
    }
}

bool ChromeFileLoader::_try_add_event_to_existing_calltree(const NodeData& node_data)
{
    while(_current_call) 
    {
        bool current_call_contains_new_event = 
            _current_call->data.start_ts <= node_data.start_ts && node_data.stop_ts <= _current_call->data.stop_ts;

        if (current_call_contains_new_event)
        {
            _add_new_event_call(_current_call, node_data);
            return true;
        }
        else 
        {
            _current_call = _current_call->parent.lock();
        }
    }
    return false;
}

void ChromeFileLoader::_add_new_event_call(const std::shared_ptr<TreeNode>& caller,
                                           const NodeData& node_data)
{
    caller->total_children_duration += node_data.get_duration();
    _current_call = _add_new_call(node_data, caller, caller->children);
}

std::shared_ptr<TreeNode> ChromeFileLoader::_add_new_call(const NodeData& node_data,
                                                          const std::shared_ptr<TreeNode>& parent,
                                                          std::vector<std::pair<std::shared_ptr<TreeNode>, int>>& calls)

{
    std::shared_ptr<TreeNode> event_node = std::make_shared<TreeNode>(node_data);
    event_node->parent = parent;
    event_node->total_duration = event_node->data.get_duration();
    event_node->total_children_duration = 0;
    calls.emplace_back(event_node, 1);

    return event_node;
}

void ChromeFileLoader::_add_new_calltree(const NodeData& node_data)
{
    _current_call = _add_new_call(node_data, nullptr, _trees);
}

} // namespace anomaly
} // namespace HawkTracer

