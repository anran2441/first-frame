// L3 任务索引：01 地图/常量；02 CellRect；03 InitCamera；
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
// TODO(L3-01-A): 扩大行数/列数，让地图超出一屏；保留原有墙格。
const int WORLD_ROWS = 200, WORLD_COLS = 120;
const int WORLD_W = WORLD_COLS * TILE, WORLD_H = WORLD_ROWS * TILE;
// 框架提供有效的静态占位相机，尚未实现跟随。
Camera2D camera = {{0, 0}, {0, 0}, 0, 1};
int wall[WORLD_ROWS][WORLD_COLS];
Player player;
Bullet bullets[MAX_BULLETS];
Enemy enemies[MAX_ENEMIES];
int score, spawnTimer, spawnSequence;
bool gameOver;
bool practiceMode = true;
bool debugMode = false;

// 如有需要，可换成你在 L2 完成的配置表。
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

// 前置：任务 01 的网格单位。目标：返回该格在世界坐标中的矩形。
Rectangle CellRect(int row, int col) {
    // TODO(L3-02): 将 row（行）/col（列）换算成 TILE 大小的世界坐标矩形。
    float x = col * TILE;
    float y = row * TILE;
    return {x, y, TILE, TILE};
}

// 前置：任务 02。目标：检测整个身体，而不只是左上角。
bool HitsWall(Rectangle rect) {
    // TODO(L3-06): 检测这个世界坐标矩形是否与任一墙格相交。
    (void)rect;
    return false;
}
// 前置：任务 01。目标：使用世界边界，而不是窗口边界。
bool InsideWorld(Rectangle rect) {
    // TODO(L3-07-A): 让整个矩形都在扩大的世界范围内。
    return rect.x >= 0 && rect.y >= 0 && rect.width >= 0 && rect.height >= 0 &&
        rect.x + rect.width <= SCREEN_W && rect.y + rect.height <= SCREEN_H;
}
// 前置：阅读 Camera2D 字段。目标：定义初始的世界到屏幕视图。
// 前置：阅读 Camera2D 字段。目标：定义初始的世界到屏幕视图。
void InitCamera() {
    // TODO(L3-03): 设置 offset（屏幕偏移）、target（世界目标）、rotation 和 zoom，让玩家居中
    Vector2 playerCenter = {
        player.rect.x + player.rect.width / 2.0f,
        player.rect.y + player.rect.height / 2.0f
    };
    camera.offset = { SCREEN_W / 2.0f, SCREEN_H / 2.0f };
    camera.target = playerCenter;
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}
// 前置：任务 03。目标：移动后跟随玩家身体中心。
void UpdateCamera(){
    // TODO(L3-04): 每一帧更新相机target，跟随玩家中心
    camera.target={
        player.rect.x + player.rect.width / 2.0f,
        player.rect.y + player.rect.height / 2.0f
    };
}

void InitMap() {
    for (int row = 0; row < WORLD_ROWS; ++row)
        for (int col = 0; col < WORLD_COLS; ++col) wall[row][col] = 0;
    wall[3][7] = 1;
    wall[8][10] = 1;
    wall[12][24] = 1;
    for (int row = 5; row <= 11; ++row) wall[row][20] = 1;
    for (int col = 5; col <= 11; ++col) wall[13][col] = 1;
    for(int i=1;i<=18;i++) wall[31][i]=1;
    for(int i=1;i<=32;i++) wall[i][18]=1;
    // TODO(L3-01-B): 扩大网格后，在原窗口范围外放置一格墙。
}
void InitGame() {
    InitMap();
    player = {{SCREEN_W / 2.0f - 20, SCREEN_H / 2.0f - 20, 40, 40}, 5, 100};
    for (int i = 0; i < MAX_BULLETS; ++i) bullets[i] = {};
    for (int i = 0; i < MAX_ENEMIES; ++i)
        enemies[i] = {{0, 0, 0, 0}, EnemyKind::Grunt, 0, false};
    score = 0; spawnTimer = 0; spawnSequence = 0; gameOver = false;
    InitCamera(); // 框架已接好调用：ENTER/TAB 重新开始时也会执行。
}
// 前置：任务 06 和 07-A。目标：分轴试走，实现沿墙滑动。
void UpdatePlayer() {
    // TODO(L3-07-B): 先试候选 X，再从已接受的 X 试候选 Y；检查墙和世界边界。
    if (IsKeyDown(KEY_D)) player.rect.x += player.speed;
    if (IsKeyDown(KEY_A)) player.rect.x -= player.speed;
    if (IsKeyDown(KEY_S)) player.rect.y += player.speed;
    if (IsKeyDown(KEY_W)) player.rect.y -= player.speed;
}
// 前置：已有的音频生命周期。目标：每次成功分配子弹池槽位时播放一次声音。
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
                // TODO(L3-08): 仅在成功创建子弹后播放射击音效。
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
        // 框架维护的 L2 逻辑：在附近环形区域生成，沿用对象池/计时器/类型序列。
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
            // 即使尚未完成碰撞任务，生成位置仍须安全。
            bool blocked = false;
            int cells = cfg.size > TILE ? 2 : 1;
            for (int r = row; r < row + cells; ++r)
                for (int c = col; c < col + cells; ++c)
                    if (wall[r][c]) blocked = true;
            if (blocked || HitsWall(box)) continue;
            enemies[i] = {box, kind, cfg.maxHp, true};
            return;
        }
        spawnSequence = previousSequence; // 只有成功生成敌人才消耗一个类型。
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
// 前置：已有的命中处理。目标：每次实际扣除 HP 时播放一次声音。
void HandleHits() {
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (!bullets[i].active) continue;
        for (int j = 0; j < MAX_ENEMIES; ++j) {
            if (!enemies[j].active) continue;
            if (CheckCollisionRecs(bullets[i].rect, enemies[j].rect)) {
                bullets[i].active = false;
                enemies[j].hp -= 1;
                // TODO(L3-09): 每次实际命中都播放命中音效，不只是敌人死亡时。
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
// 框架已分开绘制：这里只画世界中的轮廓；文字仍由相机模式外的 DrawCase 绘制。
void DrawCaseWorld(CollisionCase test) {
    if (debugMode && test.enabled) DrawRectangleLinesEx(test.rect, 2, ORANGE);
}
// 前置：任务 03/04。目标：世界随视图移动，HUD 固定在窗口上。
void DrawGame() {
    // TODO(L3-05): 仅给世界绘制加上相机模式，并在绘制 HUD 前结束。
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // ===== 开启相机变换：世界物体全部写在这一对中间 =====
    BeginMode2D(camera);
    {
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
    }
    EndMode2D();
    // ===== 相机结束，下面全部是屏幕HUD，不跟随镜头 =====

    // 固定的 HUD 从这里开始。
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
        UpdateCamera(); // 框架已接好调用：移动后、每次绘制前执行，重新开始时也不例外。
        UpdateGameAudio(gameOver);
        DrawGame();
    }
    UnloadGameAudio();
    UnloadAssets();
    CloseWindow();
    return 0;
}
#endif
