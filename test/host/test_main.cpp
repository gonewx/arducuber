// 主机端单元测试: 用 fakedata.h 中实测的三组魔方扫描数据, 验证颜色识别、
// 色块校验和求解器 (不涉及电机、传感器等硬件)
#include <stdio.h>
#include <stdlib.h>

#include "../../global.h"
#include "../../color.h"
#include "../../validator.h"
#include "../../solver.h"
#include "../../fakedata.h"

static int failures = 0;

#define CHECK(cond, ...)                         \
    do                                           \
    {                                            \
        if (!(cond))                             \
        {                                        \
            printf("FAIL %s:%d: ", __FILE__, __LINE__); \
            printf(__VA_ARGS__);                 \
            printf("\n");                        \
            failures++;                          \
        }                                        \
    } while (0)

static CubeColors cubeColors;
static Validator validator;
static CubeSolver cubeSolver;

static void loadFake(int c)
{
    for (int f = 0; f < NFACE; f++)
        for (int p = 0; p < 9; p++)
        {
            uint8_t rgb[3] = {fake_colors[c][f * 9 + p][0],
                              fake_colors[c][f * 9 + p][1],
                              fake_colors[c][f * 9 + p][2]};
            cubeColors.setRGB(f, p, rgb);
        }
}

// 按求解结果转动一份拷贝, 检查是否还原
static bool appliesToSolved(const byte *cube)
{
    byte work[NFACE * 8];
    for (int i = 0; i < NFACE * 8; i++)
        work[i] = cube[i];
    for (int i = 0; i < cubeSolver.solve_n; i++)
        cubeSolver.rotate(work, cubeSolver.solve_fce[i], cubeSolver.solve_rot[i]);
    return cubeSolver.solved(work);
}

static void testColorHsl()
{
    Color c;
    c.setRGB(255, 255, 255);
    CHECK(c.s == 0 && c.sl == 0 && c.l == 255, "white: s=%d sl=%ld l=%d", c.s, c.sl, c.l);

    // 色相整体旋转了 1/6 圈 (红/蓝分居 0 两侧): 红约 cmax/6, 绿约 cmax/2, 蓝约 5cmax/6。
    // 这些颜色的通道差值都大于 31, 旧代码在 AVR 的 16 位 int 上计算 cmax * diff 会溢出
    c.setRGB(255, 20, 10);
    CHECK(c.h > 0 && c.h < cmax / 3, "red hue: %d", c.h);
    c.setRGB(10, 200, 20);
    CHECK(c.h > cmax / 3 && c.h < 2 * cmax / 3, "green hue: %d", c.h);
    c.setRGB(10, 20, 250);
    CHECK(c.h > 2 * cmax / 3 && c.h < cmax, "blue hue: %d", c.h);
}

// 期望: 第 0、2 组数据至少有一种 red/orange 区分方式能识别并求解; 第 1 组 (小米魔方)
// 现有算法识别不出, 记录为已知情况, 防止无意中改变行为
static void testFakeCubes()
{
    const int expectSolvable[3] = {1, 0, 1};
    const int expectMoves[3] = {31, -1, 40};

    for (int c = 0; c < 3; c++)
    {
        loadFake(c);
        int solvedWith = -1;
        for (int t = 0; t < COLOR_STRATEGIES; t++)
        {
            byte cube[NFACE * 8];
            cubeColors.determine_colors(cube, t);
            if (!validator.valid_pieces(cube))
                continue;
            if (!cubeSolver.solve(cube))
                continue;
            CHECK(cubeSolver.solve_n > 0 && cubeSolver.solve_n < MV_MAX, "cube %d moves %d", c, cubeSolver.solve_n);
            CHECK(appliesToSolved(cube), "cube %d strategy %d: solution does not solve", c, t);
            if (solvedWith < 0)
            {
                solvedWith = t;
                CHECK(cubeSolver.solve_n == expectMoves[c], "cube %d: %d moves, expected %d", c, cubeSolver.solve_n, expectMoves[c]);
            }
        }
        CHECK((solvedWith >= 0) == (expectSolvable[c] != 0), "cube %d solvable=%d", c, solvedWith >= 0);
    }
}

// 随机打乱 -> 求解 -> 验证
static void testRandomScrambles()
{
    srand(12345);
    for (int n = 0; n < 500; n++)
    {
        byte cube[NFACE * 8];
        int o = 0;
        for (byte f = 0; f < NFACE; f++)
            for (int i = 0; i < 8; i++)
                cube[o++] = f;

        for (int k = 0; k < 30; k++)
            cubeSolver.rotate(cube, rand() % NFACE, 1 + rand() % 3);

        CHECK(validator.valid_pieces(cube), "scramble %d not valid", n);
        CHECK(cubeSolver.solve(cube), "scramble %d not solved", n);
        CHECK(cubeSolver.solve_n < MV_MAX, "scramble %d too many moves %d", n, cubeSolver.solve_n);
        CHECK(appliesToSolved(cube), "scramble %d: solution does not solve", n);
        if (failures > 10)
            return;
    }
}

// 白平衡: 默认值不改变读数; 按比例缩放后白色三通道相等; 用实测数据里的白色中心校准后,
// 第 0、2 组仍能识别并求解 (旧的 "截断" 做法会让第 0 组 6 种策略里只剩 1 种成功)
static void testWhiteBalance()
{
    const uint8_t def[3] = {255, 255, 255};
    uint8_t rgb[3] = {12, 200, 255};
    white_balance(rgb, def);
    CHECK(rgb[0] == 12 && rgb[1] == 200 && rgb[2] == 255, "default white changed rgb");

    const uint8_t w[3] = {200, 160, 120};
    uint8_t white[3] = {200, 160, 120};
    white_balance(white, w);
    CHECK(white[0] == 160 && white[1] == 160 && white[2] == 160, "white not balanced: %d %d %d", white[0], white[1], white[2]);

    uint8_t strong[3] = {250, 40, 30};
    white_balance(strong, w);
    CHECK(strong[0] == 200 && strong[1] == 40 && strong[2] == 40, "proportional: %d %d %d", strong[0], strong[1], strong[2]);

    for (int c = 0; c < 3; c += 2)
    {
        // 白色中心 = 6 个中心里 sl 最小的
        int wf = 0;
        long best = 1L << 30;
        for (int f = 0; f < NFACE; f++)
        {
            Color x;
            x.setRGB(fake_colors[c][f * 9 + 8][0], fake_colors[c][f * 9 + 8][1], fake_colors[c][f * 9 + 8][2]);
            if (x.sl < best)
            {
                best = x.sl;
                wf = f;
            }
        }
        const uint8_t *wc = fake_colors[c][wf * 9 + 8];
        for (int f = 0; f < NFACE; f++)
            for (int p = 0; p < 9; p++)
            {
                uint8_t v[3] = {fake_colors[c][f * 9 + p][0], fake_colors[c][f * 9 + p][1], fake_colors[c][f * 9 + p][2]};
                white_balance(v, wc);
                cubeColors.setRGB(f, p, v);
            }
        bool ok = false;
        for (int t = 0; t < COLOR_STRATEGIES && !ok; t++)
        {
            byte cube[NFACE * 8];
            cubeColors.determine_colors(cube, t);
            ok = validator.valid_pieces(cube) && cubeSolver.solve(cube) && appliesToSolved(cube);
        }
        CHECK(ok, "cube %d not solvable after white balance", c);
    }
}

int main()
{
    testColorHsl();
    testWhiteBalance();
    testFakeCubes();
    testRandomScrambles();
    if (failures)
    {
        printf("%d check(s) failed\n", failures);
        return 1;
    }
    printf("all host tests passed\n");
    return 0;
}
