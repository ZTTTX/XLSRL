#include "RewriteHandler.h"

namespace xls {

NodeHandler::NodeHandler(Package* package) : p_(package) {
    InitializeHandlerMap();
}

void NodeHandler::SetInsertionPointNode(Node* LastNode) {
    InsertionPointNode_ = LastNode;
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
    Node* RefNode;
    for (const auto& node : sub.NodesInvolved) {
    //Set a map for all nodes related to this rewrite
        if (node.ReplaceSelfWith != "Gen" && node.ReplaceSelfWith != "Kill") {
            for (const auto& refnode : node.Operands) {
                XLS_ASSIGN_OR_RETURN(RefNode, CurFunc->GetNode(refnode));
                NodeMap[refnode] = RefNode;
            }
        }
        if (node.ReplaceSelfWith == "Gen") {
            NodeGenMap[node.OperationName] = node;
        }
    }

    handler.SetNodeMap(NodeMap);
    handler.SetInsertionPointNode(RefNode);

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

void NodeHandler::InitializeHandlerMap() {
    // Register all operation handlers here.
    handler_map_["kAdd"] = [this](const JsonNode& node) { return this->HandlekAdd(node);};
    handler_map_["kUMul"] = [this](const JsonNode& node) { return this->HandlekUMul(node);};
    handler_map_["Literal"] = [this](const JsonNode& node) { return this->HandleLiteral(node);};

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
    if (operands.size() != 2) {
        return absl::InvalidArgumentError("Expected two operands for kAdd operation.");
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

absl::Status NodeHandler::HandlekUMul(const JsonNode& node) {
    //Generates a kUMul node, bitwidth must be specified.
    const auto& operands = node.Operands;
    if (operands.size() != 2) {
        return absl::InvalidArgumentError("Expected two operands for kAdd operation.");
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
    XLS_ASSIGN_OR_RETURN(Node* NewNode, CurFunc_->MakeNode<ArithOp>(operand2->loc(), operand1, operand2, node.BitWidth, Op::kUMul));
    NodeMap_[node.OperationName] = NewNode;
    return absl::OkStatus();
}

absl::Status NodeHandler::HandleLiteral(const JsonNode& node) {
    //Generates a literal (constant) node at the default location, because it lacks dependency
    //Uses bitwdith value for the width, if bitwidth is not defined it will use smallest possible width
    Node* NewNode;
    if (node.BitWidth > 0) {
        XLS_ASSIGN_OR_RETURN(NewNode, CurFunc_->MakeNode<Literal>(InsertionPointNode_->loc(), Value(UBits(node.Value, node.BitWidth))));
    } else {
        XLS_ASSIGN_OR_RETURN(NewNode, CurFunc_->MakeNode<Literal>(InsertionPointNode_->loc(), Value(UBits(node.Value, Bits::MinBitCountUnsigned(node.Value)))));
    }
    NodeMap_[node.OperationName] = NewNode;
    return absl::OkStatus();
}

}  // namespace xls
