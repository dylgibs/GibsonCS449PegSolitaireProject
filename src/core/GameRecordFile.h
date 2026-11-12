#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "GameRecord.h"

// Plain-text storage for finished games. One file may hold many records.
//
//   PEG-SOLITAIRE-RECORD v1
//   board: english-standard
//   date: 2026-11-10 14:32:05
//   result: lost
//   pegs: 5
//   moves: 27
//   3,1 3,3          <- "row,col row,col" = from, to (one move per line)
//   ...
//
// All functions throw std::runtime_error with a readable message on I/O or format errors.
namespace RecordFile {

std::string serialize(const GameRecord& record);
std::vector<GameRecord> parse(std::istream& in);
std::vector<GameRecord> load(const std::string& path);
void append(const std::string& path, const GameRecord& record);

}  // namespace RecordFile
