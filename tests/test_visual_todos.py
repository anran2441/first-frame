"""tests/check_visual_todos.py 的回归测试（老师用，CI 不跑）

    python3 tests/test_visual_todos.py

盯的是两件相反的事：
  * 别把「没做」判成「做了」—— 尤其是脚手架里那段提示注释，注释里就写着
    DrawSprite(SpriteId::Player, ...)，不涂掉注释就会误判成满分。
  * 别把「做了」判成「没做」—— 孩子换个写法（DrawTexturePro、先存进局部
    变量再画）同样算做完。
"""
import importlib.util
import unittest
from pathlib import Path

SPEC = importlib.util.spec_from_file_location('checker', Path(__file__).with_name('check_visual_todos.py'))
checker = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(checker)

# 一行没改的脚手架：两个临时方块还在，★TODO 提示注释里提到 DrawSprite
SCAFFOLD = '''void DrawGame() {
    DrawFloor();
    // ★TODO① 把下面的"方块"改成贴图：DrawSprite(SpriteId::Player, player.rect);
    DrawRectangleRec(player.rect, BLUE);
    for (int i = 0; i < MAX_BULLETS; i++)
        if (bullets[i].active) DrawRectangleRec(bullets[i].rect, BLACK);
    for (int i = 0; i < MAX_ENEMIES; i++)
        if (enemies[i].active)
            DrawRectangleRec(enemies[i].rect, RED);   // ★TODO⑤ 改成 DrawSprite(...)
    DrawText("HP", 16, 14, 24, BLACK);
}'''

DONE_PLAYER = SCAFFOLD.replace(
    'DrawRectangleRec(player.rect, BLUE);', 'DrawSprite(SpriteId::Player, player.rect);').replace(
    'DrawRectangleRec(bullets[i].rect, BLACK);', 'DrawSprite(SpriteId::Bullet, bullets[i].rect);')
DONE_ENEMY = SCAFFOLD.replace(
    'DrawRectangleRec(enemies[i].rect, RED);', 'DrawSprite(ConfigOf(enemies[i].kind).sprite, enemies[i].rect);')
DONE_ALL = DONE_PLAYER.replace(
    'DrawRectangleRec(enemies[i].rect, RED);', 'DrawSprite(ConfigOf(enemies[i].kind).sprite, enemies[i].rect);')


class VisualChecks(unittest.TestCase):
    def check(self, source, expected):
        self.assertEqual(checker.inspect(source), expected)

    def test_reference_pattern(self):
        self.check(DONE_ALL, (True, True))

    def test_untouched_scaffold_is_not_done(self):
        # 注释里那行 DrawSprite 不算数 —— 这是最容易假阳性的一处
        self.check(SCAFFOLD, (False, False))

    def test_only_one_of_the_two(self):
        self.check(DONE_PLAYER, (True, False))
        self.check(DONE_ENEMY, (False, True))

    def test_leftover_rectangle_invalidates_that_item(self):
        self.check(DONE_ALL.replace('DrawFloor();', 'DrawFloor(); DrawRectangleRec(player.rect, BLUE);'), (False, True))
        self.check(DONE_ALL.replace('DrawFloor();', 'DrawFloor(); DrawRectangleRec(enemies[i].rect, RED);'), (True, False))

    def test_other_rectangles_are_fine(self):
        # 画 HUD 的方块跟这两个 TODO 无关，不该误伤
        self.check(DONE_ALL.replace('DrawFloor();', 'DrawFloor(); DrawRectangleRec(hudRect, GRAY);'), (True, True))

    def test_deleting_without_drawing_is_not_done(self):
        import re
        self.check(re.sub(r'DrawRectangleRec\([^;]*\);', '', SCAFFOLD), (False, False))

    def test_stacking_sprites_next_to_the_old_rectangles_is_not_done(self):
        self.check(SCAFFOLD.replace(
            'DrawFloor();',
            'DrawFloor(); DrawSprite(SpriteId::Player, player.rect); DrawSprite(SpriteId::Bullet, player.rect);'), (False, False))

    def test_drawing_the_wrong_thing_does_not_count(self):
        # 两个贴图都画的是玩家 == 子弹那块没换
        self.check(DONE_PLAYER.replace('DrawSprite(SpriteId::Bullet, bullets[i].rect);', 'DrawSprite(SpriteId::Player, player.rect);'),
                   (False, False))

    def test_enemy_sprite_must_be_looked_up_by_kind(self):
        # 换了贴图，但所有敌人都画成同一种 —— ⑤ 的考点就是「按种类取图」
        self.check(DONE_ALL.replace('DrawSprite(ConfigOf(enemies[i].kind).sprite, enemies[i].rect);',
                                    'DrawSprite(SpriteId::Grunt, enemies[i].rect);'), (True, False))

    def test_alternative_spellings_still_count(self):
        # 手册教 DrawSprite，孩子用 DrawTexturePro 也应当算做完
        self.check(DONE_PLAYER.replace('DrawSprite(SpriteId::Player, player.rect);',
                                       'DrawTexturePro(g_tex[0], src, player.rect, origin, 0, WHITE);'), (True, False))
        # 先把配置取进局部变量再画，同样算「按种类」
        self.check(DONE_ALL.replace('DrawSprite(ConfigOf(enemies[i].kind).sprite, enemies[i].rect);',
                                    'EnemyConfig c = ConfigOf(enemies[i].kind); DrawSprite(c.sprite, enemies[i].rect);'), (True, True))

    def test_comments_and_strings_do_not_count(self):
        self.check('void DrawFloor() { DrawSprite(SpriteId::Floor, r); }\n'
                   'void DrawGame() {\n'
                   '    // DrawSprite(SpriteId::Player, player.rect);\n'
                   '    /* DrawSprite(SpriteId::Bullet, bullets[i].rect); */\n'
                   '    DrawText("DrawSprite(ConfigOf(enemies[i].kind).sprite, enemies[i].rect)", 0, 0, 0, BLACK);\n'
                   '}', (False, False))

    def test_missing_draw_game_body(self):
        self.check('void DrawGame();', (False, False))
        self.check('int main() { return 0; }', (False, False))


if __name__ == '__main__':
    unittest.main(verbosity=2)
