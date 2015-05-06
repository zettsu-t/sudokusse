#!/usr/bin/ruby
# -*- coding: utf-8 -*-
# 数独のグループを"sudokuConst*.h"に出力する
# Copyright (C) 2012-2015 Zettsu Tatsuya

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

# 生成するヘッダファイル
class SudokuConst
  def initialize(basename)
    @filename = "sudokuConst" + basename + ".h"
  end

  # 文字列を生成してファイルに書き出す
  def write
    open(@filename, "w") do |file|
      file.puts(getCppString)
    end
  end
end

# 定数一覧を出力するヘッダファイル
class SudokuConstAll < SudokuConst
  def initialize
    super("All")
  end

  # ファイルに出力するすべての文字列を生成する
  def getCppString
    getGroupString + "\n" + getReverseGroupString + "\n" + getCellLookUpString
  end

  # SudokuMap::Group_を生成する
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

  # 各列
  def getRowsStrSet
    SUDOKU_ROWS_IN_MAP.times.map do |row|
      "{" + SUDOKU_COLUMNS_IN_MAP.times.map { |column| (row * SUDOKU_COLUMNS_IN_MAP + column) }.join(",") + "}"
    end
  end

  # 各行
  def getColumnsStrSet
    SUDOKU_COLUMNS_IN_MAP.times.map do |column|
      "{" + SUDOKU_ROWS_IN_MAP.times.map { |row| (row * SUDOKU_COLUMNS_IN_MAP + column) }.join(",") + "}"
    end
  end

  # 3*3を表示
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

  # SudokuMap::ReverseGroup_を生成する
  def getReverseGroupString
    str = "const SudokuIndex SudokuMap::ReverseGroup_"
    str += "[Sudoku::SizeOfAllCells][Sudoku::SizeOfGroupsPerCell] {\n"

    # マスの縦、横、3*3(番号の対応は上と整合性をとる)を表示
    str + SUDOKU_ROWS_IN_MAP.times.map do |row|
      SUDOKU_COLUMNS_IN_MAP.times.map do |column|
        group = (row / SUDOKU_BOX_IN_ROW) * SUDOKU_BOX_HORIZONTAL_SIZE + (column / SUDOKU_BOX_IN_COLUMN)
        "{#{row},#{column},#{group}}"
      end.join(",")
    end.join(",\n") + "\n};\n"
  end

  # SudokuCell::CellLookUp_を生成する
  def getCellLookUpString
    str = "const SudokuCellLookUp SudokuCell::CellLookUp_"
    str += "[Sudoku::SizeOfLookUpCell] {\n"

    # 候補のビットマップごとに、唯一の候補かどうかと、候補の数を返す
    # 候補1..9がbit0..8に対応する
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
      # .を後で改行に置き換える
      elementStr += "." if (bitmap % 8) == 7
      elementStr
    end.join(",").gsub(/\.(,?)/,"\\1\n") + "};\n"
    # 最後の要素の後には,はないので、あれば?で一致させる
  end
end

# すべてのインクルードファイルを出力する
[:SudokuConstAll].each do |className|
  Object.const_get(className).new.write
end

0
