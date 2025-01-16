import os
from shutil import copyfile

os.remove("../src/main.c")
copyfile("../src/main_bis.c", "../src/main.c")
os.remove("../src/main_bis.c")
