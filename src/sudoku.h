#pragma once

//
// sudoku.h - Sudoku solver and utilities
//
// Written by Ryan Antkowiak 
//

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "platform_defs.h"

PUSH_WARN_DISABLE
WARN_DISABLE(4505, "-Wunused-function")

namespace rda
{
    namespace sudoku
    {
        // type for the board cell
        using cell_t = int8_t;

        // type for the board
        using board_t = std::vector<cell_t>;

        // defines a (col, row) point in a 9x9 sudoku board
        struct point_t
        {
            point_t() = default;

            point_t(const cell_t c, const cell_t r)
                : column(c), row(r)
            {
            }

            point_t(const point_t&) = default;

            // the column number (along the horizontal)
            cell_t column = 0;

            // the row number (along vertical)
            cell_t row = 0;

            // convert this point_t into a single index into a board_t
            size_t idx() const
            {
                return idx(column, row);
            }

            // assignment operator
            point_t& operator = (const point_t& rhs)
            {
                row = rhs.row;
                column = rhs.column;
                return *this;
            }

            // addition operator
            point_t operator + (const point_t& rhs) const
            {
                return point_t(rhs.column + column, rhs.row + row);
            }

            // addition assignment operator
            point_t& operator += (const point_t& rhs)
            {
                column += rhs.column;
                row += rhs.row;
                return *this;
            }

            // returns if the location is valid (within range of a sudoku board
            bool valid() const
            {
                return column >= 0 && column <= 9 && row >= 0 && row <= 9;
            }

            // static method to convert a (col, row) pair into an index into a board_t
            static size_t idx(const cell_t c, const cell_t r)
            {
                // there are 9 columns per row, so multiply the row number by 9, and add the column
                return 9 * static_cast<size_t>(r) + static_cast<size_t>(c);
            }

            // static method to convert from a board_t index to a point_t (col, row) pair
            static point_t r_idx(const cell_t index)
            {
                // the column number is the remainder (modulo) of 9.
                // the row number is the floor of the index divided by 9.
                return point_t{static_cast<cell_t>(index % 9), static_cast<cell_t>(index / 9)};
            }

            // Given the point p, returns a point that corresponds to the top-left most point in the
            // 3x3 "mini square box" on the sudoku board. the floor of the integer division handles this properly.
            // This should return one of: (0,0), (3,0), (6,0), (0,3), (3,3), (6,3), (0,6), (3,6), (6,6)
            static point_t get_box_start(const point_t& p)
            {
                point_t start;

                start.column = (p.column / 3) * 3;
                start.row = (p.row / 3) * 3;

                return start;
            }
        };

        // iterate through all rows in p.column, and filter all values out of the possible candidates.
        static void remove_candidates_sudoku_column(std::vector<cell_t> & candidates, const board_t & board, const cell_t column)
        {
            for (cell_t row = 0; row < 9; ++row)
                candidates.erase(std::remove(candidates.begin(), candidates.end(), board[point_t::idx(column, row)]), candidates.end());
        }

        // iterate through all columns in row, and filter all values out of the possible candidates.
        static void remove_candidates_sudoku_row(std::vector<cell_t>& candidates, const board_t& board, const cell_t row)
        {
            for (cell_t col = 0; col < 9; ++col)
                candidates.erase(std::remove(candidates.begin(), candidates.end(), board[point_t::idx(col, row)]), candidates.end());
        }

        // iterate through the points in the 3x3 mini-box, and filter values out of possible candidates.
        static void remove_candidates_sudoku_3x3_box(std::vector<cell_t>& candidates, const board_t& board, const point_t& p)
        {
            // find the top-left most corner of this points 3x3 "mini box"
            const point_t box_start = point_t::get_box_start(p);

            // iterate through the points in the 3x3 mini-box, and filter values out of possible candidates.
            for (cell_t col = box_start.column; col < box_start.column + 3; ++col)
                for (cell_t row = box_start.row; row < box_start.row + 3; ++row)
                    candidates.erase(std::remove(candidates.begin(), candidates.end(), board[point_t::idx(col, row)]), candidates.end());
        }

        // iterate through possible "chess king" moves, and filter out possible candidates.
        static void remove_candidates_chess_king(std::vector<cell_t>& candidates, const board_t& board, const point_t& p)
        {
            static const std::vector<point_t> king_move_deltas = { {-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1} };

            for (auto& delta : king_move_deltas)
            {
                const point_t board_position = p + delta;

                if (board_position.valid())
                    candidates.erase(std::remove(candidates.begin(), candidates.end(), board[board_position.idx()]));
            }
        }

        // iterate through possible "chess knight" moves, and filter out possible candidates.
        static void remove_candidates_chess_knight(std::vector<cell_t>& candidates, const board_t& board, const point_t& p)
        {
            static const std::vector<point_t> knight_move_deltas = { {-2, -1}, {-2, 1}, {2, -1}, {2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2} };

            for (auto& delta : knight_move_deltas)
            {
                const point_t board_position = p + delta;

                if (board_position.valid())
                    candidates.erase(std::remove(candidates.begin(), candidates.end(), board[board_position.idx()]));
            }
        }

        // for a given point p on a sudoku board, return a vector of possible candidate values
        static std::vector<cell_t> find_candidates(const board_t &board, const point_t &p)
        {
            // vector to hold the possible candidates. initially set to all possible values
            std::vector<cell_t> candidates = {1, 2, 3, 4, 5, 6, 7, 8, 9};

            remove_candidates_sudoku_column(candidates, board, p.column);

            remove_candidates_sudoku_row(candidates, board, p.row);

            remove_candidates_sudoku_3x3_box(candidates, board, p);

            return candidates;
        }

        // returns true if the board has been completed
        static bool is_completed(const board_t &board)
        {
            // first check if it is a valid board
            if (board.size() != 81)
                return false;

            // the board is complete if none of the cells are at the '0' initial vlaue
            return std::none_of(board.cbegin(), board.cend(), [](auto &cell) { return cell == 0; });
        }

        // recursive method to solve a board
        static board_t solve_recursive(const board_t &board, cell_t start_index = 0)
        {
            // completed boards should just be returned
            if (is_completed(board))
                return board;

            // iterate over the cells in the board
            for (cell_t i = start_index; i < 81; ++i)
            {
                // if this cell is already solved, continue on to the next cell
                if (board[i] != 0)
                    continue;

                // build a list of possible candidate values for this cell on the board
                auto possibilties = find_candidates(board, point_t::r_idx(i));

                // iterate over list of possible candidate values
                for (auto pos : possibilties)
                {
                    // copy the board to a temporary
                    board_t b(board);

                    // place the candidate value into the board
                    b[i] = pos;

                    // attempt to solve the board. Can start at the next index.
                    auto attempt = solve_recursive(b, i + 1);

                    // if the board has been completed, return it
                    if (is_completed(attempt))
                        return attempt;
                }

                // none of the candidates were viable. return empty board
                return board_t();
            }

            // no solution. return empty board
            return board_t();
        }

        // print a sudoku board
        static void print(const board_t &board)
        {
            for (size_t i = 0; i < board.size(); ++i)
            {
                std::cout << std::to_string(board[i]) << " ";

                // new line after every 9 values
                if ((i + 1) % 9 == 0)
                    std::cout << std::endl;
            }
        }

    } // namespace sudoku

} // namespace rda

POP_WARN_DISABLE
