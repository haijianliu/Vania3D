
#include "Engine.hpp"

void Model::BoneTransform(float TimeInSeconds, std::vector<Matrix4>& Transforms)
{
	Matrix4 Identity;
	Identity.setIdentity();

    float TicksPerSecond = m_pScene->mAnimations[0]->mTicksPerSecond != 0 ? m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f;
    float TimeInTicks = TimeInSeconds * TicksPerSecond;
    float AnimationTime = fmod(TimeInTicks, m_pScene->mAnimations[0]->mDuration);

    ReadNodeHeirarchy(AnimationTime, this->rootNode, Identity);

    Transforms.resize(this->numBones);

    for (uint i = 0 ; i < this->numBones ; i++) {
			Transforms[i] = this->bones[i]->transformation;
    }
}


void Model::ReadNodeHeirarchy(float AnimationTime, const Node<Matrix4>* node, const Matrix4& ParentTransform)
{
	std::string NodeName = node->name;

    const aiAnimation* pAnimation = m_pScene->mAnimations[0];

	Matrix4 NodeTransformation = node->data;

    const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

    if (pNodeAnim) {
        // Interpolate scaling and generate scaling transformation matrix
        aiVector3D Scaling;
        CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
			Matrix4 ScalingM;
			ScalingM.setScaleTransform(Scaling.x, Scaling.y, Scaling.z);

        // Interpolate rotation and generate rotation transformation matrix
        aiQuaternion RotationQ;
        CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
        Matrix4 RotationM = Matrix4(RotationQ.GetMatrix());

        // Interpolate translation and generate translation transformation matrix
        aiVector3D Translation;
        CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
			Matrix4 TranslationM;
			TranslationM.setTranslationTransform(Translation.x, Translation.y, Translation.z);

        // Combine the above transformations
        NodeTransformation = TranslationM * RotationM * ScalingM;
    }

	Matrix4 GlobalTransformation = ParentTransform * NodeTransformation;


    if (this->boneMapping.find(NodeName) != this->boneMapping.end()) {
        uint BoneIndex = this->boneMapping[NodeName];
        this->bones[BoneIndex]->transformation = this->rootNode->data * GlobalTransformation * this->bones[BoneIndex]->offset;
				// m_BoneInfo[BoneIndex].FinalTransformation = GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;

    }

    for (unsigned int i = 0 ; i < node->children.size() ; i++) {
        ReadNodeHeirarchy(AnimationTime, node->children[i], GlobalTransformation);
    }
}


const aiNodeAnim* Model::FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
{
    for (uint i = 0 ; i < pAnimation->mNumChannels ; i++) {
        const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

			if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
            return pNodeAnim;
        }
    }

    return NULL;
}


uint Model::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    for (uint i = 0 ; i < pNodeAnim->mNumPositionKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
            return i;
        }
    }

    assert(0);

    return 0;
}


uint Model::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumRotationKeys > 0);

    for (uint i = 0 ; i < pNodeAnim->mNumRotationKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
            return i;
        }
    }

    assert(0);

    return 0;
}


uint Model::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumScalingKeys > 0);

    for (uint i = 0 ; i < pNodeAnim->mNumScalingKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
            return i;
        }
    }

    assert(0);

    return 0;
}


void Model::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    if (pNodeAnim->mNumPositionKeys == 1) {
        Out = pNodeAnim->mPositionKeys[0].mValue;
        return;
    }

    uint PositionIndex = FindPosition(AnimationTime, pNodeAnim);
    uint NextPositionIndex = (PositionIndex + 1);
    assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
    float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
    float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
    const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
    aiVector3D Delta = End - Start;
    Out = Start + Factor * Delta;
}


void Model::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
    if (pNodeAnim->mNumRotationKeys == 1) {
        Out = pNodeAnim->mRotationKeys[0].mValue;
        return;
    }

    uint RotationIndex = FindRotation(AnimationTime, pNodeAnim);
    uint NextRotationIndex = (RotationIndex + 1);
    assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
    float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
    float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
    const aiQuaternion& EndRotationQ   = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out = Out.Normalize();
}


void Model::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    if (pNodeAnim->mNumScalingKeys == 1) {
        Out = pNodeAnim->mScalingKeys[0].mValue;
        return;
    }

    uint ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
    uint NextScalingIndex = (ScalingIndex + 1);
    assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
    float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
    float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
    const aiVector3D& End   = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
    aiVector3D Delta = End - Start;
    Out = Start + Factor * Delta;
}

/*------------------------------------------------------------------------------
< Constructor >
------------------------------------------------------------------------------*/
Model::Model(const char* path) {
	this->rootNode = new Node<Matrix4>("root");
	this->load(path);
}


/*------------------------------------------------------------------------------
< Destructor >
------------------------------------------------------------------------------*/
Model::~Model() {
	deleteVector(this->meshes);
	deleteVector(this->bones);
	delete this->rootNode;
	deleteVector(this->animations);
}


/*------------------------------------------------------------------------------
< draw >
------------------------------------------------------------------------------*/
void Model::draw() {
	for(unsigned int i = 0; i < this->meshes.size(); i++) this->meshes[i]->draw();
}


/*------------------------------------------------------------------------------
< load model >
------------------------------------------------------------------------------*/
// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
void Model::load(const char* path) {
	// read file via ASSIMP
	Assimp::Importer importer;
	this->m_pScene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	// check for errors
	if(!this->m_pScene || this->m_pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !this->m_pScene->mRootNode) {
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}
	this->rootNode->data = m_pScene->mRootNode->mTransformation;
	// process assimp root node recursively
	this->processNode(this->m_pScene->mRootNode, this->rootNode, this->m_pScene);
	this->processAnimation(this->m_pScene);
	this->BoneTransform(1.0, this->Transforms);
}


/*------------------------------------------------------------------------------
< process node >
processes a node in a recursive function
the node object only contains indices to index the actual objects in the scene
the scene contains all the data
Processes each individual mesh located at the node and repeats this process on its children nodes (if any)
Processes the bone node heirarchy located at the node and calculate the final transformation
------------------------------------------------------------------------------*/
void Model::processNode(aiNode* ainode, Node<Matrix4>* node, const aiScene* aiscene) {
	// process each mesh located at the current node
	for (unsigned int i = 0; i < ainode->mNumMeshes; i++) {
		aiMesh* mesh = aiscene->mMeshes[ainode->mMeshes[i]];
		this->meshes.push_back(processMesh(mesh, aiscene));
	}
	// save the node heirarchy and all the transformation matrices and names
	node->data = ainode->mTransformation;

	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < ainode->mNumChildren; i++) {
		node->children.push_back(new Node<Matrix4>(ainode->mChildren[i]->mName.data));
		node->children[i]->parent = node;
		Model::processNode(ainode->mChildren[i], node->children[i], aiscene);
	}
}


/*------------------------------------------------------------------------------
< process Animation >
processes each individual aiAnimation located at the aiScene
processes each individual channel (aiNodeAnim) located at the aiAnimation
copy all the keyframe data to the animations member
------------------------------------------------------------------------------*/
void Model::processAnimation(const aiScene* aiscene) {
	// copy aiAnimation
	for (unsigned int i = 0; i < aiscene->mNumAnimations; i++) {
		this->animations.push_back(new Animation());
		this->animations[i]->name = aiscene->mAnimations[i]->mName.data;
		this->animations[i]->duration = aiscene->mAnimations[i]->mDuration;
		this->animations[i]->ticksPerSecond = aiscene->mAnimations[i]->mTicksPerSecond;
		// copy aiNodeAnim
		for (unsigned int j = 0; j < aiscene->mAnimations[i]->mNumChannels; j++) {
			this->animations[i]->keyframes.push_back(new Keyframe());
			this->animations[i]->keyframes[j]->nodeName = aiscene->mAnimations[i]->mChannels[j]->mNodeName.data;
			// copy keyframes
			for (unsigned int k = 0; k < aiscene->mAnimations[i]->mChannels[j]->mNumPositionKeys; k++) {
				VectorKey vectorKey;
				vectorKey.time = aiscene->mAnimations[i]->mChannels[j]->mPositionKeys[k].mTime;
				vectorKey.value = aiscene->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue;
				this->animations[i]->keyframes[j]->positionKeys.push_back(vectorKey);
			}
			for (unsigned int k = 0; k < aiscene->mAnimations[i]->mChannels[j]->mNumRotationKeys; k++) {
				QuaternionKey quaternionKey;
				quaternionKey.time = aiscene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mTime;
				quaternionKey.value = aiscene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue;
				this->animations[i]->keyframes[j]->rotationKeys.push_back(quaternionKey);
			}
			for (unsigned int k = 0; k < aiscene->mAnimations[i]->mChannels[j]->mNumScalingKeys; k++) {
				VectorKey vectorKey;
				vectorKey.time = aiscene->mAnimations[i]->mChannels[j]->mScalingKeys[k].mTime;
				vectorKey.value = aiscene->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue;
				this->animations[i]->keyframes[j]->scalingKeys.push_back(vectorKey);
			}
		}
	}
}


/*------------------------------------------------------------------------------
< process mesh>
load mesh vertices data to vao
if vertices have bone weights then load them to vao too
------------------------------------------------------------------------------*/
Mesh* Model::processMesh(aiMesh* mesh, const aiScene* scene) {
	// data to fill
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	/* vertices */
	// Walk through each of the mesh's vertices
	for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
		// positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.position = vector;
		// normals
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.normal = vector;
		// texture coordinates
		// does the mesh contain texture coordinates?
		if(mesh->mTextureCoords[0]) {
			glm::vec2 vec;
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.uv = vec;
		}
		else vertex.uv = glm::vec2(0.0f, 0.0f);
		// tangent
//		vector.x = mesh->mTangents[i].x;
//		vector.y = mesh->mTangents[i].y;
//		vector.z = mesh->mTangents[i].z;
//		vertex.tangent = vector;
		// bitangent
//		vector.x = mesh->mBitangents[i].x;
//		vector.y = mesh->mBitangents[i].y;
//		vector.z = mesh->mBitangents[i].z;
//		vertex.bitangent = vector;
		vertices.push_back(vertex);
	}

	/* indices */
	// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for(unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	/* bones */

	// set bones
	for (unsigned int i = 0 ; i < mesh->mNumBones ; i++) {
		unsigned int boneIndex = 0;
		const char* boneName = mesh->mBones[i]->mName.data;

		// index
		if (this->boneMapping.find(boneName) == this->boneMapping.end()) {
			boneIndex = this->numBones;
			this->numBones++;
			Bone bone;
			this->bones.push_back(new Bone());
		}
		else {
			boneIndex = this->boneMapping[boneName];
		}
		this->boneMapping[boneName] = boneIndex;
		this->bones[boneIndex]->offset = mesh->mBones[i]->mOffsetMatrix;

		// set vertices
		for (unsigned int j = 0 ; j < mesh->mBones[i]->mNumWeights ; j++) {
			unsigned int vertexID = mesh->mBones[i]->mWeights[j].mVertexId;
			float weight = mesh->mBones[i]->mWeights[j].mWeight;

			for (int k = 0 ; k < NUM_BONES_PER_VEREX ; k++) {
				if (vertices[vertexID].weight[k] == 0.0) {
					vertices[vertexID].boneID[k] = boneIndex;
					vertices[vertexID].weight[k] = weight;
					break;
				}
				if (k == NUM_BONES_PER_VEREX - 1) {
					printf("[WARNNING] vertex: %d weight: %f out of the number of NUM_BONES_PER_VEREX!\n", vertexID, weight);
				}
			}

		}
	}

	// return a mesh object created from the extracted mesh data
	return new Mesh(vertices, indices);
}
