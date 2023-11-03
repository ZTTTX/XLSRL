#ifndef REWRITE_HANDLER_H
#define REWRITE_HANDLER_H

#include "JsonHandler.h"  // Include the header file for JsonNode
#include "xls/ir/function_base.h"
#include "xls/ir/function.h"
#include "xls/common/status/status_macros.h"
#include "xls/common/exit_status.h"

#include <string>
#include <functional>
#include <unordered_map>

namespace xls {

class NodeHandler;

// Function to handle a single substitution.
absl::Status HandleSingleSub(Package* p, const JsonSingleSub& sub);

class NodeHandler {
public:
    explicit NodeHandler(Package* package); 
    absl::Status DispatchNodeOperation(const JsonNode& node);
    void SetCurrentFunction(Function* func);
    void SetNodeMap(const std::unordered_map<std::string, Node*>& node_map);
    void SetInsertionPointNode(Node* LastNode);
    absl::StatusOr<std::vector<Node*>> HandleKill(const JsonNode& node,  std::vector<Node*> DummyNodeVec);
    absl::Status HandleSubstitution(const JsonNode& node);
    bool AreDependenciesSatisfied(const JsonNode& node);
private:
    Package* p_; 
    Function* CurFunc_;
    std::unordered_map<std::string, Node*> NodeMap_;
    Node* InsertionPointNode_;

    std::unordered_map<std::string, std::function<absl::Status(const JsonNode&)>> handler_map_;

    //ToDo add more handler
    absl::Status HandlekAdd(const JsonNode& node);
    absl::Status HandlekUMul(const JsonNode& node);
    absl::Status HandleLiteral(const JsonNode& node);


    // Helper method to initialize the handler map.
    void InitializeHandlerMap();
};


}  // namespace xls

#endif