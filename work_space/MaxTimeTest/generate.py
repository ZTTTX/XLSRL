import os
import re



def parse_timed_nodes(file_path):
    with open(file_path, 'r') as file:
        content = file.read()

    # timed_nodes 섹션을 찾기
    timed_nodes_pattern = re.compile(r'timed_nodes \{(.*?)\}', re.DOTALL)
    node_info_pattern = re.compile(r'node: "(.*?)"\s*node_delay_ps: (\d+)\s*path_delay_ps: (\d+)')

    # 각 timed_nodes의 정보를 저장할 딕셔너리
    timed_nodes_dict = {}

    for match in timed_nodes_pattern.finditer(content):
        timed_nodes_section = match.group(1)
        for node_info in node_info_pattern.finditer(timed_nodes_section):
            node, node_delay_ps, path_delay_ps = node_info.groups()
            timed_nodes_dict[node] = {
                'node_delay_ps': int(node_delay_ps),
                'path_delay_ps': int(path_delay_ps)
            }

    return timed_nodes_dict


def find_max_total_delay_ps(timed_nodes_dict):
    max_total_delay_ps = -1

    for node, info in timed_nodes_dict.items():
        total_delay_ps = info['node_delay_ps'] + info['path_delay_ps']
        if total_delay_ps > max_total_delay_ps:
            max_total_delay_ps = total_delay_ps

    return max_total_delay_ps



if __name__ == '__main__':

    schedule_folder_path = "/home/sylee/XLSRL/work_space/MaxTimeTest/test/schedule_txt"

    # multiple_rewrites = "/home/sylee/XLSRL/work_space/MaxTimeTest/test.opt_schedule.txt"
    multiple_rewrites = "/home/sylee/XLSRL/work_space/Sha256/sha256.opt_schedule.txt"
    # multiple_rewrites = "/home/sylee/XLSRL/work_space/MaxTimeTest/test/schedule_txt/sha256_0_schedule.txt"
    timed_nodes_dict = parse_timed_nodes(multiple_rewrites)
    max_total_delay_ps = find_max_total_delay_ps(timed_nodes_dict)
    print(max_total_delay_ps)

    # for schedule in sorted(os.listdir(schedule_folder_path)):
    #     timed_nodes_dict = parse_timed_nodes(os.path.join(schedule_folder_path,schedule))
    #     max_total_delay_ps = find_max_total_delay_ps(timed_nodes_dict)
    #     print(max_total_delay_ps)

    
