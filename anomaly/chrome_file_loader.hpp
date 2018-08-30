#ifndef HAWKTRACER_ANOMALY_CHROME_FILE_LOADER_HPP
#define HAWKTRACER_ANOMALY_CHROME_FILE_LOADER_HPP

#include "file_loader.hpp"

#include <client/call_graph.hpp>
#include <thirdparty/jsonxx/jsonxx.h>

#include <fstream>

using NodeData = HawkTracer::client::CallGraph::NodeData;
using TreeNode = HawkTracer::client::CallGraph::TreeNode;

namespace HawkTracer
{
namespace anomaly
{

class ChromeFileLoader : public FileLoader
{
public:
    bool init(const std::string& file_name) override;
    std::vector<std::pair<std::shared_ptr<TreeNode>, int>> get_trees() override;

private:
    bool _parse_file();
    void _make_trees(std::vector<NodeData>& events);
    void _add_event(const NodeData& node_data);
    bool _try_add_event_to_existing_calltree(const NodeData& node_data);
    void _add_new_event_call(const std::shared_ptr<TreeNode>& caller,
                             const NodeData& node_data);
    void _add_new_calltree(const NodeData& node_data);
    std::shared_ptr<TreeNode> _add_new_call(const NodeData& node_data,
                                            const std::shared_ptr<TreeNode>& parent,
                                            std::vector<std::pair<std::shared_ptr<TreeNode>, int>>& calls);
    std::ifstream _file;
    jsonxx::Object _json_obj;
    std::vector<std::pair<std::shared_ptr<TreeNode>, int>> _trees;
    std::unordered_map<int, std::vector<NodeData>> _events;
    std::shared_ptr<TreeNode> _current_call;
};

} // namespace anomaly
} // namespace HawkTracer

#endif // HAWKTRACER_ANOMALY_CHROME_FILE_LOADER_HPP
