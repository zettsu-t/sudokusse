#!/usr/bin/python3
# coding: utf-8

'''
Solving Sudoku-X puzzles and checking their results
Copyright (C) 2018 Zettsu Tatsuya

usage:
$ python3 solve_sudoku_x.py puzzleFilename (1 or more)
  or
$ python3 --log text_solutions (solution per line)
'''

import re
import sys
import subprocess
from optparse import OptionParser

EXECUTABLE_FILENAME='bin/sudokusse_diagonal'

def check_solution(lines):
    status_code = 0
    rows = lines[-9:]
    columns = [''.join(i) for i in zip(*rows)]
    boxes = []

    for y in range(0, 9, 3):
        for x in range(0, 9, 3):
            cells = ''
            for z in range(0, 3):
                cells += rows[y + z][x:x + 3]
            boxes.append(cells)

    bars = []
    bars.append(''.join([(rows[i][i]) for i in range(0, 9)]))
    bars.append(''.join([(rows[8 - i][i]) for i in range(0, 9)]))

    cells = []
    for cellset in [rows, columns, boxes, bars]:
        cells.extend(cellset)
    for ninecells in cells:
        if ''.join(sorted(ninecells)) != '123456789':
            print('Unexpected cells')
            status_code = 1
    return status_code

def parse_solution(line):
    status_code = 0
    matched = re.match(r'^\d{81}', line)
    if matched is None:
        return status_code
    lines = list(filter(bool, re.split(r'(\d{9})', line)))
    return check_solution(lines)

def solve_puzzle(stdout_data):
    status_code = 0
    lines = []
    for line in re.split(r'\r?\n', stdout_data.decode('utf-8')):
        matched = re.match(r'^\d:\d:\d:\d:\d:\d:\d:\d:\d:', line)
        if matched is not None:
            lines.append(''.join(line.split(':')))
    return check_solution(lines)

def main():
    exit_status_code = 0
    parser = OptionParser()
    parser.add_option("-l", "--log", dest="log_filename",
                      help="parse solutions in a file instead of solving")
    (options, args) = parser.parse_args()

    if options.log_filename is not None:
        count = 0
        with open(options.log_filename, 'r') as infile:
            line = infile.readline()
            while line:
                count += 1
                status_code = parse_solution(line.strip())
                if status_code:
                    exit_status_code = status_code
                line = infile.readline()
        if exit_status_code == 0:
            print('All {0} cases passed'.format(count))
    else:
        for filename in sys.argv[1:]:
            with open(filename, 'r') as infile:
                proc = subprocess.Popen([EXECUTABLE_FILENAME, '-1'], stdin=infile, stdout=subprocess.PIPE)
                stdout_data = proc.communicate()[0]
                status_code = solve_puzzle(stdout_data)
                if status_code == 0:
                    print('{0} solved'.format(filename))
                else:
                    exit_status_code = status_code
    sys.exit(exit_status_code)

if __name__ == "__main__":
    main()
