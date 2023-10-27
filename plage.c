#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "perlin.c"

struct Pixel{
    float red;
    float green;
    float blue;
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

typedef struct elem_pile elem_pile;
struct elem_pile {
    int coord_x;
    int coord_y;
    elem_pile *precedent;
} ;

typedef struct pile pile;
struct pile {
    elem_pile *premier;
    elem_pile *dernier;
} ;


void surface(SURFACE *s,int width,int height){
    s->data = (struct Pixel *)malloc(width*height*sizeof(struct Pixel));
    if(s->data != NULL){
        s->width = width;
        s->height = height;
        s->depth = RAND_MAX;
    }
}

pile creer_pile(){
    pile *p = (pile *)malloc(sizeof(pile));
    p->dernier = NULL;
    p->premier = NULL;
    elem_pile *elem_pile = malloc(sizeof(*elem_pile));
    if(p == NULL && elem_pile == NULL){
        exit(EXIT_FAILURE);
    }
    return *p;
}

int pile_vide(pile p){
    if (&p == NULL){
        return 1;
    }
    return 0;
}

void empile(pile *p, int x, int y){
    int a = pile_vide(*p);
    if (a == 1){
        exit(EXIT_FAILURE);
    }
    elem_pile *nouv = malloc(sizeof(*nouv));
    nouv->coord_x = x;
    nouv->coord_y = y;
    nouv->precedent = p->premier;
    if (p->premier == NULL && p->dernier == NULL){
        p->dernier = nouv;
    }
    p->premier = nouv;
}

void sommet(pile p, int *x, int *y){
    int a = pile_vide(p);
    if (a == 1){
        exit(EXIT_FAILURE); 
    }
    elem_pile *premier = p.premier;
    *x = premier->coord_x;
    *y = premier->coord_y;

}

void depile(pile *p){  
    int a = pile_vide(*p);
    if (a == 1){
        exit(EXIT_FAILURE);
    }   
    elem_pile *suppr = p->premier;
    p->premier = p->premier->precedent;
    free(suppr); 
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


/*
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
*/

void linear_gradient(struct Pixel startColor, struct Pixel endColor, int x, int y, SURFACE *s) {
    float stepR = (endColor.red - startColor.red) / (float)s->width;
    float stepG = (endColor.green - startColor.green) / (float)s->width;
    float stepB = (endColor.blue - startColor.blue) / (float)s->width;

    for (int i = y; i < y + s->height; ++i) {
        struct Pixel currentColor;
        currentColor.red = (unsigned char)(startColor.red + stepR * (i - x));
        currentColor.green = (unsigned char)(startColor.green + stepG * (i - x));
        currentColor.blue = (unsigned char)(startColor.blue + stepB * (i - x));

        for (int j = x; j < x + s->width; ++j) {
            s->data[j * s->width + i] = currentColor;
        }
    }
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

void courbe_bezier_epaisse(SURFACE *s, struct Point P1, struct Point P2, struct Point P3, struct Point P4, int N, struct Pixel couleur, int largeur) {
    for (double t = 0; t < 1; t += 1.0 / N) {
        double x = P1.x * pow((1 - t), 3) + 3 * P2.x * t * pow((1 - t), 2) + 3 * P3.x * pow(t, 2) * (1 - t) + P4.x * pow(t, 3);
        double y = P1.y * pow((1 - t), 3) + 3 * P2.y * t * pow((1 - t), 2) + 3 * P3.y * pow(t, 2) * (1 - t) + P4.y * pow(t, 3);
        struct Point p;
        p.x = x;
        p.y = y;
        
        for (int i = -largeur; i <= largeur; i++) {
            for (int j = -largeur; j <= largeur; j++) {
                struct Point offset;
                offset.x = p.x + i;
                offset.y = p.y + j;
                draw_point(s, offset, couleur);
            }
        }
    }
}


void fill_vert(SURFACE *s, struct Pixel targetColor, struct Pixel fillColor, int startX, int endX,int startY, int endY) {
    for (int x = startX; x < endX; x++) {
        for (int y = startY; y < endY; y++) {
            struct Pixel *currentPixel = &s->data[y * s->width + x];
            if(currentPixel->red == targetColor.red && currentPixel->green == targetColor.green && currentPixel->blue == targetColor.blue){
                break;
            }
            *currentPixel = fillColor;
        }
    }
}

void remplir(SURFACE *s,struct Pixel color, struct Point p_act){
    pile p = creer_pile();
    empile(&p,p_act.x,p_act.y);
    int x_act,y_act;
    while(p.premier != NULL){
        
        sommet(p,&(x_act),&(y_act));
        p_act.x = x_act;
        p_act.y = y_act;
        depile(&p);
        draw_point(s,p_act,color);
        int dy[] = {-1,1,0,0};
        int dx[] = {0,0,-1,1};
        for(int i =0;i < 4; i++){
            int x_nouv = x_act + dx[i];
            int y_nouv = y_act + dy[i];
            struct Pixel *currentPixel = &s->data[y_nouv * s->width + x_nouv];
            if(currentPixel->red != color.red || currentPixel->green != color.green || currentPixel->blue != color.blue){
                empile(&p,x_nouv,y_nouv);
            }
        }
    }
}


// void apply_perlin_noise(SURFACE *s, struct Pixel targetColor, int startY, int endY,float freq,int depth) {
//     for (int y = startY; y < endY; y++) {
//         for (int x = 0; x < s->width; x++) {
//             struct Pixel *currentPixel = &s->data[y * s->width + x];
//             if (currentPixel->red == targetColor.red && currentPixel->green == targetColor.green && currentPixel->blue == targetColor.blue) {
//                 float noise = perlin2d(x, y, freq, depth);
//                 currentPixel->red = ((unsigned int)((1+noise)*127.5) + currentPixel->red)/2;
//                 currentPixel->green = ((unsigned int)((1+noise)*127.5) + currentPixel->green)/2;
//                 currentPixel->blue = ((unsigned int)((1+noise)*127.5) + currentPixel->blue)/2;
//             }
//         }
//     }
// }

    

int main(){
    SURFACE surf;
    surface(&surf,1000,1000);
    assert(surf.data != NULL);
    struct Pixel sky = {99,201,250};
    struct Pixel sky2 = {223,247,251};
    struct Pixel sand = {241,170,80};
    struct Pixel foam = {255,255,255};
    struct Pixel water1 = {9,153,226};
    struct Pixel water2 = {25,192,234};
    struct Pixel light_water = {225,225,255};
    struct Pixel grey = {125,125,125};
    struct Pixel brown = {97,51,10};
    struct Point P1 = {0,701}; struct Point P2 = {196,728}; struct Point P3 = {280,802}; struct Point P4 = {385,726}; struct Point P5 = {527,678}; struct Point P6 = {583,768}; struct Point P7 = {735,654}; struct Point P8 = {787,614}; struct Point P9 = {930,724}; struct Point P10 = {1000,591};
    struct Point P11 = {0,696}; struct Point P12 = {174,664}; struct Point P13 = {282,718}; struct Point P14 = {385,721};
    struct Point P15 = {230,689}; struct Point P16 = {230,679}; struct Point P17 = {270,679}; struct Point P18 = {270,689}; struct Point P19 = {230,699}; struct Point P20 = {270,699}; struct Point P21 = {249,687};
    struct Point I1 = {529,457};
    struct Point I2 = {529,407};
    struct Point I3 = {799,409};
    struct Point I4 = {799,469};
    struct Point I5 = {519,447};
    struct Point I6 = {519,397};
    struct Point I7 = {809,399};
    struct Point I8 = {809,459};
    struct Point R2 = {635,441};
    struct Point L1 = {550,449};
    struct Point L2 = {760,449};
    fill(&surf,sky);
    linear_gradient(sky,sky2,0,0,&surf);
    draw_rectangle(&surf,0,450,1000,1000,sand);
    courbe_bezier_epaisse(&surf,P1,P2,P3,P4,5000,foam,10);
    courbe_bezier_epaisse(&surf,P4,P5,P6,P7,5000,foam,10);
    courbe_bezier_epaisse(&surf,P7,P8,P9,P10,5000,foam,10);
    // courbe_bezier_epaisse(&surf,P11,P12,P13,P14,5000,water2,10);
    
    courbe_bezier_epaisse(&surf,I5,I6,I7,I8,5000,brown,5);
    courbe_bezier_epaisse(&surf,I1,I2,I3,I4,5000,sand,10);
    
    fill_vert(&surf,foam,water1,0,1000,450,1000);
    
    courbe_bezier(&surf,P15,P16,P17,P18,5000,light_water);
    courbe_bezier(&surf,P15,P19,P20,P18,5000,light_water);
    remplir(&surf,light_water,P21);
    draw_line(&surf,L1,L2,sand);
    remplir(&surf,sand,R2);
    FILE *output = fopen("draw.ppm","w");
    assert(output != NULL);
    ppm_write(&surf,output);
    fclose(output);
    free_surface(&surf);

    return 0;
}