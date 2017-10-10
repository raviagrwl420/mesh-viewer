#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

struct Vector {
	float x, y, z;

	Vector () {};
	
	Vector (float x, float y, float z) : x(x), y(y), z(z) {};

	float length () const;

	Vector *operator+ (const Vector &vec) const;

	Vector *operator- (const Vector &vec) const;

	Vector *normalize () const;

	Vector *scalar_mult (float scalar) const;
};

Vector *cross (const Vector&, const Vector&);

#endif