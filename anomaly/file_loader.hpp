#ifndef HAWKTRACER_ANOMALY_FILE_LOADER_HPP
#define HAWKTRACER_ANOMALY_FILE_LOADER_HPP

#include <client/call_graph.hpp>

#include <fstream>
#include <vector>

using TreeNode = HawkTracer::client::CallGraph::TreeNode;

namespace HawkTracer
{
namespace anomaly
{

class FileLoader
{
public:
    virtual ~FileLoader() {}
    virtual bool init(const std::string& file_name) = 0;
    virtual std::vector<std::pair<std::shared_ptr<TreeNode>, int>> get_trees() = 0;
};

} // namespace anomaly
} // namespace HawkTracer

#endif // HAWKTRACER_ANOMALY_FILE_LOADER_HPP
