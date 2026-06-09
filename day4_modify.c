#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─────────────────────────────────────────
   DAY 4: Modify Objects + Save/Load
   Builds on Day 3. Adds:
   - modifyLine()      update endpoints
   - modifyRectangle() update corners
   - modifyCircle()    update centre/radius
   - modifyTriangle()  update all 3 vertices
   - saveToFile()      persist canvas to .txt
   - loadFromFile()    restore canvas from .txt
   ───────────────────────────────────────── */

#define ROWS     24
#define COLS     80
#define MAX_OBJ  50

typedef enum { SHAPE_LINE, SHAPE_RECTANGLE, SHAPE_CIRCLE, SHAPE_TRIANGLE } ShapeType;

typedef struct {
    ShapeType type;
    int active;
    int x1, y1, x2, y2, x3, y3;
    int radius;
    char symbol;
} Shape;

char  canvas[ROWS][COLS];
Shape objects[MAX_OBJ];
int   objectCount = 0;

/* ── Canvas helpers ───────────────────────── */
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

/* ── Drawing primitives ───────────────────── */
void drawLine(int r1,int c1,int r2,int c2,char sym) {
    int dr=abs(r2-r1),dc=abs(c2-c1);
    int sr=(r1<r2)?1:-1,sc=(c1<c2)?1:-1,err=dr-dc;
    while(1){
        plot(r1,c1,sym);
        if(r1==r2&&c1==c2)break;
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
    int x=0,y=radius,d=1-radius;
    while(y>=x){
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

void redraw() {
    clearCanvas();
    for (int i = 0; i < objectCount; i++) {
        Shape *s = &objects[i];
        if (!s->active) continue;
        switch (s->type) {
            case SHAPE_LINE:      drawLine(s->y1,s->x1,s->y2,s->x2,s->symbol); break;
            case SHAPE_RECTANGLE: drawRectangle(s->y1,s->x1,s->y2,s->x2);      break;
            case SHAPE_CIRCLE:    drawCircle(s->y1,s->x1,s->radius);            break;
            case SHAPE_TRIANGLE:  drawTriangle(s->y1,s->x1,s->y2,s->x2,s->y3,s->x3); break;
        }
    }
}

int addObject(Shape s) {
    if (objectCount >= MAX_OBJ) { printf("Max objects reached.\n"); return -1; }
    s.active = 1;
    objects[objectCount] = s;
    return objectCount++;
}

void deleteObject(int id) {
    if (id<0||id>=objectCount||!objects[id].active){ printf("Invalid ID.\n"); return; }
    objects[id].active = 0;
    printf("Object %d deleted.\n", id);
    redraw();
}

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
                printf("%-12s (%d,%d) to (%d,%d)\n","Line",s->x1,s->y1,s->x2,s->y2); break;
            case SHAPE_RECTANGLE:
                printf("%-12s (%d,%d) to (%d,%d)\n","Rectangle",s->x1,s->y1,s->x2,s->y2); break;
            case SHAPE_CIRCLE:
                printf("%-12s centre=(%d,%d) r=%d\n","Circle",s->x1,s->y1,s->radius); break;
            case SHAPE_TRIANGLE:
                printf("%-12s (%d,%d),(%d,%d),(%d,%d)\n","Triangle",
                    s->x1,s->y1,s->x2,s->y2,s->x3,s->y3); break;
        }
    }
    if (!found) printf("  (no active objects)\n");
}

/* ── MODIFY FUNCTIONS ────────────────────────
   Each updates the struct fields then calls
   redraw() so the canvas updates immediately.
   ─────────────────────────────────────────── */
void modifyLine(int id, int x1, int y1, int x2, int y2) {
    if (id<0||id>=objectCount||!objects[id].active
        || objects[id].type!=SHAPE_LINE) {
        printf("Error: ID %d is not an active line.\n", id); return;
    }
    objects[id].x1=x1; objects[id].y1=y1;
    objects[id].x2=x2; objects[id].y2=y2;
    redraw();
    printf("Line %d updated to (%d,%d)->(%d,%d).\n", id, x1,y1,x2,y2);
}

void modifyRectangle(int id, int x1, int y1, int x2, int y2) {
    if (id<0||id>=objectCount||!objects[id].active
        || objects[id].type!=SHAPE_RECTANGLE) {
        printf("Error: ID %d is not an active rectangle.\n", id); return;
    }
    objects[id].x1=x1; objects[id].y1=y1;
    objects[id].x2=x2; objects[id].y2=y2;
    redraw();
    printf("Rectangle %d updated to (%d,%d)->(%d,%d).\n", id, x1,y1,x2,y2);
}

void modifyCircle(int id, int cx, int cy, int r) {
    if (id<0||id>=objectCount||!objects[id].active
        || objects[id].type!=SHAPE_CIRCLE) {
        printf("Error: ID %d is not an active circle.\n", id); return;
    }
    objects[id].x1=cx; objects[id].y1=cy;
    objects[id].radius=r;
    redraw();
    printf("Circle %d updated: centre=(%d,%d) radius=%d.\n", id, cx,cy,r);
}

void modifyTriangle(int id, int x1,int y1, int x2,int y2, int x3,int y3) {
    if (id<0||id>=objectCount||!objects[id].active
        || objects[id].type!=SHAPE_TRIANGLE) {
        printf("Error: ID %d is not an active triangle.\n", id); return;
    }
    objects[id].x1=x1; objects[id].y1=y1;
    objects[id].x2=x2; objects[id].y2=y2;
    objects[id].x3=x3; objects[id].y3=y3;
    redraw();
    printf("Triangle %d updated.\n", id);
}

/* ── SAVE / LOAD ─────────────────────────────
   saveToFile() writes the visual canvas as-is.
   loadFromFile() reads it back character by
   character (display only; no object list).
   ─────────────────────────────────────────── */
void saveToFile(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) { printf("Error: cannot open %s\n", filename); return; }
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) fputc(canvas[r][c], fp);
        fputc('\n', fp);
    }
    fclose(fp);
    printf("Canvas saved to %s\n", filename);
}

void loadFromFile(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) { printf("Error: cannot open %s\n", filename); return; }
    clearCanvas();
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int ch = fgetc(fp);
            if (ch == EOF || ch == '\n') break;
            canvas[r][c] = (char)ch;
        }
        /* consume rest of line */
        int ch;
        while ((ch = fgetc(fp)) != '\n' && ch != EOF);
    }
    fclose(fp);
    printf("Canvas loaded from %s\n", filename);
}

int main() {
    clearCanvas();

    Shape s1 = { SHAPE_LINE,      0, 2,1,  40,10, 0,0, 0, '*' };
    Shape s2 = { SHAPE_RECTANGLE, 0, 50,2, 75,12, 0,0, 0, ' ' };
    Shape s3 = { SHAPE_CIRCLE,    0, 15,18, 0,0,  0,0, 5, '*' };
    Shape s4 = { SHAPE_TRIANGLE,  0, 55,14, 75,14, 65,22, 0, '*' };

    int id1 = addObject(s1);
    int id2 = addObject(s2);
    int id3 = addObject(s3);
    addObject(s4);
    redraw();

    printf("=== Initial canvas ===");
    displayCanvas();

    /* Modify line: move it lower */
    modifyLine(id1, 5, 5, 45, 15);
    printf("\n=== After modifying line (id=%d) ===", id1);
    displayCanvas();

    /* Modify circle: bigger radius */
    modifyCircle(id3, 15, 18, 8);
    printf("\n=== After modifying circle (id=%d) radius 5->8 ===", id3);
    displayCanvas();

    /* Modify rectangle: move it */
    modifyRectangle(id2, 45, 1, 72, 8);
    printf("\n=== After modifying rectangle (id=%d) ===", id2);
    displayCanvas();

    /* Save and reload */
    saveToFile("canvas_save.txt");
    clearCanvas();
    loadFromFile("canvas_save.txt");
    printf("\n=== After save → clear → load ===");
    displayCanvas();

    listObjects();
    printf("\nDay 4 complete: modify + save/load working!\n");
    return 0;
}
