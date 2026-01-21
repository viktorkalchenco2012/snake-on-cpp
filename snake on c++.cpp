#include <iostream>
#include <conio.h>
#include <windows.h>
#include <ctime>

using namespace std;

bool gameOver;
const short width = 20;
const short height = 20;
short x, y, fruitX, fruitY, score;
short tailX[100], tailY[100];
short nTail;
enum eDirection { STOP = 0, LEFT, RIGHT, UP, DOWN };
eDirection dir;
time_t startTime;
const short timeLimitSeconds = 100;

void Setup() {
      gameOver = false;
      dir = STOP;
      x = width / 2;
      y = height / 2;
      fruitX = rand() % width;
      fruitY = rand() % height;
      score = 0;
      nTail = 0;
      startTime = time(NULL);
}

void Draw() {
      system("cls");
      for (short i = 0; i < width + 3; i++)
            cout << "-";
      cout << endl;

      for (short i = 0; i < height; i++) {
            for (short j = 0; j < width; j++) {
                  if (j == 0)
                        cout << "|";
                  if (i == y && j == x)
                        cout << "@";
                  else if (i == fruitY && j == fruitX)
                        cout << "0";
                  else {
                        bool print = false;
                        for (short k = 0; k < nTail; k++) {
                              if (tailX[k] == j && tailY[k] == i) {
                                    cout << "o";
                                    print = true;
                              }
                        }
                        if (!print)
                              cout << " ";
                  }

                  if (j == width - 1)
                        cout << "|";
            }
            cout << endl;
      }

      for (short i = 0; i < width + 3; i++)
            cout << "#";
      cout << endl;
      cout << "Score:" << score / 10 << endl << endl << "use WASD to move" << endl << endl << "GOOD luck" << endl;
}

void Input() {
      if (_kbhit()) {
            switch (_getch()) {
            case 'a':
                  dir = LEFT;
                  break;
            case 'd':
                  dir = RIGHT;
                  break;
            case 'w':
                  dir = UP;
                  break;
            case 's':
                  dir = DOWN;
                  break;
            }
      }
}


void Logic() {
      short prevX = tailX[0];
      short prevY = tailY[0];
      short prev2X, prev2Y;
      tailX[0] = x;
      tailY[0] = y;
      for (short i = 1; i < nTail; i++) {
            prev2X = tailX[i];
            prev2Y = tailY[i];
            tailX[i] = prevX;
            tailY[i] = prevY;
            prevX = prev2X;
            prevY = prev2Y;
      }
      switch (dir) {
      case LEFT:
            x--;
            break;
      case RIGHT:
            x++;
            break;
      case UP:
            y--;
            break;
      case DOWN:
            y++;
            break;
      default:
            break;
      }
      if (x >= width) x = 0; else if (x < 0) x = width - 1;
      if (y >= height) y = 0; else if (y < 0) y = height - 1;

      for (short i = 0; i < nTail; i++)
            if (tailX[i] == x && tailY[i] == y)
                  gameOver = true;

      if (x == fruitX && y == fruitY) {
            score += 10;
            fruitX = rand() % width;
            fruitY = rand() % height;
            nTail++;
      }

      time_t currentTime = time(NULL);
      if (difftime(currentTime, startTime) >= timeLimitSeconds)
            gameOver = true;
}

int main() {
      Setup();
      while (!gameOver) {
            Draw();
            Input();
            Logic();
            Sleep(100);
      }
      cout << "Game over!" << endl;
      return 0;
}
