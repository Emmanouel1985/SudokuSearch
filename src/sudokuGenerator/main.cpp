#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <functional>
#include <optional>

#include <random>

#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>


// This file will be generated automatically when cur_you run the CMake
// configuration step. It creates a namespace called `SudokuSearch`. You can modify
// the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <ranges>
#include <print>
#include <format>
#include <mutex>

std::random_device rd;
std::mutex m;

struct Sudoku
{
    std::array<std::array<char, 9>, 9> m;
    char _ = '\x0';
    auto operator<=>(const Sudoku& other) const = default;
    bool operator==(const Sudoku& other) const = default;

    static Sudoku getRandomSudoku()
    {
        using namespace std::literals;
        std::array<std::array<std::array<int, 9>, 9>, 9> m;
        for (auto& x : m)
        {
            for (auto& y : x)
            {
                y.fill(1);
            }
        }
        thread_local std::mt19937 g([&] { std::lock_guard lock{ ::m }; return rd(); }());
        Sudoku sudoku;
        auto getRandomSudoku = [&](auto ii, auto jj, auto&& getRandomSudokuRec)
            {
                if (jj >= 9)
                {
                    return true;
                }
                if (std::ranges::any_of(std::views::join(m), [](const auto& x) 
                    {
                        return std::ranges::all_of(x, [](auto v) { return v <= 0; });
                    }))
                {
                    return false;
                }
                std::array<char, 9> choices{'1','2','3','4','5','6','7','8','9'};
                for (auto [t, c] : std::views::zip(m[ii][jj], choices))
                {
                    if (t <= 0)
                    {
                        c = '\x0';
                    }
                }
                std::shuffle(choices.begin(), choices.end(), g);
                for (auto c : choices)
                {
                    if (c < '1' || c > '9')
                    {
                        continue;
                    }
                    auto ic = static_cast<std::size_t>(c - '1');
                    sudoku.m[ii][jj] = c;
                    for (auto kk : std::views::iota(0UL, 9UL))
                    {
                        if (kk != jj)
                        {
                            --m[ii][kk][ic];
                        }
                        if (kk != ii)
                        {
                            --m[kk][jj][ic];
                        }
                        if ((ii / 3) * 3 + kk / 3 != ii && (jj / 3) * 3 + kk % 3 != jj)
                        {
                            --m[(ii / 3) * 3 + kk / 3][(jj / 3) * 3 + kk % 3][ic];
                        }
                    }
                    if (getRandomSudokuRec((ii + 1) % 9, jj + (ii / 8), getRandomSudokuRec))
                    {
                        return true;
                    }
                    for (auto kk : std::views::iota(0UL, 9UL))
                    {
                        if (kk != jj)
                        {
                            ++m[ii][kk][ic];
                        }
                        if (kk != ii)
                        {
                            ++m[kk][jj][ic];
                        }
                        if ((ii / 3) * 3 + kk / 3 != ii && (jj / 3) * 3 + kk % 3 != jj)
                        {
                            ++m[(ii / 3) * 3 + kk / 3][(jj / 3) * 3 + kk % 3][ic];
                        }
                    }
                }
                return false;
            };
        getRandomSudoku(0UL, 0UL, getRandomSudoku);
        return sudoku;
    }
};

template<std::size_t I1, std::size_t I2>
void swapRows(Sudoku& sudoku)
{
    std::swap(sudoku.m[I1], sudoku.m[I2]);
}

template<std::size_t I1, std::size_t I2>
void swapRowBlocks(Sudoku& sudoku)
{
    for (auto ii = 0UL; ii < 3UL; ++ii)
    {
        std::swap(sudoku.m[3 * I1 + ii], sudoku.m[3 * I2 + ii]);
    }
}

void transpose(Sudoku& sudoku)
{
    for (auto ii = 0UL; ii < 9UL; ++ii)
    {
        for (auto jj = ii + 1UL; jj < 9UL; ++jj)
        {
            std::swap(sudoku.m[ii][jj], sudoku.m[jj][ii]);
        }
    }
}

void relabel(Sudoku& sudoku, const std::array<char, 9>& labels)
{
    for (auto& r : sudoku.m)
    {
        std::ranges::for_each(r, [&](char& c){ c = labels[static_cast<std::size_t>(c - '1')]; });
    }
}
void minLabeling(Sudoku& sudoku)
{
    std::array<char, 9> newLabels;
    for (auto ii = 0UL; ii < 9UL; ++ii)
    {
        newLabels[static_cast<std::size_t>(sudoku.m[0][ii] - '1')] = static_cast<char>('1' + ii);
    }
    relabel(sudoku, newLabels);
}

using transform_func_t = void (*)(Sudoku&);
template<std::size_t II>
std::array<transform_func_t, 6> funcs = {
    [](Sudoku&) {},
    swapRows<3*II+0, 3 * II + 1>,
    swapRows<3 * II + 0, 3 * II + 2>,
    swapRows<3 * II + 1, 3 * II + 2>,
    [](Sudoku& sudoku) { swapRows<3 * II + 0, 3 * II + 1>(sudoku); swapRows<3 * II + 0, 3 * II + 2>(sudoku);  },
    [](Sudoku& sudoku) { swapRows<3 * II + 0, 3 * II + 1>(sudoku); swapRows<3 * II + 1, 3 * II + 2>(sudoku);  }
};
std::array<transform_func_t, 6> blockFuncs = {
    [](Sudoku&) {},
    swapRowBlocks<0, 1>,
    swapRowBlocks<0, 2>,
    swapRowBlocks<1, 2>,
    [](Sudoku& sudoku) { swapRowBlocks<0, 1>(sudoku); swapRowBlocks<0, 2>(sudoku);  },
    [](Sudoku& sudoku) { swapRowBlocks<0, 1>(sudoku); swapRowBlocks<1, 2>(sudoku);  }
};


void normalize(Sudoku& sudoku)
{
    auto normal = sudoku;
    for (auto [f1, f2, f3, g1, g2, g3, h1, h2] : std::views::cartesian_product(funcs<0>, funcs<1>, funcs<1>, funcs<0>, funcs<1>, funcs<2>, blockFuncs, blockFuncs))
    {
        for (bool doTrans : {false, true})
        {
            auto testSudoku = sudoku;
            f1(testSudoku);
            f2(testSudoku);
            f3(testSudoku);
            h1(testSudoku);
            transpose(testSudoku);
            g1(testSudoku);
            g2(testSudoku);
            g3(testSudoku);
            h2(testSudoku);
            if (doTrans)
            {
                transpose(testSudoku);
            }
            minLabeling(testSudoku);
            if (testSudoku < normal)
            {
                normal = testSudoku;
            }
        }
    }
    sudoku = normal;
}

void generateSudokus(std::size_t nthreads, std::uint32_t nsudokus, bool bSkipNormalize)
{
    std::atomic_int64_t count{ nsudokus };
    auto lambda = [&]() {
        while (true)
        {
            auto curCount = count.load();
            while (count.compare_exchange_strong(curCount, curCount - 1)) 
            {
                if (curCount <= 0)
                {
                    return;
                }
            }
            auto sudoku = Sudoku::getRandomSudoku();
            if (!bSkipNormalize)
            {
                normalize(sudoku);
            }
            std::println("{} {}", &sudoku.m[0][0], curCount);
        }
        };
    auto threads = std::views::repeat(lambda, nthreads)
        | std::views::transform([](auto func) { return std::jthread{ func }; })
        | std::ranges::to<std::vector>();
    threads.clear();
}

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, const char **argv)
{
  try {
    CLI::App app{ std::format("{} version {}", SudokuSearch::cmake::project_name, SudokuSearch::cmake::project_version) };

    std::size_t nthreads{1UL};
    app.add_option("-t,--threads", nthreads, "Number of threads to use");
    
    std::uint32_t nsudokus{1UL};
    app.add_option("-s,--sudokus", nsudokus, "Number of sudokus to print");

    bool bSkipNormalize = false;
    app.add_flag("-n,--dont-normalize", bSkipNormalize, "Don't normalize sudokus");

    bool show_version = false;
    app.add_flag("--version", show_version, "Show version information");

    CLI11_PARSE(app, argc, argv);

    if (show_version) {
      std::println("{}", SudokuSearch::cmake::project_version);
      return EXIT_SUCCESS;
    }

    generateSudokus(nthreads, nsudokus, bSkipNormalize);
  } catch (const std::exception &e) {
    spdlog::error("Unhandled exception in main: {}", e.what());
  }
}
