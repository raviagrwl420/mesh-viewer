#include <vector.h>

Vector* cross (const Vector &vec1, const Vector &vec2) {
	return new Vector(vec1.y * vec2.z - vec1.z * vec2.y, vec1.z * vec2.x - vec1.x * vec2.z, vec1.x * vec2.y - vec1.y * vec2.x);
};