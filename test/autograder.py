import os
import subprocess
import re

pattern = re.compile(r'Score:\s*([\d.]+)')

# Print autograder header

print("")
print("┏━━━┓━━━━━┏┓━━━━━━━━━━━━━━━━━━━┏┓━━━━━━━")
print("┃┏━┓┃━━━━┏┛┗┓━━━━━━━━━━━━━━━━━━┃┃━━━━━━━")
print("┃┃━┃┃┏┓┏┓┗┓┏┛┏━━┓┏━━┓┏━┓┏━━┓━┏━┛┃┏━━┓┏━┓")
print("┃┗━┛┃┃┃┃┃━┃┃━┃┏┓┃┃┏┓┃┃┏┛┗━┓┃━┃┏┓┃┃┏┓┃┃┏┛")
print("┃┏━┓┃┃┗┛┃━┃┗┓┃┗┛┃┃┗┛┃┃┃━┃┗┛┗┓┃┗┛┃┃┃━┫┃┃━")
print("┗┛━┗┛┗━━┛━┗━┛┗━━┛┗━┓┃┗┛━┗━━━┛┗━━┛┗━━┛┗┛━")
print("━━━━━━━━━━━━━━━━━┏━┛┃━━━━━━━━━━━━━━━━━━━")
print("━━━━━━━━━━━━━━━━━┗━━┛━━━━━━━━━━━━━━━━━━━")
print("")

# Run tests

ready_queue_result = None
try:
    ready_queue_result = subprocess.run(['./ready_queue_tests'], stdout=subprocess.PIPE, text=True, timeout=10)
except subprocess.TimeoutExpired:
    ready_queue_result = None

scheduler_result = None
try:
    scheduler_result = subprocess.run(['../src/scheduler', 'test_quick.csv'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                                      timeout=60)
except subprocess.TimeoutExpired:
    scheduler_result = None
    
# if the small passes, let's run the big fella
big_scheduler_result = None
try:
    big_scheduler_result = subprocess.run(['../src/scheduler', 'test_grader.csv'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                                        timeout=300)
except subprocess.TimeoutExpired:
    big_scheduler_result = None

q_passed = 0
if ready_queue_result is not None:
    result = re.search(r'Checks: (\d+), Failures: (\d+), Errors: (\d+)', ready_queue_result.stdout)

    checks = int(result.group(1))
    failures = int(result.group(2))
    errors = int(result.group(3))

    q_passed = checks - failures - errors
    print(f"Ready Queue Tests: {checks - failures - errors}/{checks}")
else:
    print(f"Ready Queue Tests: 0/8 (timeout)")

subgrade = (30 / 8) * q_passed

if scheduler_result is not None and scheduler_result.returncode == 0:
    print(f"Scheduler: passed")
    subgrade += 20
elif scheduler_result is not None and scheduler_result.returncode != 0:
    print(f"Scheduler FAILED: {scheduler_result.stdout}")
else:
    print(f"Scheduler FAILED: timeout")

print(f"Subgrade: {subgrade}")

if big_scheduler_result is not None and big_scheduler_result.returncode == 0:
    score = pattern.search(big_scheduler_result.stdout)
    if score is not None:
        score = float(score.group(1))
    else:
        print(f"Big Scheduler: {big_scheduler_result.stdout} | Score: 999")
    print(f"Big Scheduler: SUCCESS | Score: {score}")
elif big_scheduler_result is not None and big_scheduler_result.returncode != 0:
    print(f"Big Scheduler FAILED: {big_scheduler_result.stdout} | Score: 999")
else:
    print(f"Big Scheduler FAILED: timeout | Score: 999")