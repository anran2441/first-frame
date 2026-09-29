// ============================================================
//  assets.h —— 素材门面（老师已写好，直接用，别改）
//  三个动作就够：
//    LoadAssets()          开场把所有贴图读进显存（main 里调一次）
//    DrawSprite(id, box)   把编号 id 的贴图放大到装下矩形 box（等比、居中）
//    UnloadAssets()        关窗前释放
//  为什么用「编号」(enum) 不用字符串名字？——编号写错当场编译报红，
//  字符串打错要等运行才发现。想加怪：SpriteId 加个名字 + g_paths 加行路径。
//
//  素材图四周照惯例留了一圈透明边。要是把整张画布（连透明边）塞进 box，
//  角色看着就比 box 小一圈，配置表里的 size 等于白写。所以加载时先用
//  GetImageAlphaBorder 量出「真正有像素的那块」，画的时候只画那一块、
//  等比缩到刚好装下 box 再居中 —— box 多大，角色看起来就有多大。
// ============================================================
#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"

// 每张贴图一个编号；COUNT 永远排最后，自动等于“共几张”
enum class SpriteId { Player, Bullet, Grunt, Runner, Heavy, Elite, Floor, COUNT };

static Texture2D g_tex[(int)SpriteId::COUNT];
static Rectangle g_crop[(int)SpriteId::COUNT];   // 每张贴图里有像素的那块（去了透明边）

// 编号 → 文件路径（顺序必须和上面的 enum 一一对应）
static const char *g_paths[(int)SpriteId::COUNT] = {
    "assets/player.png",   // Player
    "assets/bullet.png",   // Bullet
    "assets/grunt.png",    // Grunt  普通怪
    "assets/runner.png",   // Runner 快速怪
    "assets/heavy.png",    // Heavy  重甲怪
    "assets/elite.png",    // Elite  精英怪
    "assets/floor.png",    // Floor  地面
};

inline void LoadAssets() {
    for (int i = 0; i < (int)SpriteId::COUNT; i++) {
        // 先读成 Image —— 变成纹理后就量不出透明边了
        Image img = LoadImage(g_paths[i]);
        g_crop[i] = GetImageAlphaBorder(img, 0.1f);           // 淡到快看不见的像素不算内容
        if (g_crop[i].width <= 0 || g_crop[i].height <= 0)    // 缺图、或整张全透明
            g_crop[i] = Rectangle{ 0, 0, (float)img.width, (float)img.height };
        g_tex[i] = LoadTextureFromImage(img);
        UnloadImage(img);                                     // Image 用完要还，不然漏内存
        SetTextureFilter(g_tex[i], TEXTURE_FILTER_BILINEAR);   // 缩放不锯齿
    }
}

// 把 id 这张贴图（去掉透明边的那块）等比放进 box 正中；
// 贴图没读到就退回品红方块，方便发现缺图
inline void DrawSprite(SpriteId id, Rectangle box) {
    Texture2D t = g_tex[(int)id];
    if (t.id == 0) { DrawRectangleRec(box, MAGENTA); return; }   // 品红 = 这张图没读到

    Rectangle src = g_crop[(int)id];
    if (src.width <= 0 || src.height <= 0)                       // 没量到就整张画，别除零
        src = Rectangle{ 0, 0, (float)t.width, (float)t.height };

    // 宽高两个比例取小的那个：等比缩放，且两个方向都装得下
    float sx = box.width / src.width, sy = box.height / src.height;
    float scale = (sx < sy) ? sx : sy;
    Rectangle dst = { box.x + (box.width  - src.width  * scale) / 2.0f,
                      box.y + (box.height - src.height * scale) / 2.0f,
                      src.width * scale, src.height * scale };
    DrawTexturePro(t, src, dst, Vector2{ 0, 0 }, 0.0f, WHITE);
}

inline void UnloadAssets() {
    for (int i = 0; i < (int)SpriteId::COUNT; i++) UnloadTexture(g_tex[i]);
}

#endif // ASSETS_H
