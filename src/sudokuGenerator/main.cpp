#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <format>
#include <mutex>
#include <print>
#include <random>
#include <ranges>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>


// This file will be generated automatically when cur_you run the CMake
// configuration step. It creates a namespace called `SudokuSearch`. You can modify
// the source template at `configured_files/config.hpp.in`.

#include <internal_use_only/config.hpp>

constexpr auto N = 9UL;// NOLINT

class Sudoku
{
public:
  // NOLINTNEXTLINE(readability-function-cognitive-complexity,cppcoreguidelines-pro-type-member-init,hicpp-member-init)
  Sudoku()
  {
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
    using namespace std::literals;
    static std::random_device rndDev;
    static std::mutex mux;
    std::array<std::array<std::array<int, N>, N>, N> digits;// NOLINT
    for (auto &x : digits) {
      for (auto &y : x) { y.fill(1); }
    }
    thread_local std::mt19937 gen([&] {
      const std::scoped_lock lock{ mux };
      return rndDev();
    }());
    Sudoku sudoku;
    auto modifyDigit = [&](auto irow, auto jcol, auto chr, auto incdec) {
      auto cIdx = static_cast<std::size_t>(chr - '1');
      for (auto offset : std::views::iota(0UL, N)) {
        if (offset != jcol) { digits[irow][offset][cIdx] += incdec; }
        if (offset != irow) { digits[offset][jcol][cIdx] += incdec; }
        if (((irow / 3) * 3) + (offset / 3) != irow && ((jcol / 3) * 3) + (offset % 3) != jcol) {
          digits[((irow / 3) * 3) + (offset / 3)][((jcol / 3) * 3) + (offset % 3)][cIdx] += incdec;
        }
      }
    };
    auto getRandomSudoku = [&](auto irow, auto jcol, auto &&getRandomSudokuRec) {
      if (jcol >= N) { return true; }
      if (std::ranges::any_of(std::views::join(digits),
            [](const auto &x) { return std::ranges::all_of(x, [](auto val) { return val <= 0; }); })) {
        return false;
      }
      std::array<char, N> choices{ '1', '2', '3', '4', '5', '6', '7', '8', '9' };
      for (auto [val, chr] : std::views::zip(digits[irow][jcol], choices)) {
        if (val <= 0) { chr = '\x0'; }
      }
      std::shuffle(choices.begin(), choices.end(), gen);
      for (auto chr : choices) {
        if (chr < '1' || chr > '9') { continue; }
        sudoku.m[irow][jcol] = chr;
        modifyDigit(irow, jcol, chr, -1);
        if (getRandomSudokuRec((irow + 1) % N, jcol + ((irow + 1) / N), getRandomSudokuRec)) { return true; }
        modifyDigit(irow, jcol, chr, +1);
      }
      return false;
    };
    getRandomSudoku(0UL, 0UL, getRandomSudoku);
    // NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
  }

  auto operator<=>(const Sudoku &other) const = default;
  bool operator==(const Sudoku &other) const = default;

  explicit operator std::string_view() const { return std::string_view{ m[0].data(), N * N }; }


  template<std::size_t I1, std::size_t I2> friend void swapRows(Sudoku &sudoku);
  template<std::size_t I1, std::size_t I2> friend void swapRows(Sudoku &sudoku);
  template<std::size_t I1, std::size_t I2> friend void swapRowBlocks(Sudoku &sudoku);
  friend void transpose(Sudoku &sudoku);
  friend void relabel(Sudoku &sudoku, const std::array<char, N> &labels);
  friend void minLabeling(Sudoku &sudoku);

private:
  std::array<std::array<char, N>, N> m;
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
template<std::size_t I1, std::size_t I2> void swapRows(Sudoku &sudoku) { std::swap(sudoku.m[I1], sudoku.m[I2]); }

template<std::size_t I1, std::size_t I2> void swapRowBlocks(Sudoku &sudoku)
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
  for (auto ii = 0UL; ii < 3UL; ++ii) { std::swap(sudoku.m[(3 * I1) + ii], sudoku.m[(3 * I2) + ii]); }
}

void transpose(Sudoku &sudoku)
{
  for (auto ii = 0UL; ii < N; ++ii) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    for (auto jj = ii + 1UL; jj < N; ++jj) { std::swap(sudoku.m[ii][jj], sudoku.m[jj][ii]); }
  }
}

void relabel(Sudoku &sudoku, const std::array<char, N> &labels)
{
  for (auto &row : sudoku.m) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    std::ranges::for_each(row, [&](char &chr) { chr = labels[static_cast<std::size_t>(chr - '1')]; });
  }
}

void minLabeling(Sudoku &sudoku)
{
  std::array<char, N> newLabels;// NOLINT
  for (auto ii = 0UL; ii < N; ++ii) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    newLabels[static_cast<std::size_t>(sudoku.m[0][ii] - '1')] = static_cast<char>('1' + ii);
  }
  relabel(sudoku, newLabels);
}

constexpr auto N_TRANFORM_FUNCS = 6;
using transform_func_t = void (*)(Sudoku &);
template<std::size_t II>
// NOLINTNEXTLINE(cert-err58-cpp)
const std::array<transform_func_t, N_TRANFORM_FUNCS> funcs = { [](Sudoku &) {},
  swapRows<(3 * II) + 0, (3 * II) + 1>,
  swapRows<(3 * II) + 0, (3 * II) + 2>,
  swapRows<(3 * II) + 1, (3 * II) + 2>,
  [](Sudoku &sudoku) {
    swapRows<(3 * II) + 0, (3 * II) + 1>(sudoku);
    swapRows<(3 * II) + 0, (3 * II) + 2>(sudoku);
  },
  [](Sudoku &sudoku) {
    swapRows<(3 * II) + 0, (3 * II) + 1>(sudoku);
    swapRows<(3 * II) + 1, (3 * II) + 2>(sudoku);
  } };
// NOLINTNEXTLINE(cert-err58-cpp)
const std::array<transform_func_t, N_TRANFORM_FUNCS> blockFuncs = { [](Sudoku &) {},
  swapRowBlocks<0, 1>,
  swapRowBlocks<0, 2>,
  swapRowBlocks<1, 2>,
  [](Sudoku &sudoku) {
    swapRowBlocks<0, 1>(sudoku);
    swapRowBlocks<0, 2>(sudoku);
  },
  [](Sudoku &sudoku) {
    swapRowBlocks<0, 1>(sudoku);
    swapRowBlocks<1, 2>(sudoku);
  } };


void normalize(Sudoku &sudoku)
{
  auto normal = sudoku;
  for (auto [foo1, foo2, foo3, bar1, bar2, bar3, baz1, baz2] :
    std::views::cartesian_product(funcs<0>, funcs<1>, funcs<1>, funcs<0>, funcs<1>, funcs<2>, blockFuncs, blockFuncs)) {
    for (const bool doTrans : { false, true }) {
      auto testSudoku = sudoku;
      foo1(testSudoku);
      foo2(testSudoku);
      foo3(testSudoku);
      baz1(testSudoku);
      transpose(testSudoku);
      bar1(testSudoku);
      bar2(testSudoku);
      bar3(testSudoku);
      baz2(testSudoku);
      if (doTrans) { transpose(testSudoku); }
      minLabeling(testSudoku);
      if (testSudoku < normal) { normal = testSudoku; }
    }
  }
  sudoku = normal;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void generateSudokus(std::size_t nthreads, std::uint32_t nsudokus, bool bSkipNormalize)
{
  std::atomic_int64_t count{ nsudokus };
  auto threads = std::views::iota(0UL, nthreads) | std::views::transform([&](auto) {
    return std::jthread{ [&]() {
      while (true) {
        auto curCount = count.load();
        while (count.compare_exchange_strong(curCount, curCount - 1)) {
          if (curCount <= 0) { return; }
        }
        Sudoku sudoku;
        if (!bSkipNormalize) { normalize(sudoku); }
        std::println("{}", static_cast<std::string_view>(sudoku));
      }
    } };
  });
  std::ignore = std::ranges::to<std::vector>(threads);
}

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, const char **argv)
{
  try {
    CLI::App app{ std::format(
      "{} version {}", SudokuSearch::cmake::project_name, SudokuSearch::cmake::project_version) };

    std::size_t nthreads{ 1UL };
    app.add_option("-t,--threads", nthreads, "Number of threads to use");

    std::uint32_t nsudokus{ 1UL };
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
