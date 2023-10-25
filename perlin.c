// #include <stdio.h>

// double lin_inter(double a, double b, double t){
//     return (1-t)*a+t*b;
// }

// double cosine_interpolate(double a, double b, double t)
// {
//     double c = (1 - cos(t * 3.1415927)) * .5;

//     return (1-c) * a + c * b;
// }

// double smooth_noise(double x)
// {
//     int integer_x = (int)x;
//     double fractional_x = x - integer_x;


//     double a = noise(integer_x);
//     //Bruit du point suivant :
//     double b = noise(integer_x + 1);

//     //Interpolation :
//     return linear_interpolate(a, b, fractional_x);
// }