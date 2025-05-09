#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <vector>
#include <unordered_map>

#define MAX_NUM_BONES_PER_VERTEX 4
#define MAX_NUM_JOINTS 320u

#pragma region helpers

namespace AssimpGLMHelpers {
	static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
	{
		glm::mat4 to{};
		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
		to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
		to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
		to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
		return to;
	}

	static inline glm::vec3 GetGLMVec(const aiVector3D& vec)
	{
		return glm::vec3(vec.x, vec.y, vec.z);
	}

	static inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation)
	{
		return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
	}
}

#pragma endregion helpers


struct AssimpVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 aTexCoords;
	uint32_t joints[4];
	float weights[4];
};

struct BoneData {
	int joints[MAX_NUM_BONES_PER_VERTEX];
	float weights[MAX_NUM_BONES_PER_VERTEX];
	int count = 0;
};


struct AssimpPrimitive {
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	uint32_t index_count;
};

struct AssimpMesh {
	std::vector<AssimpPrimitive*> meshes;
	struct UniformBlock {
		glm::mat4 matrix;
		glm::mat4 jointMatrix[MAX_NUM_JOINTS]{};
		uint32_t jointCount{ 0 };
	} uniformBlock;
};

struct AssimpNode {
	AssimpMesh* mesh;
	glm::mat4 transform;
	std::string name;
	AssimpNode* parent;
	std::vector<AssimpNode*> children;
	bool skin = false;
};

struct SkinnedModel {
	std::string label;
	AssimpNode* node;
	uint8_t skeleton_index;
};


struct AssimpBoneInfo {
	uint32_t id;
	glm::mat4 offset;
};


struct Skeleton {
	std::string label;
	int8_t parent_index{ -1 };
	AssimpNode* node;
	std::unordered_map<std::string, AssimpBoneInfo> m_BoneInfoMap;
	int m_BoneCounter = 0;
};
