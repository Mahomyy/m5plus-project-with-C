#include <M5StickCPlus.h>
#include <stdlib.h>
#include <stdint.h>
#include "EEPROM.h"

const int BOARD_SIZE = 8;
const  int JEWEL_TYPES = 5;
const int SCORE_TO_WIN = 100;
const int MAX_MOVES = 25;
const int SPECIAL_DIAMOND = 5;
int selectedRow = -1;
int selectedCol = -1;
bool bothButtonsPressed = false;
bool changed = true;
const float ACCELEROMETER_SENSITIVITY = 0.18;   
const int SPECIAL_DIAMOND_TYPE = 6;  


bool matchThreeOnly = true;  // Flag to determine the game variant

int board[BOARD_SIZE][BOARD_SIZE];
int score = 0;
int moves = 0;

struct GameData {
  int8_t selectedRow;  
  int8_t selectedCol;  
  int16_t score;        
  int8_t moves;         
 
};


void saveGame() {
  int address = 0;

  // Saving the selectedRow, selectedCol, score, and moves
  EEPROM.put(address, GameData{
    static_cast<int8_t>(selectedRow),
    static_cast<int8_t>(selectedCol),
    static_cast<int16_t>(score),
    static_cast<int8_t>(moves)
  });
  address += sizeof(GameData);

  // Saving the game board
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      EEPROM.put(address, static_cast<int8_t>(board[i][j]));
      address += sizeof(int8_t);
    }
  }

  EEPROM.commit();
}




void chooseGameMode() {
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setCursor(10, 30);
  M5.Lcd.print("Choose Game Mode:");
  M5.Lcd.setCursor(20, 60);
  M5.Lcd.print("A. Match-3 Only");
  M5.Lcd.setCursor(20, 80);
  M5.Lcd.print("B. Swap Any");

  while (true) {
    M5.update();
    if (M5.BtnA.wasReleased()) {
      matchThreeOnly = true;
      break;
    } else if (M5.BtnB.wasReleased()) {
      matchThreeOnly = false;
      break;
    }
  }
}
void drawJewel(int row, int col, int type, bool selected = false) {
  int jewelSize = M5.Lcd.width() / BOARD_SIZE;
  int borderSize = 2;  

  int x = col * jewelSize - borderSize;
  int y = row * jewelSize - borderSize;
  int width = jewelSize + 2 * borderSize;
  int height = jewelSize + 2 * borderSize;

  if (!changed) {
    
    int temp = width;
    width = height;
    height = temp;
  }

  if (selected) {
    M5.Lcd.drawRect(x, y, width, height, TFT_WHITE);
  } else {
    M5.Lcd.drawRect(x, y, width, height, TFT_BLACK);
  }

  int jewelColor;
  switch (type) {
    case 0: jewelColor = TFT_RED; break;
    case 1: jewelColor = TFT_BLUE; break;
    case 2: jewelColor = TFT_GREEN; break;
    case 3: jewelColor = TFT_YELLOW; break;
    case 4: jewelColor = TFT_PURPLE; break;
    default: jewelColor = TFT_WHITE; break;
  }

  M5.Lcd.fillRect(col * jewelSize + borderSize, row * jewelSize + borderSize,
                  jewelSize - 2 * borderSize, jewelSize - 2 * borderSize, jewelColor);
}


void drawScore() {
  // Clear the entire area with a black rectangle
  M5.Lcd.fillRect(0, M5.Lcd.height() - 40, M5.Lcd.width(), 40, TFT_BLACK);

  // Draw the updated score and moves left
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setCursor(5, M5.Lcd.height() - 40);
  M5.Lcd.print("Score: " + String(score) + "   Moves: " + String(MAX_MOVES - moves));
}






void drawBoard(int selectedRow, int selectedCol) {
    if (moves < MAX_MOVES) {
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      if (changed) { 
        // Checking if the current jewel is one of the initially selected ones
        bool initiallySelected = (i == selectedRow && j == selectedCol) || (i == selectedRow + 1 && j == selectedCol);
        drawJewel(i, j, board[i][j], initiallySelected);
      } else {  
        // Checking if the current jewel is one of the initially selected ones
        bool initiallySelected = (i == selectedRow && j == selectedCol) || (i == selectedRow && j == selectedCol + 1);
        drawJewel(i, j, board[i][j], initiallySelected);
      }
    }
  }
  drawScore();  // Draw the score
}}

// normally I should have put new number of colors for my jewels and also new boardsize, but I didn't have time for this due of the other projects. But if i had time i would have used dynamic allocation of those variables to make it work (I think this is the only feature in the project where we should use dynamic allocated memory)
void generateNewLevel(bool isNewLevel) { 
  if (isNewLevel) {
    moves = 0;
    score = 0;
  }

  // A new game board for the new level
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      // Ensuring that no three jewels of the same type are placed consecutively in a row
      do {
        board[i][j] = random(JEWEL_TYPES);
      } while ((j >= 2) && (board[i][j] == board[i][j - 1]) && (board[i][j] == board[i][j - 2]));

      // Ensuring that no three jewels of the same type are placed consecutively in a column
      if (i >= 2 && (board[i][j] == board[i - 1][j]) && (board[i][j] == board[i - 2][j])) {
        board[i][j] = (board[i][j] + 1) % JEWEL_TYPES;  // Changing the type to avoid consecutive matches
      }
    }
  }

  // Reset the selected item to (0, 0)
  selectedRow = 0;
  selectedCol = 0;

  // Redrawing the board with the new level
  drawBoard(selectedRow, selectedCol);
}

void reset() {
  // Reset game state to initial values
  selectedRow = 0;
  selectedCol = 0;

  // Reset the game board
  generateNewLevel(false);

  drawBoard(selectedRow, selectedCol);
  drawScore();
}


void loadGame() {
  int address = 0;

  GameData loadedData;
  EEPROM.get(address, loadedData);
  address += sizeof(GameData);

  // Load the game board
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      EEPROM.get(address, board[i][j]);
      address += sizeof(int);
    }
  }

  // Update the game state
  selectedRow = loadedData.selectedRow;
  selectedCol = loadedData.selectedCol;
  score = loadedData.score;
  moves = loadedData.moves;

  // Redraw the game board with the updated state
  drawBoard(selectedRow, selectedCol);

  // Draw the score
  drawScore();

  // Setting isNewLevel to false to avoid reseting moves and score
  generateNewLevel(false);
}




bool checkMatch(int row, int col) {
  // Checking for matches in the horizontal direction
  int countHorizontal = 1;
  for (int j = col - 1; j >= 0 && board[row][j] == board[row][col]; --j) {
    countHorizontal++;
  }
  for (int j = col + 1; j < BOARD_SIZE && board[row][j] == board[row][col]; ++j) {
    countHorizontal++;
  }

  // Checking for matches in the vertical direction
  int countVertical = 1;
  for (int i = row - 1; i >= 0 && board[i][col] == board[row][col]; --i) {
    countVertical++;
  }
  for (int i = row + 1; i < BOARD_SIZE && board[i][col] == board[row][col]; ++i) {
    countVertical++;
  }

  // Checking if there is a match of three or more in either direction
  return countHorizontal >= 3 || countVertical >= 3;
}


void swapJewels(int row1, int col1, int row2, int col2) {
  if (matchThreeOnly) {
    moves += 1;
    // Checking if swapping creates a match of three or more in the horizontal direction
    int temp1 = board[row1][col1];
    int temp2 = board[row2][col2];
    board[row1][col1] = temp2;
    board[row2][col2] = temp1;

    bool horizontalMatch = checkMatch(row1, col1) || checkMatch(row2, col2);

    // Checking if swapping creates a match of three or more in the vertical direction
    board[row1][col1] = temp1;
    board[row2][col2] = temp2;

    bool verticalMatch = checkMatch(row1, col1) || checkMatch(row2, col2);

    // Undo the swap if it doesn't create a match in either direction
    if (!horizontalMatch && !verticalMatch) {
      // Swap back
      board[row1][col1] = temp1;
      board[row2][col2] = temp2;
      drawBoard(-1, -1); 
      return;
    }
  }

  // Performing the swap if it's allowed
  moves += 1;
  int temp = board[row1][col1];
  board[row1][col1] = board[row2][col2];
  board[row2][col2] = temp;
  drawBoard(-1, -1); 
}


void shiftDown(int row, int col) {
     score += 10;
  for (int i = row; i > 0; --i) {
    board[i][col] = board[i - 1][col];
  }
  board[0][col] = random(JEWEL_TYPES);
    // Reset the selected position to (0, 0)
  selectedRow = 0;
  selectedCol = 0;

}


void generateSpecialDiamond(int row, int col) {
  board[row][col] = SPECIAL_DIAMOND_TYPE;
}
void activateSpecialDiamonds() {
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      if (board[i][j] == SPECIAL_DIAMOND) {
        // Activate the special diamond
        int color = board[i][j]; // Save the original color
        board[i][j] = -1; // Mark the special diamond for removal

        // Remove the entire column
        for (int k = 0; k < BOARD_SIZE; ++k) {
          if (board[k][j] != -1) {
            board[k][j] = -1; // Mark for removal
          }
        }

        // Fill the column with new jewels
        for (int k = 0; k < BOARD_SIZE; ++k) {
          board[k][j] = random(JEWEL_TYPES);
        }

        // Restore the original color in a random position in the new column
        int randomRow = random(BOARD_SIZE);
        board[randomRow][j] = color;
      }
    }
  }
}

void checkMatches() {
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      if (board[i][j] != -1) {
        // Check for matches in the horizontal direction
        int countHorizontal = 1;
        for (int k = j - 1; k >= 0 && board[i][k] == board[i][j]; --k) {
          countHorizontal++;
        }
        for (int k = j + 1; k < BOARD_SIZE && board[i][k] == board[i][j]; ++k) {
          countHorizontal++;
        }

        // Checking for matches in the vertical direction
        int countVertical = 1;
        for (int k = i - 1; k >= 0 && board[k][j] == board[i][j]; --k) {
          countVertical++;
        }
        for (int k = i + 1; k < BOARD_SIZE && board[k][j] == board[i][j]; ++k) {
          countVertical++;
        }

        // Checking if there is a match of three or more in either direction
        if (countHorizontal >= 3 || countVertical >= 3) {
          // Handle matches of 3 or more

          // Remove the matched jewels
          for (int k = j; k <= j + countHorizontal - 1; ++k) {
            board[i][k] = -1;  // Mark for removal
          }
          for (int k = i; k <= i + countVertical - 1; ++k) {
            board[k][j] = -1;  // Mark for removal
          }

          for (int k = i - 1; k >= 0; --k) {
            if (board[k][j] != -1) {
              board[k + countVertical][j] = board[k][j];
              board[k][j] = -1;  
            }
          }

          // Shift down to fill the empty spaces
          shiftDown(i, j);
        }

        // Check if there is a match of four or more in either direction
        if (countHorizontal >= 4) {
          // Handle matches of 4 or more in the horizontal direction
          generateSpecialDiamond(i, j);
        }

        if (countVertical >= 4) {
          // Handle matches of 4 or more in the vertical direction
          generateSpecialDiamond(i, j);
        }
      }
    }
  }

  // Activating special diamonds
  activateSpecialDiamonds();

  // Filling the empty spaces with new jewels
  for (int j = 0; j < BOARD_SIZE; ++j) {
    int writeIndex = BOARD_SIZE - 1;
    for (int i = BOARD_SIZE - 1; i >= 0; --i) {
      if (board[i][j] != -1) {
        board[writeIndex--][j] = board[i][j];
      }
    }
    // Filling the remaining empty spaces with new jewels
    for (int i = writeIndex; i >= 0; --i) {
      board[i][j] = random(JEWEL_TYPES);
    }
  }
}


void gameOver() {
  // Filling the screen with black color
  M5.Lcd.fillScreen(TFT_BLACK);

  // Display the "Game Over" message
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setCursor(20, 50);
  M5.Lcd.print("Game Over");
 
}



void checkWinLossConditions() {
  if (score >= SCORE_TO_WIN) {
    generateNewLevel(true);
  } else if (moves >= MAX_MOVES) {
     M5.Lcd.fillScreen(TFT_BLACK);
    gameOver();  
  }
}


void handleButtonPress() {

    if (changed) {

        float accX, accY, accZ;
  M5.IMU.getAccelData(&accX, &accY, &accZ);

  int selectedButton = 0;  // Default to no movement

  if (accX < -ACCELEROMETER_SENSITIVITY) {
    selectedButton = 0;  // Move right
  } else if (accX > ACCELEROMETER_SENSITIVITY) {
    selectedButton = 1;  // Move left
  } else if (accY < -ACCELEROMETER_SENSITIVITY) {
    selectedButton = 2;  // Move down
  } else if (accY > ACCELEROMETER_SENSITIVITY) {
    selectedButton = 3;  // Move up
  }

  if (selectedRow != -1 && selectedCol != -1) {
    switch (selectedButton) {
      case 0:
        // Move right (increase column) for tilting right
        if (selectedCol < BOARD_SIZE - 1) {
          selectedCol++;
        }
        break;
      case 1:
        // Move left (decrease column) for tilting left
        if (selectedCol > 0) {
          selectedCol--;
        }
        break;
      case 2:
        // Move down (increase row) for tilting down
        if (selectedRow < BOARD_SIZE - 2) {
          selectedRow++;
        }
        break;
      case 3:
        // Move up (decrease row) for tilting up
        if (selectedRow > 0) {
          selectedRow--;
        }
        break;
      default:
        // No movement
        break;
    }

    drawBoard(selectedRow, selectedCol);
  } else {
    drawBoard(selectedRow, selectedCol);
  }

 } else { float accX, accY, accZ;
  M5.IMU.getAccelData(&accX, &accY, &accZ);

  int selectedButton = 0;  // Default to no movement

  if (accX < -ACCELEROMETER_SENSITIVITY) {
    selectedButton = 0;  // Move right
  } else if (accX > ACCELEROMETER_SENSITIVITY) {
    selectedButton = 1;  // Move left
  } else if (accY < -ACCELEROMETER_SENSITIVITY) {
    selectedButton = 2;  // Move down
  } else if (accY > ACCELEROMETER_SENSITIVITY) {
    selectedButton = 3;  // Move up
  }

  if (selectedRow != -1 && selectedCol != -1) {
    switch (selectedButton) {
      case 0:
        // Move right (increase column) for tilting right
        if (selectedCol < BOARD_SIZE - 2) {
          selectedCol++;
        }
        break;
      case 1:
        // Move left (decrease column) for tilting left
        if (selectedCol > 0) {
          selectedCol--;
        }
        break;
      case 2:
        // Move down (increase row) for tilting down
        if (selectedRow < BOARD_SIZE - 1) {
          selectedRow++;
        }
        break;
      case 3:
        // Move up (decrease row) for tilting up
        if (selectedRow > 0) {
          selectedRow--;
        }
        break;
      default:
        // No movement
        break;
    }

    drawBoard(selectedRow, selectedCol);
  } else {
    drawBoard(selectedRow, selectedCol);
  }
}}

void showMainMenu() {
  int selectedOption = 0;
  const int NUM_OPTIONS = 4;
  GameData gameData;  

  while (true) {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextSize(1);

    M5.Lcd.setCursor(10, 30);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.print("1. Save Game");

    M5.Lcd.setCursor(10, 50);
    M5.Lcd.print("2. Load Game");

    M5.Lcd.setCursor(10, 70);
    M5.Lcd.print("3. Reset Board");

    M5.Lcd.setCursor(10, 90);
    M5.Lcd.print("4. Exit Menu");


    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setCursor(0, 30 + selectedOption * 20);
    M5.Lcd.fillRect(0, 30 + selectedOption * 20, M5.Lcd.width(), 20, TFT_BLUE);
    M5.Lcd.print(">");

    M5.update();
    delay(100);

    if (M5.BtnA.wasReleased()) {
     
      selectedOption = (selectedOption + 1) % NUM_OPTIONS;
    } else if (M5.BtnB.wasReleased()) {
     
      switch (selectedOption) {
        case 0:
        
          saveGame();
          break;
        case 1:
          
          loadGame();
          break;
           case 2:
         
          reset();
          break;
        case 3:
        M5.Lcd.fillScreen(TFT_BLACK);
          return;
      }
    }
  }
}




void updateBoard() {
    checkWinLossConditions();
  checkMatches();
}

void loop() {
  M5.update();
  
  if (M5.BtnA.isPressed() && M5.BtnB.isPressed()) {
    bothButtonsPressed = !bothButtonsPressed;
  } else if (M5.BtnA.isPressed()) {
    changed = !changed;
  } else if (M5.BtnB.isPressed()) {
    showMainMenu();
  }

  delay(100); 

  if (!bothButtonsPressed) {
    handleButtonPress();
  }

   if (bothButtonsPressed) {
     if (changed) {
    swapJewels(selectedRow, selectedCol, selectedRow + 1, selectedCol);
    // Reseting the flag to prevent continuous swapping
    bothButtonsPressed = false;
    } else { swapJewels(selectedRow, selectedCol, selectedRow, selectedCol+1);
    // Reseting the flag to prevent continuous swapping
    bothButtonsPressed = false;
  }}



  updateBoard();
  delay(100); 
}


void setup() {
  M5.begin();
  M5.IMU.Init();
  Serial.begin(115200);
  Serial.flush();
  EEPROM.begin(512);
  M5.Lcd.fillScreen(TFT_BLACK);
  chooseGameMode();

  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      do {
        board[i][j] = random(JEWEL_TYPES);
      } while ((j >= 2) && (board[i][j] == board[i][j - 1]) && (board[i][j] == board[i][j - 2]));
      if (i >= 2 && (board[i][j] == board[i - 1][j]) && (board[i][j] == board[i - 2][j])) {
        board[i][j] = (board[i][j] + 1) % JEWEL_TYPES;  // Changing the type to avoid consecutive matches
      }
    }
  }

  // Initialize the selected item to (0, 0)
  selectedRow = 0;
  selectedCol = 0;

  M5.Lcd.fillScreen(TFT_BLACK);
  // Drawing the initial board with the selected item
  drawBoard(selectedRow, selectedCol);
}

