#!/usr/bin/python3
# coding: utf-8

'''
This script tests the code generator script sudokumap.py
Copyright (C) 2017 Zettsu Tatsuya

usage : python3 -m unittest discover tests
'''

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
    '''Testing parsing command line arguments'''

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


class TestCommonFunctions(TestCase):
    '''Testing common functions for other functions'''

    def test_count_bits(self):
        '''Testing a population counting'''

        cases = [[0, 0], [1, 1], [2, 1], [3, 2],
                 [4, 1], [5, 2], [6, 2], [7, 3]]
        for bitmap, expected in cases:
            actual = tested.SudokuConstAll.count_bits(bitmap)
            self.assertEqual(actual, expected)

    def test_get_cell_index(self):
        '''Testing cell indexes'''

        cases = [[0, 0, 0], [1, 0, 1], [8, 0, 8], [0, 1, 9],
                 [8, 1, 17], [0, 8, 72], [8, 8, 80]]
        for column, row, expected in cases:
            actual = tested.SudokuConstAll.get_cell_index(column, row)
            self.assertEqual(actual, expected)
