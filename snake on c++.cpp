#include <conio.h>
#include <iostream>
#include <windows.h>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <limits>
#include <string>
using namespace std;

// Constants
const int WIDTH = 80;
const int HEIGHT = 20;
const int MIN_FRUITS = 2;
const int MAX_FRUITS = 12; // Increased max for more variety
const int MAX_SNAKE_LENGTH = 400;

// Game state structure
struct GameState {
      // Snake
      int headX, headY;
      int tailX[MAX_SNAKE_LENGTH], tailY[MAX_SNAKE_LENGTH];
      int tailLength;

      // Fruits
      vector<pair<int, int>> fruits;

      // Game info
      int score;
      bool isGameOver;
      bool randomFruitMode; // NEW: Track if we're in random fruit mode

      // Direction
      enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN } dir;
};

// Function declarations
void InitializeGame(GameState& game, bool randomMode);
void RenderGame(const GameState& game, const string& playerName);
void ProcessInput(GameState& game);
void UpdateGame(GameState& game);
bool TryAddFruit(GameState& game);
void GenerateFruits(GameState& game);
void TeleportBorder(int& x, int& y);

// Initialize game state
void InitializeGame(GameState& game, bool randomMode) {
      game.isGameOver = false;
      game.randomFruitMode = randomMode; // Set the mode
      game.dir = GameState::STOP;
      game.headX = WIDTH / 2;
      game.headY = HEIGHT / 2;
      game.tailLength = 0;
      game.score = 0;

      GenerateFruits(game);
}

// Render game screen
void RenderGame(const GameState& game, const string& playerName) {
      system("cls");

      // Top border
      cout << string(WIDTH + 2, '-') << endl;

      // Game grid
      for (int y = 0; y < HEIGHT; y++) {
            cout << '|';

            for (int x = 0; x < WIDTH; x++) {
                  // Check what's at this position
                  if (x == game.headX && y == game.headY) {
                        cout << '@';
                  }
                  else {
                        bool printed = false;

                        // Check fruits
                        for (const auto& fruit : game.fruits) {
                              if (fruit.first == x && fruit.second == y) {
                                    cout << 'e';
                                    printed = true;
                                    break;
                              }
                        }

                        // Check tail
                        if (!printed) {
                              for (int i = 0; i < game.tailLength; i++) {
                                    if (game.tailX[i] == x && game.tailY[i] == y) {
                                          cout << 'o';
                                          printed = true;
                                          break;
                                    }
                              }
                        }

                        if (!printed) cout << ' ';
                  }
            }

            cout << "|\n";
      }

      // Bottom border
      cout << string(WIDTH + 2, '-') << endl;

      // Game info - Show mode if in random fruit mode
      cout << " " << playerName << "'s Score: " << game.score
            << " | Fruits: " << game.fruits.size();

      if (game.randomFruitMode) {
            cout << " [RANDOM MODE]";
      }

      cout << endl;
}

// Process keyboard input
void ProcessInput(GameState& game) {
      if (_kbhit()) {
            switch (_getch()) {
            case 'a': if (game.dir != GameState::RIGHT) game.dir = GameState::LEFT; break;
            case 'd': if (game.dir != GameState::LEFT) game.dir = GameState::RIGHT; break;
            case 'w': if (game.dir != GameState::DOWN) game.dir = GameState::UP; break;
            case 's': if (game.dir != GameState::UP) game.dir = GameState::DOWN; break;
            case 'x': game.isGameOver = true; break;
            case 'r': // DEBUG: Manually randomize fruits
                  if (game.randomFruitMode) {
                        GenerateFruits(game);
                  }
                  break;
            }
      }
}

// Update game state
void UpdateGame(GameState& game) {
      // Move snake tail
      if (game.tailLength > 0) {
            int prevX = game.tailX[0];
            int prevY = game.tailY[0];
            game.tailX[0] = game.headX;
            game.tailY[0] = game.headY;

            for (int i = 1; i < game.tailLength; i++) {
                  int tempX = game.tailX[i];
                  int tempY = game.tailY[i];
                  game.tailX[i] = prevX;
                  game.tailY[i] = prevY;
                  prevX = tempX;
                  prevY = tempY;
            }
      }

      // Move head based on direction
      switch (game.dir) {
      case GameState::LEFT:  game.headX--; break;
      case GameState::RIGHT: game.headX++; break;
      case GameState::UP:    game.headY--; break;
      case GameState::DOWN:  game.headY++; break;
      }

      // Handle border teleport
      TeleportBorder(game.headX, game.headY);

      // Check collision with tail
      for (int i = 0; i < game.tailLength; i++) {
            if (game.tailX[i] == game.headX && game.tailY[i] == game.headY) {
                  game.isGameOver = true;
                  return;
            }
      }

      // Check collision with fruits
      for (size_t i = 0; i < game.fruits.size(); i++) {
            if (game.fruits[i].first == game.headX && game.fruits[i].second == game.headY) {
                  game.score++;
                  game.tailLength++;

                  // Remove eaten fruit
                  game.fruits.erase(game.fruits.begin() + i);

                  // In random fruit mode, have more randomization
                  if (game.randomFruitMode) {
                        // 40% chance to add 2 fruits instead of 1
                        if (rand() % 100 < 40) {
                              TryAddFruit(game);
                              TryAddFruit(game);
                        }
                        else {
                              TryAddFruit(game);
                        }

                        // 30% chance to completely regenerate fruits
                        if (rand() % 100 < 30) {
                              GenerateFruits(game);
                        }

                        // 20% chance to add an extra random fruit
                        if (rand() % 100 < 20) {
                              TryAddFruit(game);
                        }
                  }
                  else {
                        // Normal mode behavior
                        TryAddFruit(game);
                        if (rand() % 4 == 0) TryAddFruit(game); // 25% chance for second fruit

                        // 10% chance to regenerate all fruits
                        if (rand() % 10 == 0) GenerateFruits(game);
                  }

                  break;
            }
      }

      // In random fruit mode, occasionally randomize fruits even when not eating
      if (game.randomFruitMode) {
            static int randomizeTimer = 0;
            randomizeTimer++;

            // Every 50 frames, 15% chance to randomize fruits
            if (randomizeTimer > 50) {
                  if (rand() % 100 < 15) {
                        GenerateFruits(game);
                  }
                  randomizeTimer = 0;
            }

            // Occasionally add random fruit
            if (rand() % 100 < 5) { // 5% chance each frame
                  TryAddFruit(game);
            }
      }

      // Ensure at least one fruit exists
      if (game.fruits.empty()) TryAddFruit(game);
}

// Try to add a single fruit at random position
bool TryAddFruit(GameState& game) {
      const int MAX_ATTEMPTS = 100;

      for (int attempts = 0; attempts < MAX_ATTEMPTS; attempts++) {
            int fruitX = rand() % WIDTH;
            int fruitY = rand() % HEIGHT;

            // Check collision with snake head
            if (fruitX == game.headX && fruitY == game.headY) continue;

            // Check collision with snake tail
            bool collision = false;
            for (int i = 0; i < game.tailLength; i++) {
                  if (game.tailX[i] == fruitX && game.tailY[i] == fruitY) {
                        collision = true;
                        break;
                  }
            }
            if (collision) continue;

            // Check collision with existing fruits
            for (const auto& fruit : game.fruits) {
                  if (fruit.first == fruitX && fruit.second == fruitY) {
                        collision = true;
                        break;
                  }
            }
            if (collision) continue;

            // Add the fruit
            game.fruits.emplace_back(fruitX, fruitY);
            return true;
      }

      return false;
}

// Generate new fruits with random count
void GenerateFruits(GameState& game) {
      game.fruits.clear();

      // Different fruit count range based on mode
      int minFruits, maxFruits;

      if (game.randomFruitMode) {
            // In random mode, more extreme variation
            minFruits = 1;
            maxFruits = MAX_FRUITS;
      }
      else {
            // Normal mode
            minFruits = MIN_FRUITS;
            maxFruits = MAX_FRUITS / 2;
      }

      int targetCount = minFruits + rand() % (maxFruits - minFruits + 1);

      for (int i = 0; i < targetCount; i++) {
            if (!TryAddFruit(game)) break;
      }

      // Debug output to show randomization is working
      if (game.randomFruitMode) {
            // This will briefly show in the console
            cout << "\nFruits randomized! Now have " << game.fruits.size() << " fruits.\n";
            Sleep(100); // Brief pause to see the message
      }
}

// Handle border teleportation
void TeleportBorder(int& x, int& y) {
      if (x < 0) x = WIDTH - 1;
      else if (x >= WIDTH) x = 0;

      if (y < 0) y = HEIGHT - 1;
      else if (y >= HEIGHT) y = 0;
}

// Function to get game settings from user
void GetGameSettings(string& playerName, int& delay, bool& randomMode) {
      cout << "Enter your name: ";
      // DON'T use cin.ignore() here - it's removing the first character
      cin.ignore();
      cin;
      getline(cin, playerName);

      int choice;
      cout << "\nSET DIFFICULTY\n1: Easy\n2: Medium\n3: Hard"
            << "\n4: Random Fruits Mode\nChoose: ";
      cin >> choice;

      // Clear the input buffer after reading choice
      cin.ignore(numeric_limits<streamsize>::max(), '\n');

      randomMode = false;

      switch (choice) {
      case 1: delay = 120; break;
      case 2: delay = 80; break;
      case 3: delay = 50; break;
      case 4:
            delay = 70;
            randomMode = true;
            cout << "\n=== RANDOM FRUITS MODE ACTIVATED ===\n";
            cout << "Fruit count and positions will change randomly!\n";
            Sleep(1500);
            break;
      default:
            delay = 80;
            cout << "Invalid choice, defaulting to Medium difficulty.\n";
            Sleep(1000);
      }
}

int main() {
      // Initialize random seed
      srand(static_cast<unsigned int>(time(0)));

      // Get game settings
      string playerName;
      int delay;
      bool randomMode;

      GetGameSettings(playerName, delay, randomMode);

      // Initialize game
      GameState game;
      InitializeGame(game, randomMode);

      // Game loop
      while (!game.isGameOver) {
            RenderGame(game, playerName);
            ProcessInput(game);
            UpdateGame(game);
            Sleep(delay);
      }

      // Game over
      system("cls");
      cout << "\n\n\n\t\tGAME OVER!\n\n";
      cout << "\t" << playerName << "'s final score: " << game.score << "\n\n";
      cout << "\tPress any key to exit...";
      _getch();

      return 0;
}
