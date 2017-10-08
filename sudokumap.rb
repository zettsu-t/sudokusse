#!/usr/bin/ruby
# -*- coding: utf-8 -*-
# This script writes groups of cells in a Sudoku puzzle to "sudokuConst*.h".
# Copyright (C) 2012-2017 Zettsu Tatsuya

# Constants for Sudoku
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

SUDOKU_HEAD_XMM_REGISTER = 1         # The number of an XMM register which holds the top row
SUDOKU_XMM_PER_GENERAL_REGISTER = 4  # General purpose registers per XMM register

# Generated code
class SudokuConst
  def initialize(basename)
    @filename = "sudokuConst" + basename + ".h"
  end

  # Writes generated code to a file
  def write
    open(@filename, "w") do |file|
      file.puts(getCppString)
    end
  end
end

# Generated header file
class SudokuConstAll < SudokuConst
  def initialize
    super("All")
  end

  # Generates all code in a generated file
  def getCppString
    getGroupString + "\n" + getReverseGroupString + "\n" + getCellLookUpString
  end

  # Generates SudokuMap::Group_
  def getGroupString
    str = "const SudokuIndex SudokuMap::Group_"
    str += "[Sudoku::SizeOfGroupsPerCell][Sudoku::SizeOfGroupsPerMap][Sudoku::SizeOfCellsPerGroup] {\n"
    str += getLinesString(getRowsStrSet, ",")
    str += getLinesString(getColumnsStrSet, ",")
    str += getLinesString(getBoxesStrSet, "")
    str += "};\n"
    str
  end

  def getLinesString(lineStrSet, delim)
    "{" + lineStrSet.join(",\n") + "}#{delim}\n"
  end

  def getRowsStrSet
    SUDOKU_ROWS_IN_MAP.times.map do |row|
      "{" + SUDOKU_COLUMNS_IN_MAP.times.map { |column| (row * SUDOKU_COLUMNS_IN_MAP + column) }.join(",") + "}"
    end
  end

  def getColumnsStrSet
    SUDOKU_COLUMNS_IN_MAP.times.map do |column|
      "{" + SUDOKU_ROWS_IN_MAP.times.map { |row| (row * SUDOKU_COLUMNS_IN_MAP + column) }.join(",") + "}"
    end
  end

  # 3*3 cell box (square)
  def getBoxesStrSet
    SUDOKU_BOX_IN_ROW.times.map do |y|
      startY = y * SUDOKU_BOX_VERTICAL_SIZE
      SUDOKU_BOX_IN_COLUMN.times.map do |x|

        startX = x * SUDOKU_BOX_HORIZONTAL_SIZE
        "{" + SUDOKU_BOX_HORIZONTAL_SIZE.times.map do |ofsX|
          SUDOKU_BOX_VERTICAL_SIZE.times.map do |ofsY|
            (startX+ofsX+(startY+ofsY)*SUDOKU_COLUMNS_IN_MAP)
          end
        end.join(",") + "}"

      end
    end
  end

  # Generates SudokuMap::ReverseGroup_
  def getReverseGroupString
    str = "const SudokuIndex SudokuMap::ReverseGroup_"
    str += "[Sudoku::SizeOfAllCells][Sudoku::SizeOfGroupsPerCell] {\n"

    # Joins columns, rows, and boxes in each cell
    str + SUDOKU_COLUMNS_IN_MAP.times.map do |column|
      SUDOKU_ROWS_IN_MAP.times.map do |row|
        group = (column / SUDOKU_BOX_IN_COLUMN) * SUDOKU_BOX_HORIZONTAL_SIZE + (row / SUDOKU_BOX_IN_ROW)
        "{#{column},#{row},#{group}}"
      end.join(",")
    end.join(",\n") + "\n};\n"
  end

  # Generates SudokuCell::CellLookUp_
  def getCellLookUpString
    str = "const SudokuCellLookUp SudokuCell::CellLookUp_"
    str += "[Sudoku::SizeOfLookUpCell] {\n"

    # Returns the following items for each bitmap in which bits[0..8] corresponds to candidate 1..9
    # - whether it has a unique candidate
    # - number of its candidate
    str + (2 ** SUDOKU_CELLS_IN_GROUP).times.map do |bitmap|
      count = 0
      bitmask = 1
      SUDOKU_CELLS_IN_GROUP.times do |index|
        count = count + 1 if (bitmap & bitmask != 0)
        bitmask *= 2
      end

      isunique = (count == 1) ? "true" : "false"
      ismultiple = (count > 1) ? "true" : "false"
      elementStr = "{#{isunique},#{ismultiple},#{count}}"
      # Replaces . to \n later
      elementStr += "." if (bitmap % 8) == 7
      elementStr
    end.join(",").gsub(/\.(,?)/,"\\1\n") + "};\n"
    # The last element is not followed by ',' and '?' in the regex is required.
  end
end

# Writes all header files
[:SudokuConstAll].each do |className|
  Object.const_get(className).new.write
end

0
