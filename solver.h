#ifndef CUBE_SOLVER_H
#define CUBE_SOLVER_H

#include <avr/pgmspace.h>

#include "global.h"
#include "mcmoves.h"
#include "validator.h"
#ifdef ARDUINO
// manipulate() 需要驱动电机; 主机端单元测试 (test/host) 不编译这部分
#include "tilt.h"
#include "turn.h"
#endif

//-----------------------------------------------------------------------------
// Routines to display, transform and solve a cube
//-----------------------------------------------------------------------------

// Target moves for solve to finish early to save time
// Average: 42 moves in 5 seconds
// Range:   about 37 to 47 moves in 1 to 10 seconds

#define TARGET 42

#define NPIECE 3   // maximum number of pieces indexed in one phase
#define MV_MAX 100 // maximum number of moves per solve

const byte opposite[] = {CUBE_DOWN, CUBE_BACK, CUBE_UP, CUBE_FRONT, CUBE_LEFT, CUBE_RIGHT};

#define MAP(UU, FF) (imap[((UU)*NFACE) + (FF)] * NFACE)

const int8_t imap[] = { // -1: 非法的 (上, 前) 组合
    /*       U   F   D   B   R   L */
    /* U */ -1, 0, -1, 1, 2, 3,
    /* F */ 4, -1, 5, -1, 6, 7,
    /* D */ -1, 8, -1, 9, 10, 11,
    /* B */ 12, -1, 13, -1, 14, 15,
    /* R */ 16, 17, 18, 19, -1, -1,
    /* L */ 20, 21, 22, 23, -1, -1};

const byte fmap[] = {
    /* -- */
    /* UF */ CUBE_UP, CUBE_FRONT, CUBE_DOWN, CUBE_BACK, CUBE_RIGHT, CUBE_LEFT,
    /* -- */
    /* UB */ CUBE_UP, CUBE_BACK, CUBE_DOWN, CUBE_FRONT, CUBE_LEFT, CUBE_RIGHT,
    /* UR */ CUBE_UP, CUBE_RIGHT, CUBE_DOWN, CUBE_LEFT, CUBE_BACK, CUBE_FRONT,
    /* UL */ CUBE_UP, CUBE_LEFT, CUBE_DOWN, CUBE_RIGHT, CUBE_FRONT, CUBE_BACK,

    /* FU */ CUBE_FRONT, CUBE_UP, CUBE_BACK, CUBE_DOWN, CUBE_LEFT, CUBE_RIGHT,
    /* -- */
    /* FD */ CUBE_FRONT, CUBE_DOWN, CUBE_BACK, CUBE_UP, CUBE_RIGHT, CUBE_LEFT,
    /* -- */
    /* FR */ CUBE_FRONT, CUBE_RIGHT, CUBE_BACK, CUBE_LEFT, CUBE_UP, CUBE_DOWN,
    /* FL */ CUBE_FRONT, CUBE_LEFT, CUBE_BACK, CUBE_RIGHT, CUBE_DOWN, CUBE_UP,

    /* -- */
    /* DF */ CUBE_DOWN, CUBE_FRONT, CUBE_UP, CUBE_BACK, CUBE_LEFT, CUBE_RIGHT,
    /* -- */
    /* DB */ CUBE_DOWN, CUBE_BACK, CUBE_UP, CUBE_FRONT, CUBE_RIGHT, CUBE_LEFT,
    /* DR */ CUBE_DOWN, CUBE_RIGHT, CUBE_UP, CUBE_LEFT, CUBE_FRONT, CUBE_BACK,
    /* DL */ CUBE_DOWN, CUBE_LEFT, CUBE_UP, CUBE_RIGHT, CUBE_BACK, CUBE_FRONT,

    /* BU */ CUBE_BACK, CUBE_UP, CUBE_FRONT, CUBE_DOWN, CUBE_RIGHT, CUBE_LEFT,
    /* -- */
    /* BD */ CUBE_BACK, CUBE_DOWN, CUBE_FRONT, CUBE_UP, CUBE_LEFT, CUBE_RIGHT,
    /* -- */
    /* BR */ CUBE_BACK, CUBE_RIGHT, CUBE_FRONT, CUBE_LEFT, CUBE_DOWN, CUBE_UP,
    /* BL */ CUBE_BACK, CUBE_LEFT, CUBE_FRONT, CUBE_RIGHT, CUBE_UP, CUBE_DOWN,

    /* RU */ CUBE_RIGHT, CUBE_UP, CUBE_LEFT, CUBE_DOWN, CUBE_FRONT, CUBE_BACK,
    /* RF */ CUBE_RIGHT, CUBE_FRONT, CUBE_LEFT, CUBE_BACK, CUBE_DOWN, CUBE_UP,
    /* RD */ CUBE_RIGHT, CUBE_DOWN, CUBE_LEFT, CUBE_UP, CUBE_BACK, CUBE_FRONT,
    /* RB */ CUBE_RIGHT, CUBE_BACK, CUBE_LEFT, CUBE_FRONT, CUBE_UP, CUBE_DOWN,
    /* -- */
    /* -- */

    /* LU */ CUBE_LEFT, CUBE_UP, CUBE_RIGHT, CUBE_DOWN, CUBE_BACK, CUBE_FRONT,
    /* LF */ CUBE_LEFT, CUBE_FRONT, CUBE_RIGHT, CUBE_BACK, CUBE_UP, CUBE_DOWN,
    /* LD */ CUBE_LEFT, CUBE_DOWN, CUBE_RIGHT, CUBE_UP, CUBE_FRONT, CUBE_BACK,
    /* LB */ CUBE_LEFT, CUBE_BACK, CUBE_RIGHT, CUBE_FRONT, CUBE_DOWN, CUBE_UP
    /* -- */
    /* -- */
};

const byte omap[] = {
    /* -- */
    /* UF */ 0, 0, 0, 0, 0, 0,
    /* -- */
    /* UB */ 4, 4, 4, 4, 0, 0,
    /* UR */ 6, 0, 2, 4, 4, 0,
    /* UL */ 2, 0, 6, 4, 0, 4,

    /* FU */ 4, 4, 4, 4, 2, 6,
    /* -- */
    /* FD */ 0, 0, 0, 0, 6, 2,
    /* -- */
    /* FR */ 6, 6, 2, 6, 4, 0,
    /* FL */ 2, 2, 6, 2, 0, 4,

    /* -- */
    /* DF */ 4, 4, 4, 4, 4, 4,
    /* -- */
    /* DB */ 0, 0, 0, 0, 4, 4,
    /* DR */ 6, 4, 2, 0, 4, 0,
    /* DL */ 2, 4, 6, 0, 0, 4,

    /* BU */ 0, 0, 0, 0, 2, 6,
    /* -- */
    /* BD */ 4, 4, 4, 4, 6, 2,
    /* -- */
    /* BR */ 6, 2, 2, 2, 4, 0,
    /* BL */ 2, 6, 6, 6, 0, 4,

    /* RU */ 4, 2, 0, 6, 2, 2,
    /* RF */ 2, 2, 2, 6, 2, 2,
    /* RD */ 0, 2, 4, 6, 2, 2,
    /* RB */ 6, 2, 6, 6, 2, 2,
    /* -- */
    /* -- */

    /* LU */ 4, 6, 0, 2, 6, 6,
    /* LF */ 6, 6, 6, 2, 6, 6,
    /* LD */ 0, 6, 4, 2, 6, 6,
    /* LB */ 2, 6, 2, 2, 6, 6,
    /* -- */
    /* -- */
};

class CubeSolver
{
public:
    CubeSolver()
    {
    }

    void add_mv(int f, int r)
    {
        int i = mv_n;
        bool mrg = false;
        while (i > 0)
        {
            i--;
            int fi = mv_f[i];
            if (f == fi)
            {
                r += mv_r[i];
                // r = rfix(r);
                r = rfix(r);
                if (r != 0)
                {
                    mv_r[i] = r;
                }
                else
                {
                    mv_n--;
                    while (i < mv_n)
                    {
                        mv_f[i] = mv_f[i + 1];
                        mv_r[i] = mv_r[i + 1];
                        i++;
                    }
                }
                mrg = true;
                break;
            }
            if (opposite[f] != fi)
                break;
        }
        if (!mrg)
        {
            mv_f[mv_n] = f;
            mv_r[mv_n] = rfix(r);
            mv_n++;
        }
    }

    void rot_edges(byte *cube, int r,
                   int f0, int o0, int f1, int o1,
                   int f2, int o2, int f3, int o3)
    {
        f0 *= 8;
        f1 *= 8;
        f2 *= 8;
        f3 *= 8;
        o0 += f0;
        o1 += f1;
        o2 += f2;
        o3 += f3;
        byte p;
        switch (r)
        {
        case 1:
            p = cube[o3];
            cube[o3] = cube[o2];
            cube[o2] = cube[o1];
            cube[o1] = cube[o0];
            cube[o0] = p;
            o0++;
            o1++;
            o2++;
            o3++;
            p = cube[o3];
            cube[o3] = cube[o2];
            cube[o2] = cube[o1];
            cube[o1] = cube[o0];
            cube[o0] = p;
            o0 = f0 + ((o0 + 1) & 7);
            o1 = f1 + ((o1 + 1) & 7);
            o2 = f2 + ((o2 + 1) & 7);
            o3 = f3 + ((o3 + 1) & 7);
            p = cube[o3];
            cube[o3] = cube[o2];
            cube[o2] = cube[o1];
            cube[o1] = cube[o0];
            cube[o0] = p;
            break;

        case 2:
            p = cube[o0];
            cube[o0] = cube[o2];
            cube[o2] = p;
            p = cube[o1];
            cube[o1] = cube[o3];
            cube[o3] = p;
            o0++;
            o1++;
            o2++;
            o3++;
            p = cube[o0];
            cube[o0] = cube[o2];
            cube[o2] = p;
            p = cube[o1];
            cube[o1] = cube[o3];
            cube[o3] = p;
            o0 = f0 + ((o0 + 1) & 7);
            o1 = f1 + ((o1 + 1) & 7);
            o2 = f2 + ((o2 + 1) & 7);
            o3 = f3 + ((o3 + 1) & 7);
            p = cube[o0];
            cube[o0] = cube[o2];
            cube[o2] = p;
            p = cube[o1];
            cube[o1] = cube[o3];
            cube[o3] = p;
            break;

        case 3:
            p = cube[o0];
            cube[o0] = cube[o1];
            cube[o1] = cube[o2];
            cube[o2] = cube[o3];
            cube[o3] = p;
            o0++;
            o1++;
            o2++;
            o3++;
            p = cube[o0];
            cube[o0] = cube[o1];
            cube[o1] = cube[o2];
            cube[o2] = cube[o3];
            cube[o3] = p;
            o0 = f0 + ((o0 + 1) & 7);
            o1 = f1 + ((o1 + 1) & 7);
            o2 = f2 + ((o2 + 1) & 7);
            o3 = f3 + ((o3 + 1) & 7);
            p = cube[o0];
            cube[o0] = cube[o1];
            cube[o1] = cube[o2];
            cube[o2] = cube[o3];
            cube[o3] = p;
            break;

        default:
            Serial.print(F("error rot_edges:"));
            Serial.println(r);
        }
    }

    void rotate(byte *cube, int f, int r)
    {
        r &= 3;
        switch (f)
        {
        case CUBE_UP:
            rot_edges(cube, r, CUBE_BACK, 4, CUBE_RIGHT, 0, CUBE_FRONT, 0, CUBE_LEFT, 0);
            break;
        case CUBE_FRONT:
            rot_edges(cube, r, CUBE_UP, 4, CUBE_RIGHT, 6, CUBE_DOWN, 0, CUBE_LEFT, 2);
            break;
        case CUBE_DOWN:
            rot_edges(cube, r, CUBE_FRONT, 4, CUBE_RIGHT, 4, CUBE_BACK, 0, CUBE_LEFT, 4);
            break;
        case CUBE_BACK:
            rot_edges(cube, r, CUBE_DOWN, 4, CUBE_RIGHT, 2, CUBE_UP, 0, CUBE_LEFT, 6);
            break;
        case CUBE_RIGHT:
            rot_edges(cube, r, CUBE_UP, 2, CUBE_BACK, 2, CUBE_DOWN, 2, CUBE_FRONT, 2);
            break;
        case CUBE_LEFT:
            rot_edges(cube, r, CUBE_UP, 6, CUBE_FRONT, 6, CUBE_DOWN, 6, CUBE_BACK, 6);
            break;
        default:
            Serial.print(F("error rot face:"));
            Serial.println(f);
        }
        f *= 8;
        byte p;
        switch (r)
        {
        case 1:
            p = cube[f + 7];
            cube[f + 7] = cube[f + 5];
            cube[f + 5] = cube[f + 3];
            cube[f + 3] = cube[f + 1];
            cube[f + 1] = p;
            p = cube[f + 6];
            cube[f + 6] = cube[f + 4];
            cube[f + 4] = cube[f + 2];
            cube[f + 2] = cube[f];
            cube[f] = p;
            break;

        case 2:
            p = cube[f + 1];
            cube[f + 1] = cube[f + 5];
            cube[f + 5] = p;
            p = cube[f + 3];
            cube[f + 3] = cube[f + 7];
            cube[f + 7] = p;
            p = cube[f];
            cube[f] = cube[f + 4];
            cube[f + 4] = p;
            p = cube[f + 2];
            cube[f + 2] = cube[f + 6];
            cube[f + 6] = p;
            break;

        case 3:
            p = cube[f + 1];
            cube[f + 1] = cube[f + 3];
            cube[f + 3] = cube[f + 5];
            cube[f + 5] = cube[f + 7];
            cube[f + 7] = p;
            p = cube[f];
            cube[f] = cube[f + 2];
            cube[f + 2] = cube[f + 4];
            cube[f + 4] = cube[f + 6];
            cube[f + 6] = p;
            break;

        default:
            Serial.print(F("error rot:"));
            Serial.println(r);
        }
    }

    void index_init()
    {
        idx_nc = 0;
        idx_ne = 0;
        idx = 0;
    }

    void index_last()
    {
        idx = ((idx >> 2) << 1) | (idx & 1);
    }
    void index_corner(byte *cube, byte f0, byte f1, byte f2)
    {
        int ic = validator.find_corner(cube, f0, f1, f2);
        for (int i = 0; i < idx_nc; i++)
        {
            if (ic > idx_idx[i])
                ic -= 3;
        }
        idx = (idx * idx_ic) + ic;
        idx_idx[idx_nc++] = ic;
        idx_ic -= 3;
    }

    void index_edge(byte *cube, byte f0, byte f1)
    {
        int ie = validator.find_edge(cube, f0, f1);
        for (int i = 0; i < idx_ne; i++)
        {
            if (ie > idx_idx[i])
                ie -= 2;
        }
        idx = (idx * idx_ie) + ie;
        idx_idx[idx_ne++] = ie;
        idx_ie -= 2;
    }

    void copy_cube(byte *cube0, byte *cube1)
    {
        int o = 0;
        for (byte f = 0; f < NFACE; f++)
            for (int i = 0; i < 8; i++)
            {
                cube0[o] = cube1[o];
                o++;
            }
    }

    // #define RFIX(RR) ((((RR) + 1) & 3) - 1) // Normalise to range -1 to 2
    int rfix(int rr)
    {
        return ((rr + 1) & 3) - 1;
    }

    void solve_phase(byte *cube, int mtb, const byte *mtd, int len, bool dorot = true)
    {

        int sz = len / mtb;
        idx = sz - idx;

        if (idx > 0)
        {
            int i = (idx - 1) * mtb;
            byte b = pgm_read_byte_near(mtd + i);
            i++;

            if (b != 0xFF)
            {
                const int mvm = mtb * 2 - 1;
                int mv = 0;
                int f0 = b / 3;
                int r0 = rfix(b - (f0 * 3) + 1);
                add_mv(f0, r0);
                if (dorot)
                    rotate(cube, f0, r0);
                mv++;
                while (mv < mvm)
                {
                    b >>= 4;
                    if ((mv & 1) != 0)
                    {
                        b = pgm_read_byte_near(mtd + i);
                        i++;
                    }
                    byte b0 = b & 0xF;
                    if (b0 == 0xF)
                        break;
                    int f1 = b0 / 3;
                    r0 = rfix(b0 - (f1 * 3) + 1);
                    if (f1 >= f0)
                        f1++;
                    f0 = f1;
                    add_mv(f0, r0);
                    if (dorot)
                        rotate(cube, f0, r0);
                    mv++;
                }
            }
        }
    }

    void solve_one(byte *cube, bool dorot)
    {
        mv_n = 0;

        idx_ic = 24;
        idx_ie = 24;

        index_init();
        index_edge(cube, CUBE_DOWN, CUBE_FRONT);
        index_edge(cube, CUBE_DOWN, CUBE_RIGHT);
        solve_phase(cube, mtb0, mtd0, sizeof(mtd0) / sizeof(mtd0[0]));

        index_init();
        index_corner(cube, CUBE_DOWN, CUBE_FRONT, CUBE_RIGHT);
        index_edge(cube, CUBE_FRONT, CUBE_RIGHT);
        solve_phase(cube, mtb1, mtd1, sizeof(mtd1) / sizeof(mtd1[0]));

        index_init();
        index_edge(cube, CUBE_DOWN, CUBE_BACK);
        solve_phase(cube, mtb2, mtd2, sizeof(mtd2) / sizeof(mtd2[0]));

        index_init();
        index_corner(cube, CUBE_DOWN, CUBE_RIGHT, CUBE_BACK);
        index_edge(cube, CUBE_RIGHT, CUBE_BACK);
        solve_phase(cube, mtb3, mtd3, sizeof(mtd3) / sizeof(mtd3[0]));

        index_init();
        index_edge(cube, CUBE_DOWN, CUBE_LEFT);
        solve_phase(cube, mtb4, mtd4, sizeof(mtd4) / sizeof(mtd4[0]));

        index_init();
        index_corner(cube, CUBE_DOWN, CUBE_BACK, CUBE_LEFT);
        index_edge(cube, CUBE_BACK, CUBE_LEFT);
        solve_phase(cube, mtb5, mtd5, sizeof(mtd5) / sizeof(mtd5[0]));

        index_init();
        index_corner(cube, CUBE_DOWN, CUBE_LEFT, CUBE_FRONT);
        index_edge(cube, CUBE_LEFT, CUBE_FRONT);
        solve_phase(cube, mtb6, mtd6, sizeof(mtd6) / sizeof(mtd6[0]));

        index_init();
        index_corner(cube, CUBE_UP, CUBE_RIGHT, CUBE_FRONT);
        index_corner(cube, CUBE_UP, CUBE_FRONT, CUBE_LEFT);
        index_corner(cube, CUBE_UP, CUBE_LEFT, CUBE_BACK);
        solve_phase(cube, mtb7, mtd7, sizeof(mtd7) / sizeof(mtd7[0]));

        index_init();
        index_edge(cube, CUBE_UP, CUBE_RIGHT);
        index_edge(cube, CUBE_UP, CUBE_FRONT);
        index_edge(cube, CUBE_UP, CUBE_LEFT);
        index_last();
        solve_phase(cube, mtb8, mtd8, sizeof(mtd8) / sizeof(mtd8[0]), dorot);
    }

    bool solved(byte *cube)
    {
        int o = 0;
        for (byte f = 0; f < NFACE; f++)
            for (int i = 0; i < 8; i++)
            {
                if (cube[o++] != f)
                    return false;
            }
        return true;
    }

    bool solve_nomap(byte *cube)
    {
        Serial.println(F("solve_nomap:"));

        solve_n = 0;
        copy_cube(solve_cube, cube);
        solve_one(solve_cube, true);

        if (!solved(solve_cube))
        {
            Serial.println(F("false"));
            return false;
        }

        // Record the solution
        solve_n = mv_n;
        for (int i = 0; i < mv_n; i++)
        {
            solve_fce[i] = mv_f[i];
            solve_rot[i] = mv_r[i];
        }
        return true;
    }

    void solve_remap(byte *cube, int map)
    {
        for (int f = 0; f < NFACE; f++)
            pmap[fmap[map + f]] = f;

        for (int f = 0; f < NFACE; f++)
        {
            int fs = fmap[map + f] * 8;
            int fd = f * 8;
            int os = omap[map + f];
            for (int od = 0; od < 8; od++)
            {
                solve_cube[fd + od] = pmap[cube[fs + os]];
                os = (os + 1) & 7;
            }
        }

        solve_one(solve_cube, false);
        if (mv_n < solve_n)
        {
            solve_n = mv_n;
            for (int i = 0; i < mv_n; i++)
            {
                solve_fce[i] = fmap[map + mv_f[i]];
                solve_rot[i] = mv_r[i];
            }
        }
    }

    bool solve(byte *cube)
    {
        if (!solve_nomap(cube))
            return false;

        // flash_blue();

        if (solve_n <= TARGET)
            return true;

        for (int i = 1; i < NFACE * 4; i++)
        {
            solve_remap(cube, i * NFACE);
            if (solve_n <= TARGET)
                return true;
        }

        return true;
    }

#ifdef ARDUINO
    void manipulate(byte *cube, int f, int r, int rn)
    {
        int map = MAP(uc, fc);
        if (fmap[map + CUBE_UP] == f)
        {
            uc = fmap[map + CUBE_DOWN];
            fc = fmap[map + CUBE_BACK];
            delay(50);
            Tilt(2);
        }
        else if (fmap[map + CUBE_FRONT] == f)
        {
            uc = fmap[map + CUBE_BACK];
            fc = fmap[map + CUBE_UP];
            delay(50);
            Tilt();
        }
        else if (fmap[map + CUBE_DOWN] == f)
        {
            TiltHold();
        }
        else if (fmap[map + CUBE_BACK] == f)
        {
            uc = fmap[map + CUBE_FRONT];
            fc = fmap[map + CUBE_UP];
            TiltAway();
            Spin(-2, 0);
            Tilt();
        }
        else if (fmap[map + CUBE_RIGHT] == f)
        {
            uc = fmap[map + CUBE_LEFT];
            fc = fmap[map + CUBE_UP];
            TiltAway();
            Spin(-1, 0);
            Tilt();
        }
        else
        { // fmap[map+L] == f
            uc = fmap[map + CUBE_RIGHT];
            fc = fmap[map + CUBE_UP];
            TiltAway();
            Spin(1, 0);
            Tilt();
        }
        delay(150);
        rotate(cube, f, r);
        // display_cube(cube);

        Serial.print(F("manipulate; r:"));
        Serial.print(r);
        Serial.print(F(" rn:"));
        Serial.println(rn);

        Spin(r);
    }
#endif

public:
    bool valid;
    int solve_n;
    byte solve_fce[MV_MAX];
    int8_t solve_rot[MV_MAX]; // -1 .. 2
    byte solve_cube[NFACE * 8];
    int mv_n;
    byte mv_f[MV_MAX];
    int8_t mv_r[MV_MAX]; // -1 .. 2
    int idx_ic;
    int idx_ie;
    int idx_idx[NPIECE];
    int idx_nc;
    int idx_ne;
    int idx;

    int uc = CUBE_UP;
    int fc = CUBE_FRONT;

    byte pmap[NFACE];

    Validator validator;
};

#endif
