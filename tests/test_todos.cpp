// ============================================================
//  自动评测 · 检查 4 个 TODO（不开窗口，摆好局面 → 调用你的函数 → 报对错）
//  跑法：make test
//  原理：用 UNIT_TEST 借用 src/main.cpp 的函数，跳过那边的 main。
// ============================================================
#define UNIT_TEST
#include "../src/main.cpp"
#include <cstdio>

int g_pass = 0, g_fail = 0;
static void check(const char* name, bool ok) {
    printf(ok ? "  [通过]   %s\n" : "  [未通过] %s\n", name);
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
    printf("① UpdateBullets\n");
    resetAll();
    bullets[0] = { { 100, 100, 8, 8 }, 5, 0, true };   // 向右飞
    UpdateBullets();
    check("子弹按 vx 前进（x: 100 → 105）", bullets[0].rect.x == 105);
    check("没在用的子弹不受影响", bullets[1].active == false);

    resetAll();
    bullets[0] = { { (float)(SCREEN_W - 2), 100, 8, 8 }, 5, 0, true }; // 一步飞出右边界
    UpdateBullets();
    check("飞出屏幕的子弹被回收（active=false）", bullets[0].active == false);
}

// ── ② FireBullets：依赖键盘，只能做「无按键不产生子弹」的弱检查 ──
static void test_FireBullets() {
    printf("② FireBullets（部分检查）\n");
    resetAll();
    FireBullets();                                     // 无窗口时按键读作「没按」
    int n = 0; for (int i = 0; i < MAX_BULLETS; i++) if (bullets[i].active) n++;
    check("没按方向键时不产生子弹", n == 0);
    printf("     注：能否真正射出，请在游戏里手动按方向键验证（自动测试模拟不了按键）。\n");
}

// ── ③ HandleHits：子弹撞敌人 → 双方消失、加分 ──
static void test_HandleHits() {
    printf("③ HandleHits\n");
    resetAll();
    bullets[0] = { { 200, 200, 8, 8 }, 0, 0, true };
    enemies[0] = { { 201, 201, 30, 30 }, 1.5f, true }; // 与子弹重叠
    HandleHits();
    check("相撞后子弹消失", bullets[0].active == false);
    check("相撞后敌人消失", enemies[0].active == false);
    check("相撞后加 10 分", score == 10);

    resetAll();
    bullets[0] = { { 0, 0, 8, 8 }, 0, 0, true };
    enemies[0] = { { 500, 500, 30, 30 }, 1.5f, true }; // 离得很远
    HandleHits();
    check("不相撞时不加分、双方都还在",
          score == 0 && bullets[0].active && enemies[0].active);
}

// ── ④ UpdateEnemies：敌人追人 + 撞人掉血 + 血空判负 ──
static void test_UpdateEnemies() {
    printf("④ UpdateEnemies\n");
    resetAll();
    player.rect = { 500, 300, 32, 32 };
    enemies[0] = { { 100, 300, 30, 30 }, 2.0f, true }; // 在玩家左边
    UpdateEnemies();
    check("敌人向玩家靠近（x: 100 → 102）", enemies[0].rect.x == 102);

    resetAll();
    player.rect = { 300, 300, 32, 32 }; player.hp = 100;
    enemies[0] = { { 305, 305, 30, 30 }, 2.0f, true }; // 压在玩家身上
    UpdateEnemies();
    check("撞到玩家：掉 10 血", player.hp == 90);
    check("撞到玩家：该敌人消失", enemies[0].active == false);

    resetAll();
    player.rect = { 300, 300, 32, 32 }; player.hp = 10;
    enemies[0] = { { 305, 305, 30, 30 }, 2.0f, true };
    UpdateEnemies();
    check("血量 ≤ 0 时 gameOver = true", gameOver == true);
}

int main() {
    printf("==== 自动评测：检查 4 个 TODO ====\n\n");
    test_UpdateBullets();
    test_FireBullets();
    test_HandleHits();
    test_UpdateEnemies();
    printf("\n==== 结果：通过 %d 项，未通过 %d 项 ====\n", g_pass, g_fail);
    if (g_fail == 0) printf("全部通过，太棒了！\n");
    else             printf("还有没过的，回去看看上面「未通过」那几条。\n");
    return g_fail == 0 ? 0 : 1;
}
