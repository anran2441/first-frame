// L3 task index: 01 map/constants; 02 CellRect; 03 InitCamera;
// 04 UpdateCamera; 05 DrawGame; 06 HitsWall; 07 InsideWorld/UpdatePlayer;
// 08 FireBullets; 09 HandleHits; 10 my_cases.h.
#include "raylib.h"
#include "assets.h"
#include "audio.h"
#include "my_cases.h"

struct Player { Rectangle rect; float speed; int hp; };
struct Bullet { Rectangle rect; float vx, vy; bool active; };
enum class EnemyKind { Grunt, Runner, Heavy, Elite };
struct EnemyConfig { SpriteId sprite; float speed; int maxHp; int score; float size; };
struct Enemy { Rectangle rect; EnemyKind kind; int hp; bool active; };
const int MAX_BULLETS = 128, MAX_ENEMIES = 64;
const int SCREEN_W = 1280, SCREEN_H = 720, TILE = 40;
// TODO(L3-01-A): Expand rows/columns beyond one screen; keep the original walls valid.
const int WORLD_ROWS = 18, WORLD_COLS = 32;
const int WORLD_W = WORLD_COLS * TILE, WORLD_H = WORLD_ROWS * TILE;
// Teacher wiring: a valid static placeholder, not a following camera.
Camera2D camera = {{0, 0}, {0, 0}, 0, 1};
int wall[WORLD_ROWS][WORLD_COLS];
Player player;
Bullet bullets[MAX_BULLETS];
Enemy enemies[MAX_ENEMIES];
int score, spawnTimer, spawnSequence;
bool gameOver;
bool practiceMode = true;
bool debugMode = false;

// Replace this table with your completed L2 table if desired.
EnemyConfig ConfigOf(EnemyKind kind) {
    switch (kind) {
        case EnemyKind::Grunt: return {SpriteId::Grunt, 1.8f, 1, 10, 40};
        case EnemyKind::Runner: return {SpriteId::Runner, 3.4f, 1, 15, 34};
        case EnemyKind::Heavy: return {SpriteId::Heavy, 1.0f, 3, 30, 48};
        case EnemyKind::Elite: return {SpriteId::Elite, 2.0f, 6, 80, 56};
    }
    return {SpriteId::Grunt, 1.8f, 1, 10, 40};
}
EnemyKind NextKind() {
    static const EnemyKind sequence[] = {EnemyKind::Grunt, EnemyKind::Runner,
        EnemyKind::Grunt, EnemyKind::Heavy, EnemyKind::Runner, EnemyKind::Elite};
    EnemyKind kind = sequence[spawnSequence % 6];
    spawnSequence = (spawnSequence + 1) % 6;
    return kind;
}

// Prerequisite: task 01 grid units. Goal: return this cell's world-space box.
Rectangle CellRect(int row, int col) {
    // TODO(L3-02): Convert row/column to a TILE-sized world rectangle.
    (void)row; (void)col;
    return {0, 0, 0, 0};
}
// Prerequisite: task 02. Goal: test the whole body, not only its top-left point.
bool HitsWall(Rectangle rect) {
    // TODO(L3-06): Test this world rectangle against every occupied cell.
    (void)rect;
    return false;
}
// Prerequisite: task 01. Goal: use world limits rather than the window limits.
bool InsideWorld(Rectangle rect) {
    // TODO(L3-07-A): Keep the whole rectangle inside the expanded world.
    return rect.x >= 0 && rect.y >= 0 && rect.width >= 0 && rect.height >= 0 &&
        rect.x + rect.width <= SCREEN_W && rect.y + rect.height <= SCREEN_H;
}
// Prerequisite: read Camera2D fields. Goal: define the initial world-to-screen view.
void InitCamera() {
    // TODO(L3-03): Set offset, target, rotation and zoom for a centered player view.
}
// Prerequisite: task 03. Goal: follow the player's body center after movement.
void UpdateCamera() {
    // TODO(L3-04): Update the world-space camera target from the player rectangle.
}
void InitMap() {
    for (int row = 0; row < WORLD_ROWS; ++row)
        for (int col = 0; col < WORLD_COLS; ++col) wall[row][col] = 0;
    wall[3][7] = 1;
    wall[8][10] = 1;
    wall[12][24] = 1;
    for (int row = 5; row <= 11; ++row) wall[row][20] = 1;
    for (int col = 5; col <= 11; ++col) wall[13][col] = 1;
    // TODO(L3-01-B): After expanding the grid, place a wall beyond the old window.
}
void InitGame() {
    InitMap();
    player = {{SCREEN_W / 2.0f - 20, SCREEN_H / 2.0f - 20, 40, 40}, 5, 100};
    for (int i = 0; i < MAX_BULLETS; ++i) bullets[i] = {};
    for (int i = 0; i < MAX_ENEMIES; ++i)
        enemies[i] = {{0, 0, 0, 0}, EnemyKind::Grunt, 0, false};
    score = 0; spawnTimer = 0; spawnSequence = 0; gameOver = false;
    InitCamera(); // Teacher wiring: also runs on ENTER/TAB restart.
}
// Prerequisites: tasks 06 and 07-A. Goal: slide along walls using separate axis trials.
void UpdatePlayer() {
    // TODO(L3-07-B): Try X, then Y from accepted X; check walls and world bounds.
    if (IsKeyDown(KEY_D)) player.rect.x += player.speed;
    if (IsKeyDown(KEY_A)) player.rect.x -= player.speed;
    if (IsKeyDown(KEY_S)) player.rect.y += player.speed;
    if (IsKeyDown(KEY_W)) player.rect.y -= player.speed;
}
// Prerequisite: existing audio lifecycle. Goal: one sound per successful pool allocation.
void FireBullets() {
    int vx = 0, vy = 0;
    if (IsKeyPressed(KEY_RIGHT)) vx = 9;
    if (IsKeyPressed(KEY_LEFT)) vx = -9;
    if (IsKeyPressed(KEY_UP)) vy = -9;
    if (IsKeyPressed(KEY_DOWN)) vy = 9;
    if (vx != 0 || vy != 0)
        for (int i = 0; i < MAX_BULLETS; ++i)
            if (!bullets[i].active) {
                bullets[i].rect = {player.rect.x + player.rect.width / 2 - 5,
                                  player.rect.y + player.rect.height / 2 - 5, 10, 10};
                bullets[i].vx = (float)vx; bullets[i].vy = (float)vy;
                bullets[i].active = true;
                // TODO(L3-08): Play the shot sound only after a bullet is created.
                break;
            }
}
void UpdateBullets() {
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (!bullets[i].active) continue;
        bullets[i].rect.x += bullets[i].vx;
        bullets[i].rect.y += bullets[i].vy;
        if (!InsideWorld(bullets[i].rect) || HitsWall(bullets[i].rect))
            bullets[i].active = false;
    }
}
void SpawnEnemies() {
    if (practiceMode) return;
    if (++spawnTimer < 55) return;
    spawnTimer = 0;
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (enemies[i].active) continue;
        const int previousSequence = spawnSequence;
        EnemyKind kind = NextKind();
        EnemyConfig cfg = ConfigOf(kind);
        // Teacher-owned L2 maintenance: nearby ring, same pool/timer/kind sequence.
        int first = GetRandomValue(0, 142);
        int playerCol = (int)(player.rect.x / TILE);
        int playerRow = (int)(player.rect.y / TILE);
        for (int offset = 0; offset < 143; ++offset) {
            int cell = (first + offset) % 143;
            int dx = cell % 13 - 6, dy = cell / 13 - 5;
            if (dx != -6 && dx != 6 && dy != -5 && dy != 5) continue;
            int col = playerCol + dx, row = playerRow + dy;
            Rectangle box = {(float)(col * TILE), (float)(row * TILE), cfg.size, cfg.size};
            if (col < 0 || row < 0 || box.x + box.width > WORLD_W ||
                box.y + box.height > WORLD_H || CheckCollisionRecs(box, player.rect)) continue;
            // Spawn safety stays valid even before the student's collision task is done.
            bool blocked = false;
            int cells = cfg.size > TILE ? 2 : 1;
            for (int r = row; r < row + cells; ++r)
                for (int c = col; c < col + cells; ++c)
                    if (wall[r][c]) blocked = true;
            if (blocked || HitsWall(box)) continue;
            enemies[i] = {box, kind, cfg.maxHp, true};
            return;
        }
        spawnSequence = previousSequence; // Only successful spawns consume a kind.
        return;
    }
}
void UpdateEnemies() {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].active) continue;
        float speed = ConfigOf(enemies[i].kind).speed;
        Rectangle next = enemies[i].rect;
        if (player.rect.x > next.x) next.x += speed;
        else if (player.rect.x < next.x) next.x -= speed;
        if (InsideWorld(next) && !HitsWall(next)) enemies[i].rect = next;
        next = enemies[i].rect;
        if (player.rect.y > next.y) next.y += speed;
        else if (player.rect.y < next.y) next.y -= speed;
        if (InsideWorld(next) && !HitsWall(next)) enemies[i].rect = next;
        if (CheckCollisionRecs(enemies[i].rect, player.rect)) {
            player.hp -= 10;
            enemies[i].active = false;
            if (player.hp <= 0) gameOver = true;
        }
    }
}
// Prerequisite: existing hit handling. Goal: one sound per actual HP decrement.
void HandleHits() {
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (!bullets[i].active) continue;
        for (int j = 0; j < MAX_ENEMIES; ++j) {
            if (!enemies[j].active) continue;
            if (CheckCollisionRecs(bullets[i].rect, enemies[j].rect)) {
                bullets[i].active = false;
                enemies[j].hp -= 1;
                // TODO(L3-09): Play the hit sound for every actual hit, not just deaths.
                if (enemies[j].hp <= 0) {
                    enemies[j].active = false;
                    score += ConfigOf(enemies[j].kind).score;
                }
                break;
            }
        }
    }
}
void DrawFloor() {
    for (int row = 0; row < WORLD_ROWS; ++row)
        for (int col = 0; col < WORLD_COLS; ++col) {
            Rectangle box = {(float)(col * TILE), (float)(row * TILE), (float)TILE, (float)TILE};
            DrawSprite(SpriteId::Floor, box);
            if (wall[row][col]) DrawSprite(SpriteId::Wall, box);
        }
}
void DrawCase(CollisionCase test, int y) {
    if (!test.enabled) {
        DrawText("My case: not set (edit src/my_cases.h)", 16, y, 18, GRAY);
        return;
    }
    bool actual = HitsWall(test.rect);
    DrawText(TextFormat("%s: expected %s / actual %s", test.name,
        test.expected ? "wall" : "clear", actual ? "wall" : "clear"),
        16, y, 18, actual == test.expected ? DARKGREEN : MAROON);
}
// Teacher split: world-only outline; text remains in DrawCase outside camera mode.
void DrawCaseWorld(CollisionCase test) {
    if (debugMode && test.enabled) DrawRectangleLinesEx(test.rect, 2, ORANGE);
}
// Prerequisites: tasks 03/04. Goal: world follows the view, HUD stays on the window.
void DrawGame() {
    // TODO(L3-05): Add camera mode around world drawing only, ending before the HUD.
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawFloor();
    if (debugMode) {
        for (int col = 0; col <= WORLD_COLS; ++col)
            DrawLine(col * TILE, 0, col * TILE, WORLD_H, Fade(DARKGRAY, 0.35f));
        for (int row = 0; row <= WORLD_ROWS; ++row)
            DrawLine(0, row * TILE, WORLD_W, row * TILE, Fade(DARKGRAY, 0.35f));
    }
    DrawSprite(SpriteId::Player, player.rect);
    if (debugMode) DrawRectangleLinesEx(player.rect, 2, BLUE);
    for (int i = 0; i < MAX_BULLETS; ++i)
        if (bullets[i].active) {
            DrawSprite(SpriteId::Bullet, bullets[i].rect);
            if (debugMode) DrawRectangleLinesEx(bullets[i].rect, 1, RED);
        }
    for (int i = 0; i < MAX_ENEMIES; ++i)
        if (enemies[i].active) {
            DrawSprite(ConfigOf(enemies[i].kind).sprite, enemies[i].rect);
            if (debugMode) DrawRectangleLinesEx(enemies[i].rect, 2, RED);
        }
    DrawCaseWorld({"Inside wall", {280, 120, 40, 40}, true, true});
    DrawCaseWorld({"Body overlaps", {250, 120, 40, 40}, true, true});
    DrawCaseWorld(myCase);
    // Fixed HUD starts here.
    DrawRectangle(0, 0, SCREEN_W, 112, Fade(RAYWHITE, 0.92f));
    DrawText(TextFormat("HP: %d  Score: %d  Mode: %s", player.hp, score,
        practiceMode ? "PRACTICE (no spawning)" : "COMBAT"), 16, 10, 22, DARKGRAY);
    DrawText("WASD move | Arrows shoot | TAB practice/combat | G grid/bodies | ENTER restart", 16, 38, 18, DARKGRAY);
    DrawCase({"Inside wall", {280, 120, 40, 40}, true, true}, 62);
    DrawCase({"Body overlaps", {250, 120, 40, 40}, true, true}, 84);
    DrawRectangle(0, SCREEN_H - 54, SCREEN_W, 54, Fade(RAYWHITE, 0.92f));
    DrawCase(myCase, SCREEN_H - 48);
    if (!GameAudio::ready) DrawText("Audio unavailable: check device and assets/audio WAV files", 16, SCREEN_H - 25, 18, MAROON);
    if (gameOver) {
        DrawRectangle(390, 280, 500, 150, Fade(RAYWHITE, 0.95f));
        DrawText("GAME OVER", 460, 300, 48, MAROON);
        DrawText("Press ENTER to restart", 490, 370, 24, DARKGRAY);
    }
    EndDrawing();
}
#ifndef UNIT_TEST
int main() {
    InitWindow(SCREEN_W, SCREEN_H, "Mini Roguelike - Walls and Sound");
    SetTargetFPS(60);
    LoadAssets();
    LoadGameAudio();
    InitGame();
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_TAB)) { practiceMode = !practiceMode; InitGame(); }
        if (IsKeyPressed(KEY_G)) debugMode = !debugMode;
        if (IsKeyPressed(KEY_ENTER)) InitGame();
        if (!gameOver) {
            UpdatePlayer(); FireBullets(); UpdateBullets();
            SpawnEnemies(); UpdateEnemies(); HandleHits();
        }
        UpdateCamera(); // Teacher wiring: after movement and before every draw, including restarts.
        UpdateGameAudio(gameOver);
        DrawGame();
    }
    UnloadGameAudio();
    UnloadAssets();
    CloseWindow();
    return 0;
}
#endif
