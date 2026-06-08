#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─────────────────────────────────────────
   DAY 3: Object List (Add / Delete / List)
   Builds on Day 2. Adds:
   - Shape struct to represent any object
   - addObject()    stores shape in array
   - deleteObject() soft-deletes by id
   - listObjects()  prints object table
   - redraw()       rebuilds canvas from list
   ───────────────────────────────────────── */

#define ROWS     24
#define COLS     80
#define MAX_OBJ  50

typedef enum { SHAPE_LINE, SHAPE_RECTANGLE, SHAPE_CIRCLE, SHAPE_TRIANGLE } ShapeType;

typedef struct {
    ShapeType type;
    int active;            /* 1 = visible, 0 = deleted */
    int x1, y1;            /* first point / centre      */
    int x2, y2;            /* second point              */
    int x3, y3;            /* third point (triangle)    */
    int radius;            /* circle radius             */
    char symbol;           /* drawing character         */
} Shape;

char  canvas[ROWS][COLS];
Shape objects[MAX_OBJ];
int   objectCount = 0;

/* ── Canvas helpers (same as Day 1/2) ─────── */
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

/* ── Drawing primitives (same as Day 2) ───── */
void drawLine(int r1, int c1, int r2, int c2, char sym) {
    int dr=abs(r2-r1), dc=abs(c2-c1);
    int sr=(r1<r2)?1:-1, sc=(c1<c2)?1:-1, err=dr-dc;
    while(1) {
        plot(r1,c1,sym);
        if(r1==r2 && c1==c2) break;
        int e2=2*err;
        if(e2>-dc){err-=dc;r1+=sr;}
        if(e2< dr){err+=dr;c1+=sc;}
    }
}

void drawRectangle(int r1,int c1,int r2,int c2) {
    if(r1>r2){int t=r1;r1=r2;r2=t;}
    if(c1>c2){int t=c1;c1=c2;c2=t;}
    for(int c=c1;c<=c2;c++){plot(r1,c,'_');plot(r2,c,'_');}
    for(int r=r1;r<=r2;r++){plot(r,c1,'|');plot(r,c2,'|');}
    plot(r1,c1,'+');plot(r1,c2,'+');
    plot(r2,c1,'+');plot(r2,c2,'+');
}

void drawCircle(int cr,int cc,int radius) {
    int x=0, y=radius, d=1-radius;
    while(y>=x) {
        plot(cr+x,cc+2*y,'*'); plot(cr-x,cc+2*y,'*');
        plot(cr+x,cc-2*y,'*'); plot(cr-x,cc-2*y,'*');
        plot(cr+y,cc+2*x,'*'); plot(cr-y,cc+2*x,'*');
        plot(cr+y,cc-2*x,'*'); plot(cr-y,cc-2*x,'*');
        if(d<0){d+=2*x+3;}else{d+=2*(x-y)+5;y--;}
        x++;
    }
}

void drawTriangle(int r1,int c1,int r2,int c2,int r3,int c3) {
    drawLine(r1,c1,r2,c2,'*');
    drawLine(r2,c2,r3,c3,'*');
    drawLine(r3,c3,r1,c1,'*');
}

/* ── Redraw: rebuild canvas from object list ─
   Clears canvas, replays every active object.
   Called after any add/delete/modify.
   ─────────────────────────────────────────── */
void redraw() {
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
                drawTriangle(s->y1,s->x1, s->y2,s->x2, s->y3,s->x3);
                break;
        }
    }
}

/* ── addObject ───────────────────────────────
   Copies shape into next free slot.
   Returns assigned ID, or -1 if full.
   ─────────────────────────────────────────── */
int addObject(Shape s) {
    if (objectCount >= MAX_OBJ) {
        printf("Error: maximum %d objects reached.\n", MAX_OBJ);
        return -1;
    }
    s.active = 1;
    objects[objectCount] = s;
    return objectCount++;
}

/* ── deleteObject ────────────────────────────
   Soft-delete: sets active=0.
   No array shuffling needed; redraw skips it.
   ─────────────────────────────────────────── */
void deleteObject(int id) {
    if (id < 0 || id >= objectCount || !objects[id].active) {
        printf("Error: invalid object ID %d\n", id);
        return;
    }
    objects[id].active = 0;
    printf("Object %d deleted.\n", id);
    redraw();
}

/* ── listObjects ─────────────────────────────
   Prints a table of all active objects
   with their IDs and parameters.
   ─────────────────────────────────────────── */
void listObjects() {
    int found = 0;
    printf("\n%-4s %-12s %s\n", "ID", "Type", "Parameters");
    printf("--------------------------------------------------\n");
    for (int i = 0; i < objectCount; i++) {
        if (!objects[i].active) continue;
        found = 1;
        Shape *s = &objects[i];
        printf("%-4d ", i);
        switch (s->type) {
            case SHAPE_LINE:
                printf("%-12s (%d,%d) to (%d,%d)\n",
                    "Line", s->x1,s->y1, s->x2,s->y2);
                break;
            case SHAPE_RECTANGLE:
                printf("%-12s (%d,%d) to (%d,%d)\n",
                    "Rectangle", s->x1,s->y1, s->x2,s->y2);
                break;
            case SHAPE_CIRCLE:
                printf("%-12s centre=(%d,%d) radius=%d\n",
                    "Circle", s->x1,s->y1, s->radius);
                break;
            case SHAPE_TRIANGLE:
                printf("%-12s (%d,%d), (%d,%d), (%d,%d)\n",
                    "Triangle", s->x1,s->y1, s->x2,s->y2, s->x3,s->y3);
                break;
        }
    }
    if (!found) printf("  (no active objects)\n");
}

int main() {
    clearCanvas();

    /* Add test objects */
    Shape s1 = { SHAPE_LINE, 0, 2,1, 40,10, 0,0, 0, '*' };
    Shape s2 = { SHAPE_RECTANGLE, 0, 50,2, 78,12, 0,0, 0, ' ' };
    Shape s3 = { SHAPE_CIRCLE, 0, 15,18, 0,0, 0,0, 6, '*' };
    Shape s4 = { SHAPE_TRIANGLE, 0, 55,14, 76,14, 65,22, 0, '*' };

    (void)addObject(s1);
    int id2 = addObject(s2); (void)id2;
    (void)addObject(s3);
    (void)addObject(s4);
    redraw();

    printf("After adding 4 objects:");
    listObjects();
    displayCanvas();

    /* Delete rectangle (id2) */
    deleteObject(id2);
    printf("\nAfter deleting rectangle (id=%d):", id2);
    listObjects();
    displayCanvas();

    printf("\nDay 3 complete: add, delete, list working!\n");
    return 0;
}
