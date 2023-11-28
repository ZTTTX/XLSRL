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

absl::StatusOr<std::vector<Node*>> NodeHandler::HandleKill(const JsonNode& node, std::vector<Node*> DummyNodeVec)  {
    XLS_ASSIGN_OR_RETURN(Node* DummyNode, CurFunc_->MakeNode<Literal>(InsertionPointNode_->loc(), Value(UBits(0, node.BitWidth))));
    XLS_ASSIGN_OR_RETURN(Node* NodeToRemove, CurFunc_->GetNode(node.OperationName));
    XLS_RETURN_IF_ERROR(NodeToRemove->ReplaceUsesWith(DummyNode));
    XLS_RETURN_IF_ERROR(CurFunc_->RemoveNode(NodeToRemove));
    NodeMap_.erase(node.OperationName); 
    DummyNodeVec.push_back(DummyNode);
    return DummyNodeVec;
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
        std::string error_msg = "One or more nodes cannot be generated due to dependency. Details:\n";
        for (const auto& pair : NodeGenMap) {
            error_msg += "Key: " + pair.first + " Value: " + pair.second.toString() + "\n";
        }
        return absl::InvalidArgumentError(error_msg);
    }


    for (const auto& node : sub.NodesInvolved) {
    //Substitude output nodes
        if (node.ReplaceSelfWith != "Gen" && node.ReplaceSelfWith != "Kill") {
            XLS_RETURN_IF_ERROR(handler.HandleSubstitution(node));
        }
    }
    std::vector<Node*> DummyNodeVec;
    for (const auto& node : sub.NodesInvolved) {
    //Kill every node that has kill or substitution flag
        if (node.ReplaceSelfWith != "Gen") {
            XLS_ASSIGN_OR_RETURN(DummyNodeVec, handler.HandleKill(node, DummyNodeVec));
        }
    }
    for (Node* DummyNode : DummyNodeVec) {
        XLS_RETURN_IF_ERROR(CurFunc->RemoveNode(DummyNode));
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

void NodeHandler::InitializeHandlerMap() {
    // Register all operation handlers here.
    handler_map_["kAdd"] = [this](const JsonNode& node) { return this->HandlekAdd(node);};
    handler_map_["kSub"] = [this](const JsonNode& node) { return this->HandlekSub(node);};
    handler_map_["kUMul"] = [this](const JsonNode& node) { return this->HandlekUMul(node);};
    handler_map_["kSMul"] = [this](const JsonNode& node) { return this->HandlekSMul(node);};
    handler_map_["kUDiv"] = [this](const JsonNode& node) { return this->HandlekUMul(node);};
    handler_map_["kSDiv"] = [this](const JsonNode& node) { return this->HandlekUMul(node);};
    handler_map_["Literal"] = [this](const JsonNode& node) { return this->HandleLiteral(node);};
    handler_map_["kNeg"] = [this](const JsonNode& node) { return this->HandlekNeg(node);};
    handler_map_["kShll"] = [this](const JsonNode& node) { return this->HandlekShll(node);};
    handler_map_["kShrl"] = [this](const JsonNode& node) { return this->HandlekShll(node);};
    handler_map_["kNot"] = [this](const JsonNode& node) { return this->HandlekNot(node);};
    handler_map_["kConcat"] = [this](const JsonNode& node) { return this->HandlekConcat(node);};


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

absl::Status NodeHandler::HandlekSub(const JsonNode& node) {
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
    XLS_ASSIGN_OR_RETURN(Node* NewNode, CurFunc_->MakeNode<BinOp>(operand2->loc(), operand1, operand2, Op::kSub));
    NodeMap_[node.OperationName] = NewNode;

    return absl::OkStatus();
}

absl::Status NodeHandler::HandlekUMul(const JsonNode& node) {
    //Generates a kUMul node, bitwidth must be specified.
    const auto& operands = node.Operands;
    if (operands.size() != 2) {
        return absl::InvalidArgumentError("Expected two operands for kUMul operation.");
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

absl::Status NodeHandler::HandlekSMul(const JsonNode& node) {
    //Generates a kSMul node, bitwidth must be specified.
    const auto& operands = node.Operands;
    if (operands.size() != 2) {
        return absl::InvalidArgumentError("Expected two operands for kSMul operation.");
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
    XLS_ASSIGN_OR_RETURN(Node* NewNode, CurFunc_->MakeNode<ArithOp>(operand2->loc(), operand1, operand2, node.BitWidth, Op::kSMul));
    NodeMap_[node.OperationName] = NewNode;
    return absl::OkStatus();
}

absl::Status NodeHandler::HandlekUDiv(const JsonNode& node) {
    //Generates a kUDiv node, bitwidth must be specified.
    const auto& operands = node.Operands;
    if (operands.size() != 2) {
        return absl::InvalidArgumentError("Expected two operands for kUDiv operation.");
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
    XLS_ASSIGN_OR_RETURN(Node* NewNode, CurFunc_->MakeNode<ArithOp>(operand2->loc(), operand1, operand2, node.BitWidth, Op::kUDiv));
    NodeMap_[node.OperationName] = NewNode;
    return absl::OkStatus();
}

absl::Status NodeHandler::HandlekSDiv(const JsonNode& node) {
    //Generates a kSDiv node, bitwidth must be specified.
    const auto& operands = node.Operands;
    if (operands.size() != 2) {
        return absl::InvalidArgumentError("Expected two operands for kSDiv operation.");
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
    XLS_ASSIGN_OR_RETURN(Node* NewNode, CurFunc_->MakeNode<ArithOp>(operand2->loc(), operand1, operand2, node.BitWidth, Op::kSDiv));
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

absl::Status NodeHandler::HandlekNeg(const JsonNode& node) {
        const auto& operands = node.Operands;
    if (operands.size() != 1) {
        return absl::InvalidArgumentError("Expected one operands for kNeg operation.");
    }  
    Node* operand1;
    if (CurFunc_->GetNode(node.Operands[0]).ok()) {
        XLS_ASSIGN_OR_RETURN(operand1, CurFunc_->GetNode(node.Operands[0]));
    } else {
        operand1 = NodeMap_[node.Operands[0]];
    }
    XLS_ASSIGN_OR_RETURN(Node* NewNode, CurFunc_->MakeNode<UnOp>(operand1->loc(), operand1, Op::kNeg));
    NodeMap_[node.OperationName] = NewNode;

    return absl::OkStatus();
}

absl::Status NodeHandler::HandlekShll(const JsonNode& node) {
    const auto& operands = node.Operands;
    if (operands.size() != 2) {
        return absl::InvalidArgumentError("Expected two operands for kShll operation.");
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
    XLS_ASSIGN_OR_RETURN(Node* NewNode, CurFunc_->MakeNode<BinOp>(operand2->loc(), operand1, operand2, Op::kShll));
    NodeMap_[node.OperationName] = NewNode;
    return absl::OkStatus();
}

absl::Status NodeHandler::HandlekShrl(const JsonNode& node) {
    const auto& operands = node.Operands;
    if (operands.size() != 2) {
        return absl::InvalidArgumentError("Expected two operands for kShrl operation.");
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
    XLS_ASSIGN_OR_RETURN(Node* NewNode, CurFunc_->MakeNode<BinOp>(operand2->loc(), operand1, operand2, Op::kShrl));
    NodeMap_[node.OperationName] = NewNode;
    return absl::OkStatus();
}

absl::Status NodeHandler::HandlekNot(const JsonNode& node) {
    const auto& operands = node.Operands;
    if (operands.size() != 1) {
        return absl::InvalidArgumentError("Expected one operands for kNot operation.");
    }  
    Node* operand1;
    if (CurFunc_->GetNode(node.Operands[0]).ok()) {
        XLS_ASSIGN_OR_RETURN(operand1, CurFunc_->GetNode(node.Operands[0]));
    } else {
        operand1 = NodeMap_[node.Operands[0]];
    }
    XLS_ASSIGN_OR_RETURN(Node* NewNode, CurFunc_->MakeNode<UnOp>(operand1->loc(), operand1, Op::kNot));
    NodeMap_[node.OperationName] = NewNode;

    return absl::OkStatus();
}

absl::Status NodeHandler::HandlekConcat(const JsonNode& node) {
    std::vector<Node*> OperandsInvolved;
    const auto& operands = node.Operands;
    Node* TempOperand;
    for (const auto& CurOperand : operands) {
        if (CurFunc_->GetNode(CurOperand).ok()) {
            XLS_ASSIGN_OR_RETURN(TempOperand, CurFunc_->GetNode(CurOperand));
        } else {
            TempOperand = NodeMap_[CurOperand];
        }
        OperandsInvolved.push_back(TempOperand);
    }
    XLS_ASSIGN_OR_RETURN(Node* NewNode, CurFunc_->MakeNode<Concat>(TempOperand->loc(), absl::MakeSpan(OperandsInvolved)));
    NodeMap_[node.OperationName] = NewNode;
}

}  // namespace xls
