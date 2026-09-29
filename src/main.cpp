// ============================================================
//  迷你肉鸽 · 脚手架
//  操作：WASD 移动，方向键射击，死后 Enter 重开。
//  已写好（当范例读）：main、InitGame、UpdatePlayer、SpawnEnemies、DrawGame。
//  你来填 4 个 ★TODO，顺序：
//    ① UpdateBullets ② FireBullets（能射击）
//    ③ HandleHits（能杀敌）   ④ UpdateEnemies（敌人追人，成型）
//  编译运行 make run；自查 make test。详解见 学生手册-TODO说明.md
// ============================================================

#include "raylib.h"

// ---- 数据：一种东西一个 struct；active = 这一格在不在用（对象池标志）----
struct Player { Rectangle rect; float speed; int hp; };
struct Bullet { Rectangle rect; float vx, vy; bool active; };
struct Enemy  { Rectangle rect; float speed;        bool active; };

const int MAX_BULLETS = 128, MAX_ENEMIES = 64;
const int SCREEN_W = 1920, SCREEN_H = 1080;

// ---- 全局状态：所有函数共用 ----
Player player;
Bullet bullets[MAX_BULLETS];
Enemy  enemies[MAX_ENEMIES];
int  score;
int  spawnTimer;
bool gameOver;

void InitGame();
void UpdatePlayer();    // 范例：照它写下面的 TODO
void SpawnEnemies();
void DrawGame();
void FireBullets();     // ★TODO②
void UpdateBullets();   // ★TODO①
void UpdateEnemies();   // ★TODO④
void HandleHits();      // ★TODO③

// ---- main：开窗 → 每帧「更新 → 绘制」→ 关窗（已写好）----
#ifndef UNIT_TEST                 // 自动测试借用本文件函数时跳过 main（勿删）
int main() {
    InitWindow(SCREEN_W, SCREEN_H, "Mini Roguelike");
    SetTargetFPS(60);
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
    CloseWindow();
    return 0;
}
#endif

// 初始化 / 重开：状态清回开局
void InitGame() {
    player = { { 480, 270, 32, 32 }, 4.0f, 100 };
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
    score = 0;
    spawnTimer = 0;
    gameOver = false;
}

// 范例 · 玩家移动：读键盘 → 逐轴改坐标（照这个写 TODO）
void UpdatePlayer() {
    if (IsKeyDown(KEY_D)) player.rect.x += player.speed;
    if (IsKeyDown(KEY_A)) player.rect.x -= player.speed;
    if (IsKeyDown(KEY_S)) player.rect.y += player.speed;
    if (IsKeyDown(KEY_W)) player.rect.y -= player.speed;
}

// 刷怪：每 60 帧找一个空位，在顶部随机冒一个敌人（FireBullets 找空位同理）
void SpawnEnemies() {
    if (++spawnTimer >= 60) {
        spawnTimer = 0;
        for (int i = 0; i < MAX_ENEMIES; i++)
            if (!enemies[i].active) {
                enemies[i].rect = { (float)GetRandomValue(0, SCREEN_W - 30), 0, 30, 30 };
                enemies[i].speed = 1.5f;
                enemies[i].active = true;
                break;
            }
    }
}

// ★ TODO ① UpdateBullets：子弹飞行 + 出界回收
//   每颗 active 子弹按 vx/vy 移动；飞出屏幕(0~SCREEN_W / 0~SCREEN_H)就 active=false。仿 UpdatePlayer。
void UpdateBullets(){
    for(int i=0;i<=MAX_BULLETS-1;i++){
        if(bullets[i].active==false){
            continue;
        }
        bullets[i].rect.x+=bullets[i].vx;
        bullets[i].rect.y+=bullets[i].vy;
        int xx=bullets[i].rect.x;
        int yy=bullets[i].rect.y;
        if(xx>SCREEN_W||xx<1||yy>SCREEN_H||yy<1){
            bullets[i].active=false;     
        }
    }
}

// ★ TODO ② FireBullets：按方向键发射
//   按 ↑↓←→ 找一颗 !active 的子弹，放到玩家身上、设速度(右 vx=8/左 -8、上 vy=-8/下 8)、active=true。仿 SpawnEnemies 找空位。
void FireBullets() {
    for(int i=0;i<=MAX_BULLETS-1;i++){
        if(bullets[i].active==false){
            bullets[i].rect = { player.rect.x, player.rect.y, 8, 8 };
            bullets[i].vx=0;
            bullets[i].vy=0;
            if(IsKeyPressed(KEY_UP)){
                bullets[i].vy=-8;
                bullets[i].active=true;
                break;
            }
            else if(IsKeyPressed(KEY_DOWN)){
                bullets[i].vy=8;
                bullets[i].active=true;
            }
            else if(IsKeyPressed(KEY_LEFT)){
                bullets[i].vx=-8;
                bullets[i].active=true;
            }
            else if(IsKeyPressed(KEY_RIGHT)){
                bullets[i].vx=8;
                bullets[i].active=true;
            }
        }
    }
}

// ★ TODO ③ HandleHits：子弹打中敌人
//   双重 for（子弹 × 敌人）+ CheckCollisionRecs；相撞则双方 active=false、score+=10。
void HandleHits() {
    for(int i=0;i<=MAX_BULLETS-1;i++){
        for(int j=0;j<=MAX_ENEMIES-1;j++){
            if(bullets[i].active==false) continue;
            if(enemies[j].active==false) continue; 
            if(CheckCollisionRecs(bullets[i].rect,enemies[j].rect)){
                bullets[i].active=false;
                enemies[j].active=false;
                score+=10;
            }
        }
    }
}

// ★ TODO ④ UpdateEnemies：敌人追玩家 + 撞人掉血
//   每个敌人逐轴靠近 player（比大小 ±speed）；撞到 player 则 hp-=10、敌人 active=false；hp<=0 则 gameOver=true。
void UpdateEnemies() {
    for(int i=0;i<=MAX_ENEMIES-1;i++){
        if(enemies[i].active==false) continue;
        if(player.rect.x>enemies[i].rect.x){
            enemies[i].rect.x+=enemies[i].speed;
        }
        if(player.rect.x<enemies[i].rect.x){
            enemies[i].rect.x-=enemies[i].speed;
        }
        if(player.rect.y>enemies[i].rect.y){
            enemies[i].rect.y+=enemies[i].speed;
        }
        if(player.rect.y<enemies[i].rect.y){
            enemies[i].rect.y-=enemies[i].speed;
        }
        if(CheckCollisionRecs(enemies[i].rect,player.rect)){
            player.hp-=10;
            enemies[i].active=false;
            if(player.hp<=0){
                gameOver=true;
            }
        }
    }
}

// 绘制：画玩家、子弹、敌人、文字（血量/分数）（已写好）
void DrawGame() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    if (!gameOver) {
        DrawRectangleRec(player.rect, BLUE);
        for (int i = 0; i < MAX_BULLETS; i++) if (bullets[i].active) DrawRectangleRec(bullets[i].rect, BLACK);
        for (int i = 0; i < MAX_ENEMIES; i++) if (enemies[i].active) DrawRectangleRec(enemies[i].rect, RED);
        DrawText(TextFormat("HP: %d   Score: %d", player.hp, score), 10, 10, 20, DARKGRAY);
        DrawText("WASD move,  Arrow keys shoot", 10, 35, 18, GRAY);
    } else {
        DrawText("GAME OVER", 360, 240, 40, MAROON);
        DrawText(TextFormat("Score: %d", score), 410, 300, 20, DARKGRAY);
        DrawText("Press ENTER to restart", 360, 340, 20, GRAY);
    }
    EndDrawing();
}