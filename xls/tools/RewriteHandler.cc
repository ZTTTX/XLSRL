#include "RewriteHandler.h"

namespace xls {
// namespace {

RewriteHandler::RewriteHandler(Package* package) : p(package) {
  // Initialize the handler map
    HandlerMap["Commutativity"] = [this](const JsonSingleSub& sub) {absl::Status status = HandleCommutativity(sub);};
    HandlerMap["Associativity"] = [this](const JsonSingleSub& sub) {absl::Status status =  HandleAssociativity(sub);};
    HandlerMap["DistributeMultOverAdd"] = [this](const JsonSingleSub& sub) {absl::Status status =  HandleDistributeMultOverAdd(sub);};
    HandlerMap["SumSame"] = [this](const JsonSingleSub& sub) {absl::Status status =  HandleSumSame(sub);};
}

void RewriteHandler::HandleSubstitution(const JsonSingleSub& sub) {
    std::string type = sub.TypeOfSub;
    // Find the appropriate handler based on the type and call it.
    if (HandlerMap.find(type) != HandlerMap.end()) {
        HandlerMap[type](sub);
    } else {
        printf("[Error] Unsupport Subsitution Type");
        exit(1);
    }

}

// p is the package for all handler functions

absl::Status RewriteHandler::HandleCommutativity(const JsonSingleSub& sub) {
    // This function handles both add and mult commutativity.
    // It only allows one node rewrite each time.
    JsonNode JsonNodeA;
    Node* TempOperand;
    if (sub.NodesInvolved.size() != 1){
        return absl::UnknownError("[ERROR] Incorrect size for commutativity");
    } else {
        JsonNodeA = sub.NodesInvolved[0];
    }
    XLS_ASSIGN_OR_RETURN(Function* CurFuncA, p->GetFunction(JsonNodeA.FuncName));
    XLS_ASSIGN_OR_RETURN(Node* CurNodeA, CurFuncA->GetNode(JsonNodeA.OperationName));
    XLS_ASSIGN_OR_RETURN(TempOperand, CurFuncA->GetNode(JsonNodeA.Operands[0]));
    XLS_RETURN_IF_ERROR(CurNodeA->ReplaceOperandNumber(0, TempOperand));
    XLS_ASSIGN_OR_RETURN(TempOperand, CurFuncA->GetNode(JsonNodeA.Operands[1]));
    XLS_RETURN_IF_ERROR(CurNodeA->ReplaceOperandNumber(1, TempOperand));

    return absl::OkStatus();
}

absl::Status RewriteHandler::HandleAssociativity(const JsonSingleSub& sub) {
    // This function handles both add and mult associativity rewrites.
    JsonNode JsonNodeA;
    JsonNode JsonNodeB;
    Node* TempOperand;
    if (sub.NodesInvolved.size() != 2){
        return absl::UnknownError("[ERROR] Incorrect size for associativity");
    } else {
        JsonNodeA = sub.NodesInvolved[0];
        JsonNodeB = sub.NodesInvolved[1];
    }
    // For now we can only handle when two nodes are in same function, i.e., CurFuncA == CurFuncB
    // Handle Node A
    XLS_ASSIGN_OR_RETURN(Function* CurFuncA, p->GetFunction(JsonNodeA.FuncName));
    XLS_ASSIGN_OR_RETURN(Node* CurNodeA, CurFuncA->GetNode(JsonNodeA.OperationName));
    XLS_ASSIGN_OR_RETURN(TempOperand, CurFuncA->GetNode(JsonNodeA.Operands[0]));
    XLS_RETURN_IF_ERROR(CurNodeA->ReplaceOperandNumber(0, TempOperand));
    XLS_ASSIGN_OR_RETURN(TempOperand, CurFuncA->GetNode(JsonNodeA.Operands[1]));
    XLS_RETURN_IF_ERROR(CurNodeA->ReplaceOperandNumber(1, TempOperand));

    // Handle Node B
    XLS_ASSIGN_OR_RETURN(Function* CurFuncB, p->GetFunction(JsonNodeB.FuncName));
    XLS_ASSIGN_OR_RETURN(Node* CurNodeB, CurFuncB->GetNode(JsonNodeB.OperationName));
    XLS_ASSIGN_OR_RETURN(TempOperand, CurFuncB->GetNode(JsonNodeB.Operands[0]));
    XLS_RETURN_IF_ERROR(CurNodeB->ReplaceOperandNumber(0, TempOperand));
    XLS_ASSIGN_OR_RETURN(TempOperand, CurFuncB->GetNode(JsonNodeB.Operands[1]));
    XLS_RETURN_IF_ERROR(CurNodeB->ReplaceOperandNumber(1, TempOperand));   


    return absl::OkStatus();
}

absl::Status RewriteHandler::HandleDistributeMultOverAdd(const JsonSingleSub& sub) {
    JsonNode JsonNodeAdd;     
    JsonNode JsonNodeMult;
    JsonNode JsonNode_OutAdd;
    JsonNode JsonNode_OutMultA;
    JsonNode JsonNode_OutMultB;  

    // Handles the json inforamtion,
    if (sub.NodesInvolved.size() != 5) {
        return absl::UnknownError("[ERROR] Incorrect size for distribute mult over add");
    } else {
        bool HasGotA = false;
        bool AllPass = false;
        for (int i = 0; i < 5; i++) {
            if (sub.NodesInvolved[i].ReplaceSelfWith == "None" && sub.NodesInvolved[i].IsNodeOutput == 0) {
                JsonNodeAdd = sub.NodesInvolved[i];
            } else if (sub.NodesInvolved[i].ReplaceSelfWith != "None" && sub.NodesInvolved[i].IsNodeOutput == 0) {
                JsonNodeMult = sub.NodesInvolved[i];
            } else if (sub.NodesInvolved[i].ReplaceSelfWith == "Self" && sub.NodesInvolved[i].IsNodeOutput == 1) {
                JsonNode_OutAdd = sub.NodesInvolved[i];
            } else if (!HasGotA) {
                JsonNode_OutMultA = sub.NodesInvolved[i];
                HasGotA = true;
            } else {
                JsonNode_OutMultB = sub.NodesInvolved[i];
                AllPass = true;
            }
        }
        if (!AllPass) {
            return absl::UnknownError("[Error] One or more ops are undefined");
        }
    }
    Node* TempOperandA;
    Node* TempOperandB;
    //Now we locate the function. For now we assume all ops and rewrite are in one single function
    XLS_ASSIGN_OR_RETURN(Function* CurFunc, p->GetFunction(JsonNodeAdd.FuncName));
    XLS_ASSIGN_OR_RETURN(Node* NodeAdd, CurFunc->GetNode(JsonNodeAdd.OperationName));
    XLS_ASSIGN_OR_RETURN(Node* NodeMult, CurFunc->GetNode(JsonNodeMult.OperationName));
    //Make new mult node A
    XLS_ASSIGN_OR_RETURN(TempOperandA, CurFunc->GetNode(JsonNode_OutMultA.Operands[0]));
    XLS_ASSIGN_OR_RETURN(TempOperandB, CurFunc->GetNode(JsonNode_OutMultA.Operands[1]));
    XLS_ASSIGN_OR_RETURN(Node* NodeOut_MultA, CurFunc->MakeNode<ArithOp>(NodeAdd->loc(), TempOperandA, TempOperandB, JsonNode_OutMultA.BitWidth, Op::kUMul));
    //Make new mult node B
    XLS_ASSIGN_OR_RETURN(TempOperandA, CurFunc->GetNode(JsonNode_OutMultB.Operands[0]));
    XLS_ASSIGN_OR_RETURN(TempOperandB, CurFunc->GetNode(JsonNode_OutMultB.Operands[1]));
    XLS_ASSIGN_OR_RETURN(Node* NodeOut_MultB, CurFunc->MakeNode<ArithOp>(NodeAdd->loc(), TempOperandA, TempOperandB, JsonNode_OutMultB.BitWidth, Op::kUMul));
    //Make new graph output. Replce graph output to new output
    XLS_ASSIGN_OR_RETURN(Node* NodeOut_Add, CurFunc->MakeNode<BinOp>(NodeOut_MultB->loc(), NodeOut_MultA, NodeOut_MultB, Op::kAdd));
    XLS_RETURN_IF_ERROR(NodeMult->ReplaceUsesWith(NodeOut_Add));
    //Remove previous nodes in correct order
    XLS_RETURN_IF_ERROR(CurFunc->RemoveNode(NodeMult));
    XLS_RETURN_IF_ERROR(CurFunc->RemoveNode(NodeAdd));

    return absl::OkStatus();
}

absl::Status RewriteHandler::HandleSumSame(const JsonSingleSub& sub) {
    JsonNode JsonNodeA;
    Node* TempOperand;
    if (sub.NodesInvolved.size() != 1){
        return absl::UnknownError("[ERROR] Incorrect size for SumSame");
    } else {
        JsonNodeA = sub.NodesInvolved[0];
    }
    XLS_ASSIGN_OR_RETURN(Function* CurFunc, p->GetFunction(JsonNodeA.FuncName));
    XLS_ASSIGN_OR_RETURN(Node* CurNode, CurFunc->GetNode(JsonNodeA.OperationName));
    XLS_ASSIGN_OR_RETURN(TempOperand, CurFunc->GetNode(JsonNodeA.Operands[0]));
    int Multiplier = 2;
    XLS_ASSIGN_OR_RETURN(Node* NodeOut_Literal, CurFunc->MakeNode<Literal>(CurNode->loc(), Value(UBits(Multiplier, Bits::MinBitCountUnsigned(Multiplier)))));
    XLS_ASSIGN_OR_RETURN(Node* NodeOut_Mult, CurFunc->MakeNode<ArithOp>(NodeOut_Literal->loc(), TempOperand, NodeOut_Literal, JsonNodeA.BitWidth, Op::kUMul));
    
    XLS_RETURN_IF_ERROR(CurNode->ReplaceUsesWith(NodeOut_Mult));
    XLS_RETURN_IF_ERROR(CurFunc->RemoveNode(CurNode));
    
    return absl::OkStatus();
}
// }
}  // namespace xls
