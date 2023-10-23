#include "RewriteHandler.h"

namespace xls {
// namespace {

RewriteHandler::RewriteHandler(Package* package) : p(package) {
  // Initialize the handler map
    HandlerMap["Commutativity"] = [this](const JsonSingleSub& sub) {absl::Status status = HandleCommutativity(sub); };
    HandlerMap["AddAssociativity"] = [this](const JsonSingleSub& sub) {absl::Status status =  HandleAddAssociativity(sub); };

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
    JsonNode JsonNodeA;
    JsonNode JsonNodeB;
    if (sub.NodesInvolved.size() != 2){
        printf("[ERROR]Incorrect size for commutativity");
        exit(1);
    } else {
        JsonNodeA = sub.NodesInvolved[0];
        JsonNodeB = sub.NodesInvolved[1];
    }

    // Handle Node A
    XLS_ASSIGN_OR_RETURN(Function* CurFuncA, p->GetFunction(JsonNodeA.FuncName));
    XLS_ASSIGN_OR_RETURN(Node* CurNodeA, CurFuncA->GetNode(JsonNodeA.OperationName));
    std::string testtype = CurNodeA->GetOperandsString();




    printf("THIS IS COMMUTATIVITY\n");
    printf("%s", testtype.c_str()); 
    return absl::OkStatus();
}

absl::Status RewriteHandler::HandleAddAssociativity(const JsonSingleSub& sub) {
    printf("THIS IS ADD ASSOCIATIVITY\n");
    return absl::OkStatus();
}


// }
}  // namespace xls
