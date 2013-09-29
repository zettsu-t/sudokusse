#!/bin/ruby
# -*- coding: utf-8 -*-
# 数独のグループを"sudokumap.h"に出力する
# Copyright (C) 2012-2013 Zettsu Tatsuya

# 数独の規則
SUDOKU_ROWS_IN_MAP = 9          # 列の数
SUDOKU_COLUMNS_IN_MAP = 9       # 行の数
SUDOKU_GROUPS_IN_MAP = 9        # 列、行、3*3の数
SUDOKU_CELLS_IN_GROUP = 9       # それぞれの列、行、3*3に含まれる要素数
SUDOKU_BOX_VERTICAL_SIZE = 3    # 3*3のマス(縦)の数
SUDOKU_BOX_HORIZONTAL_SIZE = 3  # 3*3のマス(横)の数
SUDOKU_BOX_IN_ROW = 3           # 3*3の一辺(縦)の個数
SUDOKU_BOX_IN_COLUMN = 3        # 3*3の一辺(横)の個数
SUDOKU_LOOKUP_CELLS = 512       # 早見表のサイズ
SUDOKU_NUMBER_OF_CANDIDATES = 9 # 候補数

# XMMレジスタ番号
SUDOKU_HEAD_XMM_REGISTER = 1           # 最初の行を置くxmmレジスタ番号
SUDOKU_XMM_PER_GENERAL_REGISTER = 4    # xmmレジスタが汎用レジスタ何個分か

# インクルードヘッダ名を表示
print '#include "sudoku.h"', "\n"

# 変数名を表示
print "const SudokuIndex SudokuMap::Group_[Sudoku::SizeOfGroupsPerCell][Sudoku::SizeOfGroupsPerMap][Sudoku::SizeOfCellsPerGroup] = {\n"

# 各列を表示
print "{"
SUDOKU_ROWS_IN_MAP.times do |row|
  print "{"
  SUDOKU_COLUMNS_IN_MAP.times do |column|
    print (row * SUDOKU_COLUMNS_IN_MAP + column)
    print "," if ((column+1) < SUDOKU_COLUMNS_IN_MAP)
  end
  print "}"
  print "," if ((row+1) < SUDOKU_ROWS_IN_MAP)
  print "\n"
end
print "},\n"

# 各行を表示
print "{"
SUDOKU_COLUMNS_IN_MAP.times do |column|
  print "{"
  SUDOKU_ROWS_IN_MAP.times do |row|
    print (row * SUDOKU_COLUMNS_IN_MAP + column)
    print "," if ((row+1) < SUDOKU_ROWS_IN_MAP)
  end
  print "}"
  print "," if ((column+1) < SUDOKU_COLUMNS_IN_MAP)
  print "\n"
end
print "},\n"

# 3*3を表示
print "{"
SUDOKU_BOX_IN_ROW.times do |y|
  startY = y * SUDOKU_BOX_VERTICAL_SIZE
  SUDOKU_BOX_IN_COLUMN.times do |x|
    startX = x * SUDOKU_BOX_HORIZONTAL_SIZE
    print "{"
    SUDOKU_BOX_HORIZONTAL_SIZE.times do |ofsX|
      SUDOKU_BOX_VERTICAL_SIZE.times do |ofsY|
        print (startX+ofsX+(startY+ofsY)*SUDOKU_COLUMNS_IN_MAP)
        print "," if (((ofsX+1) < SUDOKU_BOX_HORIZONTAL_SIZE || ((ofsY+1) < SUDOKU_BOX_VERTICAL_SIZE)))
      end
    end
    print "}"
    print "," if (((x+1) < SUDOKU_BOX_IN_COLUMN) || ((y+1) < SUDOKU_BOX_IN_ROW))
    print "\n"
  end
end
print "}};\n"

# 変数名を表示
print "const SudokuIndex SudokuMap::ReverseGroup_[Sudoku::SizeOfAllCells][Sudoku::SizeOfGroupsPerCell] = {\n"

# マスの縦、横、3*3(番号の対応は上と整合性をとる)を表示
SUDOKU_ROWS_IN_MAP.times do |row|
  SUDOKU_COLUMNS_IN_MAP.times do |column|
    group = (row / SUDOKU_BOX_IN_ROW) * SUDOKU_BOX_HORIZONTAL_SIZE + (column / SUDOKU_BOX_IN_COLUMN)
    print "{", row, ",", column, ",", group, "}"
    print "," if (((column+1) < SUDOKU_COLUMNS_IN_MAP) || ((row+1) < SUDOKU_ROWS_IN_MAP))
  end
  print "\n"
end
print "};\n"

# 変数名を表示
print "const SudokuCellLookUp SudokuCell::CellLookUp_[Sudoku::SizeOfLookUpCell] = {\n"

# 候補のビットマップごとに、唯一の候補かどうかと、候補の数を返す
# 候補1..9がbit0..8に対応する
BitmapCounts = 2 ** SUDOKU_CELLS_IN_GROUP
BitmapCounts.times do |bitmap|
  count = 0
  bitmask = 1
  SUDOKU_CELLS_IN_GROUP.times do |index|
    count = count + 1 if (bitmap & bitmask != 0)
    bitmask *= 2
  end
  isunique = (count == 1) ? "true" : "false"
  ismultiple = (count > 1) ? "true" : "false"
  print "{", isunique, ",", ismultiple, ",", count, "}"
  print  "," if (bitmap + 1) != SUDOKU_LOOKUP_CELLS
  print "\n" if (bitmap % 8) == 7
end
print "};\n"

# 変数名を表示
print "const size_t SudokuSseMap::cellToRegisterIndex_[Sudoku::SizeOfAllCells] = {\n"

# マスからXMMレジスタへの対応を表示
cellToRegiserIndexArray = []
SUDOKU_ROWS_IN_MAP.times do |row|
  base = SUDOKU_XMM_PER_GENERAL_REGISTER * (SUDOKU_HEAD_XMM_REGISTER + row)
  SUDOKU_BOX_IN_COLUMN.times do |column|
    index = base + SUDOKU_BOX_IN_COLUMN - 1 - column
    SUDOKU_BOX_HORIZONTAL_SIZE.times do |column|
      cellToRegiserIndexArray << index
    end
  end
end

print cellToRegiserIndexArray.join(',')
print "};\n"

# 変数名を表示
print "const size_t SudokuSseMap::cellToRightShift_[Sudoku::SizeOfAllCells] = {\n"

# マスからXMMレジスタへの対応を表示
cellToRightShift = []
SUDOKU_ROWS_IN_MAP.times do |row|
  SUDOKU_BOX_IN_COLUMN.times do |column|
    cellToRightShift << [SUDOKU_NUMBER_OF_CANDIDATES * 2, SUDOKU_NUMBER_OF_CANDIDATES, 0]
  end
end

print cellToRightShift.flatten.join(',')
print "};\n"
0
