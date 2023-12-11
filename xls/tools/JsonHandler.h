#ifndef JSON_NODE_H
#define JSON_NODE_H

#include <json.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string_view>
#include <string>
#include <vector>
#include <sstream> 
#include <iostream>  


namespace xls {
// namespace {

class JsonNode {
public:
    int64_t Value;
    std::string OperationName;
    std::string OperationType;
    std::vector<std::string> Operands; //This stores the rewrited operands
    int Id;
    std::vector<int> Position;
    std::string ReplaceSelfWith;
    int64_t BitWidth = -1;
    std::vector<std::string> Indices;
    std::vector<int> ArraySize;

    std::string toString() const {
        std::stringstream ss;
        ss << "{" << "Value: " << Value << ", OperationName: " << OperationName
           << ", OperationType: " << OperationType << ", Id: " << Id 
           << ", ReplaceSelfWith: "<< ReplaceSelfWith 
           <<", BitWidth: " << BitWidth 
           <<", Position: [";
        // Print the Position vector
        for (size_t i = 0; i < Position.size(); ++i) {
            ss << Position[i];
            if (i < Position.size() - 1) ss << ", ";
        }
        ss << "], Operands: [";
        // Print the Operands vector
        for (size_t i = 0; i < Operands.size(); ++i) {
            ss << Operands[i];
            if (i < Operands.size() - 1) ss << ", ";
        }
        ss << "], Indices: [";
        for (size_t i = 0; i < Indices.size(); ++i) {
            ss << Indices[i];
            if (i < Indices.size() - 1) ss << ", ";
        }
        ss << "], ArraySize: [";
        for (size_t i = 0; i < ArraySize.size(); ++i) {
            ss << ArraySize[i];
            if (i < ArraySize.size() - 1) ss << ", ";
        }
        ss << "] }";
        return ss.str();
    }
};

class JsonSingleSub {
public:
    int SubId;
    std::string FuncName;
    std::vector<JsonNode> NodesInvolved;

    std::string toString() const {
        std::stringstream ss;
        ss << "{ SubId: " << SubId << ", FuncName: " << FuncName << ", NodesInvolved: [";
        // Print each JsonNode in NodesInvolved
        for (size_t i = 0; i < NodesInvolved.size(); ++i) {
            ss << NodesInvolved[i].toString();
            if (i < NodesInvolved.size() - 1) ss << ", ";
        }
        ss << "] }";
        return ss.str();
    }

    size_t GetNumberOfInvolvedNodes() const {
        return NodesInvolved.size();
    }
};

void DEBUG_PrintAllSubs(const std::vector<JsonSingleSub>& allSubs);

int GetSubIdByIndex(json_value* value, int index);

JsonNode GetSingleNode(json_value* value);

std::vector<JsonNode> GetInvolvedNode(json_value* value);

JsonSingleSub GetSingleSub(json_value* value, int CurId);

std::vector<JsonSingleSub> GetAllSubs(json_value* value);

std::vector<JsonSingleSub> ReadJsonByPath(std::string_view json_path);

void ReadJsonByPathUnitTest(std::string_view json_path);


// }  // namespaceR
}  // namespace xls

#endif // JSON_HANDLER_H