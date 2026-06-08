#include <stdio.h>
#include <stdlib.h>

#define CANVAS_WIDTH 50
#define CANVAS_HEIGHT 30

char canvas[CANVAS_HEIGHT][CANVAS_WIDTH];

void initializeCanvas() {
    for (int i = 0; i < CANVAS_HEIGHT; i++)
        for (int j = 0; j < CANVAS_WIDTH; j++)
            canvas[i][j] = '_';
}

void displayCanvas() {
    printf("\n   ");
    for (int j = 0; j < CANVAS_WIDTH; j++) printf("%d", j % 10);
    printf("\n");
    for (int i = 0; i < CANVAS_HEIGHT; i++) {
        printf("%2d ", i);
        for (int j = 0; j < CANVAS_WIDTH; j++) printf("%c", canvas[i][j]);
        printf("\n");
    }
}

void drawPoint(int x, int y) {
    if (x >= 0 && x < CANVAS_WIDTH && y >= 0 && y < CANVAS_HEIGHT)
        canvas[y][x] = '*';
}

void drawLine(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1, dy = y2 - y1;
    int sx = (dx > 0) ? 1 : -1;
    int sy = (dy > 0) ? 1 : -1;
    dx *= sx; dy *= sy;
    int x = x1, y = y1;
    if (dy <= dx) {
        int p = 2 * dy - dx;
        for (int i = 0; i <= dx; i++) {
            drawPoint(x, y);
            x += sx;
            if (p >= 0) { y += sy; p -= 2 * dx; }
            p += 2 * dy;
        }
    } else {
        int p = 2 * dx - dy;
        for (int i = 0; i <= dy; i++) {
            drawPoint(x, y);
            y += sy;
            if (p >= 0) { x += sx; p -= 2 * dy; }
            p += 2 * dx;
        }
    }
}

void drawRectangle(int x1, int y1, int x2, int y2) {
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    drawLine(x1, y1, x2, y1);
    drawLine(x1, y2, x2, y2);
    drawLine(x1, y1, x1, y2);
    drawLine(x2, y1, x2, y2);
}

void drawCircle(int xc, int yc, int r) {
    int x = 0, y = r, p = 1 - r;
    drawPoint(xc + x, yc + y);
    drawPoint(xc - x, yc + y);
    drawPoint(xc + x, yc - y);
    drawPoint(xc - x, yc - y);
    drawPoint(xc + y, yc + x);
    drawPoint(xc - y, yc + x);
    drawPoint(xc + y, yc - x);
    drawPoint(xc - y, yc - x);
    while (y > 0) {
        if (p < 0) { x++; p += 2 * x + 1; }
        else { x++; y--; p += 2 * (x - y) + 1; }
        if (x <= y) {
            drawPoint(xc + x, yc + y);
            drawPoint(xc - x, yc + y);
            drawPoint(xc + x, yc - y);
            drawPoint(xc - x, yc - y);
            drawPoint(xc + y, yc + x);
            drawPoint(xc - y, yc + x);
            drawPoint(xc + y, yc - x);
            drawPoint(xc - y, yc - x);
        }
    }
}

int main() {
    int choice, x1, y1, x2, y2, xc, yc, r;
    initializeCanvas();
    while (1) {
        printf("\n1.Display 2.Rectangle 3.Circle 4.Exit\nChoose: ");
        scanf("%d", &choice);
        if (choice == 1) displayCanvas();
        else if (choice == 2) {
            scanf("%d %d %d %d", &x1, &y1, &x2, &y2);
            drawRectangle(x1, y1, x2, y2);
        } else if (choice == 3) {
            scanf("%d %d %d", &xc, &yc, &r);
            drawCircle(xc, yc, r);
        } else if (choice == 4) break;
    }
}
