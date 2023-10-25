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
// namespace {

class RewriteHandler {
public:
    explicit RewriteHandler(Package* package);  // constructor takes in IR package

    void HandleSubstitution(const JsonSingleSub& sub);

private:
    Package* p;
    // Individual handler functions for different types of substitutions.
    absl::Status HandleCommutativity(const JsonSingleSub& sub);
    absl::Status HandleAssociativity(const JsonSingleSub& sub);
    absl::Status HandleDistributeMultOverAdd(const JsonSingleSub& sub);
    absl::Status HandleSumSame(const JsonSingleSub& sub);

    // ToDo: Add more handler

    std::unordered_map<std::string, std::function<void(const JsonSingleSub&)>> HandlerMap;
};


// }
}  // namespace xls

#endif