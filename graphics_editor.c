#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ─────────────────────────────────────────────
   CONSTANTS & TYPES
   ───────────────────────────────────────────── */
#define ROWS 40
#define COLS 80
#define MAX_OBJECTS 50
#define PI 3.14159265358979323846

typedef enum { SHAPE_LINE, SHAPE_RECTANGLE, SHAPE_CIRCLE, SHAPE_TRIANGLE } ShapeType;

typedef struct {
    ShapeType type;
    int active;
    /* Generic params: meaning depends on shape type */
    int x1, y1, x2, y2, x3, y3;   /* endpoints / corners / third vertex */
    int radius;
    char symbol;
} Shape;

/* ─────────────────────────────────────────────
   GLOBAL CANVAS & OBJECT LIST
   ───────────────────────────────────────────── */
char canvas[ROWS][COLS];
Shape objects[MAX_OBJECTS];
int objectCount = 0;

/* ─────────────────────────────────────────────
   CANVAS UTILITIES
   ───────────────────────────────────────────── */

/* Fill every cell with a space */
void clearCanvas(void) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            canvas[r][c] = ' ';
}

/* Print the canvas surrounded by a border */
void displayCanvas(void) {
    printf("\n+");
    for (int c = 0; c < COLS; c++) putchar('-');
    printf("+\n");

    for (int r = 0; r < ROWS; r++) {
        putchar('|');
        for (int c = 0; c < COLS; c++)
            putchar(canvas[r][c]);
        printf("|\n");
    }

    printf("+");
    for (int c = 0; c < COLS; c++) putchar('-');
    printf("+\n");
}

/* Safe pixel-plot: bounds check before writing */
void plot(int r, int c, char ch) {
    if (r >= 0 && r < ROWS && c >= 0 && c < COLS)
        canvas[r][c] = ch;
}

/* ─────────────────────────────────────────────
   DRAWING PRIMITIVES
   ───────────────────────────────────────────── */

/*
 * drawLine – Bresenham's line algorithm
 * Draws a line from (r1,c1) to (r2,c2) using '*' (or custom symbol).
 */
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

/*
 * drawRectangle – draws four edges using '_' for horizontal, '|' for vertical,
 * and '+' at corners.
 */
void drawRectangle(int r1, int c1, int r2, int c2) {
    /* Normalise so r1 <= r2 and c1 <= c2 */
    if (r1 > r2) { int t = r1; r1 = r2; r2 = t; }
    if (c1 > c2) { int t = c1; c1 = c2; c2 = t; }

    /* Top & bottom edges */
    for (int c = c1; c <= c2; c++) {
        plot(r1, c, '_');
        plot(r2, c, '_');
    }
    /* Left & right edges */
    for (int r = r1; r <= r2; r++) {
        plot(r, c1, '|');
        plot(r, c2, '|');
    }
    /* Corners */
    plot(r1, c1, '+'); plot(r1, c2, '+');
    plot(r2, c1, '+'); plot(r2, c2, '+');
}

/*
 * drawCircle – midpoint circle algorithm.
 * Because character cells are taller than wide, we scale the column
 * offset by 2 to achieve a roughly circular appearance.
 */
void drawCircle(int cr, int cc, int radius) {
    int x = 0, y = radius;
    int d = 1 - radius;

    while (y >= x) {
        /* Plot all eight octants, compensating for aspect ratio */
        plot(cr + x, cc + 2*y, '*');
        plot(cr - x, cc + 2*y, '*');
        plot(cr + x, cc - 2*y, '*');
        plot(cr - x, cc - 2*y, '*');
        plot(cr + y, cc + 2*x, '*');
        plot(cr - y, cc + 2*x, '*');
        plot(cr + y, cc - 2*x, '*');
        plot(cr - y, cc - 2*x, '*');

        if (d < 0) {
            d += 2*x + 3;
        } else {
            d += 2*(x - y) + 5;
            y--;
        }
        x++;
    }
}

/*
 * drawTriangle – connects three vertices with Bresenham lines.
 */
void drawTriangle(int r1, int c1, int r2, int c2, int r3, int c3) {
    drawLine(r1, c1, r2, c2, '*');
    drawLine(r2, c2, r3, c3, '*');
    drawLine(r3, c3, r1, c1, '*');
}

/* ─────────────────────────────────────────────
   REDRAW  – rebuild canvas from object list
   ───────────────────────────────────────────── */
void redraw(void) {
    clearCanvas();
    for (int i = 0; i < objectCount; i++) {
        Shape *s = &objects[i];
        if (!s->active) continue;
        switch (s->type) {
            case SHAPE_LINE:
                drawLine(s->y1, s->x1, s->y2, s->x2, s->symbol);
                break;
            case SHAPE_RECTANGLE:
                drawRectangle(s->y1, s->x1, s->y2, s->x2);
                break;
            case SHAPE_CIRCLE:
                drawCircle(s->y1, s->x1, s->radius);
                break;
            case SHAPE_TRIANGLE:
                drawTriangle(s->y1, s->x1, s->y2, s->x2, s->y3, s->x3);
                break;
        }
    }
}

/* ─────────────────────────────────────────────
   OBJECT MANAGEMENT
   ───────────────────────────────────────────── */

int addObject(Shape s) {
    if (objectCount >= MAX_OBJECTS) {
        printf("Error: maximum object limit reached.\n");
        return -1;
    }
    s.active = 1;
    objects[objectCount] = s;
    return objectCount++;
}

void deleteObject(int id) {
    if (id < 0 || id >= objectCount || !objects[id].active) {
        printf("Error: invalid object ID %d\n", id);
        return;
    }
    objects[id].active = 0;
    printf("Object %d deleted.\n", id);
    redraw();
}

void modifyLine(int id, int x1, int y1, int x2, int y2) {
    objects[id].x1 = x1; objects[id].y1 = y1;
    objects[id].x2 = x2; objects[id].y2 = y2;
    redraw();
}

void modifyRectangle(int id, int x1, int y1, int x2, int y2) {
    objects[id].x1 = x1; objects[id].y1 = y1;
    objects[id].x2 = x2; objects[id].y2 = y2;
    redraw();
}

void modifyCircle(int id, int cx, int cy, int r) {
    objects[id].x1 = cx; objects[id].y1 = cy;
    objects[id].radius = r;
    redraw();
}

void modifyTriangle(int id, int x1, int y1, int x2, int y2, int x3, int y3) {
    objects[id].x1 = x1; objects[id].y1 = y1;
    objects[id].x2 = x2; objects[id].y2 = y2;
    objects[id].x3 = x3; objects[id].y3 = y3;
    redraw();
}

/* ─────────────────────────────────────────────
   LIST OBJECTS
   ───────────────────────────────────────────── */
void listObjects(void) {
    int found = 0;
    printf("\n%-4s %-12s %s\n", "ID", "Type", "Parameters");
    printf("────────────────────────────────────────────────\n");
    for (int i = 0; i < objectCount; i++) {
        if (!objects[i].active) continue;
        found = 1;
        Shape *s = &objects[i];
        printf("%-4d ", i);
        switch (s->type) {
            case SHAPE_LINE:
                printf("%-12s (%d,%d) → (%d,%d)\n",
                       "Line", s->x1, s->y1, s->x2, s->y2);
                break;
            case SHAPE_RECTANGLE:
                printf("%-12s (%d,%d) → (%d,%d)\n",
                       "Rectangle", s->x1, s->y1, s->x2, s->y2);
                break;
            case SHAPE_CIRCLE:
                printf("%-12s center=(%d,%d) r=%d\n",
                       "Circle", s->x1, s->y1, s->radius);
                break;
            case SHAPE_TRIANGLE:
                printf("%-12s (%d,%d),(%d,%d),(%d,%d)\n",
                       "Triangle", s->x1,s->y1, s->x2,s->y2, s->x3,s->y3);
                break;
        }
    }
    if (!found) printf("  (no objects)\n");
}

/* ─────────────────────────────────────────────
   INTERACTIVE MENU
   ───────────────────────────────────────────── */
void printMenu(void) {
    printf("\n╔══════════════════════════════╗\n");
    printf("║   2D ASCII Graphics Editor   ║\n");
    printf("╠══════════════════════════════╣\n");
    printf("║  1. Draw Line                ║\n");
    printf("║  2. Draw Rectangle           ║\n");
    printf("║  3. Draw Circle              ║\n");
    printf("║  4. Draw Triangle            ║\n");
    printf("║  5. Delete Object            ║\n");
    printf("║  6. Modify Object            ║\n");
    printf("║  7. List Objects             ║\n");
    printf("║  8. Display Canvas           ║\n");
    printf("║  9. Clear All                ║\n");
    printf("║  0. Quit                     ║\n");
    printf("╚══════════════════════════════╝\n");
    printf("Choice: ");
}

int main(void) {
    clearCanvas();
    int choice;

    do {
        printMenu();
        if (scanf("%d", &choice) != 1) break;

        if (choice == 1) {
            Shape s = { .type = SHAPE_LINE, .symbol = '*' };
            printf("Line: x1 y1 x2 y2: ");
            scanf("%d %d %d %d", &s.x1, &s.y1, &s.x2, &s.y2);
            int id = addObject(s);
            redraw();
            printf("Line added (id=%d).\n", id);

        } else if (choice == 2) {
            Shape s = { .type = SHAPE_RECTANGLE };
            printf("Rectangle: x1 y1 x2 y2: ");
            scanf("%d %d %d %d", &s.x1, &s.y1, &s.x2, &s.y2);
            int id = addObject(s);
            redraw();
            printf("Rectangle added (id=%d).\n", id);

        } else if (choice == 3) {
            Shape s = { .type = SHAPE_CIRCLE };
            printf("Circle: cx cy radius: ");
            scanf("%d %d %d", &s.x1, &s.y1, &s.radius);
            int id = addObject(s);
            redraw();
            printf("Circle added (id=%d).\n", id);

        } else if (choice == 4) {
            Shape s = { .type = SHAPE_TRIANGLE };
            printf("Triangle: x1 y1  x2 y2  x3 y3: ");
            scanf("%d %d %d %d %d %d",
                  &s.x1, &s.y1, &s.x2, &s.y2, &s.x3, &s.y3);
            int id = addObject(s);
            redraw();
            printf("Triangle added (id=%d).\n", id);

        } else if (choice == 5) {
            listObjects();
            int id;
            printf("Delete ID: ");
            scanf("%d", &id);
            deleteObject(id);

        } else if (choice == 6) {
            listObjects();
            int id;
            printf("Modify ID: ");
            scanf("%d", &id);
            if (id < 0 || id >= objectCount || !objects[id].active) {
                printf("Invalid ID.\n");
            } else {
                switch (objects[id].type) {
                    case SHAPE_LINE: {
                        int x1,y1,x2,y2;
                        printf("New x1 y1 x2 y2: ");
                        scanf("%d %d %d %d",&x1,&y1,&x2,&y2);
                        modifyLine(id,x1,y1,x2,y2);
                        break;
                    }
                    case SHAPE_RECTANGLE: {
                        int x1,y1,x2,y2;
                        printf("New x1 y1 x2 y2: ");
                        scanf("%d %d %d %d",&x1,&y1,&x2,&y2);
                        modifyRectangle(id,x1,y1,x2,y2);
                        break;
                    }
                    case SHAPE_CIRCLE: {
                        int cx,cy,r;
                        printf("New cx cy radius: ");
                        scanf("%d %d %d",&cx,&cy,&r);
                        modifyCircle(id,cx,cy,r);
                        break;
                    }
                    case SHAPE_TRIANGLE: {
                        int x1,y1,x2,y2,x3,y3;
                        printf("New x1 y1 x2 y2 x3 y3: ");
                        scanf("%d %d %d %d %d %d",&x1,&y1,&x2,&y2,&x3,&y3);
                        modifyTriangle(id,x1,y1,x2,y2,x3,y3);
                        break;
                    }
                }
                printf("Object %d modified.\n", id);
            }

        } else if (choice == 7) {
            listObjects();

        } else if (choice == 8) {
            displayCanvas();

        } else if (choice == 9) {
            objectCount = 0;
            clearCanvas();
            printf("Canvas cleared.\n");
        }

    } while (choice != 0);

    printf("Goodbye!\n");
    return 0;
}
