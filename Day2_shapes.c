#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─────────────────────────────────────────
   DAY 2: Drawing Functions
   Builds on Day 1. Adds:
   - drawLine()      Bresenham algorithm
   - drawRectangle() 4 edges + corners
   - drawCircle()    Midpoint algorithm
   - drawTriangle()  3 connected lines
   ───────────────────────────────────────── */

#define ROWS 24
#define COLS 80

char canvas[ROWS][COLS];

void clearCanvas() {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            canvas[r][c] = ' ';
}

void plot(int r, int c, char ch) {
    if (r >= 0 && r < ROWS && c >= 0 && c < COLS)
        canvas[r][c] = ch;
}

void displayCanvas() {
    printf("\n+");
    for (int c = 0; c < COLS; c++) putchar('-');
    printf("+\n");
    for (int r = 0; r < ROWS; r++) {
        putchar('|');
        for (int c = 0; c < COLS; c++) putchar(canvas[r][c]);
        printf("|\n");
    }
    printf("+");
    for (int c = 0; c < COLS; c++) putchar('-');
    printf("+\n");
}

/* ── Bresenham's Line Algorithm ──────────────
   Uses integer arithmetic only (no floats).
   'err' accumulates slope; when it crosses
   a threshold the secondary axis steps by 1.
   sr/sc = step direction (+1 or -1).
   ─────────────────────────────────────────── */
void drawLine(int r1, int c1, int r2, int c2, char sym) {
    int dr = abs(r2 - r1);
    int dc = abs(c2 - c1);
    int sr = (r1 < r2) ? 1 : -1;
    int sc = (c1 < c2) ? 1 : -1;
    int err = dr - dc;

    while (1) {
        plot(r1, c1, sym);
        if (r1 == r2 && c1 == c2) break;
        int e2 = 2 * err;
        if (e2 > -dc) { err -= dc; r1 += sr; }
        if (e2 <  dr) { err += dr; c1 += sc; }
    }
}

/* ── Rectangle ───────────────────────────────
   Top/bottom: '_'   Left/right: '|'
   Four corners: '+'
   Normalises coords so order doesn't matter.
   ─────────────────────────────────────────── */
void drawRectangle(int r1, int c1, int r2, int c2) {
    if (r1 > r2) { int t = r1; r1 = r2; r2 = t; }
    if (c1 > c2) { int t = c1; c1 = c2; c2 = t; }

    for (int c = c1; c <= c2; c++) {
        plot(r1, c, '_');
        plot(r2, c, '_');
    }
    for (int r = r1; r <= r2; r++) {
        plot(r, c1, '|');
        plot(r, c2, '|');
    }
    plot(r1, c1, '+'); plot(r1, c2, '+');
    plot(r2, c1, '+'); plot(r2, c2, '+');
}

/* ── Midpoint Circle Algorithm ───────────────
   Starts at top (x=0, y=radius), steps one
   octant. 8-fold symmetry fills all 8 octants
   per step. Column doubled (×2) to compensate
   for tall character cells (aspect ratio fix).
   ─────────────────────────────────────────── */
void drawCircle(int cr, int cc, int radius) {
    int x = 0, y = radius;
    int d = 1 - radius;

    while (y >= x) {
        plot(cr + x, cc + 2 * y, '*');
        plot(cr - x, cc + 2 * y, '*');
        plot(cr + x, cc - 2 * y, '*');
        plot(cr - x, cc - 2 * y, '*');
        plot(cr + y, cc + 2 * x, '*');
        plot(cr - y, cc + 2 * x, '*');
        plot(cr + y, cc - 2 * x, '*');
        plot(cr - y, cc - 2 * x, '*');

        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

/* ── Triangle ────────────────────────────────
   Simply connects three vertices with lines.
   Reuses drawLine() for all three edges.
   ─────────────────────────────────────────── */
void drawTriangle(int r1, int c1, int r2, int c2, int r3, int c3) {
    drawLine(r1, c1, r2, c2, '*');
    drawLine(r2, c2, r3, c3, '*');
    drawLine(r3, c3, r1, c1, '*');
}

int main() {
    clearCanvas();

    /* Test all four shapes */
    drawLine(0, 0, 10, 20, '*');
    drawRectangle(1, 30, 10, 55);
    drawCircle(16, 15, 6);
    drawTriangle(14, 60, 14, 78, 22, 69);

    displayCanvas();
    printf("\nDay 2 complete: line, rectangle, circle, triangle all working!\n");
    return 0;
}
