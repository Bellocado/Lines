#include "raylib.h"
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

struct Point {
    int x, y;
    Point(int x_val, int y_val) : x(x_val), y(y_val) {}
};

struct Line {
    Point start, end;
    Line(Point start_val, Point end_val) : start(start_val), end(end_val) {}
};

enum Direction { NONE, UP, DOWN, LEFT, RIGHT };

enum GameState { MAIN_MENU, PLAYING, GAME_OVER };

bool DoLinesIntersect(Point p1, Point p2, Point p3, Point p4) {
    auto cross = [](Point p, Point q, Point r) {
        return (q.x - p.x) * (r.y - p.y) - (q.y - p.y) * (r.x - p.x);
        };

    int d1 = cross(p3, p4, p1);
    int d2 = cross(p3, p4, p2);
    int d3 = cross(p1, p2, p3);
    int d4 = cross(p1, p2, p4);

    return (d1 * d2 < 0 && d3 * d4 < 0);
}

bool DoLinesIntersectWithTolerance(Point p1, Point p2, Point p3, Point p4, int tolerance = 5) {
    if (max(p1.x, p2.x) < min(p3.x, p4.x) - tolerance || max(p3.x, p4.x) < min(p1.x, p2.x) - tolerance)
        return false;
    if (max(p1.y, p2.y) < min(p3.y, p4.y) - tolerance || max(p3.y, p4.y) < min(p1.y, p2.y) - tolerance)
        return false;

    return DoLinesIntersect(p1, p2, p3, p4);
}

void ResetGame(int& Player1_x, int& Player1_y, int& Player2_x, int& Player2_y,
    vector<Line>& player1Lines, vector<Line>& player2Lines) {
    Player1_x = rand() % 801;
    Player1_y = rand() % 601;
    Player2_x = rand() % 801;
    Player2_y = rand() % 601;
    player1Lines.clear();
    player2Lines.clear();
}

bool IsOutOfBounds(int x, int y) {
    return (x < 0 || x > 800 || y < 0 || y > 600);
}

int main() {
    srand(time(NULL));

    // Game State
    GameState currentState = MAIN_MENU;

    // Player and game variables
    int Player1_x, Player1_y, Player1_x_dest, Player1_y_dest;
    int Player2_x, Player2_y, Player2_x_dest, Player2_y_dest;
    vector<Line> player1Lines, player2Lines;
    int Player1_score = 0, Player2_score = 0;
    Direction player1_last_direction = NONE, player2_last_direction = NONE;

    // Init window
    InitWindow(800, 600, "Lines");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        switch (currentState) {
        case MAIN_MENU:
            // Draw the main menu
            DrawText("MAIN MENU", (800 - MeasureText("MAIN MENU", 40)) / 2, 100, 40, YELLOW);
            DrawText("Press ENTER to Start", (800 - MeasureText("Press ENTER to Start", 20)) / 2, 200, 20, WHITE);
            DrawText("Press ESC to Quit", (800 - MeasureText("Press ESC to Quit", 20)) / 2, 250, 20, WHITE);

            // Menu input
            if (IsKeyPressed(KEY_ENTER)) {
                currentState = PLAYING; // Start the game
                ResetGame(Player1_x, Player1_y, Player2_x, Player2_y, player1Lines, player2Lines);
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                CloseWindow(); // Close the window
            }
            break;

        case PLAYING:
            // Draw game elements
            DrawText(TextFormat("Player 1: %d", Player1_score), 10, 10, 20, YELLOW);
            DrawText(TextFormat("Player 2: %d", Player2_score), 10, 50, 20, YELLOW);
            DrawText("Lines", (800 - MeasureText("Lines", 40)) / 2, 10, 40, YELLOW);
            DrawRectangle(Player1_x, Player1_y, 3, 3, RED);
            DrawRectangle(Player2_x, Player2_y, 3, 3, GREEN);

            // Draw Player 1's lines
            for (auto& line : player1Lines)
                DrawLine(line.start.x, line.start.y, line.end.x, line.end.y, RED);

            // Draw Player 2's lines
            for (auto& line : player2Lines)
                DrawLine(line.start.x, line.start.y, line.end.x, line.end.y, GREEN);

            // Player 1 movement and collision checks
            if (IsKeyPressed(KEY_W) && player1_last_direction != DOWN) {
                Player1_y_dest = rand() % 300;
                Line newLine(Point(Player1_x, Player1_y), Point(Player1_x, Player1_y - Player1_y_dest));
                player1Lines.push_back(newLine);
                for (auto& line : player2Lines) {
                    if (DoLinesIntersectWithTolerance(Point(Player1_x, Player1_y), Point(Player1_x, Player1_y - Player1_y_dest), line.start, line.end)) {
                        Player2_score++;  // Player 1 scores 1 point for each collision
                        ResetGame(Player1_x, Player1_y, Player2_x, Player2_y, player1Lines, player2Lines);
                        if (Player2_score >= 10) {
                            currentState = GAME_OVER;
                        }
                    }
                }
                Player1_y -= Player1_y_dest;
                player1_last_direction = UP;
            }

            if (IsKeyPressed(KEY_S) && player1_last_direction != UP) {
                Player1_y_dest = rand() % 300;
                Line newLine(Point(Player1_x, Player1_y), Point(Player1_x, Player1_y + Player1_y_dest));
                player1Lines.push_back(newLine);
                for (auto& line : player2Lines) {
                    if (DoLinesIntersectWithTolerance(Point(Player1_x, Player1_y), Point(Player1_x, Player1_y + Player1_y_dest), line.start, line.end)) {
                        Player2_score++;
                        ResetGame(Player1_x, Player1_y, Player2_x, Player2_y, player1Lines, player2Lines);
                        if (Player2_score >= 10) {
                            currentState = GAME_OVER;
                        }
                    }
                }
                Player1_y += Player1_y_dest;
                player1_last_direction = DOWN;
            }

            if (IsKeyPressed(KEY_D) && player1_last_direction != LEFT) {
                Player1_x_dest = rand() % 300;
                Line newLine(Point(Player1_x, Player1_y), Point(Player1_x + Player1_x_dest, Player1_y));
                player1Lines.push_back(newLine);
                for (auto& line : player2Lines) {
                    if (DoLinesIntersectWithTolerance(Point(Player1_x, Player1_y), Point(Player1_x + Player1_x_dest, Player1_y), line.start, line.end)) {
                        Player2_score++;
                        ResetGame(Player1_x, Player1_y, Player2_x, Player2_y, player1Lines, player2Lines);
                        if (Player2_score >= 10) {
                            currentState = GAME_OVER;
                        }
                    }
                }
                Player1_x += Player1_x_dest;
                player1_last_direction = RIGHT;
            }

            if (IsKeyPressed(KEY_A) && player1_last_direction != RIGHT) {
                Player1_x_dest = rand() % 300;
                Line newLine(Point(Player1_x, Player1_y), Point(Player1_x - Player1_x_dest, Player1_y));
                player1Lines.push_back(newLine);
                for (auto& line : player2Lines) {
                    if (DoLinesIntersectWithTolerance(Point(Player1_x, Player1_y), Point(Player1_x - Player1_x_dest, Player1_y), line.start, line.end)) {
                        Player2_score++;
                        ResetGame(Player1_x, Player1_y, Player2_x, Player2_y, player1Lines, player2Lines);
                        if (Player2_score >= 10) {
                            currentState = GAME_OVER;
                        }
                    }
                }
                Player1_x -= Player1_x_dest;
                player1_last_direction = LEFT;
            }

            // Check for collisions with the wall for Player 1 (out of bounds)
            if (IsOutOfBounds(Player1_x, Player1_y)) {
                Player2_score++;  // Player 2 scores 1 point for Player 1 going out of bounds
                ResetGame(Player1_x, Player1_y, Player2_x, Player2_y, player1Lines, player2Lines);
                if (Player2_score >= 10) {
                    currentState = GAME_OVER;
                }
            }

            // Player 2 movement and collision checks
            if (IsKeyPressed(KEY_I) && player2_last_direction != DOWN) {
                Player2_y_dest = rand() % 300;
                Line newLine(Point(Player2_x, Player2_y), Point(Player2_x, Player2_y - Player2_y_dest));
                player2Lines.push_back(newLine);
                for (auto& line : player1Lines) {
                    if (DoLinesIntersectWithTolerance(Point(Player2_x, Player2_y), Point(Player2_x, Player2_y - Player2_y_dest), line.start, line.end)) {
                        Player1_score++;
                        ResetGame(Player1_x, Player1_y, Player2_x, Player2_y, player1Lines, player2Lines);
                        if (Player1_score >= 10) {
                            currentState = GAME_OVER;
                        }
                    }
                }
                Player2_y -= Player2_y_dest;
                player2_last_direction = UP;
            }

            if (IsKeyPressed(KEY_K) && player2_last_direction != UP) {
                Player2_y_dest = rand() % 300;
                Line newLine(Point(Player2_x, Player2_y), Point(Player2_x, Player2_y + Player2_y_dest));
                player2Lines.push_back(newLine);
                for (auto& line : player1Lines) {
                    if (DoLinesIntersectWithTolerance(Point(Player2_x, Player2_y), Point(Player2_x, Player2_y + Player2_y_dest), line.start, line.end)) {
                        Player1_score++;
                        ResetGame(Player1_x, Player1_y, Player2_x, Player2_y, player1Lines, player2Lines);
                        if (Player1_score >= 10) {
                            currentState = GAME_OVER;
                        }
                    }
                }
                Player2_y += Player2_y_dest;
                player2_last_direction = DOWN;
            }

            if (IsKeyPressed(KEY_L) && player2_last_direction != LEFT) {
                Player2_x_dest = rand() % 300;
                Line newLine(Point(Player2_x, Player2_y), Point(Player2_x + Player2_x_dest, Player2_y));
                player2Lines.push_back(newLine);
                for (auto& line : player1Lines) {
                    if (DoLinesIntersectWithTolerance(Point(Player2_x, Player2_y), Point(Player2_x + Player2_x_dest, Player2_y), line.start, line.end)) {
                        Player1_score++;
                        ResetGame(Player1_x, Player1_y, Player2_x, Player2_y, player1Lines, player2Lines);
                        if (Player1_score >= 10) {
                            currentState = GAME_OVER;
                        }
                    }
                }
                Player2_x += Player2_x_dest;
                player2_last_direction = RIGHT;
            }

            if (IsKeyPressed(KEY_J) && player2_last_direction != RIGHT) {
                Player2_x_dest = rand() % 300;
                Line newLine(Point(Player2_x, Player2_y), Point(Player2_x - Player2_x_dest, Player2_y));
                player2Lines.push_back(newLine);
                for (auto& line : player1Lines) {
                    if (DoLinesIntersectWithTolerance(Point(Player2_x, Player2_y), Point(Player2_x - Player2_x_dest, Player2_y), line.start, line.end)) {
                        Player1_score++;
                        ResetGame(Player1_x, Player1_y, Player2_x, Player2_y, player1Lines, player2Lines);
                        if (Player1_score >= 10) {
                            currentState = GAME_OVER;
                        }
                    }
                }
                Player2_x -= Player2_x_dest;
                player2_last_direction = LEFT;
            }

            // Check for collisions with the wall for Player 2 (out of bounds)
            if (IsOutOfBounds(Player2_x, Player2_y)) {
                Player1_score++;  // Player 1 scores 1 point for Player 2 going out of bounds
                ResetGame(Player1_x, Player1_y, Player2_x, Player2_y, player1Lines, player2Lines);
                if (Player1_score >= 10) {
                    currentState = GAME_OVER;
                }
            }

            break;

        case GAME_OVER:
            // Determine the winner
            if (Player1_score >= 10) {
                DrawText("PLAYER 1 WINS!", (800 - MeasureText("PLAYER 1 WINS!", 40)) / 2, 100, 40, GREEN);
            }
            else {
                DrawText("PLAYER 2 WINS!", (800 - MeasureText("PLAYER 2 WINS!", 40)) / 2, 100, 40, GREEN);
            }
            DrawText(TextFormat("Player 1: %d", Player1_score), (800 - MeasureText(TextFormat("Player 1: %d", Player1_score), 20)) / 2, 200, 20, YELLOW);
            DrawText(TextFormat("Player 2: %d", Player2_score), (800 - MeasureText(TextFormat("Player 2: %d", Player2_score), 20)) / 2, 250, 20, YELLOW);
            DrawText("Press R to Restart", (800 - MeasureText("Press R to Restart", 20)) / 2, 300, 20, WHITE);
            DrawText("Press ESC to Quit", (800 - MeasureText("Press ESC to Quit", 20)) / 2, 350, 20, WHITE);

            // Input for game over state
            if (IsKeyPressed(KEY_R)) {
                currentState = MAIN_MENU; // Go back to the main menu
                Player1_score = 0;
                Player2_score = 0;
                ResetGame(Player1_x, Player1_y, Player2_x, Player2_y, player1Lines, player2Lines);
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                CloseWindow(); // Close the window
            }
            break;
        }

        EndDrawing();
    }

    CloseWindow(); // Close window and OpenGL context
    return 0;
}
