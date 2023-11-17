#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "perlin.c"

typedef struct Pixel
{
    int red;
    int green;
    int blue;
} Pixel;

struct Surface
{
    int width;
    int height;
    int depth;
    struct Pixel *data;
};

typedef struct Point
{
    int x;
    int y;
} Point;

typedef struct Surface SURFACE;

int min(int a, int b, int c, int d)
{
    int min = a;
    if (b < min)
        min = b;
    if (c < min)
        min = c;
    if (d < min)
        min = d;
    return min;
}

int max(int a, int b, int c, int d)
{
    int max = a;
    if (b > max)
        max = b;
    if (c > max)
        max = c;
    if (d > max)
        max = d;
    return max;
}

typedef struct elem_pile elem_pile;

struct elem_pile
{
    int coord_x;
    int coord_y;
    elem_pile *precedent;
};

typedef struct pile pile;

struct pile
{
    elem_pile *premier;
    elem_pile *dernier;
};

Point point(int x, int y)
{
    struct Point p;
    p.x = x;
    p.y = y;
    return p;
}

void surface(SURFACE *s, int width, int height)
{
    s->data = (struct Pixel *)malloc(width * height * sizeof(struct Pixel));
    if (s->data != NULL)
    {
        s->width = width;
        s->height = height;
        s->depth = RAND_MAX;
    }
}

pile creer_pile()
{
    pile *p = (pile *)malloc(sizeof(pile));
    p->dernier = NULL;
    p->premier = NULL;
    elem_pile *elem_pile = malloc(sizeof(*elem_pile));
    if (p == NULL && elem_pile == NULL)
    {
        exit(EXIT_FAILURE);
    }
    return *p;
}

int pile_vide(pile p)
{
    if (&p == NULL)
    {
        return 1;
    }
    return 0;
}

void empile(pile *p, int x, int y)
{
    int a = pile_vide(*p);
    if (a == 1)
    {
        exit(EXIT_FAILURE);
    }
    elem_pile *nouv = malloc(sizeof(*nouv));
    nouv->coord_x = x;
    nouv->coord_y = y;
    nouv->precedent = p->premier;
    if (p->premier == NULL && p->dernier == NULL)
    {
        p->dernier = nouv;
    }
    p->premier = nouv;
}

void sommet(pile p, int *x, int *y)
{
    int a = pile_vide(p);
    if (a == 1)
    {
        exit(EXIT_FAILURE);
    }
    elem_pile *premier = p.premier;
    *x = premier->coord_x;
    *y = premier->coord_y;
}

void depile(pile *p)
{
    int a = pile_vide(*p);
    if (a == 1)
    {
        exit(EXIT_FAILURE);
    }
    elem_pile *suppr = p->premier;
    p->premier = p->premier->precedent;
    free(suppr);
}

void free_surface(SURFACE *s)
{
    free(s->data);
}

void fill(SURFACE *s, struct Pixel pixel)
{
    for (struct Pixel *i = s->data, *e = s->data + s->width * s->height; i != e; ++i)
    {
        *i = pixel;
    }
}

int iclamp(int valeur, int min, int max)
{
    if (valeur < min)
    {
        return min;
    }
    else if (valeur > max)
    {
        return max;
    }
    return valeur;
}

int ppm_write(SURFACE *s, FILE *f)
{
    int max = 255;
    int count = fprintf(f, "P3\n# ppm write\n%d %d\n%d\n", s->width, s->height, max);
    int cr = s->width;

    for (struct Pixel *i = s->data, *e = s->data + s->width * s->height; i != e; ++i)
    {
        count += fprintf(f, "%d %d %d", iclamp(i->red, 0, max), iclamp(i->green, 0, max), iclamp(i->blue, 0, max));

        if (--cr == 0)
        {
            count += fprintf(f, "\n");
        }
        else
        {
            cr = s->width;
            count += fprintf(f, " ");
        }
    }
    return count;
}

void linear_gradient(struct Pixel startColor, struct Pixel endColor, int x, int y, SURFACE *s)
{
    float stepR = (endColor.red - startColor.red) / (float)s->width;
    float stepG = (endColor.green - startColor.green) / (float)s->width;
    float stepB = (endColor.blue - startColor.blue) / (float)s->width;

    for (int i = y; i < y + s->height; ++i)
    {
        struct Pixel currentColor;
        currentColor.red = (unsigned char)(startColor.red + stepR * (i - x));
        currentColor.green = (unsigned char)(startColor.green + stepG * (i - x));
        currentColor.blue = (unsigned char)(startColor.blue + stepB * (i - x));

        for (int j = x; j < x + s->width; ++j)
        {
            s->data[j * s->width + i] = currentColor;
        }
    }
}

void draw_rectangle(SURFACE *s, int x1, int y1, int x2, int y2, struct Pixel color)
{
    if (x1 < 0 || x1 > s->width || y1 < 0 || y1 > s->height ||
        x2 < 0 || x2 > s->width || y2 < 0 || y2 > s->height)
    {
        printf("Error: invalid coordinates");
        exit(EXIT_FAILURE);
    }

    int x = (x1 < x2) ? x1 : x2;
    int y = (y1 < y2) ? y1 : y2;
    int width = abs(x2 - x1);
    int height = abs(y2 - y1);

    if (width > s->width - x)
    {
        width = s->width - x;
    }
    if (height > s->height - y)
    {
        height = s->height - y;
    }

    for (int i = y; i < y + height; ++i)
    {
        for (int j = x; j < x + width; ++j)
        {
            s->data[i * s->width + j] = color;
        }
    }
}

void draw_point(SURFACE *s, struct Point p, struct Pixel color)
{
    int x = (int)p.x;
    int y = (int)p.y;
    if (x >= 0 && x < s->width && y >= 0 && y < s->height)
    {
        s->data[y * s->width + x] = color;
    }
}

void draw_line(SURFACE *s, struct Point p1, struct Point p2, struct Pixel color, int epaisseur)
{
    int x1 = (int)p1.x;
    int y1 = (int)p1.y;
    int x2 = (int)p2.x;
    int y2 = (int)p2.y;

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    int err = dx - dy;
    int x = x1;
    int y = y1;

    while (1)
    {
        for (int i = 0; i < epaisseur; i++)
        {
            for (int j = 0; j < epaisseur; j++)
            {
                struct Point point;
                point.x = x + i;
                point.y = y + j;
                draw_point(s, point, color);
            }
        }

        if (x == x2 && y == y2)
        {
            break;
        }

        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x += sx;
        }
        if (x == x2 && y == y2)
        {
            for (int i = 0; i < epaisseur; i++)
            {
                for (int j = 0; j < epaisseur; j++)
                {
                    struct Point lastPoint;
                    lastPoint.x = x + i;
                    lastPoint.y = y + j;
                    draw_point(s, lastPoint, color);
                }
            }
            break;
        }

        if (e2 < dx)
        {
            err += dx;
            y += sy;
        }
    }
}

void draw_triangle(SURFACE *s, struct Point p1, struct Point p2, struct Point p3, struct Pixel color)
{
    draw_line(s, p1, p2, color, 1);
    draw_line(s, p2, p3, color, 1);
    draw_line(s, p3, p1, color, 1);
}

void cercle(SURFACE *s, int r, int centreX, int centreY, struct Pixel couleur)
{
    for (double t = 0; t < 6.29; t += 1.0 / r)
    {
        double x = (int)(centreX + r * cos(t));
        double y = (int)(centreY + r * sin(t));
        struct Point p;
        p.x = x;
        p.y = y;
        draw_point(s, p, couleur);
    }
}

void courbe_bezier(SURFACE *s, struct Point P1, struct Point P2, struct Point P3, struct Point P4, int N, struct Pixel couleur, int epaisseur)
{
    for (double t = 0; t < 1; t += 1.0 / N)
    {
        double x = P1.x * pow((1 - t), 3) + 3 * P2.x * t * pow((1 - t), 2) + 3 * P3.x * pow(t, 2) * (1 - t) + P4.x * pow(t, 3);
        double y = P1.y * pow((1 - t), 3) + 3 * P2.y * t * pow((1 - t), 2) + 3 * P3.y * pow(t, 2) * (1 - t) + P4.y * pow(t, 3);
        struct Point p;
        p.x = x;
        p.y = y;

        for (int i = -epaisseur; i <= epaisseur; i++)
        {
            for (int j = -epaisseur; j <= epaisseur; j++)
            {
                struct Point offset;
                offset.x = p.x + i;
                offset.y = p.y + j;
                draw_point(s, offset, couleur);
            }
        }
    }
}

void fill_brown(SURFACE *s, struct Pixel targetColor, struct Pixel fillColor, int startX, int endX, int startY, int endY)
{
    for (int x = startX; x < endX; x++)
    {
        for (int y = startY; y < endY; y++)
        {
            struct Pixel *currentPixel = &s->data[y * s->width + x];
            if (currentPixel->red == targetColor.red && currentPixel->green == targetColor.green && currentPixel->blue == targetColor.blue)
            {
                break;
            }
            *currentPixel = fillColor;
        }
    }
}

void degrade_vers_blanc_bas(SURFACE *s, struct Point P1, struct Point P2, int n, struct Pixel color)
{
    // n correspond au nombre de pixels dont on veut le dégradé
    int x_min = P1.x;
    int y_min = P1.y;
    int x_max = P2.x;
    int y_max = P2.y;
    double facteur_ammortissement = 1.55;
    for (int i = x_min; i < x_max; i++)
    {
        int temp;
        for (int j = y_min; j < y_max; j++)
        {
            struct Pixel *currentPixel = &s->data[j * s->width + i];
            if (currentPixel->red == color.red && currentPixel->green == color.green && currentPixel->blue == color.blue)
            {
                temp = j;
            }
        }
        double rouge = (255 - color.red) / n;
        double brown = (255 - color.green) / n;
        double bleu = (255 - color.blue) / n;
        for (int k = 0; k < n; k++)
        {
            struct Pixel *currentPixel = &s->data[(temp - k) * s->width + i];
            struct Pixel PIX = {color.red + rouge * (n - k) / facteur_ammortissement, color.green + brown * (n - k) / facteur_ammortissement, +color.blue + bleu * (n - k) / facteur_ammortissement};
            if (currentPixel->red == color.red && currentPixel->green == color.green && currentPixel->blue == color.blue)
            {
                struct Point P = {i, temp - k};
                draw_point(s, P, PIX);
            }
        }
    }
}

// Si stopAtChange = 0 le remplissage s'arrête dès qu'il y'a un changement de couleur par rapport du point p_act
// Si stopatChange = 1 le remplissage s'arrête dès qu'on rencontre un pixel de la couleur color (utiliser pour des surfaces totalement entourrées de la couleur color genre les sphères)
void remplir(SURFACE *s, struct Pixel color, struct Point p_act, int stopAtChange)
{
    pile p = creer_pile();
    empile(&p, p_act.x, p_act.y);
    int x_act, y_act;
    struct Pixel start = s->data[p_act.y * s->width + p_act.x];

    while (p.premier != NULL)
    {

        sommet(p, &(x_act), &(y_act));
        p_act.x = x_act;
        p_act.y = y_act;
        depile(&p);
        draw_point(s, p_act, color);
        int dy[] = {-1, 1, 0, 0};
        int dx[] = {0, 0, -1, 1};
        for (int i = 0; i < 4; i++)
        {
            int x_nouv = x_act + dx[i];
            int y_nouv = y_act + dy[i];
            if (x_nouv >= 0 && x_nouv < s->width && y_nouv >= 0 && y_nouv < s->height)
            {
                struct Pixel *currentPixel = &s->data[y_nouv * s->width + x_nouv];
                if ((currentPixel->red == start.red && currentPixel->green == start.green && currentPixel->blue == start.blue) && !stopAtChange)
                {
                    empile(&p, x_nouv, y_nouv);
                }
                if ((currentPixel->red != color.red || currentPixel->green != color.green || currentPixel->blue != color.blue) && stopAtChange)
                {
                    empile(&p, x_nouv, y_nouv);
                }
            }
        }
    }
}

void trace_feuille(SURFACE *s, struct Point center, struct Point end, struct Point P1, struct Point P2, struct Point P3, struct Point P4, struct Pixel color, struct Point R)
{
    courbe_bezier(s, center, P1, P2, end, 5000, color, 1);
    courbe_bezier(s, center, P3, P4, end, 5000, color, 1);
    remplir(s, color, R, 1);
}

void courbe_bezier_3Pt(SURFACE *s, struct Point P1, struct Point P2, struct Point P3, struct Pixel couleur, int N, int epaisseur)
{
    for (double t = 0; t < 1; t += 1.0 / N)
    {
        double x = P1.x * pow((1 - t), 2) + 2 * P2.x * t * (1 - t) + P3.x * pow(t, 2);
        double y = P1.y * pow((1 - t), 2) + 2 * P2.y * t * (1 - t) + P3.y * pow(t, 2);
        struct Point p;
        p.x = x;
        p.y = y;

        for (int i = -epaisseur; i <= epaisseur; i++)
        {
            for (int j = -epaisseur; j <= epaisseur; j++)
            {
                struct Point offset;
                offset.x = p.x + i;
                offset.y = p.y + j;
                draw_point(s, offset, couleur);
            }
        }
    }
}

void cercle_rempli(SURFACE *s, int r, int centreX, int centreY, struct Pixel couleur)
{
    cercle(s,r,centreX,centreY,couleur);
    remplir(s,couleur,point(centreX,centreY),1);
}

int main()
{
    SURFACE surf;
    surface(&surf, 1000, 1000);
    assert(surf.data != NULL);

    // Couleurs

    Pixel sky = {99, 201, 250};
    Pixel sky2 = {223, 247, 251};
    Pixel light_sand = {251, 211, 126};
    Pixel dark_sand = {241, 170, 80};
    Pixel sand = {248, 193, 100};
    Pixel foam = {255, 255, 255};
    Pixel light_water = {24, 193, 236};
    Pixel water = {13, 172, 227};
    Pixel dark_water = {9, 153, 226};
    Pixel white = {225, 225, 255};
    Pixel grey = {125, 125, 125};
    Pixel brown = {97, 51, 10};
    Pixel light_brown = {147, 101, 60};
    Pixel green = {10, 240, 4};
    Pixel yellow = {240, 234, 4};
    Pixel red = {250, 15, 30};
    Pixel dark_red = {150, 5, 0};
    Pixel vert = {73, 166, 59};
    Pixel black = {0,0,0};
    Pixel light_blue = {150,152,254};

    // Remplissage du ciel avec dégradé

    fill(&surf, sky);
    linear_gradient(sky, sky2, 0, 0, &surf);

    // Création d'un rectangle pour le sable

    draw_rectangle(&surf, 0, 450, 1000, 1000, light_sand);

    // Tracé du bord de l'eau (mousse)

    courbe_bezier(&surf, point(0, 701), point(196, 728), point(280, 802), point(385, 726), 5000, foam, 10);
    courbe_bezier(&surf, point(385, 726), point(527, 678), point(583, 768), point(735, 654), 5000, foam, 10);
    courbe_bezier(&surf, point(735, 654), point(787, 614), point(930, 724), point(1000, 591), 5000, foam, 10);

    // Tracé de l'île

    courbe_bezier(&surf, point(529, 457), point(529, 407), point(799, 409), point(799, 469), 5000, light_sand, 10);
    courbe_bezier(&surf, point(519, 447), point(519, 397), point(809, 399), point(809, 459), 5000, brown, 5);
    draw_line(&surf, point(550, 449), point(760, 449), light_sand, 1);
    remplir(&surf, light_sand, point(635, 441), 1);

    // Tracé de l'arbre

    courbe_bezier(&surf, point(608, 408), point(572, 287), point(595, 289), point(618, 409), 5000, brown, 1);
    remplir(&surf, brown, point(602, 364), 1);
    trace_feuille(&surf, point(589, 319), point(642, 381), point(628, 328), point(643, 365), point(625, 351), point(614, 351), green, point(615, 334));
    trace_feuille(&surf, point(589, 319), point(565, 380), point(585, 328), point(570, 345), point(565, 328), point(555, 345), green, point(569, 343));
    trace_feuille(&surf, point(589, 319), point(666, 330), point(630, 305), point(645, 315), point(630, 325), point(645, 330), green, point(630, 323));
    trace_feuille(&surf, point(589, 319), point(523, 345), point(580, 310), point(548, 310), point(580, 325), point(540, 320), green, point(566, 319));
    trace_feuille(&surf, point(589, 319), point(638, 298), point(590, 278), point(612, 287), point(600, 293), point(607, 300), green, point(605, 293));

    // Remplissage de l'eau et dégradé

    fill_brown(&surf, foam, light_water, 0, 1000, 450, 1000);
    // degrade_vers_blanc_bas(&surf,point(0,450),point(999,750),150,light_water); J'ai du enlever le degradé ça faisait bugger le remplissage du fond de l'eau
    courbe_bezier(&surf, point(0, 546), point(61, 509), point(138, 531), point(185, 502), 5000, dark_water, 0);
    courbe_bezier(&surf, point(158, 502), point(207, 512), point(274, 494), point(363, 501), 5000, dark_water, 0);
    courbe_bezier(&surf, point(363, 501), point(411, 478), point(492, 508), point(590, 450), 5000, dark_water, 0);
    remplir(&surf, dark_water, point(217, 478), 0);
    courbe_bezier(&surf, point(0, 588), point(90, 541), point(203, 557), point(281, 541), 5000, water, 0);
    courbe_bezier(&surf, point(281, 541), point(323, 525), point(488, 507), point(583, 513), 5000, water, 0);
    courbe_bezier(&surf, point(583, 513), point(704, 489), point(819, 522), point(879, 450), 5000, water, 0);
    remplir(&surf, water, point(379, 519), 0);

    // Sphères à la surface de l'eau

    courbe_bezier(&surf, point(230, 689), point(230, 679), point(270, 679), point(270, 689), 5000, white, 1);
    courbe_bezier(&surf, point(230, 689), point(230, 699), point(270, 699), point(270, 689), 5000, white, 1);
    remplir(&surf, white, point(249, 687), 1);

    // Soleil

    draw_line(&surf, point(900, 0), point(999, 0), yellow, 1);
    draw_line(&surf, point(999, 0), point(999, 99), yellow, 1);
    courbe_bezier_3Pt(&surf, point(900, 0), point(900, 99), point(999, 99), yellow, 5000, 1);
    remplir(&surf, yellow, point(958, 52), 1);

    // Ombres sables

    courbe_bezier(&surf, point(616, 723), point(600, 818), point(689, 907), point(876, 1000), 5000, dark_sand, 0);
    draw_line(&surf, point(999, 655), point(999, 999), dark_sand, 1);
    draw_line(&surf, point(876, 999), point(998, 999), dark_sand, 1);
    remplir(&surf, dark_sand, point(890, 811), 0);
    courbe_bezier(&surf, point(0, 822), point(169, 823), point(180, 923), point(391, 1000), 5000, sand, 0);
    draw_line(&surf, point(0, 823), point(0, 999), sand, 1);
    draw_line(&surf, point(1, 999), point(390, 999), sand, 1);
    remplir(&surf, sand, point(99, 924), 1);
    // courbe_bezier_3Pt(&surf,S21,S22,S23,dark_sand,5000,0);
    // Parasol

    courbe_bezier_3Pt(&surf, point(717, 791), point(778, 680), point(847, 793), red, 5000, 1);
    courbe_bezier_3Pt(&surf, point(717, 791), point(778, 810), point(847, 793), dark_red, 5000, 1);
    courbe_bezier_3Pt(&surf, point(717, 791), point(778, 748), point(847, 793), red, 5000, 1);
    remplir(&surf, red, point(776, 757), 1);
    remplir(&surf, dark_red, point(777, 788), 0);
    draw_line(&surf, point(778, 879), point(778, 760), red, 3);
    // draw_line(&surf, point(779, 879), point(779, 760), red,1);
    // draw_line(&surf, point(777, 879), point(777, 760), red,1);

    // Chaise longue

    draw_line(&surf, point(825, 898), point(838, 919), brown, 2);
    draw_line(&surf, point(836, 924), point(930, 842), brown, 2);
    draw_line(&surf, point(821, 900), point(906, 828), brown, 2);
    draw_line(&surf, point(904, 830), point(926, 843), brown, 2);
    draw_line(&surf, point(822, 875), point(846, 878), brown, 2);
    draw_line(&surf, point(825, 877), point(837, 892), brown, 2);
    draw_line(&surf, point(836, 894), point(915, 910), brown, 2);
    draw_line(&surf, point(905, 864), point(903, 908), brown, 2);
    draw_line(&surf, point(878, 850), point(876, 883), brown, 2);
    draw_line(&surf, point(903, 906), point(877, 884), brown, 3);
    courbe_bezier(&surf, point(839, 893), point(876, 897), point(902, 883), point(926, 845), 5000, red, 0);
    courbe_bezier(&surf, point(828, 878), point(856, 888), point(889, 858), point(905, 831), 5000, red, 0);
    remplir(&surf, red, point(836, 883), 0);
    remplir(&surf, red, point(864, 880), 0);
    remplir(&surf, red, point(899, 858), 0);
    remplir(&surf, red, point(897, 876), 0);
    remplir(&surf, red, point(910, 863), 0);

    //Ballon
    courbe_bezier_3Pt(&surf,point(401,826),point(381,856),point(401,886),light_blue,5000,1);
    courbe_bezier_3Pt(&surf,point(409,826),point(429,856),point(409,886),red,5000,1);
    cercle(&surf,32,405,856,black);
    remplir(&surf,red,point(429,857),0);
    remplir(&surf,light_blue,point(407,856),0);
    remplir(&surf,white,point(380,855),0);
    
    //Bateau
    draw_line(&surf,point(160,449),point(147,420),light_brown,1);
    draw_line(&surf,point(230,449),point(243,420),light_brown,1);
    draw_line(&surf,point(160,449),point(230,449),light_brown,1);
    draw_line(&surf,point(147,420),point(243,420),light_brown,1);
    remplir(&surf,light_brown,point(197,434),1);
    draw_line(&surf,point(195,422),point(195,375),light_brown,3);
    draw_triangle(&surf,point(198,377),point(198,414),point(243,414),foam);
    draw_triangle(&surf,point(194,377),point(194,414),point(149,414),foam);
    remplir(&surf,foam,point(181,403),1);
    remplir(&surf,foam,point(211,403),1);
    cercle(&surf,6,166,429,light_blue);
    remplir(&surf,light_blue,point(166,430),1);

    //Nuage
    //cercle_rempli(&surf,15,139,110,foam);
    //cercle_rempli(&surf,18,152,98,foam);
    //cercle_rempli(&surf,18,177,107,foam);

    //cercle_rempli(&surf,23,402,164,foam);
    //cercle_rempli(&surf,17,425,150,foam);
    // Ecriture dans le fichier

    FILE *output = fopen("draw.ppm", "w");
    assert(output != NULL);
    ppm_write(&surf, output);
    fclose(output);
    free_surface(&surf);

    return 0;
}