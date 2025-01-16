#!/usr/bin/python3

import subprocess
import sys

def call(cmd):
    out = ""
    try:
        out = subprocess.check_output(cmd, shell=True, universal_newlines=True, stderr=subprocess.STDOUT)
    except Exception as e:
        out = e.output
    return out

out = call(f"gcc -{sys.argv[1]}")
if "undefined reference to `main'" not in out:
    raise Exception()