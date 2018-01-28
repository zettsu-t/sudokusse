#!/usr/bin/python3
# coding: utf-8

'''
Solving Sudoku-X puzzles and checking their results
Copyright (C) 2018 Zettsu Tatsuya

usage:
$ python3 solve_sudoku_x.py puzzleFilename (1 or more)
'''

import re
import sys
import subprocess

def solve_puzzle(stdout_data):
    status_code = 0
    lines = []
    for line in re.split(r'\r?\n', stdout_data.decode('utf-8')):
        matched = re.match(r'^\d:\d:\d:\d:\d:\d:\d:\d:\d:', line)
        if matched is not None:
            lines.append(''.join(line.split(':')))

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
        print(ninecells)
        if ''.join(sorted(ninecells)) == '123456789':
            print('OK')
        else:
            print('Unexpected cells')
            status_code = 1

    return status_code

def main():
    exit_status_code = 0
    for filename in sys.argv[1:]:
        infile = open(filename, 'r')
        proc = subprocess.Popen(['bin/sudokusse', '-1'], stdin=infile, stdout=subprocess.PIPE)
        stdout_data = proc.communicate()[0]
        status_code = solve_puzzle(stdout_data)
        if status_code:
            exit_status_code = status_code

    sys.exit(exit_status_code)

if __name__ == "__main__":
    main()
