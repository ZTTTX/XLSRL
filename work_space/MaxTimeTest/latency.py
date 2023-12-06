import os

from one_rewrite import ReadScheduleIR
from one_rewrite import read_SDC_pipeline_result
from one_rewrite import register_SDC_result




def test_gschedule():
    test_folder_path = "/home/sylee/XLSRL/work_space/MaxTimeTest/test"
    json_output_folder_path = os.path.join(test_folder_path, "substitution_json")
    subfolders = ["substitution_json", "substitution_ir", "schedule_txt", "verilog", "substitution_schedule_ir"]

    g_schedule_list = []

    for json_output_path in sorted(os.listdir(json_output_folder_path)):
        schedule_result_file_path = os.path.join(test_folder_path,subfolders[2],json_output_path.replace('.json', '_schedule.txt'))
        output_schedule_ir__file_path = os.path.join(test_folder_path,subfolders[4],json_output_path.replace('.json', '_substitution_schedule.ir'))
        G_schedule = ReadScheduleIR(output_schedule_ir__file_path) # substitution_schedule.ir
        schedule_dict = read_SDC_pipeline_result(schedule_result_file_path) #test.opt_schedule.txt.
        G_schedule = register_SDC_result(G_schedule, schedule_dict)
        g_schedule_list.append(G_schedule)
    return g_schedule_list

def get_latency():
    

if __name__ == '__main__':
    g_schedule_list = test_gschedule() 
    print(g_schedule_list[0].nodes)

    # to check the attributes of all nodes
    # for node, attr in g_schedule_list[0].nodes(data=True):
    #     print(f"Node: {node}, Attributes: {attr}")

    ## print result 
    # Node: 45900, Attributes: {'OperationName': 'array.45900', 'BitWidth': 32, 'OperationType': 'array', 
    # 'Operands': ['array.52570', 'array.52569', 'array.52568', 'array.52567', 'array.52566', 'array.52565', 'array.52564', 'array.52563'], 
    # 'idNum': 45900, 'Value': -1, 'Pos': (1, 21, 9), 'FuncIO': 'Not', 'Start': None, 'Width': None, 'ArraySize': [8, 8], 'Indices': None, 
    # 'node_delay_ps': 0, 'path_delay_ps': 1706, 'stage': 2} 


    # for edge1, edge2, attr in g_schedule_list[0].edges(data=True):
    #     print(f"Edge: ({edge1}, {edge2}), Attributes: {attr}")

    ## print result
    # Edge: (45900, 25300), Attributes: {}

    # print(g_schedule_list[45900, 25300])