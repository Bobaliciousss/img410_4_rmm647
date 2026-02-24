#include <stdint.h>
#include <math.h>
#include "v3math.h"

// "head minus tail"
//      With vector b as the head, subtract each element respectively from vector a
void v3_from_points( float *dst, float *a, float *b ) {
    dst[0] = b[0] - a[0];
    dst[1] = b[1] - a[1];
    dst[2] = b[2] - a[2];
}

// Add each element in the vector respectively and output into the destination vector
void v3_add( float *dst, float *a, float *b ) {
    dst[0] = a[0] + b[0];
    dst[1] = a[1] + b[1];
    dst[2] = a[2] + b[2];
}

// Subtract each element in the vector respectively and output into the destination vector
void v3_subtract( float *dst, float*a, float *b ) {
    dst[0] = a[0] - b[0];
    dst[1] = a[1] - b[1];
    dst[2] = a[2] - b[2];
}

// Multiple elements respectively, then add the results into a single float
float v3_dot_product( float *a, float *b ) {
    float a_xb_x = a[0] * b[0];
    float a_yb_y = a[1] * b[1];
    float a_zb_z = a[2] * b[2];
    return a_xb_x + a_yb_y + a_zb_z; 
}


void v3_cross_product( float *dst, float *a, float *b ) {
    dst[0] = a[1]*b[2] - a[2]*b[1];
    dst[1] = a[2]*b[0] - a[0]*b[2];
    dst[2] = a[0]*b[1] - a[1]*b[0];
}

// Scale each element in the destination vector by scalar s
void v3_scale( float *dst, float s ) {
    dst[0] = dst[0] * s;
    dst[1] = dst[1] * s;
    dst[2] = dst[2] * s;
}

// Angle between a and b
float v3_angle( float *a, float *b ) {
    float a_dot_b = v3_dot_product( a, b );
    float a_mag = sqrt( pow( a[0], 2 ) + pow( a[1], 2 ) + pow( a[2], 2 ) );
    float b_mag = sqrt( pow( b[0], 2 ) + pow( b[1], 2 ) + pow( b[2], 2 ) );
    float cosineOfAngle = a_dot_b / ( a_mag * b_mag );
    return acos(cosineOfAngle);
}

// Angle between a and b; no cos-1
float v3_angle_quick( float *a, float *b ) {
    float a_dot_b = v3_dot_product( a, b );
    float a_mag = sqrt( pow( a[0], 2 ) + pow( a[1], 2 ) + pow( a[2], 2 ) );
    float b_mag = sqrt( pow( b[0], 2 ) + pow( b[1], 2 ) + pow( b[2], 2 ) );
    float cosineOfAngle = a_dot_b / ( a_mag * b_mag );
    return cosineOfAngle;
} 

void v3_reflect(float *dst, float *v, float *n) {
    dst[0] = v[0] - ( 2 * ( v3_dot_product( n, v ) ) * n[0] );
    dst[1] = v[1] - ( 2 * ( v3_dot_product( n, v ) ) * n[1] );
    dst[2] = v[2] - ( 2 * ( v3_dot_product( n, v ) ) * n[2] );
}

// Magnitude of the given vector a
float v3_length(float *a) {
    return sqrt( pow( a[0], 2 ) + pow( a[1], 2 ) + pow( a[2], 2 ) );
}

// Convert vector a into a unit vector
void v3_normalize(float *dst, float *a) {
    float vector_mag = sqrt( pow( a[0], 2 ) + pow( a[1], 2 ) + pow( a[2], 2 ) );
    dst[0] = a[0] / vector_mag;
    dst[1] = a[1] / vector_mag;
    dst[2] = a[2] / vector_mag;
}