// ============================================================
//  assets.h —— 素材门面（老师已写好，直接用，别改）
//  三个动作就够：
//    LoadAssets()          开场把所有贴图读进显存（main 里调一次）
//    DrawSprite(id, box)   把编号 id 的贴图铺满矩形 box
//    UnloadAssets()        关窗前释放
//  为什么用「编号」(enum) 不用字符串名字？——编号写错当场编译报红，
//  字符串打错要等运行才发现。想加怪：SpriteId 加个名字 + g_paths 加行路径。
// ============================================================
#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"

// 每张贴图一个编号；COUNT 永远排最后，自动等于“共几张”
enum class SpriteId { Player, Bullet, Grunt, Runner, Heavy, Elite, Floor, COUNT };

static Texture2D g_tex[(int)SpriteId::COUNT];

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
        g_tex[i] = LoadTexture(g_paths[i]);
        SetTextureFilter(g_tex[i], TEXTURE_FILTER_BILINEAR);   // 缩放不锯齿
    }
}

// 把 id 这张贴图整张铺满 box；贴图没读到就退回品红方块，方便发现缺图
inline void DrawSprite(SpriteId id, Rectangle box) {
    Texture2D t = g_tex[(int)id];
    if (t.id == 0) { DrawRectangleRec(box, MAGENTA); return; }
    Rectangle src = { 0, 0, (float)t.width, (float)t.height };
    DrawTexturePro(t, src, box, (Vector2){ 0, 0 }, 0.0f, WHITE);
}

inline void UnloadAssets() {
    for (int i = 0; i < (int)SpriteId::COUNT; i++) UnloadTexture(g_tex[i]);
}

#endif // ASSETS_H
