add_executable(sudokuGenerator main.cpp)

target_link_libraries(
  sudokuGenerator
  PRIVATE SudokuSearch::SudokuSearch_options
          SudokuSearch::SudokuSearch_warnings)

target_link_system_libraries(
  sudokuGenerator
  PRIVATE
          CLI11::CLI11
          fmt::fmt
          spdlog::spdlog
          lefticus::tools)

target_include_directories(sudokuGenerator PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
