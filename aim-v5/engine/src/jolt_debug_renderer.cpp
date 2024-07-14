#include "jolt_debug_renderer.h"
#include "core/logger/logger.h"


void JoltDebugRenderer::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
{
	ray_cast_list.push_back(
		RayCast{
		.ro = glm::vec3(inFrom.GetX(), inFrom.GetY(), inFrom.GetZ()),
		// aca para el `rd` no hago `ro + rd` como hice antes porque son dos puntos no mas, no tiene que ver la direccion.
		.rd = glm::vec3(inTo.GetX(), inTo.GetY(), inTo.GetZ())
		});
	DEBUG("JOLT - DrawLine");
}

void JoltDebugRenderer::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow)
{
	DEBUG("JOLT - DrawTriangle");
}
// revisar por que esta dos mierdas siempre se llaman al principio
JoltDebugRenderer::Batch JoltDebugRenderer::CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount)
{
	DEBUG("JOLT - CreateTriangleBatch with Triangle");
	return new BatchImpl(inTriangles->mV, inTriangleCount * 3, nullptr, inTriangleCount * 3);
}

// revisar por que esta dos mierdas siempre se llaman al principio
JoltDebugRenderer::Batch JoltDebugRenderer::CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const std::uint32_t* inIndices, int inIndexCount)
{
	DEBUG("JOLT - CreateTriangleBatch with Vertex");
	return new BatchImpl(inVertices, inVertexCount, inIndices, inIndexCount);
}

void JoltDebugRenderer::DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor,
	const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode)
{
	DEBUG("JOLT - DrawGeometry");


	// elias
	//Im3d::PushLayerId(layer);
	const auto& b =
		*static_cast<const BatchImpl*>(inGeometry->mLODs[0].mTriangleBatch.GetPtr());
	for (std::size_t fi = 0; fi < b.idx.size(); fi += 3) {
		JPH::Float3 v0, v1, v2;
		(inModelMatrix * JPH::Vec3(b.vtx[b.idx[fi + 0]].mPosition)).StoreFloat3(&v0);
		(inModelMatrix * JPH::Vec3(b.vtx[b.idx[fi + 1]].mPosition)).StoreFloat3(&v1);
		(inModelMatrix * JPH::Vec3(b.vtx[b.idx[fi + 2]].mPosition)).StoreFloat3(&v2);

		if (inDrawMode == JPH::DebugRenderer::EDrawMode::Solid) {
			//auto col = joltToIm3d(inModelColor);
			//col.setA(solidShapeAlpha);
			//Im3d::PushColor(col);
			//{
			//	auto& ctx = Im3d::GetContext();
			//	ctx.begin(Im3d::PrimitiveMode_Triangles);
			//	ctx.vertex(joltToIm3d(v0));
			//	ctx.vertex(joltToIm3d(v1));
			//	ctx.vertex(joltToIm3d(v2));
			//	ctx.end();
			//}
			//Im3d::PopColor();
		}
		else {
			DrawLine(JPH::Vec3(v0.x, v0.y, v0.z), JPH::Vec3(v1.x, v1.y, v1.z), inModelColor);
			DrawLine(JPH::Vec3(v1.x, v1.y, v1.z), JPH::Vec3(v2.x, v2.y, v2.z), inModelColor);
			DrawLine(JPH::Vec3(v2.x, v2.y, v2.z), JPH::Vec3(v0.x, v0.y, v0.z), inModelColor);
			//Im3d::DrawLine(joltToIm3d(v0), joltToIm3d(v1), lineWidth, joltToIm3d(inModelColor));
			//Im3d::DrawLine(joltToIm3d(v1), joltToIm3d(v2), lineWidth, joltToIm3d(inModelColor));
			//Im3d::DrawLine(joltToIm3d(v2), joltToIm3d(v0), lineWidth, joltToIm3d(inModelColor));
		}
	}

	//Im3d::PopLayerId();

	// elias


	// debugrendersimple

	//const BatchImpl* batch = static_cast<const BatchImpl*>(inGeometry->mLODs[0].mTriangleBatch.GetPtr());

	//for (const Triangle& triangle : batch->mTriangles)
	//{
	//	JPH::RVec3 v0 = inModelMatrix * JPH::Vec3(triangle.mV[0].mPosition);
	//	JPH::RVec3 v1 = inModelMatrix * JPH::Vec3(triangle.mV[1].mPosition);
	//	JPH::RVec3 v2 = inModelMatrix * JPH::Vec3(triangle.mV[2].mPosition);
	//	JPH::Color color = inModelColor * triangle.mV[0].mColor;
	//	JPH::Color some_color = JPH::Color(255, 0.0, 0.0, 255);

	//	switch (inDrawMode)
	//	{
	//	case EDrawMode::Wireframe:
	//		DrawLine(v0, v1, color);
	//		DrawLine(v1, v2, color);
	//		DrawLine(v2, v0, color);
	//		break;

	//	case EDrawMode::Solid:
	//		DrawTriangle(v0, v1, v2, color, inCastShadow);
	//		break;
	//	}
	//}
	// debugrendersimple







	// Figure out which LOD to use
	const LOD* lod = inGeometry->mLODs.data();
	lod = &inGeometry->GetLOD(JPH::Vec3(mCameraPos), inWorldSpaceBounds, inLODScaleSq);







	// Draw the batch
	//const BatchImpl *batch = static_cast<const BatchImpl *>(lod->mTriangleBatch.GetPtr());


	const BatchImpl* batch = static_cast<const BatchImpl*>(inGeometry->mLODs[0].mTriangleBatch.GetPtr());

	for (const Triangle& triangle : batch->mTriangles)
	{
		JPH::RVec3 v0 = inModelMatrix * JPH::Vec3(triangle.mV[0].mPosition);
		JPH::RVec3 v1 = inModelMatrix * JPH::Vec3(triangle.mV[1].mPosition);
		JPH::RVec3 v2 = inModelMatrix * JPH::Vec3(triangle.mV[2].mPosition);
		JPH::Color color = inModelColor * triangle.mV[0].mColor;
		JPH::Color some_color = JPH::Color(255, 0.0, 0.0, 255);

		switch (inDrawMode)
		{
		case EDrawMode::Wireframe:
			DrawLine(v0, v1, color);
			DrawLine(v1, v2, color);
			DrawLine(v2, v0, color);
			break;

		case EDrawMode::Solid:
			DrawTriangle(v0, v1, v2, color, inCastShadow);
			break;
		}
	}
}

void JoltDebugRenderer::DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor, float inHeight) {
	DEBUG("JOLT - DrawText3D");
}

