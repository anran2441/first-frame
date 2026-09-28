// ============================================================
//  迷你肉鸽 · 第 2 课「怪物图鉴」
//  新本领：类型(enum) + 配置表(数据驱动)。让敌人分「普通/快速/重甲/精英」，
//  每种的速度/血量/分数/长相全查一张「配置表」——想改强弱，只改表，逻辑不动。
//  操作：WASD 移动，方向键射击，死后 Enter 重开。
//
//  你要填 6 个 ★TODO（顺序做，做一个 → make → 玩一下 / make test → 再下一个）：
//    ① 玩家和子弹改用贴图画      ② 把配置表填全（快速/重甲/精英三行）
//    ③ 刷怪时记住种类 + 设满血    ④ 敌人速度改成按种类查表
//    ⑤ 敌人改用「它那种」的贴图画  ⑥ 命中改成扣血 + 按种类给分
//  自查 make test；跑起来 make run。
// ============================================================

#include "raylib.h"
#include "assets.h"

// ---- 数据：一种东西一个 struct ----
struct Player { Rectangle rect; float speed; int hp; };
struct Bullet { Rectangle rect; float vx, vy; bool active; };

// 敌人「种类」：给每种起个名字（已给）
enum class EnemyKind { Grunt, Runner, Heavy, Elite };

// 配置表的一行：一种怪的全部数值（已给）
struct EnemyConfig { SpriteId sprite; float speed; int maxHp; int score; float size; };

// 敌人实例：记住自己是哪种 + 当前血量（已给）
struct Enemy { Rectangle rect; EnemyKind kind; int hp; bool active; };

const int MAX_BULLETS = 128, MAX_ENEMIES = 64;
const int SCREEN_W = 1280, SCREEN_H = 720;
const int TILE = 40;

// ---- 全局状态 ----
Player player;
Bullet bullets[MAX_BULLETS];
Enemy  enemies[MAX_ENEMIES];
int  score;
int  spawnTimer;
bool gameOver;

// ★TODO② 配置表：普通怪已给作范例。照它补上快速/重甲/精英三个 case。
//   一行的意思：{ 用哪张贴图, 速度, 满血血量, 击杀得分, 画多大 }
//   目标“性格”：快速怪——比普通快、脆(1血)；重甲——慢、肉(3血)、分高；
//               精英——最肉(6血)、够猛、分最高（压轴）。具体数字你自己定。
EnemyConfig ConfigOf(EnemyKind kind) {
    switch (kind) {
        case EnemyKind::Grunt: return { SpriteId::Grunt, 1.8f, 1, 10, 40 };   // 范例
        // 在这里补：case EnemyKind::Runner: return { SpriteId::Runner, ..., ..., ..., ... };
        // 在这里补：case EnemyKind::Heavy:  return { SpriteId::Heavy,  ..., ..., ..., ... };
        // 在这里补：case EnemyKind::Elite:  return { SpriteId::Elite,  ..., ..., ..., ... };
    }
    return { SpriteId::Grunt, 1.8f, 1, 10, 40 };   // 没填的种类先当普通怪（填好后各不相同）
}

// 出怪节奏：固定轮换，保证四种都露脸，精英隔一阵才来（已给）
EnemyKind NextKind() {
    static int n = 0;
    static const EnemyKind seq[] = {
        EnemyKind::Grunt, EnemyKind::Runner, EnemyKind::Grunt,
        EnemyKind::Heavy, EnemyKind::Runner, EnemyKind::Elite,
    };
    return seq[n++ % 6];
}

void InitGame();
void DrawFloor();
void UpdatePlayer();
void SpawnEnemies();
void DrawGame();
void FireBullets();
void UpdateBullets();
void UpdateEnemies();
void HandleHits();

// ---- main：开窗 → 读贴图 → 每帧「更新 → 绘制」→ 收尾（已给）----
#ifndef UNIT_TEST
int main() {
    InitWindow(SCREEN_W, SCREEN_H, "Mini Roguelike · Bestiary");
    SetTargetFPS(60);
    LoadAssets();
    InitGame();
    while (!WindowShouldClose()) {
        if (!gameOver) {
            UpdatePlayer();
            FireBullets();
            UpdateBullets();
            SpawnEnemies();
            UpdateEnemies();
            HandleHits();
        } else if (IsKeyPressed(KEY_ENTER)) InitGame();
        DrawGame();
    }
    UnloadAssets();
    CloseWindow();
    return 0;
}
#endif

// 初始化 / 重开（已给）
void InitGame() {
    player = { { SCREEN_W / 2 - 20, SCREEN_H / 2 - 20, 40, 40 }, 5.0f, 100 };
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i] = { { 0, 0, 0, 0 }, EnemyKind::Grunt, 0, false };
    score = 0; spawnTimer = 0; gameOver = false;
}

// 铺地面：整屏铺满地砖（已给）
void DrawFloor() {
    for (int y = 0; y < SCREEN_H; y += TILE)
        for (int x = 0; x < SCREEN_W; x += TILE)
            DrawSprite(SpriteId::Floor, { (float)x, (float)y, TILE, TILE });
}

// 玩家移动（已给，照它写别的 TODO）
void UpdatePlayer() {
    if (IsKeyDown(KEY_D)) player.rect.x += player.speed;
    if (IsKeyDown(KEY_A)) player.rect.x -= player.speed;
    if (IsKeyDown(KEY_S)) player.rect.y += player.speed;
    if (IsKeyDown(KEY_W)) player.rect.y -= player.speed;
}

// 刷怪：每 55 帧在顶部找空位冒一只（找空位、取种类、查配置都已给）
void SpawnEnemies() {
    if (++spawnTimer >= 55) {
        spawnTimer = 0;
        for (int i = 0; i < MAX_ENEMIES; i++)
            if (!enemies[i].active) {
                EnemyKind k = NextKind();                 // 这次出哪种
                EnemyConfig cfg = ConfigOf(k);            // 查它的配置
                enemies[i].rect   = { (float)GetRandomValue(0, SCREEN_W - (int)cfg.size), 0, cfg.size, cfg.size };
                enemies[i].kind   = EnemyKind::Grunt;     // ★TODO③ 改成 k（把这次的种类记下来）
                enemies[i].hp     = 1;                    // ★TODO③ 改成 cfg.maxHp（满血开场）
                enemies[i].active = true;
                break;
            }
    }
}

// 子弹飞行 + 出界回收（已给）
void UpdateBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        bullets[i].rect.x += bullets[i].vx;
        bullets[i].rect.y += bullets[i].vy;
        if (bullets[i].rect.x < 0 || bullets[i].rect.x > SCREEN_W ||
            bullets[i].rect.y < 0 || bullets[i].rect.y > SCREEN_H)
            bullets[i].active = false;
    }
}

// 按方向键发射（已给）
void FireBullets() {
    int vx = 0, vy = 0;
    if (IsKeyPressed(KEY_RIGHT)) vx = 9;
    if (IsKeyPressed(KEY_LEFT))  vx = -9;
    if (IsKeyPressed(KEY_UP))    vy = -9;
    if (IsKeyPressed(KEY_DOWN))  vy = 9;
    if (vx != 0 || vy != 0)
        for (int i = 0; i < MAX_BULLETS; i++)
            if (!bullets[i].active) {
                bullets[i].rect = { player.rect.x + player.rect.width / 2 - 5,
                                    player.rect.y + player.rect.height / 2 - 5, 10, 10 };
                bullets[i].vx = (float)vx; bullets[i].vy = (float)vy;
                bullets[i].active = true;
                break;
            }
}

// 子弹打中敌人（现在是「一枪一个 +10」，太简单了）
void HandleHits() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!enemies[j].active) continue;
            if (CheckCollisionRecs(bullets[i].rect, enemies[j].rect)) {
                bullets[i].active = false;
                // ★TODO⑥ 改成「扣血」：把下面两行换掉——
                //   enemies[j].hp -= 1;                            // 先扣 1 血
                //   if (enemies[j].hp <= 0) {                      // 血空了才死
                //       enemies[j].active = false;
                //       score += ConfigOf(enemies[j].kind).score;  // 按“它是哪种”给分
                //   }
                enemies[j].active = false;   // 临时：一下就死（做完 TODO⑥ 删掉这两行）
                score += 10;                 // 临时：不管什么怪都只给 10 分
                break;
            }
        }
    }
}

// 敌人追人 + 撞人掉血（追人逻辑已给）
void UpdateEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;
        float sp = 1.5f;   // ★TODO④ 改成 ConfigOf(enemies[i].kind).speed（速度按种类查表）
        if (player.rect.x > enemies[i].rect.x) enemies[i].rect.x += sp;
        if (player.rect.x < enemies[i].rect.x) enemies[i].rect.x -= sp;
        if (player.rect.y > enemies[i].rect.y) enemies[i].rect.y += sp;
        if (player.rect.y < enemies[i].rect.y) enemies[i].rect.y -= sp;
        if (CheckCollisionRecs(enemies[i].rect, player.rect)) {
            player.hp -= 10;
            enemies[i].active = false;
            if (player.hp <= 0) gameOver = true;
        }
    }
}

// 绘制：铺地面 → 画玩家/子弹/敌人 → HUD
void DrawGame() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    if (!gameOver) {
        DrawFloor();
        // ★TODO① 把下面两处的“方块”改成贴图：DrawSprite(SpriteId::Player, ...) / DrawSprite(SpriteId::Bullet, ...)
        DrawRectangleRec(player.rect, BLUE);                                        // ★TODO① 玩家改贴图
        for (int i = 0; i < MAX_BULLETS; i++)
            if (bullets[i].active) DrawRectangleRec(bullets[i].rect, BLACK);        // ★TODO① 子弹改贴图
        for (int i = 0; i < MAX_ENEMIES; i++)
            if (enemies[i].active)
                DrawRectangleRec(enemies[i].rect, RED);                            // ★TODO⑤ 改成 DrawSprite(ConfigOf(enemies[i].kind).sprite, enemies[i].rect)
        DrawText(TextFormat("HP: %d   Score: %d", player.hp, score), 16, 14, 24, (Color){ 30, 36, 54, 255 });
        DrawText("WASD move,  Arrow keys shoot", 16, 44, 20, (Color){ 90, 100, 120, 255 });
    } else {
        DrawText("GAME OVER", SCREEN_W / 2 - 130, SCREEN_H / 2 - 40, 54, MAROON);
        DrawText(TextFormat("Score: %d", score), SCREEN_W / 2 - 60, SCREEN_H / 2 + 24, 24, DARKGRAY);
        DrawText("Press ENTER to restart", SCREEN_W / 2 - 120, SCREEN_H / 2 + 64, 22, GRAY);
    }
    EndDrawing();
}
