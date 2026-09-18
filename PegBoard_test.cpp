#include "PegBoard.h"
// Includes the GTest framework, which provides the TEST and EXPECT_TRUE/EXPECT_FALSE functions used below.
#include <gtest/gtest.h> 

// Tests the IsValidMove function when all conditions for a valid move are true.
TEST(PegBoardTest, ValidMove) {

 
  PegBoard board;

  // Checks that IsValidMove returns true when there is a peg at thestarting position, a peg in the middle, and the ending position is empty.

  // EXPECT_TRUE passes when the expression evaluates to true.
  EXPECT_TRUE(board.IsValidMove(true, true, true));
}

// Tests the IsValidMove function when the ending position is not empty.
TEST(PegBoardTest, InvalidMove) {

  
  PegBoard board;

  // Checks that IsValidMove returns false when the ending position is not empty.
  
  // EXPECT_FALSE passes when the expression evaluates to false.
  EXPECT_FALSE(board.IsValidMove(true, true, false));
}

