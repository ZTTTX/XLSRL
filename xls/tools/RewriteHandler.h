#ifndef REWRITE_HANDLER_H
#define REWRITE_HANDLER_H

#include "JsonHandler.h"  // Include the header file for JsonNode

#include <string>
#include <functional>
#include <unordered_map>

namespace xls {
// namespace {

class RewriteHandler {
public:
    RewriteHandler();  // Declare the constructor

    void HandleSubstitution(const JsonSingleSub& sub);

private:
    // Individual handler functions for different types of substitutions.
    void HandleCommutativity(const JsonSingleSub& sub);
    void HandleAddAssociativity(const JsonSingleSub& sub);
    // ToDo: Add more handler

    std::unordered_map<std::string, std::function<void(const JsonSingleSub&)>> HandlerMap;
};


// }
}  // namespace xls

#endif