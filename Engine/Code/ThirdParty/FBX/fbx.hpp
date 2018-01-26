#pragma once
#include <string>
#include "Engine/Render/MeshBuilder.hpp"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Math/MatrixStack.hpp"
#include "Engine/Render/Skeleton.hpp"
#include "Engine/Render/Motion.hpp"
#include "Engine/Render/Pose.hpp"

class Mesh;

#if !defined( __TOOLS_FBXLOAD__ )
#define __TOOLS_FBXLOAD__



enum ImportType : unsigned int
{
	IMPORT_ALL,
	IMPORT_MESH,
	IMPORT_SKELETON,
	IMPORT_MOTION,
	IMPORT_MOTION_AND_SKELETON,
	IMPORT_MESH_AND_SKELETON,
	NUM_IMPORT_OPTIONS
};


void FbxListFile(const std::string& filename);
bool FbxLoadMesh(MeshBuilder *mb, char const *filename, Skeleton* skeleton = nullptr);
bool FbxLoadSkeleton(Skeleton *skel, char const *filename);
bool FbxLoadMotion(Motion* motion, Skeleton* skeleton, char const *filename, unsigned int framerate = 10);

#endif

uint ConvertFBXFileToEngineFilesOrReadEngineFile(char const *fbx_filename_and_path_with_ext, char const *root_output_name_and_path, Mesh* mesh = nullptr, Skeleton* skeleton = nullptr, Motion* motion = nullptr, uint frame_rate = 10, uint import_flags = IMPORT_ALL);
