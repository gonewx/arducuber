#include <iostream>
#include <string.h>

#include "../color.h"
#include "../fakedata.h"

using namespace std;

CubeColors cubeColors;

void printColor(Color color)
{
    cout << " rgb(" << +color.r << "," << +color.g << "," << +color.b << ") h:" << color.h << " s:" << color.s << " l:" << color.l << " sl:" << color.sl << " rg:" << cubeColors.clr_ratio(color.r, color.g) << " bg:" << cubeColors.clr_ratio(color.b, color.g) << " rb:" << cubeColors.clr_ratio(color.r, color.b) << endl;
}

void print()
{
    for (int i = 0; i < NFACE; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            Color color = cubeColors.getColor(i, j);
            printColor(color);
        }
        cout << endl;
    }
}

void printOrder(int count)
{
    for (int i = 0; i < count * NFACE; i++)
    {
        Color color = cubeColors.getColor(cubeColors.getOrder(i));
        printColor(color);
    }
}
int main(int argc, char **argv)
{
    Color colors[NFACE * 9];

    for (int i = 0; i < NFACE; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            cubeColors.setRGB(i, j, fake_colors[2][i * 9 + j]);
        }
    }

    print();

    cout << "Order..." << endl;
    cubeColors.sortCenter(0);
    printOrder(1);
    cout << endl;
    cubeColors.sortCorner(0);
    printOrder(4);
    cout << endl;
    cubeColors.sortEdge(0);
    printOrder(4);
    cout << endl;

}