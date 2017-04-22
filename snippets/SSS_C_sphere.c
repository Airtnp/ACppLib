#include <math.h>
#include <windows.h>
#define PI 3.1415926
#define SX 8
#define SY 16
#define DX PI / SX
#define DY PI * 2 / SY
#define X(a, b) (cx + v[a][b].x * r), (cy + v[a][b].y * r)
typedef struct { double x, y; } Vec;
void calc(double i, double j, double rot, Vec* v) {
    double x = sin(i) * cos(j), y = sin(i) * sin(j), z = cos(i),
        s = sin(rot), c = cos(rot), c1 = 1 - c, u = 1 / sqrt(3), u2 = u * u;
    v->x = x * (c + u2 * c1) + y * (u2 * c1 - u * s) + z * (u2 * c1 + u * s);
    v->y = x * (u2 * c1 + u * s) + y * (c + u2 * c1) + z * (u2 * c1 - u * s);
}
int main() {
    HWND hwnd = GetConsoleWindow(); HDC hdc1 = GetDC(hwnd);
    double rot = 0;
    while(1) {
        RECT rect; GetClientRect(hwnd, &rect); int w = rect.right, h = rect.bottom, cx = w / 2, cy = h / 2, r = h * 0.375;
        HDC hdc2 = CreateCompatibleDC(hdc1); HBITMAP bmp = CreateCompatibleBitmap(hdc1, w, h); SelectObject(hdc2, bmp);
        SelectObject(hdc2, GetStockObject(WHITE_PEN));
        Vec v[SX + 1][SY + 1];
        for(int i = 0; i <= SX; ++i) for(int j = 0; j <= SY; ++j) calc(i * DX, j * DY, rot, &v[i][j]);
        for(int i = 0; i < SX; ++i) for(int j = 0; j < SY; ++j) {
            MoveToEx(hdc2, X(i, j), NULL); LineTo(hdc2, X(i + 1, j));
            MoveToEx(hdc2, X(i, j), NULL); LineTo(hdc2, X(i, j + 1));
        }
        BitBlt(hdc1, 0, 0, w, h, hdc2, 0, 0, SRCCOPY); DeleteObject(bmp); DeleteDC(hdc2);
        rot += 0.01; Sleep(5);
    }
}