#include "raylib.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>

// Add these lines at the top after includes
Sound moveSound;
Sound menuSound;
Sound loseSound;
bool soundsLoaded = false;

using namespace std;

// Constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int WIN_SCORE = 10;
const int PLAYER_SIZE = 3;
const int MOVE_MAX = 150;

struct Point {
    int x, y;
    Point(int x_val, int y_val) : x(x_val), y(y_val) {}
};

struct Line {
    Point start, end;
    Line(Point start_val, Point end_val) : start(start_val), end(end_val) {}
};

enum Direction { NONE, UP, DOWN, LEFT, RIGHT };
enum GameState { MAIN_MENU_1, PLAYING_BLITZ, GAME_OVER, MAIN_MENU_2, PLAYING_TURN };

struct Player {
    int x, y;
    int x_dest, y_dest;
    int score;
    Direction last_direction;
    vector<Line> lines;
    Player() : x(0), y(0), x_dest(0), y_dest(0), score(0), last_direction(NONE) {}
};

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

void ResetGame(Player& p1, Player& p2) {
    p1.x = rand() % (WINDOW_WIDTH + 1);
    p1.y = rand() % (WINDOW_HEIGHT + 1);
    p2.x = rand() % (WINDOW_WIDTH + 1);
    p2.y = rand() % (WINDOW_HEIGHT + 1);
    p1.lines.clear();
    p2.lines.clear();
    p1.last_direction = NONE;
    p2.last_direction = NONE;
}

bool IsOutOfBounds(int x, int y) {
    return (x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT);
}

void DrawMainMenu() {
    DrawText("LINEAZ!", (WINDOW_WIDTH - MeasureText("LINEAZ!", 40)) / 2, 100, 40, YELLOW);
    DrawText("Press 1 to Play Blitz!", (WINDOW_WIDTH - MeasureText("Press 1 to Play Blitz!", 20)) / 2, 200, 20, WHITE);
    DrawText("Press 2 to Take Turns", (WINDOW_WIDTH - MeasureText("Press 2 to Take Turns", 20)) / 2, 250, 20, WHITE);
    DrawText("Press ESC to Quit", (WINDOW_WIDTH - MeasureText("Press ESC to Quit", 20)) / 2, 300, 20, WHITE);
    DrawText("Press o for Options", (WINDOW_WIDTH - MeasureText("Press o for Options", 20)) / 2, 350, 20, WHITE);
}

void DrawOptionsMenu() {
    DrawText("LINEAZ!", (WINDOW_WIDTH - MeasureText("LINEAZ!", 40)) / 2, 100, 40, YELLOW);
    DrawText("The game is simple:", (WINDOW_WIDTH - MeasureText("The game is simple:", 20)) / 2, 200, 20, WHITE);
    DrawText("First Player to reach 10 points wins!", (WINDOW_WIDTH - MeasureText("First Player to reach 10 points wins!", 20)) / 2, 230, 20, WHITE);
    DrawText("Player 1 moves with wasd, Player 2 with ijkl.", (WINDOW_WIDTH - MeasureText("Player 1 moves with wasd, Player 2 with ijkl.", 20)) / 2, 260, 20, WHITE);
    DrawText("In 'Take Turns' mode, as the name suggests, you take turns.", (WINDOW_WIDTH - MeasureText("In 'Take Turns' mode, as the name suggests, you take turns.", 20)) / 2, 290, 20, WHITE);
    DrawText("In BLITZ! mode both players move simultaneously, so get ready for chaos!", (WINDOW_WIDTH - MeasureText("In BLITZ! mode both players move simultaneously, so get ready for chaos!", 20)) / 2, 320, 20, WHITE);
    DrawText("Return to the Main Menu with o", (WINDOW_WIDTH - MeasureText("Return to the Main Menu with o", 20)) / 2, 350, 20, WHITE);
}

void DrawGame(const Player& p1, const Player& p2) {
    DrawText(TextFormat("Player 1: %d", p1.score), 10, 10, 20, YELLOW);
    DrawText(TextFormat("Player 2: %d", p2.score), 10, 50, 20, YELLOW);
    DrawText("LINEAZ!", (WINDOW_WIDTH - MeasureText("LINEAZ!", 40)) / 2, 10, 40, YELLOW);
    DrawRectangle(p1.x, p1.y, PLAYER_SIZE, PLAYER_SIZE, RED);
    DrawRectangle(p2.x, p2.y, PLAYER_SIZE, PLAYER_SIZE, GREEN);
    for (const auto& line : p1.lines)
        DrawLine(line.start.x, line.start.y, line.end.x, line.end.y, RED);
    for (const auto& line : p2.lines)
        DrawLine(line.start.x, line.start.y, line.end.x, line.end.y, GREEN);
}

void DrawGameOver(int p1_score, int p2_score) {
    if (p1_score >= WIN_SCORE)
        DrawText("PLAYER 1 WINS!", (WINDOW_WIDTH - MeasureText("PLAYER 1 WINS!", 40)) / 2, 100, 40, GREEN);
    else
        DrawText("PLAYER 2 WINS!", (WINDOW_WIDTH - MeasureText("PLAYER 2 WINS!", 40)) / 2, 100, 40, GREEN);
    DrawText(TextFormat("Player 1: %d", p1_score), (WINDOW_WIDTH - MeasureText(TextFormat("Player 1: %d", p1_score), 20)) / 2, 200, 20, YELLOW);
    DrawText(TextFormat("Player 2: %d", p2_score), (WINDOW_WIDTH - MeasureText(TextFormat("Player 2: %d", p2_score), 20)) / 2, 250, 20, YELLOW);
    DrawText("Press R to Restart", (WINDOW_WIDTH - MeasureText("Press R to Restart", 20)) / 2, 300, 20, WHITE);
    DrawText("Press ESC to Quit", (WINDOW_WIDTH - MeasureText("Press ESC to Quit", 20)) / 2, 350, 20, WHITE);
}

// Helper for movement and collision
bool HandleMove(Player& mover, Player& opponent, Direction move_dir, Direction forbidden_dir, int dx, int dy, GameState& currentState) {
    if (mover.last_direction == forbidden_dir) return false;
    int dest = rand() % MOVE_MAX;
    Point start(mover.x, mover.y);
    Point end(mover.x + dx * dest, mover.y + dy * dest);
    mover.lines.push_back(Line(start, end));
    for (const auto& line : opponent.lines) {
        if (DoLinesIntersectWithTolerance(start, end, line.start, line.end)) {
            PlaySound(loseSound); // Play lose sound on collision
            opponent.score++;
            ResetGame(mover, opponent);
            if (opponent.score >= WIN_SCORE) {
                currentState = GAME_OVER;
            }
            return true;
        }
    }
    mover.x = end.x;
    mover.y = end.y;
    mover.last_direction = move_dir;
    PlaySound(moveSound); // Play move sound on successful move
    return false;
}

int main() {
    srand(time(NULL));
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "LINEAZ!");
    SetTargetFPS(60);

    // Initialize audio device and load sounds
    InitAudioDevice();
    moveSound = LoadSound("move.wav");
    menuSound = LoadSound("menu.wav");
    loseSound = LoadSound("lose.wav");
    SetMasterVolume(1.0f);

    GameState currentState = MAIN_MENU_1;
    Player player1, player2;
    int Player_turn = 1;
    int turn_counter = 0;

    ResetGame(player1, player2);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        switch (currentState) {
        case MAIN_MENU_1:
            DrawMainMenu();
            if (IsKeyPressed(KEY_ONE)) {
                currentState = PLAYING_BLITZ;
                player1.score = player2.score = 0;
                ResetGame(player1, player2);
                Player_turn = 1;
                turn_counter = 0;
                PlaySound(menuSound); // Play menu sound
            }
            if (IsKeyPressed(KEY_TWO)) {
                currentState = PLAYING_TURN;
                player1.score = player2.score = 0;
                ResetGame(player1, player2);
                Player_turn = 1;
                turn_counter = 0;
                PlaySound(menuSound); // Play menu sound
            }
            if (IsKeyPressed(KEY_O)) {
                currentState = MAIN_MENU_2;
                PlaySound(menuSound); // Play menu sound
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                PlaySound(menuSound); // Play menu sound
                CloseWindow();
            }
            break;

        case MAIN_MENU_2:
            DrawOptionsMenu();
            if (IsKeyPressed(KEY_O)) {
                currentState = MAIN_MENU_1;
                PlaySound(menuSound); // Play menu sound
            }
            break;

        case PLAYING_BLITZ:
            DrawGame(player1, player2);

            // Player 1 movement
            if (IsKeyPressed(KEY_W)) HandleMove(player1, player2, UP, DOWN, 0, -1, currentState);
            if (IsKeyPressed(KEY_S)) HandleMove(player1, player2, DOWN, UP, 0, 1, currentState);
            if (IsKeyPressed(KEY_D)) HandleMove(player1, player2, RIGHT, LEFT, 1, 0, currentState);
            if (IsKeyPressed(KEY_A)) HandleMove(player1, player2, LEFT, RIGHT, -1, 0, currentState);

            // Player 2 movement
            if (IsKeyPressed(KEY_I)) HandleMove(player2, player1, UP, DOWN, 0, -1, currentState);
            if (IsKeyPressed(KEY_K)) HandleMove(player2, player1, DOWN, UP, 0, 1, currentState);
            if (IsKeyPressed(KEY_L)) HandleMove(player2, player1, RIGHT, LEFT, 1, 0, currentState);
            if (IsKeyPressed(KEY_J)) HandleMove(player2, player1, LEFT, RIGHT, -1, 0, currentState);

            // Out of bounds checks
            if (IsOutOfBounds(player1.x, player1.y)) {
                player2.score++;
                ResetGame(player1, player2);
                PlaySound(loseSound); // Play lose sound
                if (player2.score >= WIN_SCORE) currentState = GAME_OVER;
            }
            if (IsOutOfBounds(player2.x, player2.y)) {
                player1.score++;
                ResetGame(player1, player2);
                PlaySound(loseSound); // Play lose sound
                if (player1.score >= WIN_SCORE) currentState = GAME_OVER;
            }
            break;

        case PLAYING_TURN:
            DrawGame(player1, player2);
            if (Player_turn == 1) {
                if (IsKeyPressed(KEY_W) && player1.last_direction != DOWN) {
                    if (!HandleMove(player1, player2, UP, DOWN, 0, -1, currentState)) {
                        player1.last_direction = UP;
                        Player_turn = 0;
                    }
                }
                if (IsKeyPressed(KEY_S) && player1.last_direction != UP) {
                    if (!HandleMove(player1, player2, DOWN, UP, 0, 1, currentState)) {
                        player1.last_direction = DOWN;
                        Player_turn = 0;
                    }
                }
                if (IsKeyPressed(KEY_D) && player1.last_direction != LEFT) {
                    if (!HandleMove(player1, player2, RIGHT, LEFT, 1, 0, currentState)) {
                        player1.last_direction = RIGHT;
                        Player_turn = 0;
                    }
                }
                if (IsKeyPressed(KEY_A) && player1.last_direction != RIGHT) {
                    if (!HandleMove(player1, player2, LEFT, RIGHT, -1, 0, currentState)) {
                        player1.last_direction = LEFT;
                        Player_turn = 0;
                    }
                }
                if (IsOutOfBounds(player1.x, player1.y)) {
                    player2.score++;
                    ResetGame(player1, player2);
                    PlaySound(loseSound); // Play lose sound
                    if (player2.score >= WIN_SCORE) currentState = GAME_OVER;
                }
            } else {
                if (IsKeyPressed(KEY_I) && player2.last_direction != DOWN) {
                    if (!HandleMove(player2, player1, UP, DOWN, 0, -1, currentState)) {
                        player2.last_direction = UP;
                        Player_turn = 1;
                    }
                }
                if (IsKeyPressed(KEY_K) && player2.last_direction != UP) {
                    if (!HandleMove(player2, player1, DOWN, UP, 0, 1, currentState)) {
                        player2.last_direction = DOWN;
                        Player_turn = 1;
                    }
                }
                if (IsKeyPressed(KEY_L) && player2.last_direction != LEFT) {
                    if (!HandleMove(player2, player1, RIGHT, LEFT, 1, 0, currentState)) {
                        player2.last_direction = RIGHT;
                        Player_turn = 1;
                    }
                }
                if (IsKeyPressed(KEY_J) && player2.last_direction != RIGHT) {
                    if (!HandleMove(player2, player1, LEFT, RIGHT, -1, 0, currentState)) {
                        player2.last_direction = LEFT;
                        Player_turn = 1;
                    }
                }
                if (IsOutOfBounds(player2.x, player2.y)) {
                    player1.score++;
                    ResetGame(player1, player2);
                    PlaySound(loseSound); // Play lose sound
                    if (player1.score >= WIN_SCORE) currentState = GAME_OVER;
                }
            }
            break;

        case GAME_OVER:
            DrawGameOver(player1.score, player2.score);
            if (IsKeyPressed(KEY_R)) {
                currentState = MAIN_MENU_1;
                player1.score = player2.score = 0;
                ResetGame(player1, player2);
                Player_turn = 1;
                turn_counter = 0;
                PlaySound(menuSound); // Play menu sound
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                PlaySound(menuSound); // Play menu sound
                CloseWindow();
            }
            break;
        }

        EndDrawing();
    }

    // Unload sounds and close audio device before exiting
    if (soundsLoaded) {
        UnloadSound(moveSound);
        UnloadSound(menuSound);
        UnloadSound(loseSound);
        CloseAudioDevice();
    }

    CloseWindow();
    return 0;   
}