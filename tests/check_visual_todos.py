#!/usr/bin/env python3
"""第 2 课 · ①⑤ 两项的源码核验（画面类 TODO，评测程序看不见画面）

跑法（CI 里由 grade.yml 调；也可以自己在本机跑）：
    python3 tests/check_visual_todos.py src/main.cpp

为什么另开一个脚本、不塞进 test_todos.cpp：①⑤ 是「画得好不好看」。评测程序只
include 代码、从不 InitWindow，根本渲染不出画面。能自动判的只有源码里的结构：
「那两个临时方块删掉没有、换成画贴图的调用了没有、敌人的图有没有按种类取」。
真正的观感要孩子自己 `make run` 看一眼——所以这两项即使全绿，也只是「源码检查」。

判据（刻意只认结构、不认拼写，免得孩子换个写法就被误判成没做）：
  ① 通过 = DrawGame 里 player.rect 和 bullets 的临时 DrawRectangleRec 都没了，
            并且有画贴图的调用分别用到了 player.rect 和 bullets（各自的 rect）。
  ⑤ 通过 = enemies 的临时 DrawRectangleRec 没了，有画贴图的调用用到了 enemies，
            且取图是按种类的（函数体里出现 kind —— 手册教的
            ConfigOf(enemies[i].kind).sprite 就是这种）。

输出两行 TODO_STATUS 给 CI 的 publish job 读（和评测程序同一套契约）：
    TODO_STATUS 1 <0|1> 1      ① 玩家 + 子弹
    TODO_STATUS 5 <0|1> 1      ⑤ 敌人按种类
两项都过 → 退出码 0；有没过 → 1；读不到源文件 / 找不到 DrawGame → 2 且一行
TODO_STATUS 都不发（结果是「评测不可用」，而不是冒充「没做」）。
"""
import re
import sys

# 「画贴图」的调用：DrawSprite 是手册教的写法，DrawTexture 家族属于合理变体，
# 一并认。
TEX_CALL = re.compile(r'\b(?:DrawSprite|DrawTexture\w*)\s*\(')
RECT_CALL = re.compile(r'\bDrawRectangleRec\s*\(')
# 函数定义（不是前置声明 —— 前置声明以 ';' 结尾，匹配不到 '{'）
DRAW_GAME = re.compile(r'\bvoid\s+DrawGame\s*\(\s*\)\s*\{')
# 注释 / 字符串字面量。要涂掉它们，否则脚手架里 ★TODO① 的提示注释
# （注释里就写着 "DrawSprite(SpriteId::Player, ...)"）会被当成真的画了贴图。
NOISE = re.compile(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')


def code_only(text):
    """把注释和字符串涂成等长空格（换行保留，免得把两行粘成一行）。"""
    return NOISE.sub(lambda m: re.sub(r'[^\n]', ' ', m.group()), text)


def draw_game(source):
    """取出 DrawGame() 的函数体（花括号配对）；找不到返回空串。"""
    code = code_only(source)
    start = DRAW_GAME.search(code)
    if not start:
        return ''
    depth = 1
    for index in range(start.end(), len(code)):
        if code[index] == '{':
            depth += 1
        elif code[index] == '}':
            depth -= 1
            if depth == 0:
                return code[start.end():index]
    return ''


def call_args(body, pattern):
    """取出 body 里每个匹配调用的参数串（按括号配对，扛得住嵌套）。"""
    args = []
    for match in pattern.finditer(body):
        index, depth = match.end(), 1
        while index < len(body) and depth:
            if body[index] == '(':
                depth += 1
            elif body[index] == ')':
                depth -= 1
            index += 1
        args.append(body[match.end():index - 1])
    return args


def inspect(source):
    """返回 (①是否算做完, ⑤是否算做完)。"""
    body = draw_game(source)
    if not body:
        return False, False

    rects = call_args(body, RECT_CALL)
    texs = call_args(body, TEX_CALL)

    def rect_left(marker):
        """那个临时方块还留着吗？只看调用的第一个参数。"""
        return any(marker in arg.split(',')[0] for arg in rects)

    def drawn(marker):
        """有画贴图的调用用到了它吗？参数里提到就算。"""
        return any(marker in arg for arg in texs)

    first = (not rect_left('player.rect')) and (not rect_left('bullets')) \
        and drawn('player.rect') and drawn('bullets')
    fifth = (not rect_left('enemies')) and drawn('enemies') \
        and bool(re.search(r'\bkind\b', body))
    return first, fifth


def report(path):
    try:
        with open(path, encoding='utf-8', errors='replace') as handle:
            source = handle.read()
    except OSError as exc:
        print('visual check unavailable: cannot read %s (%s)' % (path, exc))
        return 2

    if not draw_game(source):
        print('visual check unavailable: no DrawGame() body in %s' % path)
        return 2

    first, fifth = inspect(source)

    def line(number, passed, name):
        print('  [%s] %s (source check)' % ('PASS' if passed else 'FAIL', name))
        # 给 publish job 读的一行：必须纯 ASCII、不带颜色码、整行精确匹配。
        print('TODO_STATUS %d %d 1' % (number, 1 if passed else 0))

    print('\n[1] Visual -- player & bullet drawn as sprites')
    line(1, first, 'the two temporary rectangles are gone and sprites are drawn for them')
    print('\n[5] Visual -- enemy drawn with its own kind\'s sprite')
    line(5, fifth, 'the enemy rectangle is gone and the sprite is looked up by kind')
    print('\nNote: source check only -- run the game and look at it yourself.')
    return 0 if (first and fifth) else 1


def main(argv):
    if len(argv) != 2:
        print('usage: check_visual_todos.py <path-to-src/main.cpp>')
        return 2
    return report(argv[1])


if __name__ == '__main__':
    sys.exit(main(sys.argv))
