#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

#include "model.h"

cl_float * importModel(const char * filename, size_t * triangle_count) {
	const struct aiScene * scene = aiImportFile(filename,
		aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

	if (!scene) {
		const char * err = aiGetErrorString();
		fprintf(stderr, "%s\n", err);
		return NULL;
	}

	// loop over each mesh in the given scene
	for (unsigned i = 0; i<scene->mNumMeshes; i++) {
		struct aiMesh * mesh = scene->mMeshes[i];

		// TODO - Fix this for more than 1 mesh.
		*triangle_count = mesh->mNumFaces;
		cl_float * data = malloc(sizeof(cl_float) * (*triangle_count * TRIANGLE_SIZE));
		cl_float * tmp = data;

		// then for each mesh, loop over each face
		for (unsigned k = 0; k<mesh->mNumFaces; k++) {
			struct aiFace f = mesh->mFaces[k];

			// we asked assimp to triangulate faces
			// therefore each face has three indecies
			struct aiVector3D v0 = mesh->mVertices[f.mIndices[0]];
			struct aiVector3D v1 = mesh->mVertices[f.mIndices[1]];
			struct aiVector3D v2 = mesh->mVertices[f.mIndices[2]];

			// build a triangle which we can pass into our Ray Tracer
			make_triangle(
				vector3_init(v0.x, v0.y, v0.z),	
				vector3_init(v1.x, v1.y, v1.z),	
				vector3_init(v2.x, v2.y, v2.z),	
				tmp);
			tmp += TRIANGLE_SIZE;
		}

		// TODO - Fix this for more than 1 mesh.
		aiReleaseImport(scene);
		return data;
	}

	aiReleaseImport(scene);
	return NULL;
}
