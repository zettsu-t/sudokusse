const SUDOKU_BOX_SIZE = 3
const SUDOKU_GROUP_SIZE = SUDOKU_BOX_SIZE * SUDOKU_BOX_SIZE
const SUDOKU_CANDIDATE_SIZE = SUDOKU_GROUP_SIZE
const SUDOKU_CELL_SIZE = SUDOKU_GROUP_SIZE * SUDOKU_GROUP_SIZE
const SUDOKU_NO_CANDIDATES = 0
const SUDOKU_ALL_CANDIDATES = ((1 << SUDOKU_CANDIDATE_SIZE) - 1)
const SudokuCandidate=UInt8
const SudokuIndex=Int64
const SudokuMap=Array{SudokuCandidate, 2}

function overwrite_sudokumap(sudoku_map::SudokuMap, cell_index::SudokuIndex, candidate_index::SudokuIndex)
    sudoku_map[:, cell_index] .= zero(sudoku_map[1,1])
    sudoku_map[candidate_index, cell_index] = one(sudoku_map[1,1])
end

function string_to_sudokumap(line::String)
    sudoku_map = SudokuMap(ones(SudokuCandidate, SUDOKU_CANDIDATE_SIZE, SUDOKU_CELL_SIZE))
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

function to_string_sudokumap(sudoku_map::SudokuMap, one_line::Bool)
    result_str = ""

    for cell_index in 1:SUDOKU_CELL_SIZE
        for candidate in 1:SUDOKU_CANDIDATE_SIZE
            if sudoku_map[candidate, cell_index] != zero(sudoku_map[1,1])
                result_str *= string(candidate)
            end
        end

        if !one_line
            result_str *= ":"
        end

        if cell_index % SUDOKU_GROUP_SIZE == zero(sudoku_map[1,1]) && !one_line
            result_str *= "\n"
        end
    end

    result_str
end

function is_solved_group(sudoku_group)
    all(count -> (count == one(sudoku_group[1,1])), sum(sudoku_group, dims=(1))) && all(count -> (count == one(sudoku_group[1,1])), sum(sudoku_group, dims=(2)))
end

function is_solved_sudoku_rows(sudoku_map::SudokuMap)
    all(row ->
        (index = SUDOKU_GROUP_SIZE * (row - 1);
         is_solved_group(sudoku_map[:, index + 1: index + SUDOKU_GROUP_SIZE])),
        1:SUDOKU_GROUP_SIZE)
end

function is_solved_sudoku_columns(sudoku_map::SudokuMap)
    all(column ->
        (indexes = collect(column: SUDOKU_GROUP_SIZE: column + SUDOKU_GROUP_SIZE * (SUDOKU_GROUP_SIZE - 1));
         is_solved_group(view(sudoku_map, :, indexes))),
        1:SUDOKU_GROUP_SIZE)
end

function is_solved_sudoku_box_group(sudoku_map::SudokuMap, out_box_row::SudokuIndex, out_box_column::SudokuIndex)
    group = zeros(SudokuCandidate, SUDOKU_GROUP_SIZE, SUDOKU_CANDIDATE_SIZE)
    for in_box_index in 1:SUDOKU_BOX_SIZE
        base = (out_box_column - 1) * SUDOKU_BOX_SIZE
        base += (out_box_row - 1) * SUDOKU_GROUP_SIZE * SUDOKU_BOX_SIZE
        base += (in_box_index - 1) * SUDOKU_GROUP_SIZE

        sources = collect(1:SUDOKU_BOX_SIZE)
        sources .+= base
        dest = collect(1:SUDOKU_BOX_SIZE)
        dest .+= (in_box_index - 1) * SUDOKU_BOX_SIZE

        group[:, dest] = view(sudoku_map, :, sources)
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
    all(count -> (count > zero(sudoku_map[1,1])), sum(sudoku_map, dims=(1)))
end

function has_unique_candidate_sudokumap(sudoku_map::SudokuMap, cell_index::SudokuIndex)
    sum(sudoku_map[:, cell_index]) == one(sudoku_map[1,1])
end

function set_unused_candidates_sudokumap(sudoku_map::SudokuMap, cell_index::SudokuIndex, other_index::SudokuIndex, candidates::Array{SudokuCandidate,1})
    if other_index != cell_index && has_unique_candidate_sudokumap(sudoku_map, other_index)
        candidates[:] .|= sudoku_map[:, other_index]
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
            if candidate != 0
                sudoku_map[index, cell_index] = zero(sudoku_map[1,1])
            end
        end
    end
end

function set_unused_candidates_sudokumap(sudoku_map::SudokuMap, cell_index::SudokuIndex, candidates::Array{SudokuCandidate,1})
    if sum(candidates) == SUDOKU_CANDIDATE_SIZE - 1
        index = findfirst(x -> x == 0, candidates)
        overwrite_sudokumap(sudoku_map, cell_index, index)
    end
end

function fill_unused_candidates_sudokumap(sudoku_map::SudokuMap)
    for cell_index in 1:SUDOKU_CELL_SIZE
        if has_unique_candidate_sudokumap(sudoku_map, cell_index)
           continue
        end

        cell_row = div((cell_index - 1), SUDOKU_GROUP_SIZE)
        candidates_row = zeros(SudokuCandidate, SUDOKU_CANDIDATE_SIZE)
        for column_index in 1:SUDOKU_GROUP_SIZE
            in_row_index = cell_row * SUDOKU_GROUP_SIZE + column_index
            if in_row_index != cell_index
                candidates_row[:] .|= sudoku_map[:, in_row_index]
            end
        end
        set_unused_candidates_sudokumap(sudoku_map, cell_index, candidates_row)

        cell_column = (cell_index - 1) % SUDOKU_GROUP_SIZE
        candidates_column = zeros(SudokuCandidate, SUDOKU_CANDIDATE_SIZE)
        for row_index in 1:SUDOKU_GROUP_SIZE
            in_column_index = cell_column + (row_index - 1) * SUDOKU_GROUP_SIZE + 1
            if in_column_index != cell_index
                candidates_column[:] .|= sudoku_map[:, in_column_index]
            end
        end
        set_unused_candidates_sudokumap(sudoku_map, cell_index, candidates_column)

        box_base = div(cell_row, SUDOKU_BOX_SIZE) * SUDOKU_BOX_SIZE * SUDOKU_GROUP_SIZE
        box_base += div(cell_column, SUDOKU_BOX_SIZE) * SUDOKU_BOX_SIZE + 1
        candidates_box = zeros(SudokuCandidate, SUDOKU_CANDIDATE_SIZE)
        for box_index in 1:SUDOKU_GROUP_SIZE
            in_box_index = div(box_index - 1, SUDOKU_BOX_SIZE) * SUDOKU_GROUP_SIZE
            in_box_index += (box_index - 1) % SUDOKU_BOX_SIZE + box_base
            if in_box_index != cell_index
                candidates_box[:] .|= sudoku_map[:, in_box_index]
            end
        end
        set_unused_candidates_sudokumap(sudoku_map, cell_index, candidates_box)
    end
end

function find_backtracking_target(sudoku_map::SudokuMap)
    min_count = SUDOKU_CELL_SIZE
    found = false
    target_index = 0

    candidate_counts = sum(sudoku_map, dims=(1))
    for (index, count) in enumerate(candidate_counts)
        if count <= one(sudoku_map[1,1])
            continue
        end

        if min_count > count
            min_count = count
            found = true
        end
    end

    extract_row = (row -> view(candidate_counts,
                               ((row - 1) * SUDOKU_GROUP_SIZE + 1):(row * SUDOKU_GROUP_SIZE)))
    if found
        row_counts = map(row -> count(n -> (n == min_count), extract_row(row)), 1:SUDOKU_GROUP_SIZE)
        row = argmax(row_counts)
        target_index = findfirst(x -> (x == min_count), extract_row(row)) + (row - 1) * SUDOKU_GROUP_SIZE
    end

    target_index, found
end

function solve_sudokumap(sudoku_map::SudokuMap)
    n_candidates = sum(sudoku_map)
    while true
        find_unique_candidates_sudokumap(sudoku_map)
        fill_unused_candidates_sudokumap(sudoku_map)
        new_n_candidates = sum(sudoku_map)
        if n_candidates == new_n_candidates
            break
        end

        n_candidates = new_n_candidates
        if !is_consistent_sudokumap(sudoku_map)
            return (sudoku_map, false)
        end
    end

    if is_solved_sudokumap(sudoku_map)
        return (sudoku_map, true)
    end

    target_index, found = find_backtracking_target(sudoku_map)
    if !found
        return (sudoku_map, false)
    end

    for (index, candidate) in enumerate(sudoku_map[:, target_index])
        if candidate == zero(sudoku_map[1,1])
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
    num = 0
    one_line = true
    total_result = true

    if one_line
        "Solving in Julia"
    end

    for line in eachline(stdin)
        puzzle = string_to_sudokumap(line)
        solution, result = solve_sudokumap(puzzle)
        result = is_solved_sudokumap(solution)
        total_result &= result

        one_line_result = to_string_sudokumap(solution, one_line)
        print(one_line_result)
        if one_line
            println("")
        else
            if result
                println("Solved\n")
            else
                println("Not solved\n")
            end
        end
        num += 1
    end

    if total_result
        println("All ", num, " cases passed.")
    else
        println("Some of ", num, " cases are not solved correctly.")
    end
end

main()
