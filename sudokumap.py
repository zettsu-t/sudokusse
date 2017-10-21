#!/usr/bin/python3
# coding: utf-8
# This script writes code which is same as sudokumap.rb writes.
# Copyright (C) 2017 Zettsu Tatsuya

import itertools
import os
import re
import sys

# If you give a filename as a command line argument, this script writes
# code to the file, otherwise write to the file specified below.
SUDOKU_GENERATED_FILENAME = 'sudokuConstAll.h'

# Constants for Sudoku
SUDOKU_CELLS_IN_EDGE = 9        # Number of cells in a row or a column
SUDOKU_ROWS_IN_MAP = 9          # Number of cells in a row
SUDOKU_COLUMNS_IN_MAP = 9       # Number of cells in a column
SUDOKU_GROUPS_IN_MAP = 9        # Number of columns, rows, and boxes in a puzzle
SUDOKU_CELLS_IN_GROUP = 9       # Number of cells in a column, row, and box
SUDOKU_BOX_VERTICAL_SIZE = 3    # Number of boxes in a column
SUDOKU_BOX_HORIZONTAL_SIZE = 3  # Number of boxes in a row
SUDOKU_BOX_IN_ROW = 3           # Width of a box
SUDOKU_BOX_IN_COLUMN = 3        # Height of a box
SUDOKU_LOOKUP_CELLS = 512       # The number of elements in a cell look-up table
SUDOKU_NUMBER_OF_CANDIDATES = 9 # Number of candidates in a cell

# The number of an XMM register which holds the top row
SUDOKU_HEAD_XMM_REGISTER = 1
# General purpose registers per XMM register
SUDOKU_XMM_PER_GENERAL_REGISTER = 4

# Generated header file
class SudokuConstAll:
    def __init__(self):
        self.filename = SUDOKU_GENERATED_FILENAME
        if len(sys.argv) > 1:
            self.filename = sys.argv[1]

    # Writes generated code to a file
    def write(self):
        file = open(self.filename, 'w')
        file.write(self.get_cpp_str())
        file.write("\n")

    def get_cpp_str(self):
        return "\n".join([self.get_group_str(),
                          self.get_reverse_group_str(),
                          self.get_cell_lookup_str()])

    def get_group_str(self):
        str =  'const SudokuIndex SudokuMap::Group_'
        str += '[Sudoku::SizeOfGroupsPerCell]'
        str += '[Sudoku::SizeOfGroupsPerMap]'
        str += "[Sudoku::SizeOfCellsPerGroup] {\n"
        str += self.get_lines_str(self.get_rows_str_set(), ',')
        str += self.get_lines_str(self.get_columns_str_set(), ',')
        str += self.get_lines_str(self.get_boxes_str_set(), '')
        str += "};\n"
        return str

    def get_lines_str(self, line_str_set, delim):
        return '{' + ",\n".join(line_str_set) + '}' + delim + "\n"

    def get_cell_str(self, column, row):
        return str(row * SUDOKU_COLUMNS_IN_MAP + column)

    def get_cells_str(self, cell_str_func, length):
        str = '{'
        str += ','.join(list(map(cell_str_func, range(length))))
        str += '}'
        return str

    def get_row_str(self, row):
        f = lambda column: self.get_cell_str(column, row)
        return self.get_cells_str(f, SUDOKU_COLUMNS_IN_MAP)

    def get_rows_str_set(self):
        return list(map(self.get_row_str, range(SUDOKU_ROWS_IN_MAP)))

    def get_column_str(self, column):
        f = lambda row: self.get_cell_str(column, row)
        return self.get_cells_str(f, SUDOKU_ROWS_IN_MAP)

    def get_columns_str_set(self):
        return list(map(self.get_column_str,
                        range(SUDOKU_COLUMNS_IN_MAP)))

    def get_box_line_str(self, x, y, ofs_x):
        f = lambda ofs_y: str(
            x + ofs_x + (y + ofs_y) * SUDOKU_COLUMNS_IN_MAP)
        return ','.join(list(map(f, range(SUDOKU_BOX_VERTICAL_SIZE))))

    def get_box_str_set(self, x, y):
        f = lambda ofs_x: self.get_box_line_str(
            x * SUDOKU_BOX_HORIZONTAL_SIZE, y, ofs_x)

        str = '{'
        str += ','.join(list(map(f, range(SUDOKU_BOX_VERTICAL_SIZE))))
        str += '}'
        return str

    def get_3boxes_str_set(self, y):
        f = lambda x: self.get_box_str_set(x, y * SUDOKU_BOX_VERTICAL_SIZE)
        return list(map(f, range(SUDOKU_BOX_IN_COLUMN)))

    def get_boxes_str_set(self):
        return sum(list(map(self.get_3boxes_str_set,
                            range(SUDOKU_BOX_IN_ROW))), [])

    def get_reverse_group_cell_str(self, column, row):
        group = ((column // SUDOKU_BOX_IN_COLUMN)
                 * SUDOKU_BOX_HORIZONTAL_SIZE) + (row // SUDOKU_BOX_IN_ROW)
        return "{%d,%d,%d}" % (column, row, group)

    def get_reverse_group_column_str(self, column):
        f = lambda row: self.get_reverse_group_cell_str(column, row)
        return ','.join(list(map(f, range(SUDOKU_ROWS_IN_MAP))))

    def get_reverse_group_str(self):
        str = 'const SudokuIndex SudokuMap::ReverseGroup_'
        str += "[Sudoku::SizeOfAllCells][Sudoku::SizeOfGroupsPerCell] {\n"

        f = lambda column: self.get_reverse_group_column_str(column)
        str += ",\n".join(list(map(f, range(SUDOKU_COLUMNS_IN_MAP))))
        str += "\n};\n"
        return str

    def count_bits(self, bitmap):
        count = 0
        bitmask = 1
        for i in range(SUDOKU_CELLS_IN_GROUP):
            count += 1 if (bitmap & bitmask) else 0
            bitmask *= 2
        return count

    def get_element_str(self, bitmap):
        count = self.count_bits(bitmap)
        is_unique = 'true' if (count == 1) else 'false'
        is_multiple = 'true' if (count > 1) else 'false'
        str = '{%s,%s,%d}' % (is_unique, is_multiple, count)
        str += '.' if (bitmap % 8) == 7 else ''
        return str

    def get_cell_lookup_str(self):
        header = 'const SudokuCellLookUp SudokuCell::CellLookUp_'
        header += "[Sudoku::SizeOfLookUpCell] {\n"
        str = ','.join(list(map(self.get_element_str,
                                range(2 ** SUDOKU_CELLS_IN_GROUP))))
        return header + re.sub(r'\.(,?)', "\\1\n", str) + "};"

obj = SudokuConstAll()
obj.write()
sys.exit(0)
