#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRenderer.h>
#include "defines.h"
#include "core/logger/logger.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class JoltDebugRenderer : public JPH::DebugRenderer {
private:
	class BatchImpl : public JPH::RefTargetVirtual {
	public:
		std::vector<Vertex> vtx;
		std::vector<JPH::uint32> idx;

		BatchImpl(const Vertex* inVert, int inVertCount, const JPH::uint32* inIdx, int inIdxCount) {
			vtx.resize(inVertCount);
			for (int i = 0; i < inVertCount; ++i) {
				vtx[i] = inVert[i];
			}

			if (inIdx) {
				idx.resize(inIdxCount);
				for (int i = 0; i < inIdxCount; ++i) {
					idx[i] = inIdx[i];
				}
			}
			else {
				idx.resize(inIdxCount);
				for (int i = 0; i < inIdxCount; ++i)
					idx[i] = i;
			}
		}

		int mRefCount = 0;
		JPH::Array<JPH::DebugRenderer::Triangle> mTriangles;

		void AddRef() override {
			mRefCount++;
		}

		void Release() override {
			if (--mRefCount == 0) {
				delete this;
			}
		}
	};

	struct RayCast {
		glm::vec3 ro;
		glm::vec3 rd;
		float t;
	};

public:
	std::vector<RayCast> ray_cast_list;

public:
	JPH::Vec3 mCameraPos{};

	AIM_API JoltDebugRenderer() {
		Initialize();
	}

	void AIM_API DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
	void AIM_API DrawTriangle(
		JPH::RVec3Arg inV1,
		JPH::RVec3Arg inV2,
		JPH::RVec3Arg inV3,
		JPH::ColorArg inColor,
		ECastShadow inCastShadow = ECastShadow::Off) override;
	Batch AIM_API CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) override;
	Batch AIM_API CreateTriangleBatch(
		const Vertex* inVertices,
		int inVertexCount,
		const std::uint32_t* inIndices,
		int inIndexCount) override;
	void AIM_API DrawGeometry(
		JPH::RMat44Arg inModelMatrix,
		const JPH::AABox& inWorldSpaceBounds,
		float inLODScaleSq,
		JPH::ColorArg inModelColor,
		const GeometryRef& inGeometry,
		ECullMode inCullMode,
		ECastShadow inCastShadow,
		EDrawMode inDrawMode) override;
	void AIM_API DrawText3D(
		JPH::RVec3Arg inPosition,
		const std::string_view& inString,
		JPH::ColorArg inColor,
		float inHeight) override;
};
