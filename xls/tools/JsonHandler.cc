#include "JsonHandler.h"


namespace xls {
// namespace {

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
        if (CurHandleName == "BitWidth") {
            CurNode.BitWidth = CurValue -> u.integer;
        } else if (CurHandleName == "ReplaceSelfWith") {
            CurNode.ReplaceSelfWith = CurValue -> u.string.ptr;
        } else if (CurHandleName == "Value") {
            CurNode.Value = CurValue -> u.integer;
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
    CurSub.FuncName = value -> u.object.values[0].value -> u.string.ptr;
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

std::vector<JsonSingleSub> ReadJsonByPath(std::string_view json_path) {
    std::string filename_str = std::string(json_path);
    const char* filename = filename_str.c_str();
    FILE *fp;
    struct stat filestatus;
    int file_size;
    char* file_contents;
    json_char* json;
    json_value* value;
    std::vector<JsonSingleSub> AllSubs;

    if ( stat(filename, &filestatus) != 0) {
        fprintf(stderr, "File %s not found\n", filename);
        exit(1);
    }
    file_size = filestatus.st_size;
    file_contents = (char*)malloc(filestatus.st_size);
    if ( file_contents == NULL) {
        fprintf(stderr, "Memory error: unable to allocate %d bytes\n", file_size);
        exit(1);
    }
    fp = fopen(filename, "rt");
    if (fp == NULL) {
        fprintf(stderr, "Unable to open %s\n", filename);
        fclose(fp);
        free(file_contents);
        exit(1);
    }
    if ( fread(file_contents, file_size, 1, fp) != 1 ) {
        fprintf(stderr, "Unable to read content of %s\n", filename);
        fclose(fp);
        free(file_contents);
        exit(1);
    }
    fclose(fp);

    json = (json_char*)file_contents;
    value = json_parse(json,file_size);

    if (value == NULL) {
        fprintf(stderr, "Unable to parse data\n");
        free(file_contents);
        exit(1);
    }

    AllSubs = GetAllSubs(value);

    json_value_free(value);
    free(file_contents);

    return AllSubs;
}

void ReadJsonByPathUnitTest(std::string_view json_path){
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



// }  // namespaceR
}  // namespace xls

