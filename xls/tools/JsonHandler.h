#include <json.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string_view>
#include <string>
#include <vector>


namespace xls {
namespace {

class JsonNode {
public:
    int IsNodeOutput;
    std::string OperationName;
    std::string OperationType;
    std::vector<std::string> Operands;
    int Id;
    std::vector<int> Position;

    std::string toString() const {
        std::stringstream ss;
        ss << "{ IsNodeOutput: " << IsNodeOutput << ", OperationName: " << OperationName
           << ", OperationType: " << OperationType << ", Id: " << Id << ", Position: [";
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
        ss << "] }";
        return ss.str();
    }
};

class JsonSingleSub {
public:
    int SubId;
    std::string TypeOfSub;
    std::vector<JsonNode> NodesInvolved;

    std::string toString() const {
        std::stringstream ss;
        ss << "{ SubId: " << SubId << ", TypeOfSub: " << TypeOfSub << ", NodesInvolved: [";
        // Print each JsonNode in NodesInvolved
        for (size_t i = 0; i < NodesInvolved.size(); ++i) {
            ss << NodesInvolved[i].toString();
            if (i < NodesInvolved.size() - 1) ss << ", ";
        }
        ss << "] }";
        return ss.str();
    }
};

void DEBUG_PrintAllSubs(const std::vector<JsonSingleSub>& allSubs) {
    for (const auto& sub : allSubs) {
        std::cout << sub.toString() << std::endl;
    }
}

int GetSubIdByIndex(json_value* value, int index) {
    return std::stoi(value -> u.object.values[index].name);
}

JsonNode GetSingleNode(json_value* value){
    JsonNode CurNode;
    std::string CurHandleName;
    json_value* CurValue;
    int NumOfEntries, k, l, h;
    NumOfEntries = value -> u.object.length;
    for (k = 0; k < NumOfEntries; k++){
        CurHandleName = value -> u.object.values[k].name;
        CurValue = value -> u.object.values[k].value;
        if (CurHandleName == "IsNodeOutput") {
            CurNode.IsNodeOutput = CurValue -> u.integer;
        } else if (CurHandleName == "OperationName") {
            CurNode.OperationName = CurValue -> u.string.ptr;
        } else if (CurHandleName == "OperationType") {
            CurNode.OperationType = CurValue -> u.string.ptr;
        } else if (CurHandleName == "Operands") {
            for (l=0; l< CurValue->u.array.length; l++){
                CurNode.Operands.push_back(CurValue -> u.array.values[l] -> u.string.ptr);
            }     
        } else if (CurHandleName == "Id") {
            CurNode.Id = CurValue -> u.integer;
        } else if (CurHandleName == "Position") {
            for (h=0; h < CurValue->u.array.length; h++) {
                CurNode.Position.push_back(CurValue -> u.array.values[h] -> u.integer);
            }
        }
    }
    return CurNode;
}

std::vector<JsonNode> GetInvolvedNode(json_value* value){ 
    // Value here points to the array, which is the value for "NodesInvolved"
    std::vector<JsonNode> CurNodeList;
    int NumOfNode, j;
    NumOfNode = value -> u.array.length;
    for (j = 0; j < NumOfNode; j++){
        CurNodeList.push_back(GetSingleNode(value -> u.array.values[j]));
    }
    return CurNodeList;
}

JsonSingleSub GetSingleSub(json_value* value, int CurId) { 
    //Value here points to the value of one of the objects
    JsonSingleSub CurSub;
    CurSub.SubId = CurId;
    CurSub.TypeOfSub = value -> u.object.values[0].value -> u.string.ptr;
    CurSub.NodesInvolved = GetInvolvedNode(value -> u.object.values[1].value);
    return CurSub;
}

std::vector<JsonSingleSub> GetAllSubs(json_value* value) {
    std::vector<JsonSingleSub> AllSubs;
    int NumOfSub, i;
    NumOfSub = value -> u.object.length;
    for (i=0; i<NumOfSub; i++){
        AllSubs.push_back(GetSingleSub(value -> u.object.values[i].value, GetSubIdByIndex(value, i)));
    }
    return AllSubs;
}

static void HandleJson(std::string_view json_path){
    std::string filename_str = std::string(json_path);
    const char* filename = filename_str.c_str();
    FILE *fp;
    struct stat filestatus;
    int file_size;
    char* file_contents;
    json_char* json;
    json_value* value;
    if ( stat(filename, &filestatus) != 0) {
    fprintf(stderr, "File %s not found\n", filename);
    return;
    }
    file_size = filestatus.st_size;
    file_contents = (char*)malloc(filestatus.st_size);
    if ( file_contents == NULL) {
    fprintf(stderr, "Memory error: unable to allocate %d bytes\n", file_size);
    return;
    }
    fp = fopen(filename, "rt");
    if (fp == NULL) {
    fprintf(stderr, "Unable to open %s\n", filename);
    fclose(fp);
    free(file_contents);
    return;
    }
    if ( fread(file_contents, file_size, 1, fp) != 1 ) {
    fprintf(stderr, "Unable to read content of %s\n", filename);
    fclose(fp);
    free(file_contents);
    return;
    }
    fclose(fp);

    json = (json_char*)file_contents;
    value = json_parse(json,file_size);

    if (value == NULL) {
            fprintf(stderr, "Unable to parse data\n");
            free(file_contents);
            exit(1);
    }

    printf("==================\n");
    std::vector<JsonSingleSub> AllSubs;
    AllSubs = GetAllSubs(value);
    DEBUG_PrintAllSubs(AllSubs);

    json_value_free(value);
    free(file_contents);

    return;
}



}  // namespaceR
}  // namespace xls