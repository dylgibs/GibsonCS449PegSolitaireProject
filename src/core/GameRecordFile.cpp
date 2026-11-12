#include "GameRecordFile.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {

const char* const kHeader = "PEG-SOLITAIRE-RECORD v1";

std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    const auto b = s.find_first_not_of(ws);
    if (b == std::string::npos) return "";
    const auto e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}

}  // namespace

namespace RecordFile {

std::string serialize(const GameRecord& r) {
    std::ostringstream out;
    out << kHeader << "\n"
        << "board: " << r.boardKey << "\n"
        << "date: " << r.timestamp << "\n"
        << "result: " << toString(r.outcome) << "\n"
        << "pegs: " << r.pegsRemaining << "\n"
        << "moves: " << r.moves.size() << "\n";
    for (const Move& m : r.moves)
        out << m.from.row << "," << m.from.col << " " << m.to.row << "," << m.to.col << "\n";
    return out.str();
}

std::vector<GameRecord> parse(std::istream& in) {
    std::vector<GameRecord> records;
    int lineNo = 0;
    std::string line;

    auto fail = [&](const std::string& msg) {
        throw std::runtime_error("Line " + std::to_string(lineNo) + ": " + msg);
    };
    // Next non-blank line (trimmed); false at end of input.
    auto nextLine = [&](std::string& out) {
        while (std::getline(in, out)) {
            ++lineNo;
            out = trim(out);
            if (!out.empty()) return true;
        }
        return false;
    };
    auto toInt = [&](const std::string& s) {
        char* end = nullptr;
        const long v = std::strtol(s.c_str(), &end, 10);
        if (s.empty() || *end != '\0') fail("expected a number but found '" + s + "'");
        return static_cast<int>(v);
    };

    while (nextLine(line)) {
        if (line != kHeader) fail(std::string("expected '") + kHeader + "'");

        GameRecord rec;
        bool haveBoard = false, haveResult = false;
        int expectedMoves = -1;

        while (expectedMoves < 0) {
            if (!nextLine(line)) fail("unexpected end of file in record header");
            const auto colon = line.find(':');
            if (colon == std::string::npos) fail("expected 'key: value'");
            const std::string key = trim(line.substr(0, colon));
            const std::string val = trim(line.substr(colon + 1));
            if (key == "board")       { rec.boardKey = val; haveBoard = true; }
            else if (key == "date")   { rec.timestamp = val; }
            else if (key == "result") {
                try { rec.outcome = outcomeFromString(val); } catch (const std::exception& e) { fail(e.what()); }
                haveResult = true;
            }
            else if (key == "pegs")   { rec.pegsRemaining = toInt(val); }
            else if (key == "moves")  { expectedMoves = toInt(val); if (expectedMoves < 0) fail("negative move count"); }
            else fail("unknown field '" + key + "'");
        }
        if (!haveBoard) fail("record has no 'board' field");
        if (!haveResult) fail("record has no 'result' field");

        for (int i = 0; i < expectedMoves; ++i) {
            if (!nextLine(line)) fail("expected " + std::to_string(expectedMoves) + " moves but the file ended after " + std::to_string(i));
            int a, b, c, d;
            char tail;
            if (std::sscanf(line.c_str(), "%d,%d %d,%d %c", &a, &b, &c, &d, &tail) != 4)
                fail("malformed move '" + line + "' (expected 'row,col row,col')");
            rec.moves.push_back(Move{{a, b}, {c, d}});
        }
        records.push_back(std::move(rec));
    }
    return records;
}

std::vector<GameRecord> load(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open file: " + path);
    return parse(in);
}

void append(const std::string& path, const GameRecord& record) {
    std::ofstream out(path, std::ios::app);
    if (!out) throw std::runtime_error("Cannot write to file: " + path);
    out << serialize(record) << "\n";
    if (!out) throw std::runtime_error("Error while writing to file: " + path);
}

}  // namespace RecordFile
