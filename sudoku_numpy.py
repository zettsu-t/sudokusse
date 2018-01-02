#!/usr/bin/python3
# coding: utf-8

'''
Solving Sudoku puzzles with NumPy
Copyright (C) 2018 Zettsu Tatsuya

usage:
$ python3 sudoku_numpy.py puzzle_text_filename

File 'sudoku-puzzle-filename' contains Sudoku puzzle in each line.
Each line contains 81 characters 1-9 (preset numbers) or others (blanks) like below.
.3.....4..1..97.5...25.86....3...8..9....43....76....4..98.54...7.....2..5..71.8.

See sudokusse.md for details.
'''

import re
import sys
import numpy as np

# Cell index 0..80 to (row, column, box) index table
# Cells are numbered (left to right, top to botoom) as below.
#   0,  1, ... , 8
#      ...
#  72, 73, ... , 80
# Boxes are numbered below.
#   0, 1, 2
#   3, 4, 5
#   6, 7, 8

CELL_TO_NINE_CELLS = [ \
    [(0, 0, 0), (0, 1, 0), (0, 2, 0), (0, 3, 1), (0, 4, 1), (0, 5, 1), (0, 6, 2), (0, 7, 2), (0, 8, 2)], \
    [(1, 0, 0), (1, 1, 0), (1, 2, 0), (1, 3, 1), (1, 4, 1), (1, 5, 1), (1, 6, 2), (1, 7, 2), (1, 8, 2)], \
    [(2, 0, 0), (2, 1, 0), (2, 2, 0), (2, 3, 1), (2, 4, 1), (2, 5, 1), (2, 6, 2), (2, 7, 2), (2, 8, 2)], \
    [(3, 0, 3), (3, 1, 3), (3, 2, 3), (3, 3, 4), (3, 4, 4), (3, 5, 4), (3, 6, 5), (3, 7, 5), (3, 8, 5)], \
    [(4, 0, 3), (4, 1, 3), (4, 2, 3), (4, 3, 4), (4, 4, 4), (4, 5, 4), (4, 6, 5), (4, 7, 5), (4, 8, 5)], \
    [(5, 0, 3), (5, 1, 3), (5, 2, 3), (5, 3, 4), (5, 4, 4), (5, 5, 4), (5, 6, 5), (5, 7, 5), (5, 8, 5)], \
    [(6, 0, 6), (6, 1, 6), (6, 2, 6), (6, 3, 7), (6, 4, 7), (6, 5, 7), (6, 6, 8), (6, 7, 8), (6, 8, 8)], \
    [(7, 0, 6), (7, 1, 6), (7, 2, 6), (7, 3, 7), (7, 4, 7), (7, 5, 7), (7, 6, 8), (7, 7, 8), (7, 8, 8)], \
    [(8, 0, 6), (8, 1, 6), (8, 2, 6), (8, 3, 7), (8, 4, 7), (8, 5, 7), (8, 6, 8), (8, 7, 8), (8, 8, 8)]]

class SudokuSolver(object):
    '''
    Solve a Sudoku puzzle
    '''

    def __init__(self, in_filename):
        self.sudoku_maps = self.parse_file(in_filename)

    def parse_file(self, in_filename):
        '''
        Parse Sudoku puzzles in a file
        :param str filename : Name of a file that contains sudoku puzzles in each line
        '''

        sudoku_maps = []

        with open(in_filename, 'r') as infile:
            line = infile.readline()
            while line:
                sudoku_str = line.strip()
                sudoku_map = self.parse_puzzle_str(sudoku_str)
                if not sudoku_map is None:
                    sudoku_maps.append((sudoku_str, sudoku_map))
                line = infile.readline()

        return sudoku_maps

    @staticmethod
    def parse_puzzle_str(sudoku_str):
        '''
        Parse a Sudoku puzzle
        :param str sudoku_str : 81 characters that represent a Sudoku puzzle
        character 1..9 are preset numbers for cells and others leave cells empty.
        '''

        if len(sudoku_str) < 81:
            return None

        # Rows, columns, and cell candidates
        # Cell[n] elements are True if n-1 is a candidate number of the cell
        # and False otherwise.
        sudoku_map = np.full((9, 9, 9), True, dtype=bool)

        for index, num_str in enumerate(sudoku_str):
            row = index // 9
            column = index % 9
            try:
                num = int(num_str)
                if num >= 1 and num <= 9:
                    sudoku_map[row, column] = np.full((9), False, dtype=bool)
                    sudoku_map[row, column, num - 1] = True
            except ValueError:
                # Non-number characters and 0 mean empty cells to be filled
                pass

        return sudoku_map

    def solve_all(self):
        '''
        Solve all sudoku puzzles
        '''

        for sudoku_str, sudoku_map in self.sudoku_maps:
            self.solve(sudoku_str, sudoku_map)

    def solve(self, sudoku_str, sudoku_map):
        '''
        Solve a Sudoku map
        :param str sudoku_str : 81 characters that represent a Sudoku puzzle
        :param nparray(9,9,9,bool) sudoku_map : Sudoku puzzle to be solved
        '''

        answer, result = self.solve_map(sudoku_map)
        answer_str = self.map_to_string(answer)
        result = 'Solved' if self.verify(sudoku_str, answer, answer_str) else 'Not solved'
        print(result + '\n' + answer_str)
        sys.stdout.flush()

    def solve_map(self, sudoku_map):
        '''
        Find unused candidates in rows, columns, and boxes that cells belong to
        :param nparray(9,9,9,bool) sudoku_map : Sudoku puzzle to be solved
        '''

        new_map = sudoku_map.copy()
        count = -1
        while True:
            new_map = self.filter_candidates(new_map)
            new_count = np.sum(np.sum(new_map, axis=2) == 1)
            if new_count == 81:
                return new_map, True
            # Inconsistent cells in backtracking
            if np.sum(np.sum(new_map, axis=2) == 0):
                return new_map, False
            if count == new_count:
                return self.backtrack(new_map)
            count = new_count

    def backtrack(self, sudoku_map):
        '''
        Start Backtracking
        :param nparray(9,9,9,bool) sudoku_map : Sudoku puzzle to be solved
        '''

        # How many candidates cells have
        counts = np.sum(sudoku_map, axis=2)
        # Indexes of cells that have multiple candidates
        locations = np.where(counts > 1)
        if len(locations[0]) == 0:
            return sudoku_map, False

        # Find a cell having minimum candidates but unique
        index = np.argmin(counts[locations])
        row = locations[0][index]
        column = locations[1][index]
        candidates = np.where(sudoku_map[row, column, :])

        if len(candidates[0]) == 0:
            return sudoku_map, False

        for candidate in candidates[0]:
            new_map = sudoku_map.copy()
            new_map[row, column, :] = False
            new_map[row, column, candidate] = True
            answer, solved = self.solve_map(new_map)
            if solved:
                return answer, True

        return sudoku_map, False

    @staticmethod
    def map_to_string(sudoku_map):
        '''
        Convert a sudoku map to a str
        :param nparray(9,9,9,bool) sudoku_map : Sudoku puzzle
        '''

        map_str = ''
        for row in range(0, 9):
            cell_strs = []
            for column in range(0, 9):
                cell_str = ''
                for candidate in range(0, 9):
                    if sudoku_map[row, column, candidate]:
                        cell_str += '{}'.format(candidate+1)
                cell_strs.append(cell_str)
            map_str += ','.join(cell_strs) + '\n'
        return map_str

    def verify(self, sudoku_str, answer, answer_str):
        '''
        Verify whether an answer is correct
        :param str sudoku_str : 81 characters that represent a Sudoku puzzle
        :param nparray(9,9,9,bool) answer : Solved sudoku puzzle
        :param str answer_str : Nine lines that represent the solved sudoku puzzle
        '''

        # Each cell has only one number
        count = np.all(np.sum(answer, axis=2) == 1)
        # Each row has 1..9
        row = np.all(np.sum(np.logical_or.reduce(answer, axis=0), axis=1) == 9)
        # Each column has 1..9
        column = np.all(np.sum(np.logical_or.reduce(answer, axis=0), axis=1) == 9)
        # Each box has 1..9
        box = True
        for index in range(0, 9):
            box &= (np.sum(np.logical_or.reduce(self.box_to_row(answer, index))) == 9)

        # No preset numbers are changed
        answer_line = ''.join(answer_str.split('\n')).replace(',', '')
        no_change = True
        for in_char, out_char in zip(sudoku_str, answer_line):
            no_change &= (re.match(r'[1-9]', in_char) is None or (in_char == out_char))

        result = count & row & column & box & no_change
        return result

    def filter_candidates(self, sudoku_map):
        '''
        Filter candidates in cells
        :param nparray(9,9,9,bool) sudoku_map : Sudoku puzzle to be solved
        '''

        new_map = sudoku_map.copy()
        for row_num in range(0, 9):
            for column_num in range(0, 9):
                row, column, box = CELL_TO_NINE_CELLS[row_num][column_num]
                inbox = (row % 3) * 3 + column % 3

                # Filter a cell by a row, column and box the cell belongs
                unique_set = np.full((3, 9), False, dtype=bool)
                unused_set = np.full((3, 9), True, dtype=bool)

                unique_set[0], unused_set[0] = self.collect_candidates(new_map[row], column)
                unique_set[1], unused_set[1] = self.collect_candidates(np.transpose(new_map, (1, 0, 2))[column], row)
                unique_set[2], unused_set[2] = self.collect_candidates(self.box_to_row(new_map, box), inbox)

                # Candidates which are not unique to other cells
                unique_candidates = ~np.logical_or.reduce(unique_set, axis=0)
                # Candidates which will not be used in other cells
                unused_candidates = np.logical_or.reduce(unused_set, axis=0)
                new_map[row_num, column_num] = np.logical_and(new_map[row_num, column_num], unique_candidates)
                new_map[row_num, column_num] = np.logical_and(new_map[row_num, column_num], unused_candidates)
        return new_map

    @staticmethod
    def collect_candidates(nine_cells, cell_index):
        '''
        Find candidates for a cell from eight cells of a row, column or box that the cell belongs.
        :param nparray(9,bool) nine_cells : One row, column or box
        :param int cell_index : Index for a cell to be updated
        '''

        falses = np.full((9), False, dtype=bool)
        trues = np.full((9), True, dtype=bool)

        # Exclude candidates in the target cell
        cells = nine_cells.copy()
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

    @staticmethod
    def box_to_row(sudoku_map, box_index):
        '''
        :param nparray(9,9,9,bool) sudoku_map : Sudoku puzzle to be solved
        :param int box_index : Index of a box in the sudoku map
        '''

        # I hope there are more efficient methods to inject Sudoku maps.
        new_map = np.full((9, 9), False, dtype=bool)
        from_box_row = 3 * (box_index // 3)
        from_column = 3 * (box_index % 3)
        for inbox in range(0, 3):
            from_row = from_box_row + inbox
            to_column = inbox * 3
            new_map[to_column:to_column+3, :] = \
                sudoku_map[from_row, from_column:from_column+3, :]

        return new_map

if __name__ == "__main__":
    filename = sys.argv[1] if len(sys.argv) > 1 else 'data/sudoku_example2.txt'
    SudokuSolver(filename).solve_all()
