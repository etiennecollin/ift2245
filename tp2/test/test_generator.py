import numpy as np
import sys

# get arg for number of processes

if len(sys.argv) > 1:
    num_processes = int(sys.argv[1])
else:
    print("Usage: python test_generator.py <num_processes>")

burst_time_scale = 500
burst_time_min = 10

io_time_scale = 500
io_time_min = 10

burst_num_mean = 0
burst_num_std = 10
burst_num_min = 2

arrival_time_mean = 0
arrival_time_std = (burst_time_scale + io_time_scale) * burst_num_min


def to_int(min, val):
    return np.maximum(min, np.abs(val)).astype(int)


arrival = 0
data = []
for pid in range(num_processes):
    num_bursts = to_int(burst_num_min, np.random.normal(burst_num_mean, burst_num_std))
    burst_time = to_int(burst_time_min, np.random.exponential(burst_time_scale))
    io_time = to_int(io_time_min, np.random.exponential(io_time_scale))

    line = f"{pid};{arrival};{num_bursts};{burst_time};{io_time}"

    arrival += to_int(0, np.random.normal(arrival_time_mean, arrival_time_std))

    data.append(line)

with open("generated_test.csv", "w") as f:
    f.write(str(num_processes) + "\n")
    f.write("\n".join(data))
    f.write("\n")
