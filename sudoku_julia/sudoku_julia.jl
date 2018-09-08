const SudokuCandidate=UInt32

SUDOKU_BOX_SIZE = 3
SUDOKU_GROUP_SIZE = SUDOKU_BOX_SIZE * SUDOKU_BOX_SIZE
SUDOKU_CANDIDATE_SIZE = SUDOKU_GROUP_SIZE
SUDOKU_CELL_SIZE = SUDOKU_GROUP_SIZE * SUDOKU_GROUP_SIZE
SUDOKU_NO_CANDIDATES = 0
SUDOKU_ALL_CANDIDATES = ((1 << SUDOKU_CANDIDATE_SIZE) - 1)

struct SudokuMap
   cells
end

function string_to_sudokumap(line)
    sudoku_map = SudokuMap(ones(SudokuCandidate, SUDOKU_CELL_SIZE, SUDOKU_CANDIDATE_SIZE))
    for (index, c) in enumerate(line)
        if index > SUDOKU_CELL_SIZE
            break
        end

        num = Int(c) - Int('0')
        if num > 0 && num <= SUDOKU_CANDIDATE_SIZE
            sudoku_map.cells[index, :] .= 0
            sudoku_map.cells[index, num] = 1
        end
    end
    sudoku_map
end

function print_sudokumap(sudoku_map::SudokuMap)
    for cell_index in 1:SUDOKU_CELL_SIZE
        for candidate in 1:SUDOKU_CANDIDATE_SIZE
            if sudoku_map.cells[cell_index, candidate] != 0
                print(candidate)
            end
        end
        print(":")
        if cell_index % SUDOKU_GROUP_SIZE == 0
            println("")
        end
    end
end

function is_solved_group(sudoku_group)
    candidates_count = sum(sudoku_group, dims=(1))
    all(count -> (count == 1), candidates_count)
end

function is_solved_sudoku_rows(sudoku_map::SudokuMap)
    all(row ->
        (index = SUDOKU_GROUP_SIZE * (row - 1);
         is_solved_group(sudoku_map.cells[index + 1: index + SUDOKU_GROUP_SIZE, :])),
        1:SUDOKU_GROUP_SIZE)
end

function is_solved_sudoku_columns(sudoku_map::SudokuMap)
    all(column ->
        (indexes = collect(column: SUDOKU_GROUP_SIZE: column + SUDOKU_GROUP_SIZE * (SUDOKU_GROUP_SIZE - 1));
         is_solved_group(view(sudoku_map.cells, indexes, :))),
        1:SUDOKU_GROUP_SIZE)
end

function is_solved_sudoku_box_group(sudoku_map::SudokuMap, out_box_row, out_box_column)
    group = zeros(SudokuCandidate, SUDOKU_GROUP_SIZE, SUDOKU_CANDIDATE_SIZE)
    for in_box_index in 1:SUDOKU_BOX_SIZE
        base = (out_box_column - 1) * SUDOKU_BOX_SIZE
        base += (out_box_row - 1) * SUDOKU_GROUP_SIZE * SUDOKU_BOX_SIZE
        base += (in_box_index - 1) * SUDOKU_GROUP_SIZE

        sources = collect(1:SUDOKU_BOX_SIZE)
        sources .+= base
        dest = collect(1:SUDOKU_BOX_SIZE)
        dest .+= (in_box_index - 1) * SUDOKU_BOX_SIZE

        group[dest, :] = view(sudoku_map.cells, sources, :)
    end

    is_solved_group(group)
end

function is_solved_sudoku_boxes(sudoku_map::SudokuMap)
    for out_box_row in 1:SUDOKU_BOX_SIZE
        for out_box_column in 1:SUDOKU_BOX_SIZE
            if !is_solved_sudoku_box_group(sudoku_map, out_box_row, out_box_column)
                return false
            end
        end
    end
    true
end

function is_solved_sudokumap(sudoku_map::SudokuMap)
    is_solved_sudoku_rows(sudoku_map) && is_solved_sudoku_columns(sudoku_map) && is_solved_sudoku_boxes(sudoku_map)
end

for line in eachline(stdin)
    sudoku_map = string_to_sudokumap(line)
    print_sudokumap(sudoku_map)
    result = is_solved_sudokumap(sudoku_map)
    if result
        println("Solved")
    else
        println("Not solved")
    end
end
