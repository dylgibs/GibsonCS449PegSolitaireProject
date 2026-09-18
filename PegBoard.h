#ifndef PEG_BOARD_H_
#define PEG_BOARD_H_


class PegBoard {
 public:

  // Determines whether a move is valid.

  // start_peg: True if there is a peg in the starting position.
  // middle_peg: True if there is a peg in the middle position.
  // end_empty: True if the ending position is empty.

  // A move is valid only when all three conditions are true:
  // 1. There is a peg at the starting position.
  // 2. There is a peg at the middle position.
  // 3. The ending position is empty.
  bool IsValidMove(bool start_peg, bool middle_peg, bool end_empty) {
    return start_peg && middle_peg && end_empty;
  }

  // Counts the number of pegs currently on the board.
  
  // board: An array where each element represents a position on the board.
  // true means that position contains a peg.
  // false means that position is empty.
  
  // size: The number of positions/elements in the board array.
  
  // Returns the total number of positions that contain a peg.
  int CountPegs(const bool board[], int size) {
    // Start the peg count at zero.
    int count = 0;

    // Go through every position in the board array.
    for (int i = 0; i < size; ++i) {

      // If the current position contains a peg, increase the count.
      if (board[i]) {
        ++count;
      }
    }

    // Return the total number of pegs found.
    return count;
  }
};


#endif 

