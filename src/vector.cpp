#include <vector.h>

Vector* Vector::operator+ (const Vector &vec) const {
	return new Vector(x + vec.x, y + vec.y, z + vec.z);
};

Vector* Vector::operator- (const Vector &vec) const {
	return new Vector(x - vec.x, y - vec.y, z - vec.z);
};

float Vector::length () const {
	return sqrtf(powf(x, 2) + powf(y, 2) + powf(z, 2));
};

Vector* Vector::normalize () const {
	float length = this->length();
	return new Vector(x/length, y/length, z/length);
};

Vector* Vector::scalar_mult (float scalar) const {
	return new Vector(x * scalar, y * scalar, z * scalar);
};

Vector* cross (const Vector &vec1, const Vector &vec2) {
	return new Vector(vec1.y * vec2.z - vec1.z * vec2.y, vec1.z * vec2.x - vec1.x * vec2.z, vec1.x * vec2.y - vec1.y * vec2.x);
};