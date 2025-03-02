#include "raylib.h"
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

struct Point {
    int x, y;

    // Constructor to initialize Point with x and y values
    Point(int x_val, int y_val) : x(x_val), y(y_val) {}
};

struct Line {
    Point start, end;

    // Constructor to initialize Line with two Points (start and end)
    Line(Point start_val, Point end_val) : start(start_val), end(end_val) {}
};

// Direction enum to track previous movement
enum Direction { NONE, UP, DOWN, LEFT, RIGHT };

bool DoLinesIntersect(Point p1, Point p2, Point p3, Point p4) {
    auto cross = [](Point p, Point q, Point r) {
        return (q.x - p.x) * (r.y - p.y) - (q.y - p.y) * (r.x - p.x);
        };

    int d1 = cross(p3, p4, p1);
    int d2 = cross(p3, p4, p2);
    int d3 = cross(p1, p2, p3);
    int d4 = cross(p1, p2, p4);

    // Lines are intersecting if and only if the directions change signs
    return (d1 * d2 < 0 && d3 * d4 < 0);
}

// Function to check if two line segments are close enough to intersect with a tolerance
bool DoLinesIntersectWithTolerance(Point p1, Point p2, Point p3, Point p4, int tolerance = 5) {
    // Check bounding box overlap with tolerance
    if (max(p1.x, p2.x) < min(p3.x, p4.x) - tolerance || max(p3.x, p4.x) < min(p1.x, p2.x) - tolerance)
        return false;
    if (max(p1.y, p2.y) < min(p3.y, p4.y) - tolerance || max(p3.y, p4.y) < min(p1.y, p2.y) - tolerance)
        return false;

    return DoLinesIntersect(p1, p2, p3, p4);
}

// Function to reset the game state, including player positions and lines (but not scores)
void ResetGame(int& Player1_x, int& Player1_y, int& Player2_x, int& Player2_y,
    vector<Line>& player1Lines, vector<Line>& player2Lines) {
    // Reset player positions and lines
    Player1_x = rand() % 801;
    Player1_y = rand() % 601;
    Player2_x = rand() % 801;
    Player2_y = rand() % 601;
    player1Lines.clear();
    player2Lines.clear();
}

// Check if the player's position is out of bounds
bool IsOutOfBounds(int x, int y) {
    return (x < 0 || x > 800 || y < 0 || y > 600);
}

int main() {
    srand(time(NULL));
    int Player1_x = rand() % 801;
    int Player1_y = rand() % 601;
    int Player1_x_dest, Player1_y_dest;
    int Player2_x = rand() % 801;
    int Player2_y = rand() % 601;
    int Player2_x_dest = 0, Player2_y_dest = 0;
    vector<Line> player1Lines;
    vector<Line> player2Lines;
    int Player1_score = 0;
    int Player2_score = 0;

    Direction player1_last_direction = NONE;
    Direction player2_last_direction = NONE;

    bool gamePaused = false;
    float pauseTimer = 0.0f;

    InitWindow(800, 600, "Lines");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (gamePaused) {
            pauseTimer += GetFrameTime();
            if (pauseTimer >= 3.0f) {
                gamePaused = false; // Unpause after 3 seconds
                pauseTimer = 0.0f;
                // Don't reset the scores, just reset positions and lines
                ResetGame(Player1_x, Player1_y, Player2_x, Player2_y, player1Lines, player2Lines);
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);
        DrawText(TextFormat("Player 1: %d", Player1_score), 10, 10, 20, YELLOW);
        DrawText(TextFormat("Player 2: %d", Player2_score), 10, 50, 20, YELLOW);
        DrawText("Lines", 350, 10, 40, YELLOW);
        DrawRectangle(Player1_x, Player1_y, 3, 3, RED);
        DrawRectangle(Player2_x, Player2_y, 3, 3, GREEN);

        // Draw Player 1's lines
        for (auto& line : player1Lines)
            DrawLine(line.start.x, line.start.y, line.end.x, line.end.y, RED);

        // Draw Player 2's lines
        for (auto& line : player2Lines)
            DrawLine(line.start.x, line.start.y, line.end.x, line.end.y, GREEN);

        if (gamePaused) {
            DrawText("Restarting in 3 seconds", 250, 300, 20, YELLOW);
            EndDrawing();
            continue;  // Skip the rest of the frame if the game is paused
        }

        // Movement and collision checks for Player 1
        if (IsKeyPressed(KEY_W) && player1_last_direction != DOWN) {
            Player1_y_dest = rand() % 300;
            Line newLine(Point(Player1_x, Player1_y), Point(Player1_x, Player1_y - Player1_y_dest));
            player1Lines.push_back(newLine);
            for (auto& line : player2Lines) {
                if (DoLinesIntersectWithTolerance(Point(Player1_x, Player1_y), Point(Player1_x, Player1_y - Player1_y_dest), line.start, line.end)) {
                    Player2_score++;
                    gamePaused = true;
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
                    gamePaused = true;
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
                    gamePaused = true;
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
                    gamePaused = true;
                }
            }
            Player1_x -= Player1_x_dest;
            player1_last_direction = LEFT;
        }

        // Check for collisions with the wall for Player 1
        if (IsOutOfBounds(Player1_x, Player1_y)) {
            Player2_score++;
            gamePaused = true;
            ResetGame(Player1_x, Player1_y, Player2_x, Player2_y, player1Lines, player2Lines);
        }

        // Movement and collision checks for Player 2
        if (IsKeyPressed(KEY_I) && player2_last_direction != DOWN) {
            Player2_y_dest = rand() % 300;
            Line newLine(Point(Player2_x, Player2_y), Point(Player2_x, Player2_y - Player2_y_dest));
            player2Lines.push_back(newLine);
            for (auto& line : player1Lines) {
                if (DoLinesIntersectWithTolerance(Point(Player2_x, Player2_y), Point(Player2_x, Player2_y - Player2_y_dest), line.start, line.end)) {
                    Player1_score++;
                    gamePaused = true;
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
                    gamePaused = true;
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
                    gamePaused = true;
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
                    gamePaused = true;
                }
            }
            Player2_x -= Player2_x_dest;
            player2_last_direction = LEFT;
        }

        // Check for collisions with the wall for Player 2
        if (IsOutOfBounds(Player2_x, Player2_y)) {
            Player1_score++;
            gamePaused = true;
            ResetGame(Player1_x, Player1_y, Player2_x, Player2_y, player1Lines, player2Lines);
        }

        // Check if the 'R' key is pressed to reset the game and lose scores
        if (IsKeyPressed(KEY_R)) {
            Player1_score = 0;
            Player2_score = 0;
            ResetGame(Player1_x, Player1_y, Player2_x, Player2_y, player1Lines, player2Lines);
        }

        EndDrawing();
    }

    CloseWindow(); // Close window and OpenGL context

    return 0;
}
