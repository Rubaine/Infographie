#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// Structures et piles

typedef struct Pixel // Structre pour gérér les couleus
{
    int red;
    int green;
    int blue;
} Pixel;

struct Surface // Structure pour créer la surface de dessin
{
    int width;
    int height;
    int depth;
    struct Pixel *data;
};

typedef struct Point // Structure pour gérer les points de la surface
{
    int x;
    int y;
} Point;

typedef struct Surface SURFACE;

int min(int a, int b, int c, int d) // Minimum entre 4 éléments
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

int max(int a, int b, int c, int d) // Maximum entre 4 éléments
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

struct elem_pile // Structre de la pile
{
    int coord_x;
    int coord_y;
    elem_pile *precedent;
};

typedef struct pile pile;

struct pile // Structre de la pile
{
    elem_pile *premier;
    elem_pile *dernier;
};

Point point(int x, int y) // Fonction qui renvoie une structure point à partir de coordonnées
{
    struct Point p;
    p.x = x;
    p.y = y;
    return p;
}

pile creer_pile() // Fonction pour créer une pile
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

int pile_vide(pile p) // Fonction pour vérifier si la pile est vide
{
    if (&p == NULL)
    {
        return 1;
    }
    return 0;
}

void empile(pile *p, int x, int y) // Fonction pour empiler
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

void sommet(pile p, int *x, int *y) // Fonction pour renvoyer le sommet de la pile
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

void depile(pile *p) // Fonction pour dépiler
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

// Fonctions de surface et d'écriture

void surface(SURFACE *s, int width, int height) // Définit les dimensions de la surface
{
    s->data = (struct Pixel *)malloc(width * height * sizeof(struct Pixel));
    if (s->data != NULL)
    {
        s->width = width;
        s->height = height;
        s->depth = RAND_MAX;
    }
}

void free_surface(SURFACE *s) // Libère la mémoire allouée à la surface
{
    free(s->data);
}

void fill(SURFACE *s, struct Pixel pixel) // Remplit entièrement la surface
{
    for (struct Pixel *i = s->data, *e = s->data + s->width * s->height; i != e; ++i)
    {
        *i = pixel;
    }
}

int iclamp(int valeur, int min, int max) // Fonction iclamp
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

int ppm_write(SURFACE *s, FILE *f) // Fonction pour écrire l'image dans un fichier au format ppm
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

// Fonctions de dessin

void draw_rectangle(SURFACE *s, int x1, int y1, int x2, int y2, struct Pixel color) // Fonction pour dessiner un rectangle
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

void draw_point(SURFACE *s, struct Point p, struct Pixel color) // Fonction pour dessiner un point à des coordonnées données
{
    int x = (int)p.x;
    int y = (int)p.y;
    if (x >= 0 && x < s->width && y >= 0 && y < s->height)
    {
        s->data[y * s->width + x] = color;
    }
}

void draw_line(SURFACE *s, struct Point p1, struct Point p2, struct Pixel color, int epaisseur) // Fonction pour dessiner une ligne entre deux points
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

void draw_triangle(SURFACE *s, struct Point p1, struct Point p2, struct Point p3, struct Pixel color) // Fonction pour dessiner un triangle à partir de 3 points
{
    draw_line(s, p1, p2, color, 1);
    draw_line(s, p2, p3, color, 1);
    draw_line(s, p3, p1, color, 1);
}

void courbe_bezier(SURFACE *s, struct Point P1, struct Point P2, struct Point P3, struct Point P4, int N, struct Pixel couleur, int epaisseur) // Fonction pour tracer une courbe de Bézier
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

void courbe_bezier_3Pt(SURFACE *s, struct Point P1, struct Point P2, struct Point P3, struct Pixel couleur, int N, int epaisseur) // Fonction pour tracer une courbe de Bézier avec 3 points de contrôle
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

// Fonctions de remplissage de surfaces

void _fill(SURFACE *s, struct Pixel targetColor, struct Pixel fillColor, int startX, int endX, int startY, int endY) // Fonction de remplissage naïf
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

// Si stopAtChange = 0 le remplissage s'arrête dès qu'il y'a un changement de couleur par rapport du point p_act
// Si stopatChange = 1 le remplissage s'arrête dès qu'on rencontre un pixel de la couleur color (utiliser pour des surfaces totalement entourrées de la couleur color genre les sphères)
void remplir(SURFACE *s, struct Pixel color, struct Point p_act, int stopAtChange) // Fonction de remplissage par DFS
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

void linear_gradient(struct Pixel startColor, struct Pixel endColor, int x, int y, SURFACE *s) // Fonction d'interpollation linéraire
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

// Si rempli = 1 le cercle est remplir avec la couleur passée en paramètre
void cercle(SURFACE *s, int r, int centreX, int centreY, struct Pixel couleur, int rempli) // Fonction pour dessiner un cercle de rayon r de centre centreX,centreY
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
    if (rempli)
    {
        remplir(s, couleur, point(centreX, centreY), 1);
    }
}

// Fonctions de dessin d'objets

// Ces fonction tracent les objets utiles pour notre image et ne peuvent pas être réutilisées pour placer les objets à d'autres endroits
// Elles sont seulement utilisées pour alléger le main

void trace_feuille(SURFACE *s, struct Point center, struct Point end, struct Point P1, struct Point P2, struct Point P3, struct Point P4, struct Pixel color, struct Point R)
{
    courbe_bezier(s, center, P1, P2, end, 5000, color, 1);
    courbe_bezier(s, center, P3, P4, end, 5000, color, 1);
    remplir(s, color, R, 1);
}

void arbre(SURFACE surf, Pixel c1, Pixel c2)
{
    courbe_bezier(&surf, point(608, 408), point(572, 287), point(595, 289), point(618, 409), 5000, c1, 1);
    remplir(&surf, c1, point(602, 364), 1);
    trace_feuille(&surf, point(589, 319), point(642, 381), point(628, 328), point(643, 365), point(625, 351), point(614, 351), c2, point(615, 334));
    trace_feuille(&surf, point(589, 319), point(565, 380), point(585, 328), point(570, 345), point(565, 328), point(555, 345), c2, point(569, 343));
    trace_feuille(&surf, point(589, 319), point(666, 330), point(630, 305), point(645, 315), point(630, 325), point(645, 330), c2, point(630, 323));
    trace_feuille(&surf, point(589, 319), point(523, 345), point(580, 310), point(548, 310), point(580, 325), point(540, 320), c2, point(566, 319));
    trace_feuille(&surf, point(589, 319), point(638, 298), point(590, 278), point(612, 287), point(600, 293), point(607, 300), c2, point(605, 293));
}

void parasol(SURFACE surf, Pixel c1, Pixel c2)
{
    courbe_bezier_3Pt(&surf, point(717, 791), point(778, 680), point(847, 793), c1, 5000, 1);
    courbe_bezier_3Pt(&surf, point(717, 791), point(778, 810), point(847, 793), c2, 5000, 1);
    courbe_bezier_3Pt(&surf, point(717, 791), point(778, 748), point(847, 793), c1, 5000, 1);
    remplir(&surf, c1, point(776, 757), 1);
    remplir(&surf, c2, point(777, 788), 0);
    draw_line(&surf, point(778, 879), point(778, 760), c1, 3);
}

void chaise_longue(SURFACE surf, Pixel c1, Pixel c2)
{
    draw_line(&surf, point(825, 898), point(838, 919), c1, 2);
    draw_line(&surf, point(836, 924), point(930, 842), c1, 2);
    draw_line(&surf, point(821, 900), point(906, 828), c1, 2);
    draw_line(&surf, point(904, 830), point(926, 843), c1, 2);
    draw_line(&surf, point(822, 875), point(846, 878), c1, 2);
    draw_line(&surf, point(825, 877), point(837, 892), c1, 2);
    draw_line(&surf, point(836, 894), point(915, 910), c1, 2);
    draw_line(&surf, point(905, 864), point(905, 908), c1, 2);
    draw_line(&surf, point(878, 850), point(878, 883), c1, 2);
    draw_line(&surf, point(903, 906), point(877, 884), c1, 3);
    courbe_bezier(&surf, point(839, 893), point(876, 897), point(902, 883), point(926, 845), 5000, c2, 0);
    courbe_bezier(&surf, point(828, 878), point(856, 888), point(889, 858), point(905, 831), 5000, c2, 0);
    remplir(&surf, c2, point(836, 883), 0);
    remplir(&surf, c2, point(864, 880), 0);
    remplir(&surf, c2, point(899, 858), 0);
    remplir(&surf, c2, point(897, 876), 0);
    remplir(&surf, c2, point(910, 863), 0);

    draw_line(&surf, point(747, 898), point(734, 919), c1, 2);
    draw_line(&surf, point(736, 924), point(642, 842), c1, 2);
    draw_line(&surf, point(751, 900), point(666, 828), c1, 2);
    draw_line(&surf, point(666, 830), point(644, 843), c1, 2);
    draw_line(&surf, point(749, 875), point(725, 878), c1, 2);
    draw_line(&surf, point(746, 877), point(734, 892), c1, 2);
    draw_line(&surf, point(736, 894), point(657, 910), c1, 2);
    draw_line(&surf, point(691, 850), point(691, 885), c1, 2);
    draw_line(&surf, point(668, 865), point(668, 907), c1, 2);
    draw_line(&surf, point(691, 885), point(668, 907), c1, 3);
    courbe_bezier(&surf, point(734, 893), point(697, 897), point(671, 883), point(646, 844), 5000, c2, 0);
    courbe_bezier(&surf, point(745, 878), point(717, 888), point(684, 858), point(668, 831), 5000, c2, 0);
    remplir(&surf, c2, point(737, 882), 0);
    remplir(&surf, c2, point(703, 878), 0);
    remplir(&surf, c2, point(674, 861), 0);
    remplir(&surf, c2, point(673, 873), 0);
    remplir(&surf, c2, point(664, 865), 0);
    remplir(&surf, c2, point(656, 850), 0);
}

void ballon(SURFACE surf, Pixel c1, Pixel c2, Pixel c3, Pixel c4, Pixel c5)
{
    cercle(&surf, 20, 421, 884, c1, 0);
    cercle(&surf, 5, 417, 877, c1, 0);
    courbe_bezier(&surf, point(412, 873), point(408, 873), point(405, 875), point(402, 877), 5000, c1, 0);
    courbe_bezier(&surf, point(413, 881), point(410, 884), point(408, 890), point(409, 899), 5000, c1, 0);
    courbe_bezier(&surf, point(420, 881), point(420, 889), point(424, 896), point(431, 901), 5000, c1, 0);
    courbe_bezier(&surf, point(422, 877), point(427, 878), point(434, 884), point(440, 890), 5000, c1, 0);
    courbe_bezier(&surf, point(421, 873), point(424, 870), point(428, 868), point(433, 869), 5000, c1, 0);
    courbe_bezier(&surf, point(415, 871), point(414, 869), point(415, 866), point(417, 865), 5000, c1, 0);
    remplir(&surf, c2, point(417, 876), 0);
    remplir(&surf, c2, point(415, 892), 0);
    remplir(&surf, c2, point(434, 876), 0);
    remplir(&surf, c2, point(410, 871), 0);
    remplir(&surf, c3, point(430, 889), 0);
    remplir(&surf, c4, point(406, 881), 0);
    remplir(&surf, c5, point(422, 867), 0);
}

void chateau(SURFACE surf, Pixel c1, Pixel c2, Pixel c3, Pixel c4)
{
    draw_line(&surf, point(302, 874), point(388, 871), c1, 1);
    draw_line(&surf, point(302, 874), point(302, 832), c1, 1);
    draw_line(&surf, point(302, 832), point(315, 832), c1, 1);
    draw_line(&surf, point(315, 832), point(315, 841), c1, 1);
    draw_line(&surf, point(315, 841), point(326, 841), c1, 1);
    draw_line(&surf, point(326, 841), point(326, 831), c1, 1);
    draw_line(&surf, point(326, 831), point(340, 831), c1, 1);
    draw_line(&surf, point(340, 831), point(340, 840), c1, 1);
    draw_line(&surf, point(340, 840), point(353, 840), c1, 1);
    draw_line(&surf, point(353, 840), point(353, 830), c1, 1);
    draw_line(&surf, point(353, 830), point(367, 830), c1, 1);
    draw_line(&surf, point(367, 830), point(367, 839), c1, 1);
    draw_line(&surf, point(367, 839), point(377, 839), c1, 1);
    draw_line(&surf, point(377, 839), point(377, 829), c1, 1);
    draw_line(&surf, point(377, 829), point(388, 829), c1, 1);
    draw_line(&surf, point(388, 829), point(388, 871), c1, 1);

    draw_line(&surf, point(317, 840), point(317, 815), c1, 1);
    draw_line(&surf, point(317, 815), point(330, 815), c1, 1);
    draw_line(&surf, point(330, 815), point(330, 821), c1, 1);
    draw_line(&surf, point(330, 821), point(341, 821), c1, 1);
    draw_line(&surf, point(341, 821), point(341, 814), c1, 1);
    draw_line(&surf, point(341, 814), point(352, 814), c1, 1);
    draw_line(&surf, point(352, 814), point(352, 820), c1, 1);
    draw_line(&surf, point(352, 820), point(363, 820), c1, 1);
    draw_line(&surf, point(363, 820), point(363, 813), c1, 1);
    draw_line(&surf, point(363, 813), point(374, 813), c1, 1);
    draw_line(&surf, point(374, 813), point(374, 838), c1, 1);

    draw_line(&surf, point(332, 821), point(332, 796), c1, 1);
    draw_line(&surf, point(332, 796), point(341, 796), c1, 1);
    draw_line(&surf, point(341, 796), point(341, 803), c1, 1);
    draw_line(&surf, point(341, 803), point(350, 803), c1, 1);
    draw_line(&surf, point(350, 803), point(350, 796), c1, 1);
    draw_line(&surf, point(350, 796), point(359, 796), c1, 1);
    draw_line(&surf, point(359, 796), point(359, 820), c1, 1);
    draw_line(&surf, point(345, 802), point(345, 777), c1, 2);

    draw_triangle(&surf, point(346, 777), point(356, 781), point(346, 785), c2);
    courbe_bezier(&surf, point(338, 873), point(343, 850), point(350, 850), point(358, 873), 5000, c1, 0);
    remplir(&surf, c3, point(335, 852), 0);
    remplir(&surf, c3, point(348, 828), 0);
    remplir(&surf, c3, point(344, 809), 0);
    remplir(&surf, c2, point(349, 781), 0);
    remplir(&surf, c4, point(347, 867), 0);
}

void bateau(SURFACE surf, Pixel c1, Pixel c2, Pixel c3)
{
    draw_line(&surf, point(160, 449), point(147, 420), c1, 1);
    draw_line(&surf, point(230, 449), point(243, 420), c1, 1);
    draw_line(&surf, point(160, 449), point(230, 449), c1, 1);
    draw_line(&surf, point(147, 420), point(243, 420), c1, 1);
    remplir(&surf, c1, point(197, 434), 1);
    draw_line(&surf, point(195, 422), point(195, 375), c1, 3);
    draw_triangle(&surf, point(198, 377), point(198, 414), point(243, 414), c2);
    draw_triangle(&surf, point(194, 377), point(194, 414), point(149, 414), c2);
    remplir(&surf, c2, point(181, 403), 1);
    remplir(&surf, c2, point(211, 403), 1);
    cercle(&surf, 6, 166, 429, c3, 0);
    remplir(&surf, c3, point(166, 430), 1);
}

void tabouret(SURFACE surf, Pixel c1)
{

    draw_line(&surf, point(773, 914), point(776, 893), c1, 2);
    draw_line(&surf, point(798, 914), point(795, 893), c1, 2);
    draw_line(&surf, point(786, 911), point(786, 893), c1, 2);
    courbe_bezier(&surf, point(786, 894), point(776, 892), point(771, 890), point(774, 886), 5000, c1, 0);
    courbe_bezier(&surf, point(787, 894), point(796, 892), point(801, 890), point(800, 886), 5000, c1, 0);
    courbe_bezier(&surf, point(774, 886), point(777, 882), point(781, 881), point(786, 881), 5000, c1, 0);
    courbe_bezier(&surf, point(786, 881), point(793, 881), point(797, 882), point(801, 886), 5000, c1, 0);
    remplir(&surf, c1, point(786, 887), 0);
}

void nuage(SURFACE surf, Pixel c1)
{
    draw_line(&surf, point(0, 214), point(459, 214), c1, 1);
    courbe_bezier_3Pt(&surf, point(459, 214), point(426, 156), point(383, 183), c1, 5000, 0);
    courbe_bezier_3Pt(&surf, point(383, 183), point(346, 101), point(249, 127), c1, 5000, 0);
    courbe_bezier_3Pt(&surf, point(249, 127), point(194, 57), point(135, 125), c1, 5000, 0);
    courbe_bezier_3Pt(&surf, point(135, 125), point(75, 129), point(59, 177), c1, 5000, 0);
    courbe_bezier_3Pt(&surf, point(59, 177), point(22, 174), point(0, 214), c1, 5000, 0);
    remplir(&surf,c1,point(220,169),1);
    draw_line(&surf, point(1000, 234), point(834, 234), c1, 1);
    courbe_bezier_3Pt(&surf, point(834, 234), point(855, 196), point(888, 212), c1, 5000, 0);
    courbe_bezier_3Pt(&surf, point(888, 212), point(911, 177), point(948, 196), c1, 5000, 0);
    courbe_bezier_3Pt(&surf, point(948, 196), point(969, 149), point(1000, 173), c1, 5000, 0);
    remplir(&surf,c1,point(952,212),1);
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
    Pixel white = {255, 255, 255};
    Pixel light_water = {24, 193, 236};
    Pixel water = {13, 172, 227};
    Pixel dark_water = {9, 153, 226};
    Pixel grey = {125, 125, 125};
    Pixel brown = {97, 51, 10};
    Pixel light_brown = {147, 101, 60};
    Pixel green = {10, 240, 4};
    Pixel yellow = {240, 234, 4};
    Pixel red = {250, 15, 30};
    Pixel dark_red = {150, 5, 0};
    Pixel vert = {73, 166, 59};
    Pixel black = {0, 0, 0};
    Pixel light_blue = {150, 152, 254};

    // Remplissage du ciel avec dégradé

    fill(&surf, sky);
    linear_gradient(sky, sky2, 0, 0, &surf);

    // Création d'un rectangle pour le sable

    draw_rectangle(&surf, 0, 450, 1000, 1000, light_sand);

    // Tracé du bord de l'eau (mousse)

    courbe_bezier(&surf, point(0, 701), point(196, 728), point(280, 802), point(385, 726), 5000, white, 10);
    courbe_bezier(&surf, point(385, 726), point(527, 678), point(583, 768), point(735, 654), 5000, white, 10);
    courbe_bezier(&surf, point(735, 654), point(787, 614), point(930, 724), point(1000, 591), 5000, white, 10);

    // Tracé de l'île

    courbe_bezier(&surf, point(529, 457), point(529, 407), point(799, 409), point(799, 469), 5000, light_sand, 10);
    courbe_bezier(&surf, point(519, 447), point(519, 397), point(809, 399), point(809, 459), 5000, brown, 5);
    draw_line(&surf, point(550, 449), point(760, 449), light_sand, 1);
    remplir(&surf, light_sand, point(635, 441), 1);

    // Tracé de l'arbre

    arbre(surf, brown, green);

    // Remplissage de l'eau et dégradé

    _fill(&surf, white, light_water, 0, 1000, 450, 1000);
    courbe_bezier(&surf, point(0, 546), point(61, 509), point(138, 531), point(185, 502), 5000, dark_water, 0);
    courbe_bezier(&surf, point(185, 502), point(196, 497), point(274, 494), point(363, 501), 5000, dark_water, 0);
    courbe_bezier(&surf, point(363, 501), point(407, 502), point(492, 508), point(590, 450), 5000, dark_water, 0);
    remplir(&surf, dark_water, point(217, 478), 0);
    courbe_bezier(&surf, point(0, 643), point(135, 600), point(261, 624), point(363, 565), 5000, water, 0);
    courbe_bezier(&surf, point(363, 565), point(414, 546), point(619, 549), point(723, 573), 5000, water, 0);
    courbe_bezier(&surf, point(723, 573), point(759, 582), point(892, 558), point(1000, 494), 5000, water, 0);
    remplir(&surf, water, point(379, 519), 0);

    // Sphères à la surface de l'eau

    // courbe_bezier(&surf, point(230, 689), point(230, 679), point(270, 679), point(270, 689), 5000, white, 1);
    // courbe_bezier(&surf, point(230, 689), point(230, 699), point(270, 699), point(270, 689), 5000, white, 1);
    // remplir(&surf, white, point(249, 687), 1);

    // Soleil

    draw_line(&surf, point(900, 0), point(999, 0), yellow, 1);
    draw_line(&surf, point(999, 0), point(999, 99), yellow, 1);
    courbe_bezier_3Pt(&surf, point(900, 0), point(900, 99), point(999, 99), yellow, 5000, 1);
    remplir(&surf, yellow, point(958, 52), 1);

    // Nuages

    nuage(surf, white);

    // Ombres sables

    courbe_bezier(&surf, point(616, 723), point(600, 818), point(689, 907), point(876, 1000), 5000, dark_sand, 0);
    draw_line(&surf, point(999, 655), point(999, 999), dark_sand, 1);
    draw_line(&surf, point(876, 999), point(998, 999), dark_sand, 1);
    remplir(&surf, dark_sand, point(890, 811), 0);
    courbe_bezier(&surf, point(0, 822), point(169, 823), point(180, 923), point(391, 1000), 5000, sand, 0);
    draw_line(&surf, point(0, 823), point(0, 999), sand, 1);
    draw_line(&surf, point(1, 999), point(390, 999), sand, 1);
    remplir(&surf, sand, point(99, 924), 1);

    // Parasol

    parasol(surf, red, dark_red);

    // Chaise longue

    chaise_longue(surf, brown, red);

    // Tabouret

    tabouret(surf, brown);

    // Ballon

    ballon(surf, black, white, red, yellow, green);

    // Chateau

    chateau(surf, black, red, dark_sand, sand);

    // Bateau

    bateau(surf, light_brown, white, light_blue);

    //  Ecriture dans le fichier

    FILE *output = fopen("draw.ppm", "w");
    assert(output != NULL);
    ppm_write(&surf, output);
    fclose(output);
    free_surface(&surf);

    return 0;
}