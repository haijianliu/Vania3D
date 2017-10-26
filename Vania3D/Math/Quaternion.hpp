
#ifndef Quaternion_hpp
#define Quaternion_hpp

class Quaternion {
public:
	// value
	float x, y, z, w;

	// constructor
	Quaternion(float x, float y, float z, float w);
	~Quaternion ();

	// functions
	void normalize();
	glm::vec3 degrees();

	// operators
	Quaternion operator*(const Quaternion& right) const;
	Quaternion operator*(const glm::vec3& vector) const;
};

#endif /* Quaternion_hpp */
