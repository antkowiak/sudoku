#pragma once

//
// sudoku.h - Sudoku solver and utilities
//
// Written by Ryan Antkowiak (antkowiak@gmail.com)
//

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace rda
{
    namespace sudoku
    {
        // type for the board cell
        using cell_t = uint8_t;

        // type for the board
        using board_t = std::vector<cell_t>;

        // defines a (col, row) point in a 9x9 sudoku board
        struct point_t
        {
            // the column number (along the horizontal)
            cell_t column = 0;

            // the row number (along vertical)
            cell_t row = 0;

            // conver this point_t into a single index into a board_t
            cell_t idx() const
            {
                return 9 * row + column;
            }

            // static method to convert a (col, row) pair into an index into a board_t
            static cell_t idx(const cell_t c, const cell_t r)
            {
                // there are 9 columns per row, so multiply the row number by 9, and add the column
                return 9 * r + c;
            }

            // static method to convert from a board_t index to a point_t (col, row) pair
            static point_t r_idx(const cell_t index)
            {
                // the column number is the remainder (modulo) of 9.
                // the row number is the floor of the index divided by 9.
                return point_t{static_cast<cell_t>(index % 9), static_cast<cell_t>(index / 9)};
            }
        };

        // Given the point p, returns a point that corresponds to the top-left most point in the
        // 3x3 "mini square box" on the sudoku board. the floor of the integer division handles this properly.
        // This should return one of: (0,0), (3,0), (6,0), (0,3), (3,3), (6,3), (0,6), (3,6), (6,6)
        static point_t get_box_start(const point_t &p)
        {
            point_t start;

            start.column = (p.column / 3) * 3;
            start.row = (p.row / 3) * 3;

            return start;
        }

        // for a given point p on a sudoku board, return a vector of possible (candidate) values
        std::vector<cell_t> valid_possibilities(const board_t &board, const point_t &p)
        {
            // vector to hold the possible candidates. initially set to all possible values
            std::vector<cell_t> possible = {1, 2, 3, 4, 5, 6, 7, 8, 9};

            // iterate through all rows in p.column, and filter all values out of the "possible" candidates.
            for (cell_t row = 0; row < 9; ++row)
                possible.erase(std::remove(possible.begin(), possible.end(), board[point_t::idx(p.column, row)]), possible.end());

            // iterate through all columns in p.row, and filter all values out of the "possible" candidates.
            for (cell_t col = 0; col < 9; ++col)
                possible.erase(std::remove(possible.begin(), possible.end(), board[point_t::idx(col, p.row)]), possible.end());

            // find the top-left most corner of this points 3x3 "mini box"
            const point_t box_start = get_box_start(p);

            // iterate through the points in the 3x3 mini-box, and filter values out of "possible" candidates.
            for (cell_t col = box_start.column; col < box_start.column + 3; ++col)
                for (cell_t row = box_start.row; row < box_start.row + 3; ++row)
                    possible.erase(std::remove(possible.begin(), possible.end(), board[point_t::idx(col, row)]), possible.end());

            // return vector of possibilities
            return possible;
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
                auto possibilties = valid_possibilities(board, point_t::r_idx(i));

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
            for (cell_t i = 0; i < board.size(); ++i)
            {
                std::cout << std::to_string(board[i]) << " ";

                // new line after every 9 values
                if ((i + 1) % 9 == 0)
                    std::cout << std::endl;
            }
        }

    } // namespace sudoku

} // namespace rda
