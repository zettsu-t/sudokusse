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

function overwrite_sudokumap(sudoku_map::SudokuMap, cell_index, candidate)
    sudoku_map.cells[cell_index, :] .= 0
    sudoku_map.cells[cell_index, candidate] = 1
end

function string_to_sudokumap(line)
    sudoku_map = SudokuMap(ones(SudokuCandidate, SUDOKU_CELL_SIZE, SUDOKU_CANDIDATE_SIZE))
    for (index, c) in enumerate(line)
        if index > SUDOKU_CELL_SIZE
            break
        end

        num = Int(c) - Int('0')
        if num > 0 && num <= SUDOKU_CANDIDATE_SIZE
            overwrite_sudokumap(sudoku_map, index, num)
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
    all(count -> (count == 1), sum(sudoku_group, dims=(1))) && all(count -> (count == 1), sum(sudoku_group, dims=(2)))
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

function is_consistent_sudokumap(sudoku_map::SudokuMap)
    all(candidate -> (candidate > 0), sudoku_map.cells)
end

function has_unique_candidate_sudokumap(sudoku_map::SudokuMap, cell_index)
    sum(sudoku_map.cells[cell_index, :]) == 1
end

function set_unused_candidates_sudokumap(sudoku_map::SudokuMap, cell_index, other_index, candidates)
    if other_index != cell_index && has_unique_candidate_sudokumap(sudoku_map, other_index)
        candidates[:] .|= sudoku_map.cells[other_index, :]
    end
end

function find_unique_candidates_sudokumap(sudoku_map::SudokuMap)
    for cell_index in 1:SUDOKU_CELL_SIZE
        if has_unique_candidate_sudokumap(sudoku_map, cell_index)
           continue
        end

        row = div(cell_index - 1, SUDOKU_GROUP_SIZE)
        column = (cell_index - 1) % SUDOKU_GROUP_SIZE
        box_base = div(row, SUDOKU_BOX_SIZE) * SUDOKU_BOX_SIZE * SUDOKU_GROUP_SIZE
        box_base += div(column, SUDOKU_BOX_SIZE) * SUDOKU_BOX_SIZE + 1

        candidates = zeros(SudokuCandidate, SUDOKU_CANDIDATE_SIZE)
        for index in 1:SUDOKU_GROUP_SIZE
            in_row_index = row * SUDOKU_GROUP_SIZE + index
            in_column_index = column + (index - 1) * SUDOKU_GROUP_SIZE + 1
            in_box_index = div(index - 1, SUDOKU_BOX_SIZE) * SUDOKU_GROUP_SIZE
            in_box_index += (index - 1) % SUDOKU_BOX_SIZE + box_base
            for other_index in [in_row_index, in_column_index, in_box_index]
                set_unused_candidates_sudokumap(sudoku_map, cell_index, other_index, candidates)
            end
        end

        for (index, candidate) in enumerate(candidates)
            if candidate > 0
                sudoku_map.cells[cell_index, index] = 0
            end
        end
    end
end

function set_unused_candidates_sudokumap(sudoku_map::SudokuMap, cell_index, candidates)
    if sum(candidates) == SUDOKU_CANDIDATE_SIZE - 1
        index = findfirst(x -> x == 0, candidates)
        overwrite_sudokumap(sudoku_map, cell_index, index)
    end
end

## ToDo : merge with is_solved_sudoku_rows()
function fill_unused_candidates_sudokumap(sudoku_map::SudokuMap)
    for cell_index in 1:SUDOKU_CELL_SIZE
        if has_unique_candidate_sudokumap(sudoku_map, cell_index)
           continue
        end

        cell_row = div((cell_index - 1), SUDOKU_GROUP_SIZE)
        candidates = zeros(SudokuCandidate, SUDOKU_CANDIDATE_SIZE)
        for column_index in 1:SUDOKU_GROUP_SIZE
            in_row_index = cell_row * SUDOKU_GROUP_SIZE + column_index
            if in_row_index != cell_index
                candidates[:] .|= sudoku_map.cells[in_row_index, :]
            end
        end
        set_unused_candidates_sudokumap(sudoku_map, cell_index, candidates)

        cell_column = (cell_index - 1) % SUDOKU_GROUP_SIZE
        candidates = zeros(SudokuCandidate, SUDOKU_CANDIDATE_SIZE)
        for row_index in 1:SUDOKU_GROUP_SIZE
            in_column_index = cell_column + (row_index - 1) * SUDOKU_GROUP_SIZE + 1
            if in_column_index != cell_index
                candidates[:] .|= sudoku_map.cells[in_column_index, :]
            end
        end
        set_unused_candidates_sudokumap(sudoku_map, cell_index, candidates)

        box_base = div(cell_row, SUDOKU_BOX_SIZE) * SUDOKU_BOX_SIZE * SUDOKU_GROUP_SIZE
        box_base += div(cell_column, SUDOKU_BOX_SIZE) * SUDOKU_BOX_SIZE + 1
        candidates = zeros(SudokuCandidate, SUDOKU_CANDIDATE_SIZE)
        for box_index in 1:SUDOKU_GROUP_SIZE
            in_box_index = div(box_index - 1, SUDOKU_BOX_SIZE) * SUDOKU_GROUP_SIZE
            in_box_index += (box_index - 1) % SUDOKU_BOX_SIZE + box_base
            if in_box_index != cell_index
                candidates[:] .|= sudoku_map.cells[in_box_index, :]
            end
        end
        set_unused_candidates_sudokumap(sudoku_map, cell_index, candidates)
    end
end

function find_backtracking_target(sudoku_map::SudokuMap)
    min_count = SUDOKU_CELL_SIZE
    found = false
    target_index = 1

    candidate_counts = sum(sudoku_map.cells, dims=(2))
    for (index, count) in enumerate(candidate_counts)
        if count <= 1
            continue
        end

        if count == 2
            return index, true
        end

        if min_count > count
            target_index = index
            min_count = count
            found = true
        end
    end

    target_index, found
end

function solve_sudokumap(sudoku_map::SudokuMap)
    n_candidates = sum(sudoku_map.cells)
    while true
        if is_solved_sudokumap(sudoku_map)
            return (sudoku_map, true)
        end

        if is_consistent_sudokumap(sudoku_map)
            return (sudoku_map, false)
        end

        find_unique_candidates_sudokumap(sudoku_map)
        fill_unused_candidates_sudokumap(sudoku_map)
        new_n_candidates = sum(sudoku_map.cells)
        if n_candidates == new_n_candidates
            break
        end

        n_candidates = new_n_candidates
    end

    target_index, found = find_backtracking_target(sudoku_map)
    if !found
        return (sudoku_map, false)
    end

    for (index, candidate) in enumerate(sudoku_map.cells[target_index, :])
        if candidate == 0
            continue
        end

        new_map = deepcopy(sudoku_map)
        overwrite_sudokumap(new_map, target_index, index)
        result_map, result = solve_sudokumap(new_map)
        if result
            return (result_map, result)
        end
    end

    return (sudoku_map, false)
end

function main()
    num = 1
    for line in eachline(stdin)
        puzzle = string_to_sudokumap(line)
        solution, result = solve_sudokumap(puzzle)
        result = is_solved_sudokumap(solution)

        print_sudokumap(solution)
        if result
            println(num, " Solved")
        else
            println(num, " Not solved")
        end
        num += 1
    end
end

main()
