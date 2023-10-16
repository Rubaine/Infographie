#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

struct Pixel{
    int red;
    int green;
    int blue;
};
struct Surface{
    int width;
    int height;
    int depth;
    struct Pixel *data;
};
struct Point{
    double x;
    double y;
};
typedef struct Surface SURFACE;

int min(int a, int b, int c, int d) {
    int min = a;
    if (b < min) min = b;
    if (c < min) min = c;
    if (d < min) min = d;
    return min;
}

int max(int a, int b, int c, int d) {
    int max = a;
    if (b > max) max = b;
    if (c > max) max = c;
    if (d > max) max = d;
    return max;
}

void surface(SURFACE *s,int width,int height){
    s->data = (struct Pixel *)malloc(width*height*sizeof(struct Pixel));
    if(s->data != NULL){
        s->width = width;
        s->height = height;
        s->depth = RAND_MAX;
    }
}

void free_surface(SURFACE *s){
    free(s->data);
}

void fill(SURFACE *s, struct Pixel pixel){
    for(struct Pixel *i = s->data, *e = s->data + s->width * s->height;i!=e; ++i){
        *i = pixel;
    }
}

int iclamp(int valeur, int min, int max){
    if(valeur<min){
        return min;
    }
    else if(valeur>max){
        return max;
    }
    return valeur;
}

int ppm_write(SURFACE *s, FILE *f) {
    int max = 255;
    int count = fprintf(f, "P3\n# ppm write\n%d %d\n%d\n", s->width, s->height, max);
    int cr = s->width;

    for (struct Pixel *i = s->data, *e = s->data + s->width * s->height; i != e; ++i) {
        count += fprintf(f, "%d %d %d", iclamp(i->red, 0, max), iclamp(i->green, 0, max), iclamp(i->blue, 0, max));

        if (--cr == 0) {
            count += fprintf(f, "\n");
        } else {
            cr = s->width;
            count += fprintf(f, " ");
        }
    }
    return count;
}

int ppm_read(SURFACE *s, FILE *f) {
    if (fgetc(f) != 'P') return 0;
    if (fgetc(f) != '3') return 0;
    if (fgetc(f) != '\n') return 0;

    char c;
    while ((c = fgetc(f)) == '#') {
        while (fgetc(f) != '\n');
    }
    ungetc(c, f);

    int width, height, max;
    if (fscanf(f, "%d%d%d", &width, &height, &max) != 3) return 0;

    SURFACE surf;
    surface(&surf, width, height);
    if (surf.data == NULL) return 0;

    for (struct Pixel *i = surf.data, *e = surf.data + surf.width * surf.height; i != e; ++i) {
        int red, green, blue;
        if (fscanf(f, "%d%d%d", &red, &green, &blue) != 3) {
            free_surface(&surf);
            return 0;
        }
        i->red = (double)red / max;
        i->green = (double)green / max;
        i->blue = (double)blue / max;
    }
    surf.depth = max;
    free_surface(s);
    *s = surf;
    return 1;
}

void draw_rectangle(SURFACE *s, int x1, int y1, int x2, int y2, struct Pixel color) {
    if (x1 < 0 || x1 > s->width || y1 < 0 || y1 > s->height ||
        x2 < 0 || x2 > s->width || y2 < 0 || y2 > s->height) {
        printf("Error: invalid coordinates");
        exit(EXIT_FAILURE);
    }

    int x = (x1 < x2) ? x1 : x2;
    int y = (y1 < y2) ? y1 : y2;
    int width = abs(x2 - x1);
    int height = abs(y2 - y1);

    if (width > s->width - x) {
        width = s->width - x;
    }
    if (height > s->height - y) {
        height = s->height - y;
    }
    
    for (int i = y; i < y + height; ++i) {
        for (int j = x; j < x + width; ++j) {
            s->data[i * s->width + j] = color;   
        }
    }
}

void draw_point(SURFACE *s, struct Point p, struct Pixel color) {
    int x = (int)p.x;
    int y = (int)p.y;

    if (x >= 0 && x < s->width && y >= 0 && y < s->height) {
        s->data[y * s->width + x] = color;
    }
}

void draw_line(SURFACE *s, struct Point p1, struct Point p2, struct Pixel color) {
    int x1 = (int)p1.x;
    int y1 = (int)p1.y;
    int x2 = (int)p2.x;
    int y2 = (int)p2.y;

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int sx = (x1<x2) ? 1 : -1;
    int sy = (y1<y2) ? 1 : -1;

    int err = dx - dy;
    int x = x1;
    int y = y1;

    while (1) {
        struct Point point;
        point.x = x;
        point.y = y;
        draw_point(s, point, color);

        if (x == x2 && y == y2) {
            break;
        }

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (x == x2 && y == y2) {
            struct Point lastPoint;
            lastPoint.x = x;
            lastPoint.y = y;
            draw_point(s, lastPoint, color);
            break;
        }

        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

void draw_triangle(SURFACE *s, struct Point p1, struct Point p2, struct Point p3, struct Pixel color) {
    draw_line(s, p1, p2, color);
    draw_line(s, p2, p3, color);
    draw_line(s, p3, p1, color);
}
void cercle(SURFACE *s,int r, int centreX, int centreY,struct Pixel couleur){
    for(double t=0;t<6.29;t+=1.0/r){
        double x=(int)(centreX+r*cos(t));
        double y=(int)(centreY+r*sin(t));
        struct Point p;
        p.x = x;
        p.y = y;
        draw_point(s,p,couleur);
    }
}

void courbe_bezier(SURFACE *s,struct Point P1,struct Point P2,struct Point P3,struct Point P4, int N,struct Pixel couleur){
    for(double t=0;t<1;t+=1.0/N){
        double x= P1.x*pow((1-t),3) + 3*P2.x*t*pow((1-t),2)+ 3*P3.x*pow(t,2)*(1-t) + P4.x*pow(t,3);
        double y= P1.y*pow((1-t),3) + 3*P2.y*t*pow((1-t),2)+ 3*P3.y*pow(t,2)*(1-t) + P4.y*pow(t,3);
        struct Point p;
        p.x = x;
        p.y = y;
        draw_point(s,p,couleur);
    }
}

void _fill(SURFACE *s, struct Point P1, struct Point P2, struct Point P3, struct Point P4, struct Pixel targetColor, struct Pixel fillColor) {
    int min_y = min(P1.y, P2.y, P3.y, P4.y);
    int max_y = max(P1.y, P2.y, P3.y, P4.y);

    for (int i = min_y; i < max_y; ++i) {
        if (s->data[i].red == targetColor.red && s->data[i].green == targetColor.green && s->data[i].blue == targetColor.blue) {
            s->data[i] = fillColor;
        }
    }
}



int main(){
    SURFACE surf;
    surface(&surf,1000,1000);
    assert(surf.data != NULL);

    struct Pixel sky = {173,252,252};
    struct Pixel sand = {236,209,161};
    struct Pixel foam = {255,255,255};
    struct Point P1 = {170,1000};
    struct Point P2 = {570,880};
    struct Point P3 = {270,795};
    struct Point P4 = {420,735};
    struct Point P5 = {474,665};
    struct Point P6 = {267,614};
    struct Point P7 = {374,518};
    
    fill(&surf,sky);
    draw_rectangle(&surf,0,500,1000,1000,sand);
    courbe_bezier(&surf,P1,P2,P3,P4,2000,foam);
    courbe_bezier(&surf,P4,P5,P6,P7,2000,foam);
    _fill(&surf,P1,P2,P3,P4,sand,sky);

    FILE *output = fopen("draw.ppm","w");
    assert(output != NULL);
    ppm_write(&surf,output);
    fclose(output);
    free_surface(&surf);

    return 0;
}