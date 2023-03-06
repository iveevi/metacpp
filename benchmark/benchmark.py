# Benchmarking compilte-time lisp interpreter in C++
import csv
import matplotlib.pyplot as plt
import os
import psutil
import resource
import seaborn as sns
import subprocess

programs = [
    {'name': 'v1', 'dir' : 'lisp_raw', 'source' : 'main.cpp'},
    {'name' : 'current', 'dir': '..', 'source' : 'lisp_main.cpp'},
]

subsources = {
    'list' : 5 * '(list 1 2 3 4 5 6 7 8 9 10)',
    'sum_ii' : 5 * '(+ 1 2 3 4 5 6 7 8 9 10)',
    'prod_ii' : 5 * '(* 1 2 3 4 5 6 7 8 9 10)'
}

def get_field(fields, name):
    for field in fields:
        if name in field[0]:
            return field[1].strip()
    return None
 
def get_benchmark_statistics(program, source):
    source = '-DLISP_SOURCE="\\"' + source + '\\""'
    cmd = [
        '/bin/time', '-v', 'g++', '-std=c++20',
        '-I', '..', source, program['dir'] + '/' + program['source'],
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

max_chars = 2000
process_iterations = 10
data = {}

for subsource_type, subsource in subsources.items():
    print("\n### Subsource type: " + subsource_type)

    data[subsource_type] = {}
    for program in programs:
        data[subsource_type][program['name']] = []

    source_iterations = max_chars // len(subsource)
    for i in range(1, source_iterations + 1):
        source = i * (subsource + ' ')
        chars = len(source)

        printable_source = source if len(source) < 50 else source[:50] + '...'
        print('Source: "', printable_source, '", length = ', chars, sep='')

        for program in programs:
            avg_user_time = 0
            avg_rss = 0

            for _ in range(0, process_iterations):
                user_time, rss = get_benchmark_statistics(program, source)
                user_time = float(user_time)
                rss = float(rss)/1024 # Megabytes
                data[subsource_type][program['name']].append((chars, user_time, rss))

print(data)

# TODO: flare colors, but also write to CSV

# Plotting
sns.set()

# Create the time plots
ax_times = []
for i, program in enumerate(programs):
    ax = plt.subplot(2, 2, i + 1, sharey=ax_times[-1] if i > 0 else None)

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
    ax = plt.subplot(2, 2, i + 3, sharey=ax_memory[-1] if i > 0 else None)
    ax.set_xlabel('Source size (# of chars)')

    for item in data:
        label = program['name'] + ' ' + item
        chars, user_time, rss = zip(*data[item][program['name']])
        sns.lineplot(x=chars, y=rss, ax=ax, label=label, marker='o')

    ax.legend()
    ax_memory.append(ax)

ax_memory[0].set_ylabel('Memory (MB)')

# Set figure size
plt.gcf().set_size_inches(15, 10)

# Show the plots
plt.tight_layout()
plt.show()
