#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════
   DAY 5: COMPLETE 2D ASCII GRAPHICS EDITOR
   ═══════════════════════════════════════════════════════
   Final version. Combines everything from Days 1-4
   and adds a full interactive menu system.

   Features:
     1. drawLine()      – Bresenham algorithm
     2. drawRectangle() – 4 edges + corners
     3. drawCircle()    – Midpoint algorithm
     4. drawTriangle()  – 3 Bresenham lines
     5. addObject()     – stores shape in list
     6. deleteObject()  – soft-delete by ID
     7. modifyObject()  – edit any shape's params
     8. listObjects()   – print object table
     9. redraw()        – rebuild canvas from list
    10. saveToFile()    – persist canvas to disk
    11. loadFromFile()  – restore canvas from disk
    12. Interactive menu loop with full input handling
   ═══════════════════════════════════════════════════════ */

#define ROWS     24
#define COLS     80
#define MAX_OBJ  50

typedef enum { SHAPE_LINE, SHAPE_RECTANGLE, SHAPE_CIRCLE, SHAPE_TRIANGLE } ShapeType;

typedef struct {
    ShapeType type;
    int active;            /* 1=visible  0=deleted         */
    int x1, y1;            /* start/centre/vertex 1        */
    int x2, y2;            /* end/corner/vertex 2          */
    int x3, y3;            /* vertex 3 (triangle only)     */
    int radius;            /* radius   (circle only)       */
    char symbol;           /* character used to draw       */
} Shape;

char  canvas[ROWS][COLS];
Shape objects[MAX_OBJ];
int   objectCount = 0;

/* ────────────────────────────────────────────────────────
   SECTION 1: CANVAS UTILITIES
   ──────────────────────────────────────────────────────── */

void clearCanvas() {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            canvas[r][c] = ' ';
}

/* Bounds-checked pixel writer */
void plot(int r, int c, char ch) {
    if (r >= 0 && r < ROWS && c >= 0 && c < COLS)
        canvas[r][c] = ch;
}

/* Print 2D array between +----+ borders */
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

/* ────────────────────────────────────────────────────────
   SECTION 2: DRAWING PRIMITIVES
   ──────────────────────────────────────────────────────── */

/*
 * drawLine – Bresenham's line algorithm
 * ────────────────────────────────────
 * Integer-only. The error variable tracks accumulated
 * fractional slope. When |err| exceeds the threshold,
 * the secondary axis advances one step.
 *
 * dr   = absolute row distance
 * dc   = absolute col distance
 * sr   = row step direction (+1 down, -1 up)
 * sc   = col step direction (+1 right, -1 left)
 * err  = running error, starts at dr - dc
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
 * drawRectangle
 * ─────────────
 * Top/bottom rows:  '_'
 * Left/right cols:  '|'
 * Four corners:     '+'
 * Normalises r1<=r2 and c1<=c2 first.
 */
void drawRectangle(int r1, int c1, int r2, int c2) {
    if (r1 > r2) { int t = r1; r1 = r2; r2 = t; }
    if (c1 > c2) { int t = c1; c1 = c2; c2 = t; }
    for (int c = c1; c <= c2; c++) { plot(r1,c,'_'); plot(r2,c,'_'); }
    for (int r = r1; r <= r2; r++) { plot(r,c1,'|'); plot(r,c2,'|'); }
    plot(r1,c1,'+'); plot(r1,c2,'+');
    plot(r2,c1,'+'); plot(r2,c2,'+');
}

/*
 * drawCircle – Midpoint circle algorithm
 * ────────────────────────────────────────
 * Iterates one octant (x=0..radius, y decreasing).
 * 8-fold symmetry fills all quadrants per step.
 * Column offset multiplied by 2 to correct for the
 * aspect ratio of terminal character cells (tall:wide ≈ 2:1).
 *
 * Decision var d:
 *   d < 0  → stay on same row  (d += 2x+3)
 *   d >= 0 → step row inward   (d += 2(x-y)+5, y--)
 */
void drawCircle(int cr, int cc, int radius) {
    int x = 0, y = radius, d = 1 - radius;
    while (y >= x) {
        plot(cr+x, cc+2*y, '*'); plot(cr-x, cc+2*y, '*');
        plot(cr+x, cc-2*y, '*'); plot(cr-x, cc-2*y, '*');
        plot(cr+y, cc+2*x, '*'); plot(cr-y, cc+2*x, '*');
        plot(cr+y, cc-2*x, '*'); plot(cr-y, cc-2*x, '*');
        if (d < 0) { d += 2*x + 3; }
        else       { d += 2*(x-y) + 5; y--; }
        x++;
    }
}

/*
 * drawTriangle
 * ────────────
 * Connects three vertices with Bresenham lines.
 * Edge 1: v1→v2   Edge 2: v2→v3   Edge 3: v3→v1
 */
void drawTriangle(int r1,int c1, int r2,int c2, int r3,int c3) {
    drawLine(r1,c1, r2,c2, '*');
    drawLine(r2,c2, r3,c3, '*');
    drawLine(r3,c3, r1,c1, '*');
}

/* ────────────────────────────────────────────────────────
   SECTION 3: OBJECT LIST MANAGEMENT
   ──────────────────────────────────────────────────────── */

/*
 * redraw – full canvas rebuild
 * ────────────────────────────
 * Clears the array, then replays every active object.
 * Called after every add/delete/modify to keep the
 * canvas consistent with the object list.
 */
void redraw() {
    clearCanvas();
    for (int i = 0; i < objectCount; i++) {
        Shape *s = &objects[i];
        if (!s->active) continue;
        switch (s->type) {
            case SHAPE_LINE:
                drawLine(s->y1,s->x1, s->y2,s->x2, s->symbol);
                break;
            case SHAPE_RECTANGLE:
                drawRectangle(s->y1,s->x1, s->y2,s->x2);
                break;
            case SHAPE_CIRCLE:
                drawCircle(s->y1,s->x1, s->radius);
                break;
            case SHAPE_TRIANGLE:
                drawTriangle(s->y1,s->x1, s->y2,s->x2, s->y3,s->x3);
                break;
        }
    }
}

/* Append a shape; returns its ID or -1 on overflow */
int addObject(Shape s) {
    if (objectCount >= MAX_OBJ) {
        printf("Error: maximum %d objects reached.\n", MAX_OBJ);
        return -1;
    }
    s.active = 1;
    objects[objectCount] = s;
    return objectCount++;
}

/* Soft-delete: mark inactive, then redraw */
void deleteObject(int id) {
    if (id < 0 || id >= objectCount || !objects[id].active) {
        printf("Error: invalid object ID %d\n", id);
        return;
    }
    objects[id].active = 0;
    printf("Object %d deleted.\n", id);
    redraw();
}

/* Print table of all active objects */
void listObjects() {
    int found = 0;
    printf("\n%-4s %-12s %s\n", "ID", "Type", "Parameters");
    printf("----------------------------------------------------\n");
    for (int i = 0; i < objectCount; i++) {
        if (!objects[i].active) continue;
        found = 1;
        Shape *s = &objects[i];
        printf("%-4d ", i);
        switch (s->type) {
            case SHAPE_LINE:
                printf("%-12s (%d,%d) -> (%d,%d)\n",
                    "Line", s->x1,s->y1, s->x2,s->y2); break;
            case SHAPE_RECTANGLE:
                printf("%-12s (%d,%d) -> (%d,%d)\n",
                    "Rectangle", s->x1,s->y1, s->x2,s->y2); break;
            case SHAPE_CIRCLE:
                printf("%-12s centre=(%d,%d) radius=%d\n",
                    "Circle", s->x1,s->y1, s->radius); break;
            case SHAPE_TRIANGLE:
                printf("%-12s (%d,%d), (%d,%d), (%d,%d)\n",
                    "Triangle", s->x1,s->y1, s->x2,s->y2, s->x3,s->y3); break;
        }
    }
    if (!found) printf("  (no active objects)\n");
}

/* ────────────────────────────────────────────────────────
   SECTION 4: MODIFY FUNCTIONS
   Each updates the struct then calls redraw().
   ──────────────────────────────────────────────────────── */

void modifyLine(int id, int x1,int y1, int x2,int y2) {
    if (id<0||id>=objectCount||!objects[id].active
        ||objects[id].type!=SHAPE_LINE) {
        printf("Error: ID %d is not an active line.\n", id); return;
    }
    objects[id].x1=x1; objects[id].y1=y1;
    objects[id].x2=x2; objects[id].y2=y2;
    redraw();
    printf("Line %d modified.\n", id);
}

void modifyRectangle(int id, int x1,int y1, int x2,int y2) {
    if (id<0||id>=objectCount||!objects[id].active
        ||objects[id].type!=SHAPE_RECTANGLE) {
        printf("Error: ID %d is not an active rectangle.\n", id); return;
    }
    objects[id].x1=x1; objects[id].y1=y1;
    objects[id].x2=x2; objects[id].y2=y2;
    redraw();
    printf("Rectangle %d modified.\n", id);
}

void modifyCircle(int id, int cx,int cy, int r) {
    if (id<0||id>=objectCount||!objects[id].active
        ||objects[id].type!=SHAPE_CIRCLE) {
        printf("Error: ID %d is not an active circle.\n", id); return;
    }
    objects[id].x1=cx; objects[id].y1=cy;
    objects[id].radius=r;
    redraw();
    printf("Circle %d modified.\n", id);
}

void modifyTriangle(int id, int x1,int y1, int x2,int y2, int x3,int y3) {
    if (id<0||id>=objectCount||!objects[id].active
        ||objects[id].type!=SHAPE_TRIANGLE) {
        printf("Error: ID %d is not an active triangle.\n", id); return;
    }
    objects[id].x1=x1; objects[id].y1=y1;
    objects[id].x2=x2; objects[id].y2=y2;
    objects[id].x3=x3; objects[id].y3=y3;
    redraw();
    printf("Triangle %d modified.\n", id);
}

/* ────────────────────────────────────────────────────────
   SECTION 5: FILE I/O
   ──────────────────────────────────────────────────────── */

void saveToFile(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) { printf("Error: cannot open %s\n", filename); return; }
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) fputc(canvas[r][c], fp);
        fputc('\n', fp);
    }
    fclose(fp);
    printf("Canvas saved to '%s'.\n", filename);
}

void loadFromFile(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) { printf("Error: file '%s' not found.\n", filename); return; }
    clearCanvas();
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int ch = fgetc(fp);
            if (ch == EOF || ch == '\n') break;
            canvas[r][c] = (char)ch;
        }
        int ch; while ((ch = fgetc(fp)) != '\n' && ch != EOF);
    }
    fclose(fp);
    printf("Canvas loaded from '%s'.\n", filename);
}

/* ────────────────────────────────────────────────────────
   SECTION 6: INTERACTIVE MENU
   ──────────────────────────────────────────────────────── */

void printMenu() {
    printf("\n");
    printf("╔══════════════════════════════════════╗\n");
    printf("║      2D ASCII GRAPHICS EDITOR        ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║  1. Draw Line                        ║\n");
    printf("║  2. Draw Rectangle                   ║\n");
    printf("║  3. Draw Circle                      ║\n");
    printf("║  4. Draw Triangle                    ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║  5. Delete Object                    ║\n");
    printf("║  6. Modify Object                    ║\n");
    printf("║  7. List All Objects                 ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║  8. Display Canvas                   ║\n");
    printf("║  9. Save Canvas to File              ║\n");
    printf("║ 10. Load Canvas from File            ║\n");
    printf("║ 11. Clear All                        ║\n");
    printf("║  0. Quit                             ║\n");
    printf("╚══════════════════════════════════════╝\n");
    printf("Choice: ");
}

/* Flush stdin after scanf to avoid leftover '\n' */
void flushInput() {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

int main() {
    clearCanvas();
    int choice;

    printf("\nWelcome to the 2D ASCII Graphics Editor!\n");
    printf("Canvas: %d rows x %d cols\n", ROWS, COLS);
    printf("Coordinates: x=column (0-%d), y=row (0-%d)\n", COLS-1, ROWS-1);

    do {
        printMenu();
        if (scanf("%d", &choice) != 1) { flushInput(); continue; }
        flushInput();

        /* ── 1. DRAW LINE ──────────────────────── */
        if (choice == 1) {
            Shape s = { SHAPE_LINE, 0, 0,0, 0,0, 0,0, 0, '*' };
            printf("Enter x1 y1 x2 y2 (start and end): ");
            scanf("%d %d %d %d", &s.x1, &s.y1, &s.x2, &s.y2);
            flushInput();
            int id = addObject(s);
            if (id >= 0) {
                redraw();
                printf("Line added with ID=%d.\n", id);
            }

        /* ── 2. DRAW RECTANGLE ─────────────────── */
        } else if (choice == 2) {
            Shape s = { SHAPE_RECTANGLE, 0, 0,0, 0,0, 0,0, 0, ' ' };
            printf("Enter x1 y1 x2 y2 (top-left and bottom-right): ");
            scanf("%d %d %d %d", &s.x1, &s.y1, &s.x2, &s.y2);
            flushInput();
            int id = addObject(s);
            if (id >= 0) {
                redraw();
                printf("Rectangle added with ID=%d.\n", id);
            }

        /* ── 3. DRAW CIRCLE ────────────────────── */
        } else if (choice == 3) {
            Shape s = { SHAPE_CIRCLE, 0, 0,0, 0,0, 0,0, 0, '*' };
            printf("Enter cx cy radius (centre x, centre y, radius): ");
            scanf("%d %d %d", &s.x1, &s.y1, &s.radius);
            flushInput();
            if (s.radius <= 0) {
                printf("Error: radius must be > 0.\n");
            } else {
                int id = addObject(s);
                if (id >= 0) { redraw(); printf("Circle added with ID=%d.\n", id); }
            }

        /* ── 4. DRAW TRIANGLE ──────────────────── */
        } else if (choice == 4) {
            Shape s = { SHAPE_TRIANGLE, 0, 0,0, 0,0, 0,0, 0, '*' };
            printf("Enter x1 y1  x2 y2  x3 y3 (three vertices): ");
            scanf("%d %d %d %d %d %d",
                &s.x1, &s.y1, &s.x2, &s.y2, &s.x3, &s.y3);
            flushInput();
            int id = addObject(s);
            if (id >= 0) { redraw(); printf("Triangle added with ID=%d.\n", id); }

        /* ── 5. DELETE ─────────────────────────── */
        } else if (choice == 5) {
            listObjects();
            int id;
            printf("Enter ID to delete: ");
            scanf("%d", &id);
            flushInput();
            deleteObject(id);

        /* ── 6. MODIFY ─────────────────────────── */
        } else if (choice == 6) {
            listObjects();
            int id;
            printf("Enter ID to modify: ");
            scanf("%d", &id);
            flushInput();

            if (id < 0 || id >= objectCount || !objects[id].active) {
                printf("Invalid ID.\n");
            } else {
                switch (objects[id].type) {
                    case SHAPE_LINE: {
                        int x1,y1,x2,y2;
                        printf("New x1 y1 x2 y2: ");
                        scanf("%d %d %d %d",&x1,&y1,&x2,&y2);
                        flushInput();
                        modifyLine(id,x1,y1,x2,y2);
                        break;
                    }
                    case SHAPE_RECTANGLE: {
                        int x1,y1,x2,y2;
                        printf("New x1 y1 x2 y2: ");
                        scanf("%d %d %d %d",&x1,&y1,&x2,&y2);
                        flushInput();
                        modifyRectangle(id,x1,y1,x2,y2);
                        break;
                    }
                    case SHAPE_CIRCLE: {
                        int cx,cy,r;
                        printf("New cx cy radius: ");
                        scanf("%d %d %d",&cx,&cy,&r);
                        flushInput();
                        modifyCircle(id,cx,cy,r);
                        break;
                    }
                    case SHAPE_TRIANGLE: {
                        int x1,y1,x2,y2,x3,y3;
                        printf("New x1 y1 x2 y2 x3 y3: ");
                        scanf("%d %d %d %d %d %d",&x1,&y1,&x2,&y2,&x3,&y3);
                        flushInput();
                        modifyTriangle(id,x1,y1,x2,y2,x3,y3);
                        break;
                    }
                }
            }

        /* ── 7. LIST ───────────────────────────── */
        } else if (choice == 7) {
            listObjects();

        /* ── 8. DISPLAY ────────────────────────── */
        } else if (choice == 8) {
            displayCanvas();

        /* ── 9. SAVE ───────────────────────────── */
        } else if (choice == 9) {
            char fname[128];
            printf("Enter filename to save (e.g. picture.txt): ");
            scanf("%127s", fname);
            flushInput();
            saveToFile(fname);

        /* ── 10. LOAD ──────────────────────────── */
        } else if (choice == 10) {
            char fname[128];
            printf("Enter filename to load: ");
            scanf("%127s", fname);
            flushInput();
            loadFromFile(fname);

        /* ── 11. CLEAR ALL ─────────────────────── */
        } else if (choice == 11) {
            objectCount = 0;
            clearCanvas();
            printf("Canvas and object list cleared.\n");

        } else if (choice != 0) {
            printf("Invalid choice. Enter 0-11.\n");
        }

    } while (choice != 0);

    printf("\nThank you for using the 2D ASCII Graphics Editor!\n");
    return 0;
}
