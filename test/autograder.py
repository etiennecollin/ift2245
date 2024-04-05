import contextlib
import os
import re
import subprocess
import yaml

# Parse the tests config

with open("autograder_config.yaml") as f:
    tests_yaml = yaml.safe_load(f)
dico = {}

weight_sum = 0
for testcase, diccc in tests_yaml.items():
    dico[testcase] = diccc["weight"]
    weight_sum += diccc["weight"]

for key, weight in dico.items():
    dico[key] = weight / weight_sum

# Test runner

def run(input, timeout=10):
    input = input + "exit\n"
    try:
        program_output = subprocess.check_output(
            f"echo \"{input}\" - | valgrind --leak-check=full --leak-resolution=med --trace-children=no --track-origins=yes --vgdb=no --log-file=\"valgrind.log\" ../src/shell", 
            shell=True, 
            universal_newlines=True, 
            stderr=subprocess.STDOUT,
            timeout=timeout
            )
    except subprocess.TimeoutExpired:
        # kill the child
        subprocess.run("pkill -f shell", shell=True)
        program_output = "Error: Command timed out"
    except Exception as e:
        program_output = e.output

    print(program_output)

    with open("./valgrind.log", "r") as f:
        valgrind_output = f.read()

    return program_output, valgrind_output

# tests
perfect_score = {key:0 for (key, _) in dico.items()}
points = perfect_score.copy()

failed = 0
valgrind_output = ""
for test_case, inout in tests_yaml.items():
    for input, output in zip(inout["in"], inout["out"]):
        program_output, valgrind = run(input)
        valgrind_output += valgrind + "\n\n\n"

        perfect_score[test_case] += 1

        a = len(program_output)
        b = len(output)

        print(f"program_input: {input}")
        print(f"program_output: {program_output}")
        print(f"output: {output}")

        if program_output.strip() == output.strip():
            points[test_case] += 1

# valgrind
def pts_lost_for_mem_leaks(val):
    if len(re.findall(r"(definitely lost|indirectly lost): [1-9]", val)) == 0:
        return 0
    return 8

def pts_lost_for_invalids(val):
    return min(val.count("invalid read of size") + val.count("invalid write of size"), 5)

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
