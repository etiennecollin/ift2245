import os
import subprocess
import re

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
    scheduler_result = subprocess.run(['../src/scheduler', 'test_quick.csv'], stdout=subprocess.PIPE, text=True,
                                      timeout=30)
except subprocess.TimeoutExpired:
    scheduler_result = None

if ready_queue_result is not None:
    result = re.search(r'Checks: (\d+), Failures: (\d+), Errors: (\d+)', ready_queue_result.stdout)

    checks = int(result.group(1))
    failures = int(result.group(2))
    errors = int(result.group(3))

    print(f"Ready Queue Tests: {checks - failures - errors}/{checks}")
else:
    print(f"Ready Queue Tests: 0/8 (timeout)")

if scheduler_result is not None:
    print(f"Scheduler: passed")
else:
    print(f"Scheduler: failed (timeout)")
