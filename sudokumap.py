#!/usr/bin/python3
# coding: utf-8

'''
This script writes code which is same as sudokumap.rb writes.
Copyright (C) 2017 Zettsu Tatsuya
'''

import re
import sys

# If you give a filename as a command line argument, this script writes
# code to the file, otherwise write to the file specified below.
SUDOKU_GENERATED_FILENAME = 'sudokuConstAll.h'

# Constants for Sudoku
SUDOKU_CELLS_IN_EDGE = 9   # Number of cells in a row or a column
SUDOKU_ROWS_IN_MAP = 9     # Number of cells in a row
SUDOKU_COLUMNS_IN_MAP = 9  # Number of cells in a column
SUDOKU_GROUPS_IN_MAP = 9   # Number of columns, rows, and boxes in a puzzle
SUDOKU_CELLS_IN_GROUP = 9  # Number of cells in a column, row, and box
SUDOKU_BOX_VERTICAL_SIZE = 3    # Number of boxes in a column
SUDOKU_BOX_HORIZONTAL_SIZE = 3  # Number of boxes in a row
SUDOKU_BOX_IN_ROW = 3           # Width of a box
SUDOKU_BOX_IN_COLUMN = 3        # Height of a box

# The number of elements in a cell look-up table
SUDOKU_LOOKUP_CELLS = 512
# Number of candidates in a cell
SUDOKU_NUMBER_OF_CANDIDATES = 9

# The number of an XMM register which holds the top row
SUDOKU_HEAD_XMM_REGISTER = 1
# General purpose registers per XMM register
SUDOKU_XMM_PER_GENERAL_REGISTER = 4


class SudokuConstAll():
    '''Generated header file'''

    def __init__(self):
        self.filename = SUDOKU_GENERATED_FILENAME
        if len(sys.argv) > 1:
            self.filename = sys.argv[1]

    def write(self):
        '''Writes generated code to a file'''

        with open(self.filename, 'w') as file:
            file.write(self.get_cpp_str())
            file.write('\n')

    def get_cpp_str(self):
        '''Returns full generated code'''

        return '\n'.join([self.get_group_str(),
                          self.get_reverse_group_str(),
                          self.get_cell_lookup_str()])

    def get_group_str(self):
        '''Returns code for Sudoku cell groups'''

        def get_lines_str(line_str_set, delim):
            '''Returns code for a Sudoku cell row or column'''

            return '{' + ',\n'.join(line_str_set) + '}' + delim + '\n'

        code_str = 'const SudokuIndex SudokuMap::Group_'
        code_str += '[Sudoku::SizeOfGroupsPerCell]'
        code_str += '[Sudoku::SizeOfGroupsPerMap]'
        code_str += '[Sudoku::SizeOfCellsPerGroup] {\n'
        code_str += get_lines_str(self.get_rows_str_set(), ',')
        code_str += get_lines_str(self.get_columns_str_set(), ',')
        code_str += get_lines_str(self.get_boxes_str_set(), '')
        code_str += '};\n'
        return code_str

    @staticmethod
    def get_cell_index(column, row):
        '''
        Returns a cell index of the cell
        at the column and the row.
        '''

        return row * SUDOKU_COLUMNS_IN_MAP + column

    @staticmethod
    def get_cells_str(cell_str_func, length):
        '''
        Converts numbers 0..(length-1) to a C++ array
        via cell_str_func.
        '''
        code_str = '{'
        code_str += ','.join(list(map(cell_str_func, range(length))))
        code_str += '}'
        return code_str

    def get_rows_str_set(self):
        '''Returns a set of C++ arrays for Sudoku rows'''

        def get_row_str(row):
            '''Returns a C++ array for a Sudoku row'''

            def mapper_func(column):
                '''Binds the column to map'''

                return str(self.get_cell_index(column, row))

            return self.get_cells_str(mapper_func, SUDOKU_COLUMNS_IN_MAP)

        return list(map(get_row_str, range(SUDOKU_ROWS_IN_MAP)))

    def get_columns_str_set(self):
        '''Returns a set of C++ arrays for Sudoku columns'''

        def get_column_str(column):
            '''Returns a C++ array for a Sudoku column'''

            def mapper_func(row):
                '''Binds the row to map'''

                return str(self.get_cell_index(column, row))

            return self.get_cells_str(mapper_func, SUDOKU_ROWS_IN_MAP)

        return list(map(get_column_str, range(SUDOKU_COLUMNS_IN_MAP)))

    @staticmethod
    def get_box_line_str(base_x, base_y, ofs_x):
        '''Returns code for a line in a Sudoku 3x3 box'''

        def mapper_func(ofs_y):
            '''Binds the ofs_y to map'''

            return str(base_x + ofs_x + (base_y + ofs_y) *
                       SUDOKU_COLUMNS_IN_MAP)

        return ','.join(list(map(mapper_func,
                                 range(SUDOKU_BOX_VERTICAL_SIZE))))

    def get_box_str_set(self, box_x, box_y):
        '''Returns code for a Sudoku 3x3 box'''

        def mapper_func(ofs_x):
            '''Binds the ofs_x to map'''

            return self.get_box_line_str(box_x * SUDOKU_BOX_HORIZONTAL_SIZE,
                                         box_y, ofs_x)

        code_str = '{'
        code_str += ','.join(list(map(mapper_func,
                                      range(SUDOKU_BOX_VERTICAL_SIZE))))
        code_str += '}'
        return code_str

    def get_boxes_str_set(self):
        '''Returns a set of C++ arrays for Sudoku 3x3 boxes'''

        def get_3boxes_str_set(box_y):
            '''Returns a set of C++ arrays for Sudoku 3x3 boxes in a row'''

            def mapper_func(box_x):
                '''Binds the box_x to map'''
                return self.get_box_str_set(box_x, box_y *
                                            SUDOKU_BOX_VERTICAL_SIZE)

            return list(map(mapper_func, range(SUDOKU_BOX_IN_COLUMN)))

        return sum(list(map(get_3boxes_str_set,
                            range(SUDOKU_BOX_IN_ROW))), [])

    @staticmethod
    def get_reverse_group_cell_str(column, row):
        '''Returns code of a cell in a cell reverse lookup table'''

        group = ((column // SUDOKU_BOX_IN_COLUMN) *
                 SUDOKU_BOX_HORIZONTAL_SIZE) + (row // SUDOKU_BOX_IN_ROW)
        return '{' + '{},{},{}'.format(column, row, group) + '}'

    def get_reverse_group_column_str(self, column):
        '''Returns code of a column in a cell reverse lookup table'''

        def mapper_func(row):
            '''Binds the row to map'''

            return self.get_reverse_group_cell_str(column, row)

        return ','.join(list(map(mapper_func, range(SUDOKU_ROWS_IN_MAP))))

    def get_reverse_group_str(self):
        '''Returns code of a cell reverse lookup table'''

        code_str = 'const SudokuIndex SudokuMap::ReverseGroup_'
        code_str += '[Sudoku::SizeOfAllCells][Sudoku::SizeOfGroupsPerCell] {\n'

        def mapper_func(column):
            '''Binds the column to map'''

            return self.get_reverse_group_column_str(column)

        code_str += ',\n'.join(list(map(mapper_func,
                                        range(SUDOKU_COLUMNS_IN_MAP))))
        code_str += '\n};\n'
        return code_str

    @staticmethod
    def count_bits(bitmap):
        '''Returns the number of bit 1s in the integet bitmap'''

        count = 0
        bitmask = 1
        bit_pos = 0
        while bit_pos < SUDOKU_CELLS_IN_GROUP:
            count += 1 if (bitmap & bitmask) else 0
            bitmask *= 2
            bit_pos += 1
        return count

    def get_cell_lookup_element_str(self, bitmap):
        '''Returns code of an element in a cell lookup table'''

        count = self.count_bits(bitmap)
        is_unique = 'true' if (count == 1) else 'false'
        is_multiple = 'true' if (count > 1) else 'false'
        code_str = '{' + '{},{},{}'.format(is_unique, is_multiple, count) + '}'
        code_str += '.' if (bitmap % 8) == 7 else ''
        return code_str

    def get_cell_lookup_str(self):
        '''Returns code of a cell forward lookup table'''

        header = 'const SudokuCellLookUp SudokuCell::CellLookUp_'
        header += '[Sudoku::SizeOfLookUpCell] {\n'
        code_str = ','.join(list(map(self.get_cell_lookup_element_str,
                                     range(2 ** SUDOKU_CELLS_IN_GROUP))))
        return header + re.sub(r'\.(,?)', '\\1\n', code_str) + '};'

SudokuConstAll().write()
sys.exit(0)
