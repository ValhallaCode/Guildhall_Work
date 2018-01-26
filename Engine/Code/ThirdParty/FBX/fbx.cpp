#include "ThirdParty/FBX/fbx.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MatrixStack.hpp"
#include "Engine/Render/MeshBuilder.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/RHI/Mesh.hpp"
#include "Engine/Input/FileStream.hpp"
#include "Engine/Core/BuildConfig.hpp" 
#include <string>
#include <algorithm>

#if defined(TOOLS_BUILD) 
// If we have FBX IMPORTING ENABLED, make sure to include the library and header files.
//#define __PLACEMENT_NEW_INLINE 
#include <fbxsdk.h>
#pragma comment(lib, "libfbxsdk-md.lib")


struct fbx_skin_weight_t
{
	fbx_skin_weight_t() :
		indices(0, 0, 0, 0),
		weights(0.0f, 0.0f, 0.0f, 0.0f)
	{}

	void reset()
	{
		indices = UintVector4(0, 0, 0, 0);
		weights = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	UintVector4 indices;
	Vector4 weights;
};

FbxScene* FbxLoadScene(char const *filename, bool import_embedded);
void FbxUnloadScene(FbxScene *scene);
void FbxPrintNode(FbxNode *node, int depth);
void FbxListScene(FbxScene *scene);
void FbxPrintAttribute(FbxNodeAttribute* pAttribute, int depth);
char const* GetAttributeTypeName(FbxNodeAttribute::EType type);
void FbxImportVertex(MeshBuilder *out, Matrix4&transform, FbxMesh *mesh, int32_t poly_idx, int32_t vert_idx, fbx_skin_weight_t* skinWeight);
bool FbxGetPosition(Vector3* out_pos, Matrix4& transform, FbxMesh *mesh, int poly_idx, int vert_idx);
Matrix4 GetGeometricTransform(FbxNode *node);
static void ImportMesh(MeshBuilder *out, MatrixStack &mat_stack, FbxMesh *mesh, Skeleton* skeleton = nullptr);
Matrix4 ToEngineMatrix(FbxMatrix const &fbx_mat);
Matrix4 GetNodeWorldTransform(FbxNode *node);
void ImportMeshes(MeshBuilder *out, FbxNode *node, MatrixStack &mat_stack, Skeleton* skeleton = nullptr);
void FlipX(MeshBuilder *mb);
void TriangulateScene(FbxScene *scene);
void ConvertSceneToEngineBasis(FbxScene *scene);
bool GetNormal(Vector3 *out, Matrix4& transform, FbxMesh *mesh, int poly_idx, int vert_idx);
static const std::string GetBoneName(FbxSkeleton const *skel);
static const std::string GetNodeName(FbxNode const *node);
void ImportSkeleton(Skeleton *out, FbxNode *node, FbxSkeleton *root_bone, FbxSkeleton *parent_bone, FbxPose *pose);
FbxPose* GetBindPose(FbxScene *scene);
void FlipXAxis(Skeleton* skel);
static bool FbxImportMotion(Motion* motion, Skeleton* skeleton, FbxScene* scene, FbxAnimStack* anim, float framerate);
// static float GetNativeFramefrate(FbxScene* scene);
Matrix4 GetNodeWorldTransformAtTime(FbxNode *node, FbxTime time);
void AddHighestWeight(fbx_skin_weight_t *skin_weight, uint32_t bone_idx, float weight);
void CalculateSkinWeights(std::vector<fbx_skin_weight_t>* skin_weights, FbxMesh const *mesh, Skeleton* skeleton);
bool GetUV(Vector2* out, FbxMesh *mesh, int poly_idx, int vert_idx);

void FbxListFile(const std::string& filename)
{
	FbxScene *scene = FbxLoadScene(filename.c_str(), false);
	if (nullptr == scene) {
		DebuggerPrintf("Failed to load scene: %s", filename);
		return;
	}

	//Trace("fbx", "Listing File: %s", filename);
	g_console->ConsolePrintf(Rgba(128, 128, 128, 255), "Listing File: %s", filename);
	FbxListScene(scene);

	FbxUnloadScene(scene);
}

//------------------------------------------------------------------------
// Reference:
//    $(FBXSDK_DIR)\samples\Common\Common.cxx
FbxScene* FbxLoadScene(char const *filename, bool import_embedded)
{
	// Manager - think of it as a running process of the FbxSdk - you could only ever load one of these
	// and call it good - though I usually just keep one around per import.
	FbxManager *fbx_manager = FbxManager::Create();
	if (nullptr == fbx_manager) {
		DebuggerPrintf("Could not create FBX Manager.");
		return nullptr;
	}

	// Next, set the IO settings - this is how we configure what we're importing
	// By default, we import everything, but if you wanted to explicitly not import some parts
	// you could do that here)
	FbxIOSettings *io_settings = FbxIOSettings::Create(fbx_manager, IOSROOT);
	io_settings->SetBoolProp(IMP_FBX_EXTRACT_EMBEDDED_DATA, import_embedded);

	fbx_manager->SetIOSettings(io_settings);


	// Next, create our importer (since we're loading an FBX, not saving one)
	// Second argument is the name - we don't need to name it.
	FbxImporter *importer = FbxImporter::Create(fbx_manager, "");

	bool result = importer->Initialize(filename,
		-1, // File format, -1 will let the program figure it out
		io_settings);

	if (result) {
		// First, create a scene to be imported into
		// Doesn't need a name, we're not going to do anything with it
		FbxScene *scene = FbxScene::Create(fbx_manager, "");

		// Import into the scene
		result = importer->Import(scene);
		importer->Destroy();
		importer = nullptr;

		if (result) {
			return scene;
		}
	}

	// Failed somewhere, so clean up after ourselves.
	FBX_SAFE_DESTROY(io_settings);
	FBX_SAFE_DESTROY(importer);
	FBX_SAFE_DESTROY(fbx_manager);

	return nullptr;
}

//------------------------------------------------------------------------
void FbxUnloadScene(FbxScene *scene)
{
	if (nullptr == scene) {
		return;
	}

	FbxManager *manager = scene->GetFbxManager();
	FbxIOSettings *io_settings = manager->GetIOSettings();

	FBX_SAFE_DESTROY(scene);
	FBX_SAFE_DESTROY(io_settings);
	FBX_SAFE_DESTROY(manager);
}

//------------------------------------------------------------------------
void FbxPrintNode(FbxNode *node, int depth)
{
	// Print the node's attributes.
	//Trace("fbx", "%*sNode [%s]", depth, " ", node->GetName());
	std::string space = "";
	for (int i = 0; i < depth; ++i)
	{
		space += " ";
	}
	std::string result = space + "Node [" + node->GetName() + "]";
	g_console->ConsolePrintf(Rgba(128, 128, 128, 255), result.c_str());
	for (int i = 0; i < node->GetNodeAttributeCount(); i++) {
		FbxPrintAttribute(node->GetNodeAttributeByIndex(i), depth);
	}

	// Print the nodes children
	for (int32_t i = 0; i < node->GetChildCount(); ++i) {
		FbxPrintNode(node->GetChild(i), depth + 1);
	}
}

//------------------------------------------------------------------------
void FbxListScene(FbxScene *scene)
{
	// Print the node
	FbxNode *root = scene->GetRootNode();
	FbxPrintNode(root, 0);
}

//------------------------------------------------------------------------
void FbxPrintAttribute(FbxNodeAttribute* pAttribute, int depth)
{
	if (nullptr == pAttribute) {
		return;
	}

	FbxNodeAttribute::EType type = pAttribute->GetAttributeType();

	char const* typeName = GetAttributeTypeName(type);
	char const* attrName = pAttribute->GetName();

	//Trace("fbx", "%*s- type='%s' name='%s'", depth, " ", typeName, attrName);
	std::string space = "";
	for (int i = 0; i < depth; ++i)
	{
		space += " ";
	}
	std::string result = space + "- type='" + typeName + "' name='" + attrName + "'";
	g_console->ConsolePrintf(Rgba(128, 128, 128, 255), result.c_str());
}

//------------------------------------------------------------------------
char const* GetAttributeTypeName(FbxNodeAttribute::EType type)
{
	switch (type) {
	case FbxNodeAttribute::eUnknown: return "unidentified";
	case FbxNodeAttribute::eNull: return "null";
	case FbxNodeAttribute::eMarker: return "marker";
	case FbxNodeAttribute::eSkeleton: return "skeleton";
	case FbxNodeAttribute::eMesh: return "mesh";
	case FbxNodeAttribute::eNurbs: return "nurbs";
	case FbxNodeAttribute::ePatch: return "patch";
	case FbxNodeAttribute::eCamera: return "camera";
	case FbxNodeAttribute::eCameraStereo: return "stereo";
	case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
	case FbxNodeAttribute::eLight: return "light";
	case FbxNodeAttribute::eOpticalReference: return "optical reference";
	case FbxNodeAttribute::eOpticalMarker: return "marker";
	case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
	case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
	case FbxNodeAttribute::eBoundary: return "boundary";
	case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
	case FbxNodeAttribute::eShape: return "shape";
	case FbxNodeAttribute::eLODGroup: return "lodgroup";
	case FbxNodeAttribute::eSubDiv: return "subdiv";
	default: return "unknown";
	}
}

bool FbxLoadMesh(MeshBuilder *mb, char const *filename, Skeleton* skeleton /*= nullptr*/)
{
	// First - load the scene - code is identical to when you 
	// listed the file
	FbxScene *scene = FbxLoadScene(filename, false);
	if (nullptr == scene) {
		return false;
	}

	// Second, we want to convert the basis for consitancy
	ConvertSceneToEngineBasis(scene);

	// Third, our engine only deals with triangles, 
	// but authoring programs tend to deal with a lot of 
	// different surface patch types.  For we triangulate
	// (the process of converting every non-triangle polygon to triangles )
	TriangulateScene(scene);


	// Alright, now we can start importing data, for this we'll need a matrix stack 
	// [well, it helps, you can do this without since we'll mostly be deailing with 
	// global transforms outside of what is called the "geometric transform" which is
	// always local]

	// clear all old data before importing
	mb->Clear();

	// My matrix stack treats the top as being IDENTITY
	MatrixStack mat_stack;

	// Next, get the root node, and start importing
	FbxNode *root = scene->GetRootNode();

	// import data into our builder
	ImportMeshes(mb, root, mat_stack, skeleton);

	// Clean up after ourself
	FbxUnloadScene(scene);


	// [HACK] After loading, I flip everything across the X axis
	// for conistancy.  Would love this to be part of ConvertSceneToEngineBasis,
	// but no mattter the transform I sent, it was always flipped on X [at best]
	//
	// Doing this causes it to be match Unity/Unreal/Editor in terms of orientation
	//FlipX(mb);
	return true;
}

// So, new functions and the Fbx Code that makes them up

// Conver scene to a consistant basis.
// The name is misleading, as this actually converts to a 
// right-handed system (Y-UP, X-RIGHT, Z-BACK)
// but any attempt to convert to a left-handed
// system caused it to flip across Y, which is not what I 
// want - so I handle the final basis flip as a final step after
// loading
void ConvertSceneToEngineBasis(FbxScene *scene)
{
	fbxsdk::FbxAxisSystem local_system(
		(fbxsdk::FbxAxisSystem::EUpVector) fbxsdk::FbxAxisSystem::EUpVector::eYAxis,
		(fbxsdk::FbxAxisSystem::EFrontVector) fbxsdk::FbxAxisSystem::EFrontVector::eParityOdd,  // by default points toward me.
		(fbxsdk::FbxAxisSystem::ECoordSystem) fbxsdk::FbxAxisSystem::ECoordSystem::eRightHanded);

	FbxAxisSystem scene_system = scene->GetGlobalSettings().GetAxisSystem();
	if (scene_system != local_system) {
		local_system.ConvertScene(scene);
	}
}

// Converts the scene to only use triangles for the meshes.
void TriangulateScene(FbxScene *scene)
{
	FbxGeometryConverter converter(scene->GetFbxManager());

	// Replace being true means it will destroy the old 
	// representation of the mesh after conversion.  Setting this to 
	// false would leave the old representations in.
	converter.Triangulate(scene, /*replace=*/true);
}

// Flips all geoemetry across the X access
void FlipX(MeshBuilder *mb)
{
	// LEFT AS AN EXCERCISE (mostly as it depends on how your implementation of MeshBuilder/Mesh)

	// You just want to flip the X coordinate of all vertex
	// data (positions,normals/tangents/bitangents)
	//IMPLEMENT_ME;
	for (unsigned int index = 0; index < mb->m_vertices.size(); ++index)
	{
		Vertex3_PCT& currentVert = mb->m_vertices[index];

		currentVert.m_position.x *= -1.0f;
		currentVert.m_normal.x *= -1.0f;
		currentVert.m_tangent.x *= -1.0f;
		currentVert.m_bitangent.x *= -1.0f;
	}
}

Matrix4 GetNodeWorldTransformAtTime(FbxNode *node, FbxTime time)
{
	if (nullptr == node) {
		return Matrix4();
	}

	FbxMatrix fbx_mat = node->EvaluateGlobalTransform(time);
	return ToEngineMatrix(fbx_mat);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
// The meat of the problem - Importing a mesh
// This walking of a list, loading specific attributes will be a pretty common task
// in this class.
void ImportMeshes(MeshBuilder *out, FbxNode *node, MatrixStack &mat_stack, Skeleton* skeleton /*= nullptr*/)
{
	// first, we're traversing the graph, keep track of our current world transform
	// (I will do this by by pushing directly the world transform of this node to my stack)
	// (that is, this transform becomes the new top exactly)
	Matrix4 transform = GetNodeWorldTransform(node);
	mat_stack.PushDirect(transform);

	// First, traverse the scene graph, looking for FbxMesh node attributes.
	int attrib_count = node->GetNodeAttributeCount();
	for (int ai = 0; ai < attrib_count; ++ai) {
		FbxNodeAttribute *attrib = node->GetNodeAttributeByIndex(ai);
		if (attrib->GetAttributeType() == FbxNodeAttribute::eMesh) {
			ImportMesh(out, mat_stack, (FbxMesh*)attrib, skeleton); // out was mb
		}
	}

	// Try to load any meshes that are children of this node
	int child_count = node->GetChildCount();
	for (int ci = 0; ci < child_count; ++ci) {
		FbxNode *child = node->GetChild(ci);
		ImportMeshes(out, child, mat_stack, skeleton);
	}

	// we're done with this node, so its transform off the stack
	mat_stack.Pop();
}

// This gets the world transform of a specific node
// and converts it to OUR engine matrix format
Matrix4 GetNodeWorldTransform(FbxNode *node)
{
	FbxAMatrix fbx_mat = node->EvaluateGlobalTransform();
	return ToEngineMatrix(fbx_mat);
}

Matrix4 ToEngineMatrix(FbxMatrix const &fbx_mat)
{
	//For reference, fbx_mat is stored Row Major, and can be accessed...
	FbxDouble4 row0 = fbx_mat.mData[0];
	FbxDouble4 row1 = fbx_mat.mData[1];
	FbxDouble4 row2 = fbx_mat.mData[2];
	FbxDouble4 row3 = fbx_mat.mData[3];

	// An FbxDouble4 has it's valued stored in an mData array as well
	Vector4 iBasis((float)row0.mData[0], (float)row0.mData[1], (float)row0.mData[2], (float)row0.mData[3]);
	Vector4 jBasis((float)row1.mData[0], (float)row1.mData[1], (float)row1.mData[2], (float)row1.mData[3]);
	Vector4 kBasis((float)row2.mData[0], (float)row2.mData[1], (float)row2.mData[2], (float)row2.mData[3]);
	Vector4 wBasis((float)row3.mData[0], (float)row3.mData[1], (float)row3.mData[2], (float)row3.mData[3]);

	Matrix4 flipX(Vector4(-1.0f, 0.0f, 0.0f, 0.0f), Vector4(0.0f, 1.0f, 0.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f, 0.0f), Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	Matrix4 ret(iBasis, jBasis, kBasis, wBasis);

	return MatrixMultiplicationRowMajorAB(MatrixMultiplicationRowMajorAB(flipX, ret), flipX);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
void NormailizeSkinWeights(std::vector<fbx_skin_weight_t>* skin_weights)
{
	for (unsigned int index = 0; index < skin_weights->size(); ++index)
	{
		fbx_skin_weight_t* skin_weight = &(*skin_weights)[index];
		float total = skin_weight->weights.SumOfAllParts();
		skin_weight->weights.x /= total;
		skin_weight->weights.y /= total;
		skin_weight->weights.z /= total;
		skin_weight->weights.w /= total;
	}
}

// Import a single mesh attribute
void ImportMesh(MeshBuilder *out, MatrixStack &mat_stack, FbxMesh *mesh, Skeleton* skeleton /*= nullptr*/)
{
	// Should have been triangulated before this - sort of a late check 
	// [treat this as an ASSERT_OR_DIE]
	//ASSERT_RETURN(mesh->IsTriangleMesh());


	// Geometric Transformations only apply to the current node
	// http://download.autodesk.com/us/fbx/20112/FBX_SDK_HELP/index.html?url=WS1a9193826455f5ff1f92379812724681e696651.htm,topicNumber=d0e7429
	Matrix4 geo_trans = GetGeometricTransform(mesh->GetNode());
	mat_stack.Push(geo_trans);

	// import the mesh data.
	Matrix4 transform = mat_stack.GetTop();

	std::vector<fbx_skin_weight_t> skin_weights;
	if (nullptr != skeleton) {
		CalculateSkinWeights(&skin_weights, mesh, skeleton);
		NormailizeSkinWeights(&skin_weights);
	}

	// Starting a draw call.
	out->Start(PRIMITIVE_TRIANGLE, false);

	// Load in the mesh - first, figure out how many polygons there are
	int32_t poly_count = mesh->GetPolygonCount();
	for (int32_t poly_idx = 0; poly_idx < poly_count; ++poly_idx) {

		// For each polygon - get the number of vertices that make it up (should always be 3 for a triangulated mesh)
		int32_t vert_count = mesh->GetPolygonSize(poly_idx);
		//ASSERT(vert_count == 3); // Triangle Meshes should ALWAYS have 3 verts per poly

								 // Finally, import all the data for this vertex (for now, just position)
		for (int32_t vert_idx = 0; vert_idx < vert_count; ++vert_idx) {
			int skinIndex = mesh->GetPolygonVertex(poly_idx, vert_idx);
			if (skinIndex >= 0 && !skin_weights.empty()){
				fbx_skin_weight_t skinWeight = skin_weights[skinIndex];
				FbxImportVertex(out, transform, mesh, poly_idx, vert_idx, &skinWeight);
			} 
			else
				FbxImportVertex(out, transform, mesh, poly_idx, vert_idx, nullptr);
		}
	}

	out->End();

	// pop the geometric transform
	mat_stack.Pop();
}

void CalculateSkinWeights(std::vector<fbx_skin_weight_t>* skin_weights, FbxMesh const *mesh, Skeleton* skeleton)
{
	// default them all to zero
	uint ctrl_count = (uint)mesh->GetControlPointsCount();
	skin_weights->reserve(ctrl_count);
	skin_weights->resize(ctrl_count);

	for (uint i = 0; i < ctrl_count; ++i) {
		fbx_skin_weight_t& skinWeight = (*skin_weights)[i];
		skinWeight.reset();
	}

	if (skeleton == nullptr) {
		skin_weights->clear();
		return;
	}

	// for each deformer [probably will only be one] - figure out how it affects
	// my vertices
	int deformer_count = mesh->GetDeformerCount(FbxDeformer::eSkin);
	for (int didx = 0; didx < deformer_count; ++didx) {
		FbxSkin *skin = (FbxSkin*)mesh->GetDeformer(didx, FbxDeformer::eSkin);
		if (nullptr == skin) {
			continue;
		}

		// cluster is how a single joint affects the mesh, 
		// so go over the vertices it affects and keep track of how much!
		int cluster_count = skin->GetClusterCount();
		for (int cidx = 0; cidx < cluster_count; ++cidx) {
			FbxCluster *cluster = skin->GetCluster(cidx);
			FbxNode const *link_node = cluster->GetLink();

			// Can't use it without a link node [shouldn't happen!]
			if (nullptr == link_node) {
				continue;
			}

			// Find the joint - if this skeleton doesn't have a joint
			// then we skip this cluster!
			uint32_t joint_idx = skeleton->GetJointIndex(link_node->GetName());
			if (joint_idx == (unsigned int)INVALID_INDEX) {
				continue;
			}

			// cluster stores things in an index buffer again,
			// it will store indices to the control points it affects, 
			// as well as how much weight we apply
			int *indices = cluster->GetControlPointIndices();
			int index_count = cluster->GetControlPointIndicesCount();
			double *weights = cluster->GetControlPointWeights();

			for (int i = 0; i < index_count; ++i) {
				int control_idx = indices[i];
				double weight = weights[i];

				fbx_skin_weight_t* skin_weight = &(*skin_weights)[control_idx];
				AddHighestWeight(skin_weight, (uint32_t)joint_idx, (float)weight);
			}
		}
	}
}

//------------------------------------------------------------------------
// Keep track of this influencer if it influences more than an already existing
// bone. [all bones influence at 0 at the start]
void AddHighestWeight(fbx_skin_weight_t *skin_weight, uint32_t bone_idx, float weight)
{
	if (skin_weight->weights.x == 0.0f)
	{
		skin_weight->weights.x = weight;
		skin_weight->indices.x = bone_idx;
		return;
	}
	else if (skin_weight->weights.y == 0.0f)
	{
		skin_weight->weights.y = weight;
		skin_weight->indices.y = bone_idx;
		return;
	}
	else if (skin_weight->weights.z == 0.0f)
	{
		skin_weight->weights.z = weight;
		skin_weight->indices.z = bone_idx;
		return;
	}
	else if (skin_weight->weights.w == 0.0f)
	{
		skin_weight->weights.w = weight;
		skin_weight->indices.w = bone_idx;
		return;
	}



	if (skin_weight->weights.x < weight && skin_weight->weights.y > weight && skin_weight->weights.z > weight && skin_weight->weights.w > weight)
	{
		skin_weight->weights.x = weight;
		skin_weight->indices.x = bone_idx;
	}
	else if (skin_weight->weights.y < weight && skin_weight->weights.x > weight && skin_weight->weights.z > weight && skin_weight->weights.w > weight)
	{
		skin_weight->weights.y = weight;
		skin_weight->indices.y = bone_idx;
	}
	else if (skin_weight->weights.z < weight && skin_weight->weights.y > weight && skin_weight->weights.x > weight && skin_weight->weights.w > weight)
	{
		skin_weight->weights.z = weight;
		skin_weight->indices.z = bone_idx;
	}
	else if (skin_weight->weights.w < weight && skin_weight->weights.y > weight && skin_weight->weights.z > weight && skin_weight->weights.x > weight)
	{
		skin_weight->weights.w = weight;
		skin_weight->indices.w = bone_idx;
	}
}

//------------------------------------------------------------------------
bool HasSkinWeights(FbxMesh const *mesh)
{
	int deformer_count = mesh->GetDeformerCount(FbxDeformer::eSkin);
	return deformer_count > 0;
}

// This gets a local transform that is applied ONLY to data stored at this node,
// and is always considered local to the node.
Matrix4 GetGeometricTransform(FbxNode *node)
{
	Matrix4 ret;
	ret.MakeIdentity();

	if ((node != nullptr) && (node->GetNodeAttribute() != nullptr)) {
		FbxEuler::EOrder order;

		node->GetRotationOrder(FbxNode::eSourcePivot, order);

		FbxVector4 const geo_trans = node->GetGeometricTranslation(FbxNode::eSourcePivot);
		FbxVector4 const geo_rot = node->GetGeometricRotation(FbxNode::eSourcePivot);
		FbxVector4 const geo_scale = node->GetGeometricScaling(FbxNode::eSourcePivot);

		FbxAMatrix geo_mat;
		FbxAMatrix mat_rot;
		mat_rot.SetR(geo_rot, order);
		geo_mat.SetTRS(geo_trans, mat_rot, geo_scale);

		ret = ToEngineMatrix(geo_mat);
	}

	return ret;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
// This loads in all the information for a specific vertex as defined by a 
// poly_idx and vert_idx.  For now, will only worry about position.
void FbxImportVertex(MeshBuilder *out, Matrix4&transform, FbxMesh *mesh, int32_t poly_idx, int32_t vert_idx, fbx_skin_weight_t* skinWeight)
{
	/** NEW BITS **/

	// Will demonstarate with normal, but this process is repeated for
	// uvs, colors, tangents, bitangents (called binormals in the SDK)
	Vector3 normal;
	if (GetNormal(&normal, transform, mesh, poly_idx, vert_idx)) {
		out->SetNormal(normal);
	}

	Vector2 uv;
	if (GetUV(&uv, mesh, poly_idx, vert_idx)) {
		out->SetUV(uv);
	}

	if (skinWeight != nullptr) {
		out->SetBoneWeightsAndIndices(skinWeight->indices, skinWeight->weights);
	}

	// If we get a position- push it.
	// This actually should always succeed.
	Vector3 position;
	if (FbxGetPosition(&position, transform, mesh, poly_idx, vert_idx)) {
		out->AddVertex(position);
	}
}

// Gets the position from a FbxMesh value.
// Every <poly_idx, vert_idx> pair will evaluate to a unique
// "control point" - that is, a position, or vertex that can be 
// manipulate by the program - which makes importing positions pretty easy

bool FbxGetPosition(Vector3* out_pos, Matrix4& transform, FbxMesh *mesh, int poly_idx, int vert_idx)
{
	// First, get the control point index for this poly/vert pair.
	int ctrl_idx = mesh->GetPolygonVertex(poly_idx, vert_idx);
	if (ctrl_idx < 0) {
		return false;
	}

	// Get the position for the control point.
	FbxVector4 fbx_pos = mesh->GetControlPointAt(ctrl_idx);

	// convert to an engine vertex, and save it to the out variable
	Vector3 pos = Vector3((float)fbx_pos.mData[0], (float)fbx_pos.mData[1], (float)fbx_pos.mData[2]);
	*out_pos = transform.MultiplyByVector(pos);
	out_pos->x *= -1.0f;

	// return success.
	return true;
}

//------------------------------------------------------------------------
bool GetNormal(Vector3 *out, Matrix4& transform, FbxMesh *mesh, int poly_idx, int vert_idx)
{
	// First, we need to get the geometry element we care about...
	// you can this of this as a "layer" of data for a mesh.  In this case
	// is is the normal data.

	// Note:  0 Index - meshes can potentially contain multiple layers
	// of the same type (UVs are the most common exmaple of this).  
	// For Normals, I just use the first one and continue on.
	FbxGeometryElementNormal *element = mesh->GetElementNormal(0);
	if (element == nullptr) {
		// no layer - no data
		return false;
	}

	// Next, we need to figure out how to pull the normal for this particular vertex out
	// of this layer. 
	// 
	// So, vnormal is just a packed array of normals, or potentially normals, and
	// and index buffer into those normals.
	//
	// Either way, you can think of vnormal as a packed collection of normals.
	// and we're trying to figure out which element of this array we want (elem_idx)

	// So first, get the element index.
	// (Is the data tied to the control point, or the polygon vertex)
	int elem_idx = 0;
	switch (element->GetMappingMode()) {
	case FbxGeometryElement::eByControlPoint: {
		// Get the normal by control point - most direct way.
		elem_idx = mesh->GetPolygonVertex(poly_idx, vert_idx);
	} break; // case: eByControlPoint

	case FbxGeometryElement::eByPolygonVertex: {
		// array is packed by polygon vertex - so each polygon is unique
		// (think sharp edges in the case of normals)
		elem_idx = mesh->GetPolygonVertexIndex(poly_idx) + vert_idx;
	} break; // case: eByPolygonVertex

	default:
		ASSERT_OR_DIE(0,"Unknown Mapping Mode"); // Unknown mapping mode
		return false;
	}

	// next, now that we have the element index, we figure out how this is used.
	// Is this a direct array, or an indexed array.
	// If we got an element index, fetch the data based on how it's referenced
	// (Directly, or by an index into an array);
	switch (element->GetReferenceMode()) {
	case FbxGeometryElement::eDirect: {
		// this is just an array - use elem index as an index into this array
		if (elem_idx < element->GetDirectArray().GetCount()) {
			FbxVector4 vec = element->GetDirectArray().GetAt(elem_idx);
			*out = Vector3((float)vec.mData[0], (float)vec.mData[1], (float)vec.mData[2]);
			*out = transform.MultiplyByVector(*out);
			out->x *= -1.0f;
			return true;
		}
	} break;

	case FbxGeometryElement::eIndexToDirect: {
		// This is an indexed array, so elem_idx is our offset into the 
		// index buffer.  We use that to get our index into the direct array.
		if (elem_idx < element->GetIndexArray().GetCount()) {
			int index = element->GetIndexArray().GetAt(elem_idx);
			FbxVector4 vec = element->GetDirectArray().GetAt(index);
			*out = Vector3((float)vec.mData[0], (float)vec.mData[1], (float)vec.mData[2]);
			*out = transform.MultiplyByVector(*out);
			out->x *= -1.0f;

			return true;
		}
	} break;

	default:
		ASSERT_OR_DIE(0,"Unknown Reference Type"); // Unknown reference type
		return false;
	}
	return false;
}

bool GetUV(Vector2* out, FbxMesh *mesh, int poly_idx, int vert_idx)
{
	// First, we need to get the geometry element we care about...
	// you can this of this as a "layer" of data for a mesh.  In this case
	// is is the normal data.

	// Note:  0 Index - meshes can potentially contain multiple layers
	// of the same type (UVs are the most common exmaple of this).  
	// For Normals, I just use the first one and continue on.
	FbxGeometryElementUV* element = mesh->GetElementUV(0);
	if (element == nullptr) {
		// no layer - no data
		return false;
	}

	// Next, we need to figure out how to pull the normal for this particular vertex out
	// of this layer. 
	// 
	// So, vnormal is just a packed array of normals, or potentially normals, and
	// and index buffer into those normals.
	//
	// Either way, you can think of vnormal as a packed collection of normals.
	// and we're trying to figure out which element of this array we want (elem_idx)

	// So first, get the element index.
	// (Is the data tied to the control point, or the polygon vertex)
	int elem_idx = 0;
	switch (element->GetMappingMode()) {
	case FbxGeometryElement::eByControlPoint: {
		// Get the normal by control point - most direct way.
		elem_idx = mesh->GetPolygonVertex(poly_idx, vert_idx);
	} break; // case: eByControlPoint

	case FbxGeometryElement::eByPolygonVertex: {
		// array is packed by polygon vertex - so each polygon is unique
		// (think sharp edges in the case of normals)
		elem_idx = mesh->GetPolygonVertexIndex(poly_idx) + vert_idx;
	} break; // case: eByPolygonVertex

	default:
		ASSERT_OR_DIE(0, "Unknown Mapping Mode"); // Unknown mapping mode
		return false;
	}

	// next, now that we have the element index, we figure out how this is used.
	// Is this a direct array, or an indexed array.
	// If we got an element index, fetch the data based on how it's referenced
	// (Directly, or by an index into an array);
	switch (element->GetReferenceMode()) {
	case FbxGeometryElement::eDirect: {
		// this is just an array - use elem index as an index into this array
		if (elem_idx < element->GetDirectArray().GetCount()) {
			FbxVector4 vec = element->GetDirectArray().GetAt(elem_idx);
			*out = Vector2((float)vec.mData[0], (float)-vec.mData[1]);
			return true;
		}
	} break;

	case FbxGeometryElement::eIndexToDirect: {
		// This is an indexed array, so elem_idx is our offset into the 
		// index buffer.  We use that to get our index into the direct array.
		if (elem_idx < element->GetIndexArray().GetCount()) {
			int index = element->GetIndexArray().GetAt(elem_idx);
			FbxVector4 vec = element->GetDirectArray().GetAt(index);
			*out = Vector2((float)vec.mData[0], (float)-vec.mData[1]);
			return true;
		}
	} break;

	default:
		ASSERT_OR_DIE(0, "Unknown Reference Type"); // Unknown reference type
		return false;
	}
	return false;
}

bool FbxLoadSkeleton(Skeleton *skel, char const *filename)
{
	// Same as you did for Meshes.
	FbxScene *fbx_scene = FbxLoadScene(filename, false);
	if (nullptr == fbx_scene) {
		return false;
	}

	ConvertSceneToEngineBasis(fbx_scene);

	// First, get the bind post for the scene
	skel->Clear();


	// Now, traverse the scene, and build the skeleton out with 
	// the bind pose positions
	FbxNode *root = fbx_scene->GetRootNode();

	// Bind pose - the pose which assets are authored for.
	FbxPose *pose = GetBindPose(fbx_scene);

	// Import the skeleton by traversing the scene.
	ImportSkeleton(skel, root, nullptr, nullptr, pose);

	// Flip the Axis 
	FlipXAxis(skel);

	FbxUnloadScene(fbx_scene);

	// Success if I've loaded in at least ONE bone/joint.
	return (skel->GetJointCount() > 0U);
}

void FlipXAxis(Skeleton* skel)
{
	for (unsigned int index = 0; index < skel->m_globalTransform.size(); ++index)
	{
		skel->m_globalTransform[index].MultiplyByVector(Vector3(-1.0f, 0.0f, 0.0f));
	}
}

// Get the bind pose - that is, the pose that the resources are authored
// for. If none exists - we'll assume ONLY the bind pose exists, and use
// the global transformations.
FbxPose* GetBindPose(FbxScene *scene)
{
	int pose_count = scene->GetPoseCount();
	for (int i = 0; i < pose_count; ++i) {
		FbxPose *pose = scene->GetPose(i);
		if (pose->IsBindPose()) {
			// DEBUG - Print all nodes part of this pose
			/*
			int count = pose->GetCount();
			for (int j = 0; j < count; ++j) {
			FbxNode *node = pose->GetNode(j);
			Trace( "fbx", "Pose Node: %s", node->GetName() );
			}
			*/

			return pose;
		}
	}

	return nullptr;
}

// Imports the skeleton.
void ImportSkeleton(Skeleton *out, FbxNode *node, FbxSkeleton *root_bone, FbxSkeleton *parent_bone, FbxPose *pose)
{
	for (int i = 0; i < node->GetNodeAttributeCount(); ++i) {

		// If this node is a skeleton node (a joint), 
		// then it should have a skeleton attribute. 
		FbxNodeAttribute *na = node->GetNodeAttributeByIndex(i);
		if (na->GetAttributeType() == FbxNodeAttribute::eSkeleton) {

			// Cast it, and figure out the type of bone. 
			FbxSkeleton *skel = (FbxSkeleton*)na;
			FbxSkeleton::EType type = skel->GetSkeletonType();

			// From FBXSDK Documentation.
			// eRoot,			/*!< First element of a chain. */
			// eLimb,			/*!< Chain element. */
			// eLimbNode,		/*!< Chain element. */
			// eEffector		/*!< Last element of a chain. */

			// If this is a root - we better have not have another root higher 
			// up (for now).  This is just here to catch this happening so
			// I know if I should support it in the future - you could cut this.
			if ((type == FbxSkeleton::eRoot) || (type == FbxSkeleton::eEffector)) {
				// this is a root bone - so can treat it as the root
				// of a skeleton.
				// TODO: no support for nested skeletons yet.
				if(root_bone == nullptr)//----------------------------------Was an ASSERT
					root_bone = skel;
			} // else, it is a limb or limb node, which we treat the same

			  // See if this bone exists int he pose, and if so, 
			  // get its index in the pose.
			int pose_node_idx = -1;
			if (pose != nullptr) {
				pose_node_idx = pose->Find(node);
			}

			// Get the global transform for this bone
			Matrix4 bone_transform;
			if (pose_node_idx != -1) {
				// If it exists in the pose - use the poses transform.
				// Since we're only doing the bind pose now, the pose should ONLY
				// be storing global transforms (as per the documentation).
				//
				// When we update this code to load in animated poses, this willc hange.
				// bool is_local = pose->IsLocalMatrix(pose_node_idx);
				//?ASSERT(false == is_local); // for now, should always be global (pose transforms always are)

				FbxMatrix fbx_matrix = pose->GetMatrix(pose_node_idx);
				bone_transform = ToEngineMatrix(fbx_matrix);
			}
			else {
				// Not in the pose - so we have two options
				if (parent_bone == nullptr) {
					// We have no parent [first in the skeleton], so just use 
					// the global transform of the node.
					bone_transform = ToEngineMatrix(node->EvaluateGlobalTransform());
				}
				else {
					// Otherwise, calculate my world based on my parent transform
					// and my nodes local.
					//
					// The reason behind this is EvaluateGlobalTransform actually
					// gets the animated transform, while getting the Pose transform
					// will get an unanimated transform.  So if we're a child of a transform
					// that was gotten from a pose, I should be evaluating off that.
					//
					// TODO: Evaluate Local from Parent - just in case their is siblings between this
					Matrix4 local = ToEngineMatrix(node->EvaluateLocalTransform());
					Matrix4 ptrans = out->GetJointTransform(GetBoneName(parent_bone));
					bone_transform = MatrixMultiplicationRowMajorAB(local, ptrans);
				}
			}

			// Add a joint.
			out->AddJoint(GetBoneName(skel), (parent_bone != nullptr) ? GetBoneName(parent_bone) : "", bone_transform);

			// set this as the next nodes parent, and continue down the chain
			parent_bone = skel;
			break;
		}
	}

	// import the rest
	for (int i = 0; i < node->GetChildCount(); ++i) {
		ImportSkeleton(out, node->GetChild(i), root_bone, parent_bone, pose);
	}
}

//------------------------------------------------------------------------
// Just a helper - checks for null - but otherwise gets the node name
static const std::string GetNodeName(FbxNode const *node)
{
	if (nullptr == node) {
		return std::string("");
	}
	else {
		return std::string(node->GetName());
	}
}


//------------------------------------------------------------------------
// Get the name to use for this bone (ideally, use the node, but if the bone
// doesn't exist, use the skeleton attributes name)
static const std::string GetBoneName(FbxSkeleton const *skel)
{
	if (nullptr == skel) {
		return "";
	}

	const std::string node_name = GetNodeName(skel->GetNode());
	if (!node_name.empty()) {
		return std::string(skel->GetNode()->GetName());
	}
	else {
		return std::string(skel->GetName());
	}
}


//------------------------------------------------------------------------
// Just as an extra - if you want to use the FBX files frame rate, this
// is how you can fetch it.
// static float GetNativeFramefrate(FbxScene* scene)
// {
// 	// Get the scenes authored framerate
// 	FbxGlobalSettings &settings = scene->GetGlobalSettings();
// 	FbxTime::EMode time_mode = settings.GetTimeMode();
// 	double framerate;
// 	if (time_mode == FbxTime::eCustom) {
// 		framerate = settings.GetCustomFrameRate();
// 	}
// 	else {
// 		framerate = FbxTime::GetFrameRate(time_mode);
// 	}
// }

//------------------------------------------------------------------------
static bool FbxImportMotion(Motion* motion, Skeleton* skeleton, FbxScene* scene, FbxAnimStack* anim, float framerate)
{
	// First, set the scene to use this animation - helps all the evaluation functions
	scene->SetCurrentAnimationStack(anim);

	// how far do we move through this
	FbxTime advance;
	advance.SetSecondDouble((double)(1.0f / framerate));

	// Get the start, end, and duration of this animation.
	FbxTime local_start = anim->LocalStart;
	FbxTime local_end = anim->LocalStop;
	float local_start_fl = (float)local_start.GetSecondDouble();
	float local_end_fl = (float)local_end.GetSecondDouble();

	// Okay, so local start will sometimes start BEFORE the first frame - probably to give it something to T
	// pose?  This was adding a jerky frame movement to UnityChan's walk cycle. 
	// Whatever, we'll always start at least 0.
	local_start_fl = (std::max)(0.0f, local_start_fl);
	float duration_fl = local_end_fl - local_start_fl;

	// no duration, no animation!
	if (duration_fl <= 0.0f) {
		return false;
	}

	// Alright, we have a motion, we start getting data from it
	char const *motion_name = anim->GetName();
	//float time_span = duration_fl;

	// set some identifying information
	std::string temp_name = std::string(motion_name);
	motion->SetName(temp_name);
	motion->SetDuration(duration_fl);
	motion->SetFrameRate(framerate);

	// number of frames should encompasses the duration, so say we are at 10hz,
	// but have an animation that is 0.21f seconds.  We need at least...
	//    3 intervals (Ceiling(10 * 0.21) = Ceiling(2.1) = 3)
	//    4 frames (0.0, 0.1, 0.2, 0.3)
	unsigned int frame_count = (unsigned int)ceil(duration_fl * framerate) + 1;

	// Now, for each joint in our skeleton
	// not the most efficient way to go about this, but whatever - tool step
	// and it is not slow enough to matter.
	unsigned int joint_count = skeleton->GetJointCount();

	for (unsigned int frame_idx = 0; frame_idx < frame_count; ++frame_idx)
	{
		Pose* pose = motion->GetPose(frame_idx);
		pose->m_localTransforms.resize(joint_count);
	}

	for(unsigned int joint_idx = 0; joint_idx < joint_count; ++joint_idx) {
		std::string name = skeleton->GetJointName(joint_idx);

		// get the node associated with this bone
		// I rely on the names to find this - so author of animations
		// should be sure to be using the same rig/rig names.
		// (this is only important for the import process, after that
		// everything is assuming matching indices)
		FbxNode *node = scene->FindNodeByName(name.c_str());
		FbxNode *parent = nullptr;
		unsigned int parent_idx = skeleton->GetJointParent(joint_idx);
		//ASSERT(node != nullptr);

		// get the parent node
		if (parent_idx != (unsigned int)INVALID_INDEX) {
			std::string parent_name = skeleton->GetJointName(parent_idx);
			parent = scene->FindNodeByName(parent_name.c_str());
			//ASSERT(parent != nullptr);
		}

		// Now, for this entire animation, evaluate the local transform for this bone at every interval
		// number of frames is 
		FbxTime eval_time = FbxTime(0);
		for(unsigned int frame_idx = 0; frame_idx < frame_count; ++frame_idx) {
			// Okay, get the pose we want
			Pose* pose = motion->GetPose(frame_idx);
			Matrix4 joint_world = GetNodeWorldTransformAtTime(node, eval_time);
			Matrix4 joint_local = joint_world;
			if (nullptr != parent) {
				Matrix4 parent_world = GetNodeWorldTransformAtTime(parent, eval_time);
				joint_local = MatrixMultiplicationRowMajorAB(joint_world, parent_world.GetInverse());
			}

			eval_time += advance;
			
			Transform local;
			local.scale = joint_local.GetScale();
			local.position = joint_local.GetPosition();
			local.rotation = Quaternion(joint_local);
			pose->m_localTransforms[joint_idx] = local; //pose->m_localTransforms.insert(pose->m_localTransforms.begin() + joint_idx, local);
		}
	}

	return true;
}

//------------------------------------------------------------------------
bool FbxLoadMotion(Motion *motion, Skeleton* skeleton, char const *filename, unsigned int framerate /*= 10*/)
{

	// Normal things
	FbxScene *scene = FbxLoadScene(filename, false);
	if (nullptr == scene) {
		return false;
	}
	ConvertSceneToEngineBasis(scene);

	// Framerate - how often are we sampling this
	float fr = (float)framerate;


	// Next get the animation count.
	int anim_count = scene->GetSrcObjectCount<FbxAnimStack>();
	if (anim_count > 0U) {
		// great, we have a motion
		// NOTE:  Multiple motions may be embedded in this file, you could update to extract all 
		FbxAnimStack *anim = scene->GetSrcObject<FbxAnimStack>(0);
		FbxImportMotion(motion, skeleton, scene, anim, fr);
	}

	FbxUnloadScene(scene);

	return (motion->GetDuration() > 0.0f);
}

#else 
// Stubs so that if we don't have the FBX SDK Available, our program does not crash
void FbxListFile(const std::string&) {}
#endif


uint ConvertFBXFileToEngineFilesOrReadEngineFile(char const *fbx_filename_and_path_with_ext, char const *root_output_name_and_path, Mesh* mesh /*= nullptr*/, Skeleton* skeleton /*= nullptr*/, Motion* motion /*= nullptr*/, uint frame_rate /*= 10*/, uint import_flags/* = IMPORT_ALL*/)
{
	FileBinaryStream fileStream;
	uint fileCreationCount = 0;
	frame_rate;
	fbx_filename_and_path_with_ext;

	if (import_flags == IMPORT_ALL || import_flags == IMPORT_SKELETON || import_flags == IMPORT_MESH_AND_SKELETON || import_flags == IMPORT_MOTION_AND_SKELETON)
	{
		//Read/Write Skeleton
		std::string skelPath = std::string(root_output_name_and_path) + ".skel";
		bool fileExists = fileStream.open_for_read(skelPath.c_str());
		if (fileExists)
		{
			//Read From File
			skeleton->ReadFromStream(&fileStream);
		}
		else
		{
			#if defined(TOOLS_BUILD)
			//Read FBX and Write To Custom File
			fileStream.close();
			FbxLoadSkeleton(skeleton, fbx_filename_and_path_with_ext);
			fileStream.open_for_write(skelPath.c_str());
			skeleton->WriteToStream(&fileStream);
			++fileCreationCount;
			#endif
		}
		fileStream.close();
	}

	if (import_flags == IMPORT_ALL || import_flags == IMPORT_MESH || import_flags == IMPORT_MESH_AND_SKELETON)
	{
		//Read/Write Mesh
		std::string meshPath = std::string(root_output_name_and_path) + ".mesh";
		bool fileExists = fileStream.open_for_read(meshPath.c_str());
		if (fileExists)
		{
			//Read From File
			mesh->ReadFromStream(&fileStream);
		}
		else
		{
			#if defined(TOOLS_BUILD)
			//Read FBX and Write To Custom File
			fileStream.close();
			MeshBuilder builder;
			FbxLoadMesh(&builder, fbx_filename_and_path_with_ext, skeleton);
			builder.GenerateIndices();
			*mesh = Mesh(builder);
			fileStream.open_for_write(meshPath.c_str());
			mesh->WriteToStream(&fileStream);
			++fileCreationCount;
			#endif
		}
		fileStream.close();
	}

	if (import_flags == IMPORT_ALL || import_flags == IMPORT_MOTION || import_flags == IMPORT_MOTION_AND_SKELETON)
	{
		//Read/Write Motion
		std::string motionPath = std::string(root_output_name_and_path) + ".motion";
		bool fileExists = fileStream.open_for_read(motionPath.c_str());
		if (fileExists)
		{
			//Read From File
			motion->ReadFromStream(&fileStream);
		}
		else
		{
			#if defined(TOOLS_BUILD)
			//Read FBX and Write To Custom File
			fileStream.close();
			FbxLoadMotion(motion, skeleton, fbx_filename_and_path_with_ext, frame_rate);
			motion->SetFrameRate((float)frame_rate);
			fileStream.open_for_write(motionPath.c_str());
			motion->WriteToStream(&fileStream);
			++fileCreationCount;
			#endif
		}
		fileStream.close();
	}

	return fileCreationCount;
}