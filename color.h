#ifndef CUBE_COLOR_H
#define CUBE_COLOR_H

#include "global.h"

const long cmax = 1024L;

#define CLR_R 0
#define CLR_B 1
#define CLR_O 2
#define CLR_G 3
#define CLR_W 4
#define CLR_Y 5

// red/orange 的区分方式数量, 见 sort_colors() 中的 switch (t % COLOR_STRATEGIES)
#define COLOR_STRATEGIES 6

// 参考 prime 版本
// 字段按取值范围使用较小的类型以节省 RAM (共 54 个实例); 计算过程全部使用 long,
// 因为 AVR 上 int 只有 16 位
class Color
{
public:
  Color()
  {
    setRGB(0, 0, 0);
  }

  //
  void setRGB(uint8_t red, uint8_t green, uint8_t blue)
  {
    long hh = 0;
    long ss = 0;
    long ll = 0;
    long v = red;

    sl = 0;

    if (green > v)
      v = green;
    if (blue > v)
      v = blue;
    long m = red;
    if (green < m)
      m = green;
    if (blue < m)
      m = blue;

    long vf = v + m;
    ll = vf / 2;

    if (ll > 0)
    {
      long vm = v - m;
      if (vm > 0)
      {
        if (vf <= cmax)
          vf = 2 * cmax - vf;
        ss = cmax * vm / vf;
        if (red == v)
          hh = 0 * cmax + cmax * (long(green) - long(blue)) / vm;
        else
        {
          if (green == v)
            hh = 2 * cmax + cmax * (long(blue) - long(red)) / vm;
          else
            hh = 4 * cmax + cmax * (long(red) - long(green)) / vm;
        }
      }
      hh += cmax; // rotate so R/B either side of 0
      hh = hh / 6;
      if (hh < 0)
        hh += cmax;
      else
      {
        if (hh >= cmax)
          hh -= cmax;
      }
      // Emphasize low saturation for bright colors (e.g. white)
      sl = cmax * ss / ll;
    }
    h = hh;
    s = ss;
    l = ll;
    r = red;
    g = green;
    b = blue;
  }

public:
  int16_t h;  // 0 .. cmax-1
  int16_t s;  // 0 .. cmax
  int16_t l;  // 0 .. 255
  long sl;    // 可达 cmax * cmax
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t clr;
};

// 白平衡: 按白色读数把各通道按比例缩放, 使白色的三个通道相等。
// 只缩放不截断 (旧代码先把各通道截断到白色读数, 红/黄等颜色的强通道会被削平);
// white 为默认值 (255, 255, 255) 时不改变读数。
inline void white_balance(uint8_t *rgb, const uint8_t *white)
{
  long avg = (long(white[0]) + white[1] + white[2]) / 3;
  for (int i = 0; i < 3; i++)
  {
    if (white[i] == 0)
      continue;
    long v = long(rgb[i]) * avg / white[i];
    rgb[i] = v > 255 ? 255 : uint8_t(v);
  }
}

class CubeColors
{
public:
  CubeColors()
  {
  }

  static int clr_ratio(long c0, long c1)
  {
    int ratio = 0;
    if (c0 < c1)
      ratio = -(2000 * (c1 - c0) / (c1 + c0));
    else if (c0 > c1)
    {
      ratio = (2000 * (c0 - c1) / (c1 + c0));
    }
    return ratio;
  }

  static bool cmp_h(const Color &c0, const Color &c1)
  {
    return c1.h > c0.h;
  }

  static bool cmp_s(const Color &c0, const Color &c1)
  {
    return c1.s > c0.s;
  }

  static bool cmp_sr(const Color &c0, const Color &c1)
  {
    return c1.s < c0.s;
  }

  static bool cmp_sl(const Color &c0, const Color &c1)
  {
    return c1.sl > c0.sl;
  }

  static bool cmp_slr(const Color &c0, const Color &c1)
  {
    return c1.sl < c0.sl;
  }

  static bool cmp_l(const Color &c0, const Color &c1)
  {
    return c1.l > c0.l;
  }

  static bool cmp_lr(const Color &c0, const Color &c1)
  {
    return c1.l < c0.l;
  }

  static bool cmp_r_gr(const Color &c0, const Color &c1)
  {
    return clr_ratio(c1.r, c1.g) < clr_ratio(c0.r, c0.g);
  }

  static bool cmp_r_g(const Color &c0, const Color &c1)
  {
    return clr_ratio(c1.r, c1.g) > clr_ratio(c0.r, c0.g);
  }

  static bool cmp_r_b(const Color &c0, const Color &c1)
  {
    return clr_ratio(c1.r, c1.b) > clr_ratio(c0.r, c0.b);
  }
  static bool cmp_r_br(const Color &c0, const Color &c1)
  {
    return clr_ratio(c1.r, c1.b) < clr_ratio(c0.r, c0.b);
  }
  static bool cmp_b_g(const Color &c0, const Color &c1)
  {
    return clr_ratio(c1.b, c1.g) > clr_ratio(c0.b, c0.g);
  }
  static bool cmp_b_gr(const Color &c0, const Color &c1)
  {
    return clr_ratio(c1.b, c1.g) < clr_ratio(c0.b, c0.g);
  }
  // s 起始坐标， n 数量
  void sort_clrs(const int s, const int n, bool (*cmp_fn)(const Color &, const Color &))
  {
    const int e = s + n - 2; // 4
    int is = s;              // 1
    int ie = e;              // 4
    do
    {
      int il = e + 2; // 4
      int ih = s - 2; // -1
      for (int i = is; i <= ie; i++)
      {
        if (cmp_fn(clrs[clr_ord[i + 1]], clrs[clr_ord[i]]))
        {
          int o = clr_ord[i];
          clr_ord[i] = clr_ord[i + 1];
          clr_ord[i + 1] = o;
          if (i < il)
            il = i;
          if (i > ih)
            ih = i;
        }
      }
      is = il - 1;
      if (is < s)
        is = s;
      ie = ih + 1;
      if (ie > e)
        ie = e;
    } while (is <= ie);
  }

  void sort_colors(const int t, const int s)
  {
    // 分离白色: Saturation 分离 下标为0 的白色
    sort_clrs(0, 6 * s, cmp_sl);

    //分离 Blue: 按 hue排序，为排除　R,O干扰，只取后三组，即 Y G B ，再按 RB　排序，最后一组即为 Blue
    sort_clrs(1 * s, 5 * s, cmp_h);
    sort_clrs(3 * s, 3 * s, cmp_r_br);

    // 分离 Yellow 和 Green: 按 RG 降序排序，　前二组为 (R O) , 后二组为排序的 Y G
    sort_clrs(1 * s, 4 * s, cmp_r_gr);

    // R O 排序
    sort_clrs(1 * s, 2 * s, cmp_h);

    // Red / Orange 以不同方式重试
    switch (t % COLOR_STRATEGIES)
    {
    case 0: /* already sorted by hue */
      break;
    case 1:
      sort_clrs(1 * s, 2 * s, cmp_r_g);
      break;
    case 2:
      sort_clrs(1 * s, 2 * s, cmp_b_g);
      break;
    case 3:
      sort_clrs(1 * s, 2 * s, cmp_r_b);
      break;
    case 4:
      sort_clrs(1 * s, 2 * s, cmp_slr);
      break;
    case 5:
      sort_clrs(1 * s, 2 * s, cmp_l);
      break;
    }

    int i = 0;
    for (i = 0; i < s; i++)
    {
      clrs[clr_ord[i]].clr = CLR_W;
#ifdef DEBUG
      printClr(clrs[clr_ord[i]]);
      Serial.print(F(" CLR_W "));
#endif
    }

    for (; i < 2 * s; i++)
    {
      clrs[clr_ord[i]].clr = CLR_R;
#ifdef DEBUG
      printClr(clrs[clr_ord[i]]);
      Serial.print(F(" CLR_R "));
#endif
    }
    for (; i < 3 * s; i++)
    {
      clrs[clr_ord[i]].clr = CLR_O;
#ifdef DEBUG
      printClr(clrs[clr_ord[i]]);
      Serial.print(F(" CLR_O "));
#endif
    }
    for (; i < 4 * s; i++)
    {
      clrs[clr_ord[i]].clr = CLR_Y;
#ifdef DEBUG
      printClr(clrs[clr_ord[i]]);
      Serial.print(F(" CLR_Y "));
#endif
    }
    for (; i < 5 * s; i++)
    {
      clrs[clr_ord[i]].clr = CLR_G;
#ifdef DEBUG
      printClr(clrs[clr_ord[i]]);
      Serial.print(F(" CLR_G "));
#endif
    }
    for (; i < 6 * s; i++)
    {
      clrs[clr_ord[i]].clr = CLR_B;
#ifdef DEBUG
      printClr(clrs[clr_ord[i]]);
      Serial.print(F(" CLR_B "));
      Serial.println();
#endif
    }
  }

  void sortCenter(int t)
  {
    for (int i = 0; i < NFACE; i++)
    {
      clr_ord[i] = pos(i, 8);
    }
    sort_colors(t, 1);
  }

  void sortCorner(int t)
  {
    for (int i = 0; i < NFACE; i++)
    {
      clr_ord[4 * i + 0] = pos(i, 0);
      clr_ord[4 * i + 1] = pos(i, 2);
      clr_ord[4 * i + 2] = pos(i, 4);
      clr_ord[4 * i + 3] = pos(i, 6);
    }
    sort_colors(t, 4);
  }

  void sortEdge(int t)
  {
    for (int i = 0; i < NFACE; i++)
    {
      clr_ord[4 * i + 0] = pos(i, 1);
      clr_ord[4 * i + 1] = pos(i, 3);
      clr_ord[4 * i + 2] = pos(i, 5);
      clr_ord[4 * i + 3] = pos(i, 7);
    }
    sort_colors(t, 4);
  }

  void
  determine_colors(uint8_t *cube, const int t)
  {
    sortCenter(t);
    sortCorner(t);
    sortEdge(t);

    for (int f = 0; f < NFACE; f++)
    {
      clr_map[clrs[pos(f, 8)].clr] = f;
    }
    for (int f = 0; f < NFACE; f++)
    {
      for (int o = 0; o < 8; o++)
      {
        cube[f * 8 + o] = clr_map[clrs[pos(f, o)].clr];
      }
    }
  }
  //
  void setRGB(int face, int piece, uint8_t *rgb)
  {
    clrs[pos(face, piece)].setRGB(rgb[0], rgb[1], rgb[2]);
  }

  Color getColor(int face, int piece)
  {
    return clrs[pos(face, piece)];
  }

  int getOrder(int index) { return clr_ord[index]; }

  Color getColor(int index)
  {
    return clrs[index];
  }

  uint8_t getClr(int face, int piece)
  {

    const Color &clr = clrs[pos(face, piece)];

    uint8_t c = 8; // white
    if (clr.sl > 50)
    {
      c = uint8_t(8 * clr.h / cmax);
    }
    return c;
  }

  int pos(int face, int piece)
  {
    return face * 9 + piece;
  }

#ifdef __AVR__
  void print()
  {
    Serial.println();
    for (int i = 0; i < NFACE * 9; i++)
    {
      if (i % 9 == 0)
      {
        Serial.println();
        Serial.print(F("face: "));
        Serial.print(i / 9);
      }
      else
      {
        if (i % 3 == 0)
        {
          Serial.println();
        }
      }

      printClr(clrs[i]);
    }
    Serial.println();
  }
  void printClr(const Color &clr)
  {
    Serial.print(F(" rgb("));
    Serial.print(clr.r);
    Serial.print(F(","));
    Serial.print(clr.g);
    Serial.print(F(","));
    Serial.print(clr.b);
    Serial.print(F(") "));
    Serial.print(F(" h:"));
    Serial.print(clr.h);
    Serial.print(F(" s:"));
    Serial.print(clr.s);
    Serial.print(F(" l:"));
    Serial.print(clr.l);
    Serial.print(F(" sl:"));
    Serial.print(clr.sl);
    Serial.print(F(" clr:"));
    Serial.print(clr.clr);
    Serial.print(F(" clr_ratio rg:"));
    Serial.print(clr_ratio(clr.r, clr.g));
    Serial.print(F(" clr_ratio bg:"));
    Serial.print(clr_ratio(clr.b, clr.g));
    Serial.print(F(" clr_ratio rb:"));
    Serial.print(clr_ratio(clr.r, clr.b));
  }
  void print(int face)
  {
    Serial.println();
    Serial.print(F("face: "));
    Serial.print(face);

    for (int i = 0; i < 9; i++)
    {

      int p = pos(face, i);
      Serial.print(F(" rgb("));
      Serial.print(clrs[p].r);
      Serial.print(F(","));
      Serial.print(clrs[p].g);
      Serial.print(F(","));
      Serial.print(clrs[p].b);
      Serial.print(F(") "));
    }
    Serial.println();
  }
#endif

private:
  Color clrs[NFACE * 9];
  uint8_t clr_ord[NFACE * 4];
  uint8_t clr_map[NFACE];
};

#endif