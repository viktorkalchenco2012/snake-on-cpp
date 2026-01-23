#include <conio.h>
#include <iostream>
#include <windows.h>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <limits>
#include <string>
#include <algorithm>
using namespace std;

// Constants
constexpr short WIDTH = 80;
constexpr short HEIGHT = 20;
constexpr short MIN_FRUITS = 2;
constexpr short MAX_FRUITS = 12;
constexpr short MAX_SNAKE_LENGTH = 400;
constexpr short MAX_FRUIT_ATTEMPTS = 100;

// Game state structure - using bitfields to save memory
struct GameState {
      // Snake
      short headX, headY;
      short tailX[MAX_SNAKE_LENGTH], tailY[MAX_SNAKE_LENGTH];
      short tailLength;

      // Fruits - using vector for dynamic size
      vector<pair<int, int>> fruits;
      bool fruitsGrid[HEIGHT][WIDTH] = { false }; // Quick lookup grid

      // Game info
      short score;
      bool isGameOver : 1;  // Bitfield to save memory
      bool randomFruitMode : 1;

      // Direction
      enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN } dir;

      GameState() : headX(WIDTH / 2), headY(HEIGHT / 2), tailLength(0),
            score(0), isGameOver(false), randomFruitMode(false), dir(STOP) {
            // Clear grid
            memset(fruitsGrid, 0, sizeof(fruitsGrid));
      }
};

// Function declarations
void InitializeGame(GameState& game, bool randomMode);
void RenderGame(const GameState& game, const string& playerName);
void ProcessInput(GameState& game);
void UpdateGame(GameState& game);
bool TryAddFruit(GameState& game);
void GenerateFruits(GameState& game);
inline void TeleportBorder(int& x, int& y);
inline bool IsPositionOccupied(const GameState& game, int x, int y);

// Initialize game state
void InitializeGame(GameState& game, bool randomMode) {
      game.isGameOver = false;
      game.randomFruitMode = randomMode;
      game.dir = GameState::STOP;
      game.headX = WIDTH / 2;
      game.headY = HEIGHT / 2;
      game.tailLength = 0;
      game.score = 0;

      // Clear fruits grid
      memset(game.fruitsGrid, 0, sizeof(game.fruitsGrid));
      game.fruits.clear();

      GenerateFruits(game);
}

// Optimized rendering with string buffer
void RenderGame(const GameState& game, const string& playerName) {
      static string buffer;
      buffer.clear();

      // Reserve enough space to avoid reallocations
      buffer.reserve((WIDTH + 3) * (HEIGHT + 4) + 100);

      // Top border
      buffer.append(WIDTH + 2, '-');
      buffer.push_back('\n');

      // Game grid
      for (int y = 0; y < HEIGHT; y++) {
            buffer.push_back('|');

            for (int x = 0; x < WIDTH; x++) {
                  // Check head first (most important)
                  if (x == game.headX && y == game.headY) {
                        buffer.push_back('@');
                  }
                  // Check fruits using grid lookup (fast)
                  else if (game.fruitsGrid[y][x]) {
                        buffer.push_back('e');
                  }
                  // Check tail (linear search, but optimized)
                  else {
                        bool tailFound = false;
                        // Only search if there's a tail
                        if (game.tailLength > 0) {
                              // Quick bounds check first
                              if (y >= 0 && y < HEIGHT && x >= 0 && x < WIDTH) {
                                    // Search tail positions
                                    for (int i = 0; i < game.tailLength; i++) {
                                          if (game.tailX[i] == x && game.tailY[i] == y) {
                                                buffer.push_back('o');
                                                tailFound = true;
                                                break;
                                          }
                                    }
                              }
                        }
                        if (!tailFound) {
                              buffer.push_back(' ');
                        }
                  }
            }

            buffer.push_back('|');
            buffer.push_back('\n');
      }

      // Bottom border
      buffer.append(WIDTH + 2, '-');
      buffer.push_back('\n');

      // Game info
      buffer.append(playerName);
      buffer.append("'s Score: ");
      buffer.append(to_string(game.score));
      buffer.append(" | Fruits: ");
      buffer.append(to_string(game.fruits.size()));

      if (game.randomFruitMode) {
            buffer.append(" [RANDOM MODE]");
      }

      // Clear screen and output buffer
      system("cls");
      cout << buffer;
}

// Process keyboard input
void ProcessInput(GameState& game) {
      if (_kbhit()) {
            char key = static_cast<char>(_getch());
            switch (key) {
            case 'a':
                  if (game.dir != GameState::RIGHT) game.dir = GameState::LEFT;
                  break;
            case 'd':
                  if (game.dir != GameState::LEFT) game.dir = GameState::RIGHT;
                  break;
            case 'w':
                  if (game.dir != GameState::DOWN) game.dir = GameState::UP;
                  break;
            case 's':
                  if (game.dir != GameState::UP) game.dir = GameState::DOWN;
                  break;
            case 'x':
                  game.isGameOver = true;
                  break;
            case 'r':
                  if (game.randomFruitMode) GenerateFruits(game);
                  break;
            }
      }
}

// Update game state - optimized
void UpdateGame(GameState& game) {
      // Store old head position for tail update
      static int oldHeadX, oldHeadY;

      // Move snake tail more efficiently
      if (game.tailLength > 0) {
            oldHeadX = game.headX;
            oldHeadY = game.headY;

            // Update tail positions
            for (int i = game.tailLength - 1; i > 0; i--) {
                  game.tailX[i] = game.tailX[i - 1];
                  game.tailY[i] = game.tailY[i - 1];
            }
            game.tailX[0] = oldHeadX;
            game.tailY[0] = oldHeadY;
      }

      // Move head
      switch (game.dir) {
      case GameState::LEFT:  game.headX--; break;
      case GameState::RIGHT: game.headX++; break;
      case GameState::UP:    game.headY--; break;
      case GameState::DOWN:  game.headY++; break;
      default: return; // No movement
      }

      // Teleport border
      TeleportBorder(game.headX, game.headY);

      // Check collision with tail
      for (int i = 0; i < game.tailLength; i++) {
            if (game.tailX[i] == game.headX && game.tailY[i] == game.headY) {
                  game.isGameOver = true;
                  return;
            }
      }

      // Check collision with fruits using grid lookup (fast!)
      if (game.headY >= 0 && game.headY < HEIGHT &&
            game.headX >= 0 && game.headX < WIDTH) {

            if (game.fruitsGrid[game.headY][game.headX]) {
                  game.score++;
                  game.tailLength++;

                  // Find and remove fruit
                  for (auto it = game.fruits.begin(); it != game.fruits.end(); ++it) {
                        if (it->first == game.headX && it->second == game.headY) {
                              // Update grid
                              game.fruitsGrid[it->second][it->first] = false;

                              // Remove from vector
                              game.fruits.erase(it);
                              break;
                        }
                  }

                  // Add new fruits based on mode
                  if (game.randomFruitMode) {
                        int chance = rand() % 100;
                        if (chance < 40) {
                              TryAddFruit(game);
                              TryAddFruit(game);
                        }
                        else {
                              TryAddFruit(game);
                        }

                        if (rand() % 100 < 30) GenerateFruits(game);
                        if (rand() % 100 < 20) TryAddFruit(game);
                  }
                  else {
                        TryAddFruit(game);
                        if (rand() % 4 == 0) TryAddFruit(game);
                        if (rand() % 10 == 0) GenerateFruits(game);
                  }
            }
      }

      // Random fruit mode updates
      if (game.randomFruitMode) {
            static int randomizeTimer = 0;
            randomizeTimer++;

            if (randomizeTimer > 50) {
                  if (rand() % 100 < 15) GenerateFruits(game);
                  randomizeTimer = 0;
            }

            if (rand() % 100 < 5) TryAddFruit(game);
      }

      // Ensure at least one fruit
      if (game.fruits.empty()) TryAddFruit(game);
}

// Inline helper function for position checking
inline bool IsPositionOccupied(const GameState& game, int x, int y) {
      // Check bounds
      if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return true;

      // Check head
      if (x == game.headX && y == game.headY) return true;

      // Check fruits grid (fast)
      if (game.fruitsGrid[y][x]) return true;

      // Check tail
      for (int i = 0; i < game.tailLength; i++) {
            if (game.tailX[i] == x && game.tailY[i] == y) return true;
      }

      return false;
}

// Try to add a single fruit at random position
bool TryAddFruit(GameState& game) {
      // Try fewer attempts for efficiency
      for (int attempts = 0; attempts < MAX_FRUIT_ATTEMPTS; attempts++) {
            int fruitX = rand() % WIDTH;
            int fruitY = rand() % HEIGHT;

            if (!IsPositionOccupied(game, fruitX, fruitY)) {
                  game.fruits.emplace_back(fruitX, fruitY);
                  game.fruitsGrid[fruitY][fruitX] = true;
                  return true;
            }
      }
      return false;
}

// Generate new fruits with random count
void GenerateFruits(GameState& game) {
      // Clear existing fruits
      for (const auto& fruit : game.fruits) {
            game.fruitsGrid[fruit.second][fruit.first] = false;
      }
      game.fruits.clear();

      // Determine fruit count
      int minFruits = game.randomFruitMode ? 1 : MIN_FRUITS;
      int maxFruits = game.randomFruitMode ? MAX_FRUITS : MAX_FRUITS / 2;
      int targetCount = minFruits + rand() % (maxFruits - minFruits + 1);

      // Add fruits
      for (int i = 0; i < targetCount; i++) {
            if (!TryAddFruit(game)) break;
      }
}

// Inline border teleportation for speed
inline void TeleportBorder(int& x, int& y) {
      // Using modulo for wrapping (faster than if-else chain)
      x = (x + WIDTH) % WIDTH;
      y = (y + HEIGHT) % HEIGHT;
}

// Function to get game settings from user
void GetGameSettings(string& playerName, int& delay, bool& randomMode) {
      cout << "Enter your name: ";

      // Clear input buffer only if needed
      if (cin.peek() == '\n') cin.ignore();
      getline(cin, playerName);

      // Trim leading/trailing spaces
      playerName.erase(0, playerName.find_first_not_of(" \t"));
      playerName.erase(playerName.find_last_not_of(" \t") + 1);

      int choice;
      cout << "\nSET DIFFICULTY\n1: Easy\n2: Medium\n3: Hard"
            << "\n4: Random Fruits Mode\nChoose: ";

      cin >> choice;
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
            break;
      default:
            delay = 80;
            cout << "Invalid choice, defaulting to Medium.\n";
      }
      // No Sleep() here - faster startup
}

int main() {
      // Initialize random seed
      srand(static_cast<unsigned int>(time(nullptr)));

      // Get game settings
      string playerName;
      int delay;
      bool randomMode;

      GetGameSettings(playerName, delay, randomMode);

      // Initialize game
      GameState game;
      InitializeGame(game, randomMode);

      // Performance tracking (optional)
      LARGE_INTEGER frequency, start, end;
      QueryPerformanceFrequency(&frequency);

      // Game loop
      while (!game.isGameOver) {
            QueryPerformanceCounter(&start);

            RenderGame(game, playerName);
            ProcessInput(game);
            UpdateGame(game);

            // Adaptive sleep for consistent frame rate
            QueryPerformanceCounter(&end);
            double elapsed = static_cast<double>(end.QuadPart - start.QuadPart) / frequency.QuadPart;
            int sleepTime = static_cast<int>(delay - elapsed * 1000);

            if (sleepTime > 0) {
                  Sleep(static_cast<DWORD>(sleepTime));
            }
      }

      // Game over screen
      system("cls");
      cout << "\n\n\n\t\tGAME OVER!\n\n";
      cout << "\t" << playerName << "'s final score: " << game.score << "\n\n";
      cout << "\tPress any key to exit...";
      _getch();

      return 0;
}
