
#ifndef Texture_hpp
#define Texture_hpp

class Texture {
public:
	unsigned int id;

	Texture(const char* path);
	~Texture();

	static unsigned int loadTexture(const char* path);
	static unsigned int resize(unsigned int textureID, int width, int height, unsigned int format);
};

#endif /* Texture_hpp */
