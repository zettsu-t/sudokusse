#!/usr/bin/python3
# coding: utf-8

'''
Solving N^2*N^2 Sudoku puzzles with NumPy
Copyright (C) 2018 Zettsu Tatsuya

usage:
$ python3 sudoku_numpy_n.py puzzle_text_filename

File 'sudoku-puzzle-filename' contains one N^2*N^2 Sudoku puzzle.
(Original Sudoku puzzles set N to 3 and have 9*9 cells)
Each line contains comma-separated values which are
- 1 to N^2 as preset numbers or
- other word such as 0 or . for blank (to-be-filled)
- Can omit commas when N is 3
- A..G is available instead of 10..16

An example in data/sudoku_example2.txt can be represented as below.
.,3,.,.,.,.,.,4,.
.,1,.,.,9,7,.,5,.
(and more seven lines)
'''

import re
import sys
import numpy as np

class SudokuSolver(object):
    '''
    Solve a Sudoku puzzle
    '''

    def __init__(self, in_filename):
        self.split_line_regex = re.compile('^(.*[^,]),*$')
        self.split_exdigit_regex = re.compile('[ABCDEFG]$')
        self.sudoku_map, self.sqrt_size = self.parse_file(in_filename)

    def parse_file(self, in_filename):
        '''
        Parse Sudoku puzzles in a file
        :param str filename : Name of a file that contains sudoku puzzles in each line
        :return [nparray(N,N,N,bool)] : List of Sudoku maps
        '''

        sudoku_lines = []
        with open(in_filename, 'r') as infile:
            line = infile.readline()
            while line:
                sudoku_str = line.strip()
                if len(sudoku_str):
                    sudoku_lines.append(self.parse_puzzle_line(sudoku_str))
                line = infile.readline()

        return self.parse_puzzle_lines(sudoku_lines)

    def parse_puzzle_lines(self, sudoku_lines):
        '''
        Parse Sudoku puzzle lines
        :param str[,] sudoku_lines : N lines which contains N cells
        :return numpy,array : Sudoku maps if sudoku_str is valid, None otherwise
        '''

        # Rows, columns, and cell candidates
        size = min(len(sudoku_lines), max([len(line) for line in sudoku_lines]))
        sqrt_size = int(np.floor(np.sqrt(size)))
        # Treats tailing lines as comments
        size = sqrt_size * sqrt_size
        if size < 2:
            raise ValueError("The input puzzle must be a N * N matrix but size = {}".format(size))

        sudoku_cells = [[0 for column in range(size)] for row in range(size)]
        sudoku_map = np.full((size, size, size), True, dtype=bool)

        for row, sudoku_line in enumerate(sudoku_lines):
            if row >= size:
                break
            for column, cell_num in enumerate(sudoku_line):
                if column >= size:
                    break
                if 1 <= cell_num <= size:
                    sudoku_map[row, column] = np.full((size), False, dtype=bool)
                    sudoku_map[row, column, cell_num - 1] = True
                    sudoku_cells[row][column] = cell_num

        return sudoku_map, sqrt_size

    def parse_puzzle_line(self, sudoku_str):
        '''
        Parse a Sudoku puzzle line
        :param str sudoku_str : comma sperated N cells that represent
                                a Sudoku puzzle line
        :return bool : Array that contains cell value (0 for blank)
        '''

        line = []
        # Permits redundant tailing ,s
        line_str = sudoku_str
        matched = self.split_line_regex.match(sudoku_str)
        if matched is not None:
            line_str = matched[1]

        # If no commas found in sudoku_str, treat sudoku_str as a sequence of
        # 1..9 and A..G (Hex with 0xG = decimal 16)
        if line_str.find(',') < 0:
            for num_str in line_str:
                num = 0
                try:
                    raw_num = int(num_str.strip())
                    # Filter out too large numbers later
                    if 1 <= raw_num:
                        num = raw_num
                except ValueError:
                    matched = self.split_exdigit_regex.match(num_str)
                    if matched is not None:
                        num = 10 + ord(num_str) - ord('A')
                line.append(num)

        else:
            for num_str in line_str.split(','):
                num = 0
                try:
                    raw_num = int(num_str.strip())
                    # Filter out too large numbers later
                    if 1 <= raw_num:
                        num = raw_num
                except ValueError:
                    # Non-number characters and 0 mean empty cells to be filled
                    pass
                line.append(num)

        return line

    def solve_all(self):
        '''
        Solve all sudoku puzzles
        '''

        return self.solve(self.sudoku_map, self.sqrt_size)

    def solve(self, sudoku_map, sqrt_size):
        '''
        Solve a Sudoku map
        :param nparray(N,N,N,bool) sudoku_map : Sudoku puzzle to be solved
        :sqrt_size int : sqrt(length(width and height of a puzzle))
        '''

        # Pass a copy of to prevent sudoku_map being overwritten
        map_str = self.map_to_string(sudoku_map)
        print(map_str)
        sys.stdout.flush()

        answer, result = self.solve_map(sudoku_map.copy(), sqrt_size)
        answer_str = self.map_to_string(answer)
        result = 'Solved' if self.verify(sudoku_map, answer, sqrt_size) else 'Not solved'
        print(result + '\n' + answer_str)
        sys.stdout.flush()

    def solve_map(self, sudoku_map, sqrt_size):
        '''
        Find unused candidates in rows, columns, and boxes that cells belong to
        :param nparray(N,N,N,bool) sudoku_map : Sudoku puzzle to be solved
            This method overwrites sudoku_map.
        :sqrt_size int : sqrt(length(width and height of a puzzle))
        :return (nparray(N,N,N,bool), bool) : (Answer of a sudoku maps,
            whether correctly solved or not)
        '''

        new_map = sudoku_map
        count = -1
        size = sqrt_size * sqrt_size
        n_cells = size * size

        while True:
            new_map, consistent = self.filter_candidates(new_map, sqrt_size)
            if not consistent:
                return new_map, False

            new_count = np.sum(np.sum(new_map, axis=2) == 1)
            if new_count == n_cells:
                return new_map, True

            # Inconsistent cells in backtracking
            if np.sum(np.sum(new_map, axis=2) == 0):
                return new_map, False

            # If candidates of cells do not decrease, start backtracking
            if count == new_count:
                return self.backtrack(new_map, sqrt_size)
            count = new_count

    def backtrack(self, sudoku_map, sqrt_size):
        '''
        Start Backtracking
        :param nparray(N,N,N,bool) sudoku_map : Sudoku puzzle to be solved
        :sqrt_size int : sqrt(length(width and height of a puzzle))
        :return (nparray(N,N,N,bool), bool) : (Answer of a sudoku maps,
            whether correctly solved or not)
        '''

        # How many candidates cells have
        counts = np.sum(sudoku_map, axis=2)
        # Indexes of cells that have multiple candidates
        locations = np.where(counts > 1)
        if len(locations[0]) == 0:
            return sudoku_map, False
        min_counts = np.min(counts[locations])
        locations = np.where(counts == min_counts)

        # Find a row or column which has more cells having min_counts
        row_frequency = np.unique(locations[0], return_counts=True)
        i_row = np.argmax(row_frequency[1])
        top_row_frequency = row_frequency[1][i_row]
        column_frequency = np.unique(locations[1], return_counts=True)
        i_column = np.argmax(column_frequency[1])
        top_column_frequency = column_frequency[1][i_column]

        if (top_row_frequency > top_column_frequency):
            row = row_frequency[0][i_row]
            index = np.where(locations[0] == row)[0][0]
            column = locations[1][index]
        else:
            column = column_frequency[0][i_column]
            index = np.where(locations[1] == column)[0][0]
            row = locations[0][index]

        # Find a cell having minimum candidates but unique
        candidates = np.where(sudoku_map[row, column, :])
        if len(candidates[0]) == 0:
            return sudoku_map, False

        # Guess a candidate one by one
        for candidate in candidates[0]:
            new_map = sudoku_map.copy()
            new_map[row, column, :] = False
            new_map[row, column, candidate] = True
            answer, solved = self.solve_map(new_map, sqrt_size)
            if solved:
                return answer, True

        return sudoku_map, False

    def filter_candidates(self, sudoku_map, sqrt_size):
        '''
        Filter candidates in cells
        :param nparray(N,N,N,bool) sudoku_map : Sudoku puzzle to be solved.
            This method overwrites sudoku_map.
        :sqrt_size int : sqrt(length(width and height of a puzzle))
        '''

        # Locked candidates
        for row in range(0, sudoku_map.shape[0], sqrt_size):
            for column in range(0, sudoku_map.shape[1], sqrt_size):
                box = sudoku_map[row:row+sqrt_size, column:column+sqrt_size, :]
                reduced_sum = np.sum(box, axis=(0,1))
                reduced_row = np.logical_or.reduce(box, axis=0)
                for candidate in np.where(np.sum(reduced_row, axis=0) == 1)[0]:
                    target_column = np.where(reduced_row[:,candidate] == True)[0] + column
                    sudoku_map[0:row, target_column, candidate] = False
                    sudoku_map[row+sqrt_size:sudoku_map.shape[0], target_column, candidate] = False

                reduced_column = np.logical_or.reduce(box, axis=1)
                for candidate in np.where(np.sum(reduced_column, axis=0) == 1)[0]:
                    target_row = np.where(reduced_column[:,candidate] == True)[0] + row
                    sudoku_map[target_row, 0:column, candidate] = False
                    sudoku_map[target_row, column+sqrt_size:sudoku_map.shape[1], candidate] = False

        for row in range(0, sudoku_map.shape[0]):
            for column in range(0, sudoku_map.shape[1]):
                self.filter_candidates_cell(sudoku_map, sqrt_size, row, column)
                if np.sum(sudoku_map[row, column]) == 0:
                    return sudoku_map, False

        return sudoku_map, True

    def filter_candidates_cell(self, sudoku_map, sqrt_size, row_num, column_num):
        '''
        Filter candidates in a cell
        :param nparray(N,N,N,bool) sudoku_map : Sudoku puzzle to be solved
        :sqrt_size int : sqrt(length(width and height of a puzzle))
        :param int row_num : Row number
        :param int column_num : Column number
        '''

        size = sqrt_size * sqrt_size
        box_start_row = sqrt_size * (row_num // sqrt_size)
        box_start_column = sqrt_size * (column_num // sqrt_size)
        inbox = (row_num % sqrt_size) * sqrt_size + column_num % sqrt_size

        # Filter a cell by a row, column and box the cell belongs
        n_cellset = 3
        unique_set = np.full((n_cellset, size), False, dtype=bool)
        unused_set = np.full((n_cellset, size), True, dtype=bool)

        unique_set[0], unused_set[0] = self.collect_candidates(sudoku_map[row_num], size, column_num)
        unique_set[1], unused_set[1] = self.collect_candidates(sudoku_map[:,column_num,:], size, row_num)
        unique_set[2], unused_set[2] = self.collect_candidates(self.box_to_row(sudoku_map, sqrt_size, box_start_row, box_start_column), size, inbox)

        # Candidates which are not unique to other cells
        unique_candidates = ~np.logical_or.reduce(unique_set, axis=0)
        sudoku_map[row_num, column_num] = np.logical_and(sudoku_map[row_num, column_num], unique_candidates)

        # Candidates which will not be used in other cells
        for i in range(n_cellset):
            if np.sum(sudoku_map[row_num, column_num]) > 1 and np.sum(unused_set[i]) == 1:
                sudoku_map[row_num, column_num] = unused_set[i]
                break

    def collect_candidates(self, input_cells, size, cell_index):
        '''
        Find candidates for a cell from eight cells of a row, column or box that the cell belongs.
        :param nparray(N,bool) input_cells : One row, column or box
        :size int : width and height of a puzzle
        :param int cell_index : Index for a cell to be updated
        :return (nparray(N,bool), nparray(N,bool)):
             (candidates which are unique in other cells,
              candidates which are unused in other cells)
        '''

        falses = np.full((size), False, dtype=bool)
        trues = np.full((size), True, dtype=bool)

        # Exclude candidates in the target cell
        cells = input_cells.copy()
        cells[cell_index, :] = False

        # Masks hold whether each cell has multiple candidates (not a unique candidate)
        multi_candidate_cells = np.where(np.sum(cells, axis=1) != 1)

        # Collect unused candidate for cells
        unused_candidates = ~np.logical_or.reduce(cells, axis=0)
        # If there is an unused candidate, return it.
        unused_candidates = trues if np.sum(unused_candidates) != 1 else unused_candidates

        # Collect unique candidates for cells
        cells[multi_candidate_cells] = falses
        unique_candidates = np.logical_or.reduce(cells, axis=0)

        return unique_candidates, unused_candidates

    def box_to_row(self, sudoku_map, sqrt_size, box_start_row, box_start_column):
        '''
        :param nparray(N,N,N,bool) sudoku_map : Sudoku puzzle to be solved
        :sqrt_size int : sqrt(length(width and height of a puzzle))
        :param int box_start_row : Top most index of a box in the sudoku map
        :param int box_start_column : Left most index of a box in the sudoku map
        :return nparray(N,N,bool) : candidates of nine cells in the box
        '''

        size = sqrt_size * sqrt_size
        new_map = sudoku_map[box_start_row:box_start_row+sqrt_size, box_start_column:box_start_column+sqrt_size, :].reshape(-1,size)
        return new_map

    def map_to_string(self, sudoku_map):
        '''
        Convert a sudoku map to a str
        :param nparray(N,N,N,bool) sudoku_map : Sudoku puzzle
        :return str : String which represents sudoku_map
        '''

        map_str = ''
        for row in range(0, sudoku_map.shape[0]):
            cell_strs = []
            for column in range(0, sudoku_map.shape[1]):
                cell_str = self.cell_to_string(sudoku_map, row, column)
                cell_strs.append(cell_str)
            map_str += '|'.join(cell_strs) + '\n'

        return map_str

    @staticmethod
    def cell_to_string(sudoku_map, row, column):
        '''
        Convert candidates of a sudoku map cell to a str
        :param nparray(N,N,N,bool) sudoku_map : Sudoku puzzle
        :param int row : Row number
        :param int column : Column number
        :return str : String which represents a cell
        '''

        n_canidates = sudoku_map.shape[2]
        # Blank cell
        if np.sum(sudoku_map[row, column]) == n_canidates:
            return '  '

        cells = []
        for candidate in range(0, n_canidates):
            if sudoku_map[row, column, candidate]:
                cells.append('{0:2d}'.format(candidate + 1))
        cell_strset = [x for x in cells if len(x)]
        return ','.join(cell_strset)

    def verify(self, input_map, answer, sqrt_size):
        '''
        Verify whether an answer is correct
        :param nparray(N,N,N,bool) input_map : Input sudoku puzzle
        :param answer(N,N,N,bool) input_map : Solved sudoku puzzle
        :sqrt_size int : sqrt(length(width and height of a puzzle))
        '''

        result = True

        for row in range(0, input_map.shape[0]):
            cell_strs = []
            for column in range(0, input_map.shape[1]):
                # No overwriting input_map
                if (np.sum(input_map[row, column, :]) == 1):
                    if not np.sum(np.logical_and(input_map[row, column], answer[row, column])):
                        message = 'The cell {},{} is overwritten'.format(row, column)
                        print(message)
                        result = False
                # This cell in the answer has a unique candidates
                if (np.sum(answer[row, column, :]) != 1):
                    message = 'The cell {},{} is not filled'.format(row, column)
                    print(message)
                    result = False

        # Each canidate in each row
        for row in range(0, answer.shape[0]):
            for i in range(2):
                if not np.logical_and.reduce(np.logical_or.reduce(answer[row], axis=i)):
                    message = 'The row {} has conflict'.format(row)
                    print(message)
                    result = False

        # Each canidate in each column
        for column in range(0, answer.shape[1]):
            for i in range(2):
                if not np.logical_and.reduce(np.logical_or.reduce(answer[:,column,:], axis=i)):
                    message = 'The column {} has conflict'.format(column)
                    print(message)
                    result = False

        # Each canidate in each box
        for row in range(0, answer.shape[0], sqrt_size):
            for column in range(0, answer.shape[1], sqrt_size):
                candidates = np.full(answer.shape[2], False, dtype=bool)
                for ix in range(0, sqrt_size):
                    for iy in range(0, sqrt_size):
                        candidates = np.logical_or(candidates, answer[row + iy, column + ix, :])
                if not np.logical_and.reduce(candidates):
                    message = 'The box {},{} has conflict'.format(row, column)
                    print(message)
                    result = False

        return result

if __name__ == "__main__":
    filename = sys.argv[1] if len(sys.argv) > 1 else 'data/sudoku_example1.txt'
    SudokuSolver(filename).solve_all()
