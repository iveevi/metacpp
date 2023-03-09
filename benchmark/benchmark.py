# Benchmarking compilte-time lisp interpreter in C++
import csv
import json
import matplotlib.pyplot as plt
import os
import psutil
import resource
import seaborn as sns
import subprocess
import sys

# Get path to config file
if len(sys.argv) != 2:
    print('Error: expected path to config file')
    exit(1)

config_path = sys.argv[1]
if not os.path.exists(config_path):
    print('Error: config file does not exist')
    exit(1)

# Read config file
max_chars = None
memory_cutoff = None # Megabytes
process_iterations = None
programs = None
save_path = None
scale = None
subsources = None

with open(config_path, 'r') as f:
    config = json.load(f)
    print('Config:', json.dumps(config, indent=4), sep='\n')

    max_chars = config['source_cap']
    memory_cutoff = config['memory_cutoff']
    process_iterations = config['sample_size']
    programs = config['programs']
    save_path = config['save']
    scale = config['scale']
    subsources = config['subsources']

def get_field(fields, name):
    for field in fields:
        if name in field[0]:
            return field[1].strip()
    return None

def get_benchmark_statistics(program, source):
    source = '-DLISP_SOURCE="\\"' + source + '\\""'
    cmd = [
        '/bin/time', '-v', 'g++', '-std=c++20',
        '-I', program['dir'], source, program['dir'] + '/' + program['source'],
        '-o', program['dir'] + '/main'
    ]

    proc = subprocess.Popen(' '.join(cmd), shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = proc.communicate()
    if proc.returncode != 0:
        print('Error: ' + err.decode('utf-8'))
        exit(1)

    lines = err.decode('utf-8').split('\n')
    lines = [line.strip() for line in lines if line.strip() != '']
    fields = [line.split(':') for line in lines]

    user_time = get_field(fields, 'User time')
    max_rss = get_field(fields, 'Maximum resident set size')

    return user_time, max_rss

data = {}
for subsource_type, subsource in subsources.items():
    program_cutoff = set()
    print("\n### Subsource category: " + subsource_type)

    data[subsource_type] = {}
    for program in programs:
        data[subsource_type][program['name']] = []

    source_iterations = max_chars // (scale * len(subsource))
    for i in range(1, source_iterations + 1):
        source = scale * i * (subsource + ' ')
        chars = len(source)

        printable_source = source if len(source) < 50 else source[:50] + '...'
        print('Source: "', printable_source, '", length = ', chars, sep='')

        for program in programs:
            if program['name'] in program_cutoff:
                program_name = program['name']
                print(f'Program {program_name} already exceeded projected memory cutoff')
                continue

            max_rss = 0
            for _ in range(0, process_iterations):
                user_time, rss = get_benchmark_statistics(program, source)
                user_time = float(user_time)
                rss = float(rss)/1024 # Megabytes
                max_rss = max(max_rss, rss)
                data[subsource_type][program['name']].append((chars, user_time, rss))

            if max_rss > memory_cutoff:
                program_name = program['name']
                program_cutoff.add(program_name)

print(data)

# TODO: flare colors, but also write to CSV

# Plotting
sns.set()
sns.set_style('darkgrid')

# Create the time plots
ax_times = []
num_programs = len(programs)
for i, program in enumerate(programs):
    # ax = plt.subplot(2, num_programs, i + 1, sharey=ax_times[-1] if i > 0 else None)
    ax = plt.subplot(2, num_programs, i + 1)

    for item in data:
        label = program['name'] + ' ' + item
        chars, user_time, rss = zip(*data[item][program['name']])
        sns.lineplot(x=chars, y=user_time, ax=ax, label=label, marker='o')

    ax.legend()
    ax_times.append(ax)

ax_times[0].set_ylabel('Time (s)')

# Create the memory plots
ax_memory = []
for i, program in enumerate(programs):
    # ax = plt.subplot(2, num_programs, i + 1 + num_programs, sharey=ax_memory[-1] if i > 0 else None)
    ax = plt.subplot(2, num_programs, i + 1 + num_programs)
    ax.set_xlabel('Source size (# of chars)')

    for item in data:
        label = program['name'] + ' ' + item
        chars, user_time, rss = zip(*data[item][program['name']])
        sns.lineplot(x=chars, y=rss, ax=ax, label=label, marker='o')

    ax.legend()
    ax_memory.append(ax)

ax_memory[0].set_ylabel('Memory (MB)')

# Set figure size
plt.gcf().set_size_inches(20, 10)
plt.tight_layout()

# Also save the plots
relative_path = os.path.dirname(os.path.realpath(config_path))
save_path = os.path.join(relative_path, save_path)
plt.savefig(save_path)

# Show the plots
plt.show()
