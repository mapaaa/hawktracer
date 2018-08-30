#ifndef HT_GENERATOR_TREES_TO_JSON_CONVERTER_HPP
#define HT_GENERATOR_TREES_TO_JSON_CONVERTER_HPP

#include <client/call_graph.hpp>
#include <fstream>

using TreeNode = HawkTracer::client::CallGraph::TreeNode;

namespace HawkTracer
{
namespace anomaly
{

class TreesToJsonConverter
{
public:
    bool convert(const std::vector<std::pair<std::shared_ptr<TreeNode>, int>>& trees,
                 const std::string& file_name);

private:
    std::ofstream _output_file; 
    int _id = 0;
    void _convert_tree(const std::pair<std::shared_ptr<TreeNode>, int>& tree);
    void _dfs(const std::shared_ptr<TreeNode>& node,
              int parent_id,
              int id);
    void _print_node(const std::shared_ptr<TreeNode>& node,
                     int cnt_calls,
                     int parent_id,
                     int id,
                     std::vector<int>& child_id);
    int _gen_next_id();
};

} // namespace uitiliy
} // namespaec HawkTracer

#endif // HT_GENERATOR_TREES_TO_JSON_CONVERTER_HPP
