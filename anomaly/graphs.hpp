#ifndef HAWKTRACER_ANOMALY_PATTERNS_HPP
#define HAWKTRACER_ANOMALY_PATTERNS_HPP

#include "file_loader.hpp"

#include <fstream>

namespace HawkTracer
{
namespace anomaly
{

class Graphs
{
public:
    Graphs(std::shared_ptr<FileLoader> file_loader);
    bool load_from_file(const std::string& file_name);
    std::vector<std::pair<std::shared_ptr<client::CallGraph::TreeNode>, int>> get_trees();

private:
    std::shared_ptr<FileLoader> _file_loader;
    std::vector<std::pair<std::shared_ptr<client::CallGraph::TreeNode>, int>> _trees;
};

} // namespace anomaly
} // namespace HawkTracer

#endif // HAWKTRACER_ANOMALY_PATTERNS_HPP
