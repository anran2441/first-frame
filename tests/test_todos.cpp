// ============================================================
//  自动评测 · 检查 4 个 TODO（不开窗口，摆好局面 → 调用你的函数 → 报对错）
//  跑法：make test
//  原理：用 UNIT_TEST 借用 src/main.cpp 的函数，跳过那边的 main。
//  说明：输出一律用英文，避免不同终端的中文编码（乱码）问题。
// ============================================================
#define UNIT_TEST
#include "../src/main.cpp"
#include <cstdio>

int g_pass = 0, g_fail = 0;
static void check(const char* name, bool ok) {
    printf(ok ? "  [PASS] %s\n" : "  [FAIL] %s\n", name);
    if (ok) g_pass++; else g_fail++;
}

// 每个测试前，把全局状态清成一个干净局面
static void resetAll() {
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
    player = { { 480, 270, 32, 32 }, 4.0f, 100 };
    score = 0; spawnTimer = 0; gameOver = false;
}

// ── ① UpdateBullets：子弹会飞、出界会回收 ──
static void test_UpdateBullets() {
    printf("[1] UpdateBullets\n");
    resetAll();
    bullets[0] = { { 100, 100, 8, 8 }, 5, 0, true };   // 向右飞
    UpdateBullets();
    check("bullet moves by vx (x: 100 -> 105)", bullets[0].rect.x == 105);
    check("inactive bullet stays untouched", bullets[1].active == false);

    resetAll();
    bullets[0] = { { (float)(SCREEN_W - 2), 100, 8, 8 }, 5, 0, true }; // 一步飞出右边界
    UpdateBullets();
    check("off-screen bullet is recycled (active=false)", bullets[0].active == false);
}

// ── ② FireBullets：依赖键盘，只能做「无按键不产生子弹」的弱检查 ──
static void test_FireBullets() {
    printf("[2] FireBullets (partial check)\n");
    resetAll();
    FireBullets();                                     // 无窗口时按键读作「没按」
    int n = 0; for (int i = 0; i < MAX_BULLETS; i++) if (bullets[i].active) n++;
    check("no bullet is fired when no arrow key is pressed", n == 0);
    printf("     note: real firing must be tested by hand in the game (autograder cannot press keys).\n");
}

// ── ③ HandleHits：子弹撞敌人 → 双方消失、加分 ──
static void test_HandleHits() {
    printf("[3] HandleHits\n");
    resetAll();
    bullets[0] = { { 200, 200, 8, 8 }, 0, 0, true };
    enemies[0] = { { 201, 201, 30, 30 }, 1.5f, true }; // 与子弹重叠
    HandleHits();
    check("bullet disappears on hit", bullets[0].active == false);
    check("enemy disappears on hit", enemies[0].active == false);
    check("score += 10 on hit", score == 10);

    resetAll();
    bullets[0] = { { 0, 0, 8, 8 }, 0, 0, true };
    enemies[0] = { { 500, 500, 30, 30 }, 1.5f, true }; // 离得很远
    HandleHits();
    check("no hit: no score, both still alive",
          score == 0 && bullets[0].active && enemies[0].active);
}

// ── ④ UpdateEnemies：敌人追人 + 撞人掉血 + 血空判负 ──
static void test_UpdateEnemies() {
    printf("[4] UpdateEnemies\n");
    resetAll();
    player.rect = { 500, 300, 32, 32 };
    enemies[0] = { { 100, 300, 30, 30 }, 2.0f, true }; // 在玩家左边
    UpdateEnemies();
    check("enemy moves toward player (x: 100 -> 102)", enemies[0].rect.x == 102);

    resetAll();
    player.rect = { 300, 300, 32, 32 }; player.hp = 100;
    enemies[0] = { { 305, 305, 30, 30 }, 2.0f, true }; // 压在玩家身上
    UpdateEnemies();
    check("touching player costs 10 HP", player.hp == 90);
    check("that enemy disappears after touching player", enemies[0].active == false);

    resetAll();
    player.rect = { 300, 300, 32, 32 }; player.hp = 10;
    enemies[0] = { { 305, 305, 30, 30 }, 2.0f, true };
    UpdateEnemies();
    check("HP <= 0 sets gameOver = true", gameOver == true);
}

int main() {
    printf("==== Autograder: checking the 4 TODOs ====\n\n");
    test_UpdateBullets();
    test_FireBullets();
    test_HandleHits();
    test_UpdateEnemies();
    printf("\n==== Result: %d passed, %d failed ====\n", g_pass, g_fail);
    if (g_fail == 0) printf("All checks passed. Well done!\n");
    else             printf("Some checks failed. Look at the [FAIL] lines above.\n");
    return g_fail == 0 ? 0 : 1;
}
