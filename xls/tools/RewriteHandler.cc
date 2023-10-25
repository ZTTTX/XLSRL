#include "RewriteHandler.h"

namespace xls {

NodeHandler::NodeHandler(Package* package) : p_(package) {
    InitializeHandlerMap();
}

bool NodeHandler::AreDependenciesSatisfied(const JsonNode& node) {
    for (const auto& operand : node.Operands) {
        if (NodeMap_.find(operand) == NodeMap_.end()) {
            if (!CurFunc_->GetNode(operand).ok()) {
                return false; 
            }
        }
    }
    return true;
}

void NodeHandler::SetCurrentFunction(Function* func) { CurFunc_ = func; }

void NodeHandler::SetNodeMap(const std::unordered_map<std::string, Node*>& node_map) {
    NodeMap_ = node_map;
}

void NodeHandler::UpdateNodeMap(const std::string& key, Node* value) {
    NodeMap_[key] = value;
}

void NodeHandler::InitializeHandlerMap() {
    // Register all operation handlers here.
    handler_map_["kAdd"] = [this](const JsonNode& node) { return this->HandlekAdd(node);};

}
absl::Status NodeHandler::HandleKill(const JsonNode& node)  {
    XLS_ASSIGN_OR_RETURN(Node* NodeToRemove, CurFunc_->GetNode(node.OperationName));
    XLS_RETURN_IF_ERROR(CurFunc_->RemoveNode(NodeToRemove));
    NodeMap_.erase(node.OperationName); 
    return absl::OkStatus();
}
absl::Status NodeHandler::HandleSubstitution(const JsonNode& node) {
    Node* NewNode = NodeMap_[node.ReplaceSelfWith];
    XLS_ASSIGN_OR_RETURN(Node* OldNode, CurFunc_->GetNode(node.OperationName));
    XLS_RETURN_IF_ERROR(OldNode->ReplaceUsesWith(NewNode));
    return absl::OkStatus();
}

absl::Status HandleSingleSub(Package* p, const JsonSingleSub& sub) {
    NodeHandler handler(p);
    XLS_ASSIGN_OR_RETURN(Function* CurFunc, p->GetFunction(sub.FuncName));
    handler.SetCurrentFunction(CurFunc);
    std::unordered_map<std::string, Node*> NodeMap;
    std::unordered_map<std::string, JsonNode> NodeGenMap;

    for (const auto& node : sub.NodesInvolved) {
    //Set a map for all nodes related to this rewrite
        if (node.ReplaceSelfWith != "Gen" && node.ReplaceSelfWith != "Kill") {
            for (const auto& refnode : node.Operands) {
                XLS_ASSIGN_OR_RETURN(Node* RefNode, CurFunc->GetNode(refnode));
                NodeMap[refnode] = RefNode;
            }
        }
        if (node.ReplaceSelfWith == "Gen") {
            NodeGenMap[node.OperationName] = node;
        }
    }

    handler.SetNodeMap(NodeMap);

    bool nodesWereProcessed = true; 
    while (nodesWereProcessed) {
        nodesWereProcessed = false;
        auto it = NodeGenMap.begin();
        while (it != NodeGenMap.end()) {
            const auto& node = it->second;
            if (handler.AreDependenciesSatisfied(node)) {
                XLS_RETURN_IF_ERROR(handler.DispatchNodeOperation(node));
                it = NodeGenMap.erase(it);
                nodesWereProcessed = true;
            } else {
                ++it; 
            }
        }
    }
    if (!NodeGenMap.empty()) {
        return absl::InvalidArgumentError("One or more node cannot be generated due to dependency");
    }

    for (const auto& node : sub.NodesInvolved) {
    //Substitude output nodes
        if (node.ReplaceSelfWith != "Gen" && node.ReplaceSelfWith != "Kill") {
            XLS_RETURN_IF_ERROR(handler.HandleSubstitution(node));
        }
    }
    for (const auto& node : sub.NodesInvolved) {
    //Kill every node that has kill or substitution flag
        if (node.ReplaceSelfWith != "Gen") {
            XLS_RETURN_IF_ERROR(handler.HandleKill(node));
        }
    }

    return absl::OkStatus();
}

absl::Status NodeHandler::DispatchNodeOperation(const JsonNode& node) {
    const auto& op_type = node.OperationType;
    auto it = handler_map_.find(op_type);
    if (it != handler_map_.end()) {
        return it->second(node);  // Dispatch to the appropriate handler.
    }
    return absl::UnknownError("[Error] Unsupported operation type");
}

absl::Status NodeHandler::HandlekAdd(const JsonNode& node) {
    const auto& operands = node.Operands;
    if (operands.size() < 2) {
        return absl::InvalidArgumentError("Expected at least two operands for kAdd operation.");
    }  
    Node* operand1;
    Node* operand2;
    if (CurFunc_->GetNode(node.Operands[0]).ok()) {
        XLS_ASSIGN_OR_RETURN(operand1, CurFunc_->GetNode(node.Operands[0]));
    } else {
        operand1 = NodeMap_[node.Operands[0]];
    }
    if (CurFunc_->GetNode(node.Operands[1]).ok()) {
        XLS_ASSIGN_OR_RETURN(operand2, CurFunc_->GetNode(node.Operands[1]));
    } else {
        operand2 = NodeMap_[node.Operands[1]];
    }
    XLS_ASSIGN_OR_RETURN(Node* NewNode, CurFunc_->MakeNode<BinOp>(operand2->loc(), operand1, operand2, Op::kAdd));
    NodeMap_[node.OperationName] = NewNode;

    return absl::OkStatus();
}

}  // namespace xls
