#!/usr/bin/python3
# coding: utf-8

'''
This script tests the code generator script sudokumap.py
Copyright (C) 2017 Zettsu Tatsuya

usage : python3 -m unittest discover tests
'''

import tempfile
from unittest import TestCase
import sudokumap.sudokumap as tested

class TestConstants(TestCase):
    '''Testing global constants'''

    def test_sudoku_constants(self):
        '''Testing constants for Sudoku puzzles'''

        self.assertEqual(tested.SUDOKU_CELLS_IN_EDGE, 9)
        self.assertEqual(tested.SUDOKU_ROWS_IN_MAP, 9)
        self.assertEqual(tested.SUDOKU_COLUMNS_IN_MAP, 9)
        self.assertEqual(tested.SUDOKU_GROUPS_IN_MAP, 9)
        self.assertEqual(tested.SUDOKU_CELLS_IN_GROUP, 9)
        self.assertEqual(tested.SUDOKU_BOX_VERTICAL_SIZE, 3)
        self.assertEqual(tested.SUDOKU_BOX_HORIZONTAL_SIZE, 3)
        self.assertEqual(tested.SUDOKU_BOX_IN_ROW, 3)
        self.assertEqual(tested.SUDOKU_BOX_IN_COLUMN, 3)

    def test_code_constatns(self):
        '''Testing constants for generated code'''

        self.assertEqual(tested.SUDOKU_LOOKUP_CELLS, 512)
        self.assertEqual(tested.SUDOKU_NUMBER_OF_CANDIDATES, 9)
        self.assertEqual(tested.SUDOKU_HEAD_XMM_REGISTER, 1)
        self.assertEqual(tested.SUDOKU_XMM_PER_GENERAL_REGISTER, 4)


class TestCommandLineArguments(TestCase):
    '''Testing parsing command line arguments.'''

    def test_no_arguments(self):
        '''Testing a case without command line arguments'''

        arguments = ['command']
        generator = tested.SudokuConstAll(arguments)
        self.assertEqual(generator.filename, tested.SUDOKU_GENERATED_FILENAME)

    def test_with_arguments(self):
        '''Testing a case without command line arguments'''

        filename = 'output.hpp'
        self.assertNotEqual(tested.SUDOKU_GENERATED_FILENAME, filename)

        arguments = ['command', filename]
        generator = tested.SudokuConstAll(arguments)
        self.assertEqual(generator.filename, filename)


class SudokuConstAllMocked(tested.SudokuConstAll):
    '''This class returns fixed strings instead of generating real code'''

    def __init__(self, filename):
        super().__init__(["command", filename])

    def get_rows_str_set(self):
        return ['Rows', 'Code']

    def get_columns_str_set(self):
        return ['Columns', 'Code']

    def get_boxes_str_set(self):
        return ['Boxes', 'Code']

    def get_reverse_group_str(self):
        return 'ReverseGroupCode'

    def get_cell_lookup_str(self):
        return 'ForwardGroupCode'

EXPECTED_GENERATED_CODE = 'const SudokuIndex SudokuMap::Group_'
EXPECTED_GENERATED_CODE += '[Sudoku::SizeOfGroupsPerCell]'
EXPECTED_GENERATED_CODE += '[Sudoku::SizeOfGroupsPerMap]'
EXPECTED_GENERATED_CODE += '[Sudoku::SizeOfCellsPerGroup] {\n'
EXPECTED_GENERATED_CODE += '{Rows,\nCode},\n'
EXPECTED_GENERATED_CODE += '{Columns,\nCode},\n'
EXPECTED_GENERATED_CODE += '{Boxes,\nCode}\n'
EXPECTED_GENERATED_CODE += '};\n\n'
EXPECTED_GENERATED_CODE += 'ReverseGroupCode\n'
EXPECTED_GENERATED_CODE += 'ForwardGroupCode'

class TestGenratingCode(TestCase):
    '''Testing to generate and write code'''

    def test_write(self):
        '''Testing written file'''

        # Delete the tmpfile after closing
        with tempfile.NamedTemporaryFile() as tmpfile:
            filename = tmpfile.name
            tmpfile.close()
            SudokuConstAllMocked(filename).write()

            with open(filename, 'r') as file:
                actual = ''.join(file.readlines())
                self.assertEqual(actual, EXPECTED_GENERATED_CODE + '\n')

    def test_get_cpp_str(self):
        '''Testing to bind all code'''

        actual = SudokuConstAllMocked("").get_cpp_str()
        self.assertEqual(actual, EXPECTED_GENERATED_CODE)

    def test_get_group_str(self):
        '''Testing to bind cell groups'''

        expected = 'const SudokuIndex SudokuMap::Group_'
        expected += '[Sudoku::SizeOfGroupsPerCell]'
        expected += '[Sudoku::SizeOfGroupsPerMap]'
        expected += '[Sudoku::SizeOfCellsPerGroup] {\n'
        expected += '{Rows,\nCode},\n'
        expected += '{Columns,\nCode},\n'
        expected += '{Boxes,\nCode}\n'
        expected += '};\n'
        actual = SudokuConstAllMocked("").get_group_str()
        self.assertEqual(actual, expected)


class TestCommonFunctions(TestCase):
    '''Testing common functions for other functions'''

    def test_count_bits(self):
        '''Testing population counting'''
        cases = [[0, 0], [1, 1], [2, 1], [3, 2],
                 [4, 1], [5, 2], [6, 2], [7, 3]]
        for bitmap, expected in cases:
            actual = tested.SudokuConstAll.count_bits(bitmap)
            self.assertEqual(actual, expected)

    def test_cell_index(self):
        '''Testing cell indexes'''

        cases = [[0, 0, 0], [1, 0, 1], [8, 0, 8], [0, 1, 9],
                 [8, 1, 17], [0, 8, 72], [8, 8, 80]]
        for column, row, expected in cases:
            actual = tested.SudokuConstAll.get_cell_index(column, row)
            self.assertEqual(actual, expected)

    def test_cells_str(self):
        '''Testing C++ arrays that contains cell indexes'''

        def mapped_func(number):
            '''Makes a string for the number'''
            return str(number * 3)

        cases = [[0, '{}'], [1, '{0}'], [2, '{0,3}'], [3, '{0,3,6}']]
        for length, expected in cases:
            actual = tested.SudokuConstAll.get_cells_str(mapped_func, length)
            self.assertEqual(actual, expected)

    def test_rows_str_set(self):
        '''Testing cells in Sudoku rows'''

        expected = ['{0,1,2,3,4,5,6,7,8}',
                    '{9,10,11,12,13,14,15,16,17}',
                    '{18,19,20,21,22,23,24,25,26}',
                    '{27,28,29,30,31,32,33,34,35}',
                    '{36,37,38,39,40,41,42,43,44}',
                    '{45,46,47,48,49,50,51,52,53}',
                    '{54,55,56,57,58,59,60,61,62}',
                    '{63,64,65,66,67,68,69,70,71}',
                    '{72,73,74,75,76,77,78,79,80}']
        actual = tested.SudokuConstAll([]).get_rows_str_set()
        self.assertEqual(actual, expected)

    def test_columns_str_set(self):
        '''Testing cells in Sudoku columns'''

        expected = ['{0,9,18,27,36,45,54,63,72}',
                    '{1,10,19,28,37,46,55,64,73}',
                    '{2,11,20,29,38,47,56,65,74}',
                    '{3,12,21,30,39,48,57,66,75}',
                    '{4,13,22,31,40,49,58,67,76}',
                    '{5,14,23,32,41,50,59,68,77}',
                    '{6,15,24,33,42,51,60,69,78}',
                    '{7,16,25,34,43,52,61,70,79}',
                    '{8,17,26,35,44,53,62,71,80}']
        actual = tested.SudokuConstAll([]).get_columns_str_set()
        self.assertEqual(actual, expected)

    def test_box_line_str(self):
        '''Testing lines in a Sudoku box'''

        cases = [[0, 0, 0, '0,9,18'],
                 [3, 0, 0, '3,12,21'],
                 [0, 6, 0, '54,63,72'],
                 [0, 0, 1, '1,10,19'],
                 [6, 3, 2, '35,44,53']]
        for base_x, base_y, ofs_x, expected in cases:
            actual = tested.SudokuConstAll.get_box_line_str(base_x, base_y, ofs_x)
            self.assertEqual(actual, expected)

    def test_box_str_set(self):
        '''Testing cells in a Sudoku box'''

        cases = [[0, 0, '{0,9,18,1,10,19,2,11,20}'],
                 [1, 0, '{3,12,21,4,13,22,5,14,23}'],
                 [2, 0, '{6,15,24,7,16,25,8,17,26}'],
                 [0, 1, '{9,18,27,10,19,28,11,20,29}'],
                 [1, 1, '{12,21,30,13,22,31,14,23,32}'],
                 [2, 1, '{15,24,33,16,25,34,17,26,35}'],
                 [0, 2, '{18,27,36,19,28,37,20,29,38}'],
                 [1, 2, '{21,30,39,22,31,40,23,32,41}'],
                 [2, 2, '{24,33,42,25,34,43,26,35,44}']]
        for box_x, box_y, expected in cases:
            actual = tested.SudokuConstAll([]).get_box_str_set(box_x, box_y)
            self.assertEqual(actual, expected)

    def test_boxes_str_set(self):
        '''Testing cells in Sudoku boxes'''

        expected = ['{0,9,18,1,10,19,2,11,20}',
                    '{3,12,21,4,13,22,5,14,23}',
                    '{6,15,24,7,16,25,8,17,26}',
                    '{27,36,45,28,37,46,29,38,47}',
                    '{30,39,48,31,40,49,32,41,50}',
                    '{33,42,51,34,43,52,35,44,53}',
                    '{54,63,72,55,64,73,56,65,74}',
                    '{57,66,75,58,67,76,59,68,77}',
                    '{60,69,78,61,70,79,62,71,80}']
        actual = tested.SudokuConstAll([]).get_boxes_str_set()
        self.assertEqual(actual, expected)

    def test_reverse_group_cell_str(self):
        '''Testing each element in the reverse group'''

        cases = [[0, 0, '{0,0,0}'], [8, 8, '{8,8,8}'],
                 [0, 1, '{0,1,0}'], [0, 2, '{0,2,0}'], [0, 3, '{0,3,1}'],
                 [1, 0, '{1,0,0}'], [2, 0, '{2,0,0}'], [3, 0, '{3,0,3}'],
                 [0, 8, '{0,8,2}'], [8, 0, '{8,0,6}'],
                 [5, 8, '{5,8,5}'], [8, 5, '{8,5,7}'],
                 [6, 6, '{6,6,8}'], [7, 8, '{7,8,8}'], [8, 7, '{8,7,8}']]
        for column, row, expected in cases:
            actual = tested.SudokuConstAll.get_reverse_group_cell_str(column, row)
            self.assertEqual(actual, expected)

    def test_reverse_group_column_str(self):
        '''Testing nine elements in the cell reverse group'''

        expected = '{7,0,6},{7,1,6},{7,2,6},{7,3,7},'
        expected += '{7,4,7},{7,5,7},{7,6,8},{7,7,8},{7,8,8}'
        actual = tested.SudokuConstAll([]).get_reverse_group_column_str(7)
        self.assertEqual(actual, expected)

    def test_reverse_group_str(self):
        '''Testing elements in the cells reverse group'''

        expected_header = 'const SudokuIndex SudokuMap::ReverseGroup_'
        expected_header += '[Sudoku::SizeOfAllCells]'
        expected_header += '[Sudoku::SizeOfGroupsPerCell] {'

        expected_first_line = '{0,0,0},{0,1,0},{0,2,0},{0,3,1},'
        expected_first_line += '{0,4,1},{0,5,1},{0,6,2},{0,7,2},{0,8,2},'

        expected_last_line = '{8,0,6},{8,1,6},{8,2,6},{8,3,7},'
        expected_last_line += '{8,4,7},{8,5,7},{8,6,8},{8,7,8},{8,8,8}'

        actual = tested.SudokuConstAll([]).get_reverse_group_str().split('\n')
        self.assertEqual(len(actual), 12)
        self.assertEqual(actual[0], expected_header)
        self.assertEqual(actual[1], expected_first_line)
        self.assertEqual(actual[-3], expected_last_line)
        self.assertEqual(actual[-2], '};')

    def test_cell_lookup_element_str(self):
        '''Testing an element in the cell forward group'''

        cases = [[0, '{false,false,0}'], [1, '{true,false,1}'],
                 [2, '{true,false,1}'], [3, '{false,true,2}'], [7, '{false,true,3}.'],
                 [510, '{false,true,8}'], [511, '{false,true,9}.']]
        for bitmap, expected in cases:
            actual = tested.SudokuConstAll([]).get_cell_lookup_element_str(bitmap)
            self.assertEqual(actual, expected)

    def test_cell_lookup_str(self):
        '''Testing elements in the cell forward group'''

        expected_header = 'const SudokuCellLookUp SudokuCell::CellLookUp_'
        expected_header += '[Sudoku::SizeOfLookUpCell] {'

        expected_first_line = '{false,false,0},{true,false,1},'
        expected_first_line += '{true,false,1},{false,true,2},'
        expected_first_line += '{true,false,1},{false,true,2},'
        expected_first_line += '{false,true,2},{false,true,3},'

        expected_last_line = '{false,true,6},{false,true,7},'
        expected_last_line += '{false,true,7},{false,true,8},'
        expected_last_line += '{false,true,7},{false,true,8},'
        expected_last_line += '{false,true,8},{false,true,9}'

        actual = tested.SudokuConstAll([]).get_cell_lookup_str().split('\n')
        self.assertEqual(len(actual), 66)
        self.assertEqual(actual[0], expected_header)
        self.assertEqual(actual[1], expected_first_line)
        self.assertEqual(actual[-2], expected_last_line)
        self.assertEqual(actual[-1], '};')
