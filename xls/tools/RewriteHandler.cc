#include "RewriteHandler.h"

namespace xls {
// namespace {

RewriteHandler::RewriteHandler() {
  // Initialize the handler map
    HandlerMap["Commutativity"] = [this](const JsonSingleSub& sub) { HandleCommutativity(sub); };
    HandlerMap["AddAssociativity"] = [this](const JsonSingleSub& sub) { HandleAddAssociativity(sub); };

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

void RewriteHandler::HandleCommutativity(const JsonSingleSub& sub) {
    printf("THIS IS COMMUTATIVITY\n");
}

void RewriteHandler::HandleAddAssociativity(const JsonSingleSub& sub) {
    printf("THIS IS ADD ASSOCIATIVITY\n");
}


// }
}  // namespace xls
