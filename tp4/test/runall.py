import re
import subprocess

with open("./checks.c") as f:
    string = f.read()

matches = re.findall(r"test_.*_\d", string)

dico = {
    "test_cluster_to_lba": 1,
    "test_next_cluster": 1,
    "test_check_name": 1,
    "test_path": 1,
    "test_read_boot_block": 1,
    "test_find_file_descriptor": 1,
    "test_find_file_descriptor": 1,
    "test_read_content": 1,
}

dico = {key:val/len(dico) for key, val in dico.items()}

perfect_score = {key: 0 for (key, _) in dico.items()}
points = perfect_score.copy()

failed = 0
for test_case in matches:
    ret = subprocess.call(f"./tests_build/check_tests {test_case}", shell=True)

    perfect_score[test_case[:-2]] += 1

    if ret == 0:
        points[test_case[:-2]] += 1

total = 0
for key, val in dico.items():
    print(f"{key}: {(points[key] / perfect_score[key])}")
    total += (points[key] / perfect_score[key])*val
print(f"Total: {total}")
