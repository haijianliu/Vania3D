
#ifndef Bone_hpp
#define Bone_hpp

class Bone {
public:
	// bone info
	bool haveBone = false;
	unsigned int index;
	Matrix4 nodeTransformation;
	Matrix4 offset;

	// animation transform
	Matrix4 finalTransformation;
	glm::vec3 scaling;
	glm::quat rotation;
	glm::vec3 translation;

	Bone();
	~Bone ();
	// constructor from values
	Bone(unsigned int index, Matrix4 offset);
};

#endif /* Bone_hpp */
