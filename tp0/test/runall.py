import re
import subprocess

with open("checks.c") as f:
    string = f.read()

matches = re.findall(r"test_.*_\d", string)

dico = {
    "test_strlen": 1/7,
    "test_no_of_lines": 1/7,
    "test_readline": 1/7,
    "test_memcpy": 1/7,
    "test_execute": 2/7,
    "test_parse_line": 1/7
}

# tests
perfect_score = {key:0 for (key, _) in dico.items()}
points = perfect_score.copy()

failed = 0
for test_case in matches:
    ret = subprocess.call(f"./tests_build/check_tests {test_case}", shell=True)

    perfect_score[test_case[:-2]] += 1

    if ret == 0:
        points[test_case[:-2]] += 1

# valgrind
def pts_lost_for_mem_leaks(val):
    if len(re.findall(r"(definitely lost|indirectly lost): [1-9]", val)) == 0:
        return 0
    return 15

def pts_lost_for_invalids(val):
    return min(val.count("invalid read of size") + val.count("invalid write of size"), 5)

try:
    valgrind_output = subprocess.check_output("valgrind --leak-check=full --leak-resolution=med --trace-children=no --track-origins=yes --vgdb=no ./tests_build/check_tests valgrind", shell=True, universal_newlines=True, stderr=subprocess.STDOUT)
    print("\n",valgrind_output)
except Exception as e:
    valgrind_output = e.output

try:
    pts_lost = -(pts_lost_for_mem_leaks(valgrind_output) + pts_lost_for_invalids(valgrind_output)) / 100
except Exception:
    pts_lost = -0.2
print(f"Points lost with Valgrind: {pts_lost}")
total = pts_lost

for key, val in dico.items():
    print(f"{key}: {(points[key] / perfect_score[key])}")
    total += (points[key] / perfect_score[key])*val
print(f"GRADE:{{{total}}}")