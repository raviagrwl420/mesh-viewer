#include <vector.h>

// Compute the length of the vector
float Vector::length () const {
	return sqrtf(powf(x, 2) + powf(y, 2) + powf(z, 2));
};

// Add vectors
Vector *Vector::operator+ (const Vector &vec) const {
	return new Vector(x + vec.x, y + vec.y, z + vec.z);
};

// Subtract vectors
Vector *Vector::operator- (const Vector &vec) const {
	return new Vector(x - vec.x, y - vec.y, z - vec.z);
};

// Normalize the vector
Vector *Vector::normalize () const {
	float length = this->length();
	return new Vector(x/length, y/length, z/length);
};

// Normalize the vector
Vector *Vector::scalar_mult (float scalar) const {
	return new Vector(x * scalar, y * scalar, z * scalar);
};

// Take a cross product for the vectors
Vector *cross (const Vector &vec1, const Vector &vec2) {
	return new Vector(vec1.y * vec2.z - vec1.z * vec2.y, vec1.z * vec2.x - vec1.x * vec2.z, vec1.x * vec2.y - vec1.y * vec2.x);
};