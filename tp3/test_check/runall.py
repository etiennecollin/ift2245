import contextlib
import os
import re
import subprocess

with open("checks.c") as f:
    string = f.read()

@contextlib.contextmanager
def directory(name):
    ret = os.getcwd()
    os.chdir(name)
    yield None
    os.chdir(ret)

matches = re.findall(r"test_.*_\d", string)

dico = {
    "test_pt": 1/3,
    "test_tlb": 1/3,
    "test_pm": 1/3
}

# tests
perfect_score = {key:0 for (key, _) in dico.items()}
points = perfect_score.copy()

failed = 0
for test_case in matches:
    with directory("tests_build"):
        with open("bs.txt", "w") as f:
            for i in range(256):
                for _ in range(256):
                    f.write(str(i % 10))

        ret = 1
        try:
            ret = subprocess.call(f"./check_tests {test_case}", shell=True)
        except:
            pass


        perfect_score[test_case[:-2]] += 1

        if ret == 0:
            points[test_case[:-2]] += 1

total = 0
for key, val in dico.items():
    print(f"{key}: {(points[key] / perfect_score[key])}")
    total += (points[key] / perfect_score[key])*val
print(f"SUBGRADE:{{{total}}}")