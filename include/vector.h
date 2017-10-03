#include <math.h>

struct Vector {
	float x, y, z;
	Vector () {};
	Vector (float x, float y, float z) : x(x), y(y), z(z) {};

	Vector* operator+ (const Vector &vec) const {
		return new Vector(x + vec.x, y + vec.y, z + vec.z);
	};

	Vector* operator- (const Vector &vec) const {
		return new Vector(x - vec.x, y - vec.y, z - vec.z);
	};

	float length () const {
		return sqrtf(powf(x, 2) + powf(y, 2) + powf(z, 2));
	};

	Vector* normalize () const {
		float length = this->length();
		return new Vector(x/length, y/length, z/length);
	};

	Vector* scalar_mult (float scalar) const {
		return new Vector(x * scalar, y * scalar, z * scalar);
	};
};

Vector* cross (const Vector&, const Vector&);