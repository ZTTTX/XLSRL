#include "RewriteHandler.h"

namespace xls {
// namespace {

RewriteHandler::RewriteHandler(Package* package) : p(package) {
  // Initialize the handler map
    HandlerMap["Commutativity"] = [this](const JsonSingleSub& sub) {absl::Status status = HandleCommutativity(sub); };
    HandlerMap["Associativity"] = [this](const JsonSingleSub& sub) {absl::Status status =  HandleAssociativity(sub); };
    HandlerMap["DistributeMultOverAdd"] = [this](const JsonSingleSub& sub) {absl::Status status =  HandleDistributeMultOverAdd(sub); };
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
    // This function handles both add and mult commutativity
    // It only allows one node rewrite each time
    JsonNode JsonNodeA;
    Node* TempOperand;
    if (sub.NodesInvolved.size() != 1){
        return absl::UnknownError("Incorrect size for commutativity");
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
    // This function handles both add and mult associativity rewrites
    JsonNode JsonNodeA;
    JsonNode JsonNodeB;
    Node* TempOperand;
    if (sub.NodesInvolved.size() != 2){
        return absl::UnknownError("Incorrect size for associativity");
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
    return absl::OkStatus();
}
// }
}  // namespace xls
