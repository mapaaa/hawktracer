#include "trees_to_json_converter.hpp"

namespace HawkTracer
{
namespace anomaly
{

bool TreesToJsonConverter::convert(const std::vector<std::pair<std::shared_ptr<TreeNode>, int>>& trees,
                                   const std::string& file_name)
{
    _output_file.open(file_name);
    if (!_output_file.is_open())
    {
        return false;
    }
    
    _output_file << "{\n \"trees\" : [\n";
    for (const auto& tree : trees)
    {
        _convert_tree(tree);
        if (tree != trees.back())
        {
            _output_file << ",";
        }
        _output_file << "\n";
    }
    _output_file << "]\n}\n";

    return true;
}

void TreesToJsonConverter::_convert_tree(const std::pair<std::shared_ptr<TreeNode>, int>& tree)
{
    _output_file << "[";

    int node_id = _gen_next_id();
    std::vector<int> child_id;
    _print_node(tree.first, tree.second, 0, node_id, child_id);
    for (size_t i = 0; i < child_id.size(); ++i)
    {
        _dfs(tree.first->children[i].first, node_id, child_id[i]);
    }
    _output_file.seekp(-1, std::ios_base::cur);
    _output_file << "]";
}

void TreesToJsonConverter::_dfs(const std::shared_ptr<TreeNode>& node,
                                int parent_id,
                                int id)
{
    std::vector<int> child_id;
    _print_node(node, 0, parent_id, id, child_id);
    for (size_t i = 0; i < child_id.size(); ++i)
    {
        _dfs(node->children[i].first, id, child_id[i]);
    }
}

void TreesToJsonConverter::_print_node(const std::shared_ptr<TreeNode>& node,
                                       int cnt_calls,
                                       int parent_id, 
                                       int id,
                                       std::vector<int>& child_id)
{
    _output_file << "{\n";

    _output_file << "\"id\": " << id << ",\n";
    _output_file << "\"label\": \"" << node->data.label << "\",\n";
    _output_file << "\"cnt_calls\": " << cnt_calls << ",\n";
    _output_file << "\"last_start_ts\": " << node->data.start_ts << ",\n";
    _output_file << "\"last_stop_ts\": " << node->data.stop_ts << ",\n";
    _output_file << "\"total_dur\": " << node->total_duration << ",\n";
    _output_file << "\"total_children_dur\": " << node->total_children_duration << ",\n";
    _output_file << "\"parent_id\": " <<  parent_id << ",\n";
    _output_file << "\"children\": [\n";

    for (const auto& child : node->children)
    {
        child_id.push_back(_gen_next_id());
        _output_file << "{\n";
        _output_file << "\"id\": " << child_id.back() << ",\n";
        _output_file << "\"cnt_calls\": " << child.second << "\n";
        _output_file << "}";
        if (child != node->children.back())
        {
            _output_file << ",";
        }
        _output_file << "\n";
    }
    _output_file << "]\n},";
}

int TreesToJsonConverter::_gen_next_id()
{
    ++_id;
    return _id;
}

} // namespace generator
} // namespace HawkTracer
