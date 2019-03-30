//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

//
//

//polyAbcWriter.cpp

//General Includes
//
#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MIntArray.h>
#include <maya/MFnSet.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MPlug.h>
#include <maya/MIOStream.h>
#include <time.h>

//Iterator Includes
//
#include <maya/MItMeshPolygon.h>

//Header File
//
#include "polyAbcWriter.h"

//Macros
//
#define DELIMITER "\t"
#define SHAPE_DIVIDER "*******************************************************************************\n"
#define HEADER_LINE "===============================================================================\n"
#define LINE "-------------------------------------------------------------------------------\n"


polyAbcWriter::polyAbcWriter(const MDagPath& dagPath, MStatus& status):
polyWriter(dagPath, status),
fHeadUVSet(NULL)
//Summary:	creates and initializes an object of this class
//Args   :	dagPath - the DAG path of the current node
//			status - will be set to MStatus::kSuccess if the constructor was
//					 successful;  MStatus::kFailure otherwise
{
}


polyAbcWriter::~polyAbcWriter() 
//Summary:  deletes the objects created by this class
{
	if (NULL != fHeadUVSet) delete fHeadUVSet;
}


MStatus polyAbcWriter::extractGeometry() 
//Summary:	extracts main geometry as well as all UV sets and each set's
//			coordinates
{
	MGlobal::displayInfo("polyAbcWriter::ExtactGeometry\n");
	printf("polyAbcWriter::ExtactGeometry\n");

	if (MStatus::kFailure == polyWriter::extractGeometry()) {
		return MStatus::kFailure;
	}

	MStringArray uvSetNames;
	if (MStatus::kFailure == fMesh->getUVSetNames(uvSetNames)) {
		MGlobal::displayError("MFnMesh::getUVSetNames"); 
		return MStatus::kFailure;
	}

	unsigned int uvSetCount = uvSetNames.length();
	unsigned int i;

	UVSet* currUVSet = NULL;

	for (i = 0; i < uvSetCount; i++ ) {
		if (0 == i) {
			currUVSet = new UVSet;
			fHeadUVSet = currUVSet;
		} else {
			currUVSet->next = new UVSet;
			currUVSet = currUVSet->next;
		}

		currUVSet->name = uvSetNames[i];
		currUVSet->next = NULL;

		// Retrieve the UV values
		//
		if (MStatus::kFailure == fMesh->getUVs(currUVSet->uArray, currUVSet->vArray, &currUVSet->name)) {
			return MStatus::kFailure;
		}
	}

	return MStatus::kSuccess;
}


MStatus polyAbcWriter::writeToFile(ostream& os)
{
	MGlobal::displayInfo("polyAbcWriter::writeToFile(ostream)");
	return MStatus::kSuccess;
}


MStatus polyAbcWriter::writeGeometryToArchive(Alembic::AbcGeom::OXform &xformObj)
//Summary:	outputs the geometry of this polygonal mesh in Alembic Object format to an Alembic [Ogawa] Archive
//Args   :	os - an output stream to write to
//Returns:  MStatus::kSuccess if the method succeeds
//			MStatus::kFailure if the method fails
{
	MStatus status;

	//Debugging Output
	MGlobal::displayInfo("polyAbcWriter::writeGeometryToArchive");
	//printf("polyAbcWriter::writeGeometryToArchive\n");
	MGlobal::displayInfo("fMesh->fullPathName = " + fMesh->fullPathName() );
	MGlobal::displayInfo("fMesh->partialPathName = " + fMesh->partialPathName());

	//Create a PolyMesh ABC Object (Schema) as Child of the Xform Input Object
	Alembic::AbcGeom::OPolyMesh meshObj(xformObj, fMesh->partialPathName().asUTF8());
	Alembic::AbcGeom::OPolyMeshSchema &meshSchema = meshObj.getSchema();

	//we will need to prepare the following for a call to AbcGeom::OPolyMeshSchema::Sample
	//(i.e. to write the minimum spec geometry cache information)
	// 1)  floating point interleaved vertex array + size of array
	// 2)  floating point interleaved normal array + size of array
	// 3)  floating point interleaved uv array + size of array
	// 4)  Per Face Polygon Data:
	//		i)	  unsigned int per face vertex count array
	//		ii)	  unsigned int per face vertex index array (with proper winding order considerations)
	//		iii)  unsigned int per face uv index array + size of array

	//1)  Populate mesh interleaved floating point X,Y,Z vertex array
	unsigned int vertexCount = fVertexArray.length();
	float * floatVerts = (float*)alloca(sizeof(float) * 3 * vertexCount);
	if (0 == vertexCount) {
		return MStatus::kFailure;
	}
	for (unsigned vertexIndex = 0; vertexIndex< vertexCount; vertexIndex++) {
		floatVerts[vertexIndex*3] =		(float)fVertexArray[vertexIndex].x;
		floatVerts[vertexIndex*3+1] =	(float)fVertexArray[vertexIndex].y;
		floatVerts[vertexIndex*3+2] =	(float)fVertexArray[vertexIndex].z;
	}

	//2)  Populate mesh floating point interleaved normal array + size of array

	//Get the length of maya mesh normal array
	unsigned int normalCount = fNormalArray.length();
	if (0 == normalCount) {
		MGlobal::displayInfo("Error:  No Normals");
		return MStatus::kFailure;
	}

	//convert maya normal array to interleaved float array
	float * floatNorms = (float*)alloca(sizeof(float) * 3 * normalCount);
	//unsigned int normalIndex;
	for (int normalIndex = 0; normalIndex < (int)normalCount; normalIndex++) {
		floatNorms[normalIndex * 3] = fNormalArray[normalIndex].x;
		floatNorms[normalIndex * 3 + 1] = fNormalArray[normalIndex].y;
		floatNorms[normalIndex * 3 + 2] = fNormalArray[normalIndex].z;
	}

	//Alembic::AbcGeom::ON3fGeomParam::Sample nsamp(Alembic::Abc::N3fArraySample(NULL, 0), Alembic::AbcGeom::kVertexScope);
	Alembic::AbcGeom::ON3fGeomParam::Sample nsamp(Alembic::Abc::N3fArraySample((const Alembic::Abc::N3f*)&(floatNorms[0]), normalCount), Alembic::AbcGeom::kFacevaryingScope);
	//Alembic::AbcGeom::ON3fGeomParam::Sample nsamp(Alembic::Abc::N3fArraySample((const Alembic::Abc::N3f*)&(perFaceVertexNormals[0]), perFaceVertexNormals.size() / 3), Alembic::AbcGeom::kVertexScope);
	//Alembic::Abc::UInt32ArraySample nIndexSamp(Alembic::Abc::UInt32ArraySample((const uint32_t*)&(perFaceVertexNormalIndices[0]), perFaceVertexNormalIndices.size()));
	//nsamp.setIndices(nIndexSamp);

	//char buf[256];
	//_itoa_s(perFaceVertexNormals.size() / 3, buf, 10);
	//MString intStr = MString(buf);
	//MGlobal::displayInfo("perFaceVertexNormals.size = " + intStr);


	//4.i)  Populate unsigned int per face vertex count array

	//Allocate an array to store num verts per polygon
	unsigned int numFaces = fMesh->numPolygons();
	int * numVertsPerFace = (int*)alloca(numFaces * sizeof(uint32_t));
	//Define a variable to store total number of per face vertex indices
	int numFaceVertIndices = 0;

	//Iterate over each polygon to populate per face vertex count array and count the total number of indices
	for (unsigned faceIndex = 0; faceIndex < numFaces; faceIndex++) 
	{
		//get num indices in each face
		int numVertsInFace = (int)fMesh->polygonVertexCount(faceIndex, &status);
		*(&(numVertsPerFace[faceIndex])) = numVertsInFace;

		//sum total vertices of all faces
		numFaceVertIndices += numVertsInFace;
		if (MStatus::kFailure == status) {
			MGlobal::displayError("MFnMesh::polygonVertexCount");
			return MStatus::kFailure;
		}
	}

	//4.ii)  Populate an array to store the vertex indices for each face

	//Allocate an array to store per face vertex indices for all faces
	int * faceVertIndices = (int*)alloca(numFaceVertIndices * sizeof(int));
	int tmpFaceIndices[256];

	//Define variables to use in for loop
	int faceVertIndex = 0;
	MIntArray perFaceVertIndexArray;

	//int * faceNormalIndices = (int*)alloca(numFaceVertIndices * sizeof(int));
	//int normalIndex = 0;
	//std::vector<float> perFaceVertexNormals;
	//std::vector<uint32_t> perFaceVertexNormalIndices;
	//uint32_t perFaceVertexNormalIndex = 0;

	//Iterate over each polygon again to populate per face vertex count array(with proper winding order considerations)
	for (unsigned faceIndex = 0; faceIndex < numFaces; faceIndex++) {

		//Get a temporary array containing the indices into the vertex array list for each face in the mesh
		status = fMesh->getPolygonVertices(faceIndex, perFaceVertIndexArray);
		if (MStatus::kFailure == status) {
			MGlobal::displayError("MFnMesh::getPolygonVertices");
			return MStatus::kFailure;
		}
		//read array of vertex indices for the current face
		//indexArray.get( &(faceVertIndices[vertIndex]) );
		perFaceVertIndexArray.get(&(tmpFaceIndices[0]));

		//Access per face normals if desired
		/*
		status = fMesh->getFaceNormalIds(faceIndex, normalIndexArray);
		if (MStatus::kFailure == status) {
			MGlobal::displayError("MFnMesh::getFaceNormalIds");
			return MStatus::kFailure;
		}

		//read array of normal indices for the current face
		normalIndexArray.get( &(faceNormalIndices[normalIndex]) );
		normalIndex += normalIndexArray.length();
		if (normalIndexArray.length() != indexArray.length())
			printf("\nnormalIndexArray(%d) != indexArray(%d)\n", normalIndexArray.length(), indexArray.length());
		*/

		/*
		char buf[256];
		_itoa_s(indexArray.length(), buf, 10);
		MString lengthStr = MString(buf);
		MGlobal::displayInfo("Face Index Array Length = " + lengthStr);
		*/

		//Iterate over each vert in the face to populate the array of face vertex indices
		//while reversing the winding order for the face's vertices if necessary for our rendering software
		int faceVertCount = 0;
		for (int j = perFaceVertIndexArray.length()-1; j>-1; j--)
		{
			//Access the faces individual UV data if desired
			/*
			status = fMesh->getFaceVertexColorIndex(i, j, colorIndex);

			//output each uv set index for the current vertex on the current face
			for (currUVSet = fHeadUVSet; currUVSet != NULL; currUVSet = currUVSet->next) {
				status = fMesh->getPolygonUVid(i, j, uvID, &currUVSet->name);
				if (MStatus::kFailure == status) {
					MGlobal::displayError("MFnMesh::getPolygonUVid");
					return MStatus::kFailure;
				}
			}
			*/
			
			//Alembic interchange formats require front facing polygons to be in clockwise (aka left-handed) vertex order,
			//while Maya and most, if not all, other tools  use counter-clockwise (right-handed) vertex order, so we need to reverse the winding order
			//when writing to and reading from and Alembic Archive
			faceVertIndices[faceVertIndex + faceVertCount] = tmpFaceIndices[j];

			//Populate per vertex normal data if desired
			//int perFaceVertIndex = faceVertIndices[vertIndex + j];			
			//custom populate a vertex normal array if a different order from Maya is needed-
			//perFaceVertexNormals.push_back(fNormalArray[normalIndexArray[faceVertIndex]].x);
			//perFaceVertexNormals.push_back(fNormalArray[normalIndexArray[faceVertIndex]].y);
			//perFaceVertexNormals.push_back(fNormalArray[normalIndexArray[faceVertIndex]].z);

			faceVertCount++;
		}

		//increment our per face vertIndex placeholder
		faceVertIndex += perFaceVertIndexArray.length();
	}

	//3)		Populate a floating point interleaved uv array + size of array
	MIntArray uvCounts;
	MIntArray uvIds;
	UVSet* currUVSet;
	float * floatUVs = NULL;
	unsigned int uvIndex, uvCount;
	
	//Iterate over all UV sets until we find the current set associated with this mesh
	for (currUVSet = fHeadUVSet; currUVSet != NULL; currUVSet = currUVSet->next) {
		if (currUVSet->name == fCurrentUVSetName) {

			//If we match the current uv set associated with the geometry...
			MGlobal::displayInfo("Current ");
			meshSchema.setUVSourceName(fCurrentUVSetName.asUTF8());

			//Get the uv set element count
			uvCount = currUVSet->uArray.length();

			//Allocate memory for an interleaved floating point uv array
			//and convert maya u and v arrays to an interleaved float array
			floatUVs = (float*)alloca(sizeof(float) * 2 * uvCount);
			for (uvIndex = 0; uvIndex < uvCount; uvIndex++)
			{
				floatUVs[uvIndex*2] = currUVSet->uArray[uvIndex];
				floatUVs[uvIndex*2+1] = currUVSet->vArray[uvIndex];
			}

			//4.iii) Populate unsigned int per face uv index array + size of array
			fMesh->getAssignedUVs(uvCounts, uvIds, &fCurrentUVSetName);
		}

		//Display the uv set for the loop iteration for debugging
		MGlobal::displayInfo("UV Set:  " + currUVSet->name  + "\n");
	}
	

	//Create an AbcGeom UV Sample Object Container
	Alembic::AbcGeom::OV2fGeomParam::Sample uvsamp;
	if (floatUVs)
	{
		int* uvIndices = (int32_t*)alloca(uvIds.length() * sizeof(int32_t));
		uvIds.get(&(uvIndices[0]));

		uvsamp = Alembic::AbcGeom::OV2fGeomParam::Sample(Alembic::Abc::V2fArraySample((const Alembic::Abc::V2f*)floatUVs, uvCount), Alembic::AbcGeom::kFacevaryingScope);
		Alembic::Abc::UInt32ArraySample uvIndexSamp(Alembic::Abc::UInt32ArraySample((const uint32_t*)&(uvIndices[0]), uvIds.length()));
		uvsamp.setIndices(uvIndexSamp);

	}


	//populate the alembic poly mesh base sample(s) object(s)
	Alembic::AbcGeom::OPolyMeshSchema::Sample mesh_sample(
		Alembic::Abc::V3fArraySample((const Alembic::Abc::V3f*)floatVerts, vertexCount),
		Alembic::Abc::Int32ArraySample(faceVertIndices, numFaceVertIndices),
		Alembic::Abc::Int32ArraySample(numVertsPerFace, numFaces),
		uvsamp, nsamp);
	//mesh_sample.setNormals(nsamp);

	//set the alembic poly mesh base sample(s) object(s)
	meshSchema.set(mesh_sample);
	
	//Debug Abc Object Normals
	//buf = "\0";
	//Alembic::Abc::N3fArraySample normals = mesh_sample.getNormals().getVals();
	//_itoa_s(normals.size(), buf, 10);
	//MString normalsSizeStr = MString(buf);
	//MGlobal::displayInfo("meshSchema.numSamples = " + normalsSizeStr);

	//if there is more than one set, the last set simply consists of all 
	//polygons, so we won't include it
	/*
	unsigned int setCount = fPolygonSets.length();
	if (setCount > 1) {
		//setCount--;
	}

	//MIntArray faces;
	std::vector<int32_t> faces;

	unsigned int i;
	for (i = 0; i < setCount; i++) {
		
		MObject set = fPolygonSets[i];
		MObject comp = fPolygonComponents[i];
		MFnSet fnSet(set, &status);
		if (MS::kFailure == status) {
			MGlobal::displayError("MFnSet::MFnSet");
			continue;
		}

		MFnDependencyNode setDependencyNode(set);// .name() + " ";
		MGlobal::displayInfo("Polygon Set.name:  " + setDependencyNode.name() + "\n");

		MFnDependencyNode compDependencyNode(comp);// .name() + " ";
		MGlobal::displayInfo("Polygon Comp.name:  " + compDependencyNode.name() + "\n");

		//Make sure the set is a polygonal set.  If not, continue.
		MItMeshPolygon itMeshPolygon(*fDagPath, comp, &status);

		if ((MS::kFailure == status)) {
			MGlobal::displayError("MItMeshPolygon::MItMeshPolygon");
			continue;
		}

		//add the current set's face indices to the faces array
		//
		faces.clear();
		faces.resize(itMeshPolygon.count());

		unsigned int j = 0;
		for (itMeshPolygon.reset(); !itMeshPolygon.isDone(); itMeshPolygon.next()) {
			faces[j++] = itMeshPolygon.index();
		}

		//Alembic OFaceSet SDK exepects face sets to be ordered by face number 
		std::sort(faces.begin(), faces.end());
		Alembic::AbcGeom::OFaceSetSchema::Sample fSetSamp(Alembic::Abc::Int32ArraySample(&(faces[0]), faces.size()) );

		//Find the texture that is applied to this set.  First, get the
		//shading node connected to the set.  Then, if there is an input
		//attribute called "color", search upstream from it for a texture
		//file node.
		//
		MObject shaderNode = findShader(set);
		if (MObject::kNullObj == shaderNode) {
			continue;
		}

		MString apiTypeStr = MString(shaderNode.apiTypeStr());
		//MString shaderName = MString(shaderNode.);

		MFnDependencyNode shaderDependencyNode(shaderNode);// .name() + " ";
		//MFnDependencyNode(sdNode).name() + " ";

		MGlobal::displayInfo("shaderNode.apiType:  " + apiTypeStr + "\n");
		MGlobal::displayInfo("shaderNode.name:  " + shaderDependencyNode.name() + "\n");

		std::string faceSetName = std::string(shaderDependencyNode.name().asUTF8());
		Alembic::AbcGeom::OFaceSet materialFaceSet = meshSchema.createFaceSet(faceSetName);
		Alembic::AbcGeom::OFaceSetSchema mtlFaceSetSchema = materialFaceSet.getSchema();
		mtlFaceSetSchema.set(fSetSamp);

		//stingray pbr shader
		if (shaderNode.apiType() == MFn::kPluginHardwareShader)
		{
			MPlug useColorMapPlug = MFnDependencyNode(shaderNode).findPlug("use_color_map", false, &status);
			bool useColorMap;
			useColorMapPlug.getValue(useColorMap);

			MPlug useNormalMapPlug = MFnDependencyNode(shaderNode).findPlug("use_normal_map", false, &status);
			bool useNormalMap;
			useNormalMapPlug.getValue(useNormalMap);

			MPlug useMetallicMapPlug = MFnDependencyNode(shaderNode).findPlug("use_metallic_map", false, &status);
			bool useMetallicMap;
			useMetallicMapPlug.getValue(useMetallicMap);

			MPlug useRoughnessMapPlug = MFnDependencyNode(shaderNode).findPlug("use_roughness_map", false, &status);
			bool useRoughnessMap;
			useRoughnessMapPlug.getValue(useRoughnessMap);

			MPlug useEmissivityMapPlug = MFnDependencyNode(shaderNode).findPlug("use_emissivity_map", false, &status);
			bool useEmissivityMap;
			useEmissivityMapPlug.getValue(useEmissivityMap);

			//Find Stingray PBR Diffuse/Albedo Map
			MPlug colorPlug;
			if( useColorMap )
				colorPlug = MFnDependencyNode(shaderNode).findPlug("TEX_color_map", false, &status);
			else
				colorPlug = MFnDependencyNode(shaderNode).findPlug("base_color", false, &status);

			if (MS::kFailure == status) {
				MGlobal::displayInfo("MFnDependencyNode::findPlug");
				continue;
			}

			MItDependencyGraph color_itDG(colorPlug, MFn::kFileTexture,
				MItDependencyGraph::kUpstream,
				MItDependencyGraph::kBreadthFirst,
				MItDependencyGraph::kNodeLevel,
				&status);

			if (MS::kFailure == status) {
				MGlobal::displayInfo("MItDependencyGraph::MItDependencyGraph");
				continue;
			}

			//disable automatic pruning so that we can locate a specific plug 
			color_itDG.disablePruningOnFilter();

			//If no texture file node was found, pass in an empty string as the texture filename 
			//so that color information is outputted instead
			MString colorTextureName("");
			if (color_itDG.isDone()) {

				MGlobal::displayInfo("color_itDG.isDone()");
				//	return MStatus::kFailure;
				//otherwise retrieve the filename and pass it in to output texture information
			}
			else {
				MObject textureNode = color_itDG.currentItem();// thisNode();
				MPlug filenamePlug = MFnDependencyNode(textureNode).findPlug("fileTextureName", false, &status);
				filenamePlug.getValue(colorTextureName);
				//Display the uv set for the loop iteration for debugging
				MGlobal::displayInfo("Color Texture Name =  " + colorTextureName + "\n");
			}

			//Find Stingray PBR Normal Map
			if (useNormalMap)
			{
				MPlug normalPlug = MFnDependencyNode(shaderNode).findPlug("TEX_normal_map", false, &status);

				if (MS::kFailure == status) {
					MGlobal::displayInfo("MFnDependencyNode::findPlug");
					continue;
				}

				MItDependencyGraph normal_itDG(normalPlug, MFn::kFileTexture,
					MItDependencyGraph::kUpstream,
					MItDependencyGraph::kBreadthFirst,
					MItDependencyGraph::kNodeLevel,
					&status);

				if (MS::kFailure == status) {
					MGlobal::displayInfo("MItDependencyGraph::MItDependencyGraph");
					continue;
				}

				//disable automatic pruning so that we can locate a specific plug 
				normal_itDG.disablePruningOnFilter();

				//If no texture file node was found, pass in an empty string as the texture filename 
				//so that color information is outputted instead
				MString normalTextureName("");
				if (normal_itDG.isDone()) {

					MGlobal::displayInfo("normal_itDG.isDone()");
					//	return MStatus::kFailure;
					//otherwise retrieve the filename and pass it in to output texture information
				}
				else {
					MObject textureNode = normal_itDG.currentItem();// thisNode();
					MPlug filenamePlug = MFnDependencyNode(textureNode).findPlug("fileTextureName", false, &status);
					filenamePlug.getValue(normalTextureName);
					//Display the uv set for the loop iteration for debugging
					MGlobal::displayInfo("Normal Texture Name =  " + normalTextureName + "\n");
				}
			}

			//Find Stingray PBR Metallic Map
			if (useMetallicMap)
			{
				MPlug metallicPlug = MFnDependencyNode(shaderNode).findPlug("TEX_metallic_map", false, &status);

				if (MS::kFailure == status) {
					MGlobal::displayInfo("MFnDependencyNode::findPlug");
					continue;
				}

				MItDependencyGraph metallic_itDG(metallicPlug, MFn::kFileTexture,
					MItDependencyGraph::kUpstream,
					MItDependencyGraph::kBreadthFirst,
					MItDependencyGraph::kNodeLevel,
					&status);

				if (MS::kFailure == status) {
					MGlobal::displayInfo("MItDependencyGraph::MItDependencyGraph");
					continue;
				}

				//disable automatic pruning so that we can locate a specific plug 
				metallic_itDG.disablePruningOnFilter();

				//If no texture file node was found, pass in an empty string as the texture filename 
				//so that color information is outputted instead
				MString metallicTextureName("");
				if (metallic_itDG.isDone()) {

					MGlobal::displayInfo("metallic_itDG.isDone()");
					//	return MStatus::kFailure;
					//otherwise retrieve the filename and pass it in to output texture information
				}
				else {
					MObject textureNode = metallic_itDG.currentItem();// thisNode();
					MPlug filenamePlug = MFnDependencyNode(textureNode).findPlug("fileTextureName", false, &status);
					filenamePlug.getValue(metallicTextureName);
					//Display the uv set for the loop iteration for debugging
					MGlobal::displayInfo("Metallic Texture Name =  " + metallicTextureName + "\n");
				}
			}

			//Find Stingray PBR Roughness Map
			if (useRoughnessMap)
			{
				MPlug rougnessPlug = MFnDependencyNode(shaderNode).findPlug("TEX_roughness_map", false, &status);

				if (MS::kFailure == status) {
					MGlobal::displayInfo("MFnDependencyNode::findPlug");
					continue;
				}

				MItDependencyGraph roughness_itDG(rougnessPlug, MFn::kFileTexture,
					MItDependencyGraph::kUpstream,
					MItDependencyGraph::kBreadthFirst,
					MItDependencyGraph::kNodeLevel,
					&status);

				if (MS::kFailure == status) {
					MGlobal::displayInfo("MItDependencyGraph::MItDependencyGraph");
					continue;
				}

				//disable automatic pruning so that we can locate a specific plug 
				roughness_itDG.disablePruningOnFilter();

				//If no texture file node was found, pass in an empty string as the texture filename 
				//so that color information is outputted instead
				MString roughnessTextureName("");
				if (roughness_itDG.isDone()) {

					MGlobal::displayInfo("roughness_itDG.isDone()");
					//	return MStatus::kFailure;
					//otherwise retrieve the filename and pass it in to output texture information
				}
				else {
					MObject textureNode = roughness_itDG.currentItem();// thisNode();
					MPlug filenamePlug = MFnDependencyNode(textureNode).findPlug("fileTextureName", false, &status);
					filenamePlug.getValue(roughnessTextureName);
					//Display the uv set for the loop iteration for debugging
					MGlobal::displayInfo("Roughness Texture Name =  " + roughnessTextureName + "\n");
				}
			}

			//Find Stingray PBR Emissivity Map
			MPlug emissivityPlug;
			if (useEmissivityMap)
				emissivityPlug = MFnDependencyNode(shaderNode).findPlug("TEX_emissivity_map", false, &status);
			else
				emissivityPlug = MFnDependencyNode(shaderNode).findPlug("emissivity", false, &status);

			if (MS::kFailure == status) {
				MGlobal::displayInfo("MFnDependencyNode::findPlug");
				continue;
			}

			MItDependencyGraph emissivity_itDG(emissivityPlug, MFn::kFileTexture,
				MItDependencyGraph::kUpstream,
				MItDependencyGraph::kBreadthFirst,
				MItDependencyGraph::kNodeLevel,
				&status);

			if (MS::kFailure == status) {
				MGlobal::displayInfo("MItDependencyGraph::MItDependencyGraph");
				continue;
			}

			//disable automatic pruning so that we can locate a specific plug 
			emissivity_itDG.disablePruningOnFilter();

			//If no texture file node was found, pass in an empty string as the texture filename 
			//so that color information is outputted instead
			MString emissivityTextureName("");
			if (emissivity_itDG.isDone()) {

				MGlobal::displayInfo("emissivity_itDG.isDone()");
				//	return MStatus::kFailure;
				//otherwise retrieve the filename and pass it in to output texture information
			}
			else {
				MObject textureNode = emissivity_itDG.currentItem();// thisNode();
				MPlug filenamePlug = MFnDependencyNode(textureNode).findPlug("fileTextureName", false, &status);
				filenamePlug.getValue(emissivityTextureName);
				//Display the uv set for the loop iteration for debugging
				MGlobal::displayInfo("Emissivity Texture Name =  " + emissivityTextureName + "\n");
			}

		}
		else if (shaderNode.apiType() == MFn::kBlinn || shaderNode.apiType() == MFn::kPhong || shaderNode.apiType() == MFn::kPhongExplorer)
		{

			//Find BLINN / PHONG DIFFUSE MAP
			MPlug colorPlug = MFnDependencyNode(shaderNode).findPlug("color", false, &status);
			if (MS::kFailure == status) {
				MGlobal::displayError("MFnDependencyNode::findPlug");
				continue;
			}

			MItDependencyGraph color_itDG(colorPlug, MFn::kFileTexture,
				MItDependencyGraph::kUpstream,
				MItDependencyGraph::kBreadthFirst,
				MItDependencyGraph::kNodeLevel,
				&status);

			if (MS::kFailure == status) {
				MGlobal::displayError("MItDependencyGraph::MItDependencyGraph");
				continue;
			}

			//disable automatic pruning so that we can locate a specific plug 
			color_itDG.disablePruningOnFilter();

			//If no texture file node was found, pass in an empty string as the texture filename 
			//so that color information is outputted instead
			MString colorTextureName("");
			if (color_itDG.isDone()) {

				MGlobal::displayInfo("color_itDG.isDone()");
				//	return MStatus::kFailure;
				//otherwise retrieve the filename and pass it in to output texture information
			}
			else {
				MObject textureNode = color_itDG.currentItem();// thisNode();
				MPlug filenamePlug = MFnDependencyNode(textureNode).findPlug("fileTextureName", false, &status);
				filenamePlug.getValue(colorTextureName);
				//Display the uv set for the loop iteration for debugging
				MGlobal::displayInfo("Color Texture Name =  " + colorTextureName + "\n");
			}

			//Find BLINN/PHONG NORMAL MAP
			MGlobal::displayInfo("Searching for Normal Plug");
			MPlug normalPlug = MFnDependencyNode(shaderNode).findPlug("normalCamera", false, &status);
			if (MS::kFailure == status) {
				MGlobal::displayInfo("MFnDependencyNode::findPlug");
				continue;
			}

			MItDependencyGraph normal_itDG(normalPlug, MFn::kFileTexture,
				MItDependencyGraph::kUpstream,
				MItDependencyGraph::kBreadthFirst,
				MItDependencyGraph::kNodeLevel,
				&status);

			if (MS::kFailure == status) {
				MGlobal::displayInfo("MItDependencyGraph::MItDependencyGraph");
				continue;
			}

			//disable automatic pruning so that we can locate a specific plug 
			normal_itDG.disablePruningOnFilter();

			//If no texture file node was found, pass in an empty string as the texture filename 
			//so that color information is outputted instead
			MString normalTextureName("");
			if (normal_itDG.isDone()) {

				MGlobal::displayInfo("normal_itDG.isDone()");
				//	return MStatus::kFailure;
				normalPlug.getValue(normalTextureName);
				MGlobal::displayInfo("Normal Texture Name =  " + normalTextureName + "\n");
			}
			else {
				MObject textureNode = normal_itDG.currentItem();// thisNode();
				MPlug filenamePlug = MFnDependencyNode(textureNode).findPlug("fileTextureName", false, &status);
				filenamePlug.getValue(normalTextureName);
				//Display the uv set for the loop iteration for debugging
				MGlobal::displayInfo("Normal Texture Name =  " + normalTextureName + "\n");
			}

			//FIND BLINN/PHONG SPECULAR MAP
			MGlobal::displayInfo("Searching for Specular Plug");
			MPlug specularPlug = MFnDependencyNode(shaderNode).findPlug("reflectivity", false, &status);
			if (MS::kFailure == status) {
				MGlobal::displayInfo("MFnDependencyNode::findPlug");
				continue;
			}

			MItDependencyGraph spec_itDG(specularPlug, MFn::kFileTexture,
				MItDependencyGraph::kUpstream,
				MItDependencyGraph::kBreadthFirst,
				MItDependencyGraph::kNodeLevel,
				&status);

			if (MS::kFailure == status) {
				MGlobal::displayInfo("MItDependencyGraph::MItDependencyGraph");
				continue;
			}

			//disable automatic pruning so that we can locate a specific plug 
			spec_itDG.disablePruningOnFilter();

			//If no texture file node was found, pass in an empty string as the texture filename 
			//so that color information is outputted instead

			MString specTextureName("");
			if (spec_itDG.isDone()) {
				MGlobal::displayInfo("spec_itDG.isDone()");
				//	return MStatus::kFailure;
				//otherwise retrieve the filename and pass it in to output texture information
			}
			else {
				MObject textureNode = spec_itDG.currentItem();// thisNode();
				MPlug filenamePlug = MFnDependencyNode(textureNode).findPlug("fileTextureName", false, &status);
				filenamePlug.getValue(specTextureName);
				//Display the uv set for the loop iteration for debugging
				MGlobal::displayInfo("Specular Texture Name =  " + specTextureName + "\n");
			}

			
		}
		


	}
	*/

	MGlobal::displayInfo("writeGeometryToArchiveEnd");

	//TO DO:  set geom bounds
	//Alembic::Abc::Box3d cbox;
	//cbox.extendBy(Alembic::Abc::V3d(1.0, -1.0, 0.0));
	//cbox.extendBy(Alembic::Abc::V3d(-1.0, 1.0, 3.0));
	//meshSchema.getChildBoundsProperty().set(cbox);

	//TO DO:  set the vertex tuple info
	/*
	if (MStatus::kFailure == outputVertices(os)) {
		return MStatus::kFailure;
	}

	if (MStatus::kFailure == outputVertexInfo(os)) {
		return MStatus::kFailure;
	}

	if (MStatus::kFailure == outputNormals(os)) {
		return MStatus::kFailure;
	}

	if (MStatus::kFailure == outputTangents(os)) {
		return MStatus::kFailure;
	}

	if (MStatus::kFailure == outputBinormals(os)) {
		return MStatus::kFailure;
	}

	if (MStatus::kFailure == outputColors(os)) {
		return MStatus::kFailure;
	}

	if (MStatus::kFailure == outputUVs(os)) {
		return MStatus::kFailure;
	}

	if (MStatus::kFailure == outputSets(os)) {
		return MStatus::kFailure;
	}
	os << "\n\n";
		*/
	return MStatus::kSuccess;

}


MStatus polyAbcWriter::outputFaces(ostream& os) 
//Summary:	outputs the vertex indices that comprise each face
//Args   :	os - an output stream to write to
//Returns:	MStatus::kSuccess if all faces were outputted
//			MStatus::kFailure otherwise
{
	unsigned int faceCount = fMesh->numPolygons();
	if (0 == faceCount) {
		return MStatus::kFailure;
	}

	MStatus status;
	MIntArray indexArray;

	os << "Faces:  " << faceCount << "\n";
	os << HEADER_LINE;
	os << "Format:  Index|Vertex Indices\n";
	os << LINE;

	unsigned int i;
	for (i = 0; i < faceCount; i++) {
		os << i << DELIMITER;

		unsigned int indexCount = fMesh->polygonVertexCount(i, &status);
		if (MStatus::kFailure == status) {
			MGlobal::displayError("MFnMesh::polygonVertexCount");
			return MStatus::kFailure;
		}

		status = fMesh->getPolygonVertices (i, indexArray);
		if (MStatus::kFailure == status) {
			MGlobal::displayError("MFnMesh::getPolygonVertices");
			return MStatus::kFailure;
		}

		unsigned int j;
		for (j = 0; j < indexCount; j++) {
			os << indexArray[j] << " ";
		}

		os << "\n";
	}
	os << "\n\n";

	return MStatus::kSuccess;
}


MStatus polyAbcWriter::outputVertices(Alembic::AbcGeom::OPolyMesh & polyMeshObj) 
//Summary:	outputs all vertex coordinates
//Args   :	os - an output stream to write to
//Returns:	MStatus::kSuccess if all vertex coordinates were outputted
//			MStatus::kFailure otherwise
{
	unsigned int vertexCount = fVertexArray.length();
	unsigned i;
	if (0 == vertexCount) {
		return MStatus::kFailure;
	}
	for (i = 0; i < vertexCount; i++) {
		
	}
	return MStatus::kSuccess;
}


MStatus polyAbcWriter::outputVertexInfo(ostream& os) 
//Summary:	outputs the per face per vertex information such as normal, color, and uv set
//			indices
//Args   :	os - an output stream to write to
//Returns:	MStatus::kSuccess if all per face per vertex information was outputted
//			MStatus::kFailure otherwise
{
	unsigned int faceCount = fMesh->numPolygons();
	unsigned i, j, indexCount;

	MStatus status;
	MIntArray indexArray;

	//output the header
	os << "Vertex Info:\n";
	os << HEADER_LINE;
	os << "Format:  Face|faceVertexIndex|vertexIndex|normalIndex|colorIndex|";
	
	//Add each uv set to the header
	UVSet* currUVSet;
	for (currUVSet = fHeadUVSet; currUVSet != NULL; currUVSet = currUVSet->next) {
		os << "| UV_" << currUVSet->name;
	}
	os << "\n";

	os << LINE;

	MIntArray normalIndexArray;
	int colorIndex, uvID;

	for (i = 0; i < faceCount; i++) {

		indexCount = fMesh->polygonVertexCount(i, &status);
		if (MStatus::kFailure == status) {
			MGlobal::displayError("MFnMesh::polygonVertexCount");
			return MStatus::kFailure;
		}

		status = fMesh->getPolygonVertices (i, indexArray);
		if (MStatus::kFailure == status) {
			MGlobal::displayError("MFnMesh::getPolygonVertices");
			return MStatus::kFailure;
		}

		status = fMesh->getFaceNormalIds (i, normalIndexArray);
		if (MStatus::kFailure == status) {
			MGlobal::displayError("MFnMesh::getFaceNormalIds");
			return MStatus::kFailure;
		}

		for (j = 0; j < indexCount; j++) {
			status = fMesh->getFaceVertexColorIndex(i, j, colorIndex);

			//output the face, face vertex index, vertex index, normal index, color index
			//for the current vertex on the current face
			os << i << DELIMITER << j << DELIMITER << indexArray[j] << DELIMITER
			   << normalIndexArray[j] << DELIMITER << colorIndex << DELIMITER;

			//output each uv set index for the current vertex on the current face
			for (currUVSet = fHeadUVSet; currUVSet != NULL; currUVSet = currUVSet->next) {
				status = fMesh->getPolygonUVid(i, j, uvID, &currUVSet->name);
				if (MStatus::kFailure == status) {
					MGlobal::displayError("MFnMesh::getPolygonUVid");
					return MStatus::kFailure;
				}
				os << DELIMITER << uvID;
			}
			os << "\n";
		}

		os << "\n";
	}
	os << "\n";

	return MStatus::kSuccess;
}


MStatus polyAbcWriter::outputNormals(ostream& os) 
//Summary:	outputs the normals for this polygonal mesh
//Args   :	os - an output stream to write to
//Returns:	MStatus::kSuccess if all normals were outputted
//			MStatus::kFailure otherwise
{
	unsigned int normalCount = fNormalArray.length();
	if (0 == normalCount) {
		return MStatus::kFailure;
	}

	os << "Normals:  " << normalCount << "\n";
	os << HEADER_LINE;
	os << "Format:  Index|[x, y, z]\n";
	os << LINE;

	unsigned int i;
	for (i = 0; i < normalCount; i++) {
		os << i << DELIMITER << "["
		   << fNormalArray[i].x << ", "
		   << fNormalArray[i].y << ", "
		   << fNormalArray[i].z << "]\n";
	}
	os << "\n\n";

	return MStatus::kSuccess;
}

MStatus polyAbcWriter::outputTangents(ostream& os) 
//Summary:	outputs the normals for this polygonal mesh
//Args   :	os - an output stream to write to
//Returns:	MStatus::kSuccess if all normals were outputted
//			MStatus::kFailure otherwise
{
	unsigned int tangentCount = fTangentArray.length();
	if (0 == tangentCount) {
		return MStatus::kFailure;
	}

	os << "Tangents:  " << tangentCount << "\n";
	os << HEADER_LINE;
	os << "Format:  Index|[x, y, z]\n";
	os << LINE;

	unsigned int i;
	for (i = 0; i < tangentCount; i++) {
		os << i << DELIMITER << "["
		   << fTangentArray[i].x << ", "
		   << fTangentArray[i].y << ", "
		   << fTangentArray[i].z << "]\n";
	}
	os << "\n\n";

	return MStatus::kSuccess;
}

MStatus polyAbcWriter::outputBinormals(ostream& os) 
//Summary:	outputs the normals for this polygonal mesh
//Args   :	os - an output stream to write to
//Returns:	MStatus::kSuccess if all normals were outputted
//			MStatus::kFailure otherwise
{
	unsigned int binormalCount = fBinormalArray.length();
	if (0 == binormalCount) {
		return MStatus::kFailure;
	}

	os << "Binormals:  " << binormalCount << "\n";
	os << HEADER_LINE;
	os << "Format:  Index|[x, y, z]\n";
	os << LINE;

	unsigned int i;
	for (i = 0; i < binormalCount; i++) {
		os << i << DELIMITER << "["
		   << fBinormalArray[i].x << ", "
		   << fBinormalArray[i].y << ", "
		   << fBinormalArray[i].z << "]\n";
	}
	os << "\n\n";

	return MStatus::kSuccess;
}

MStatus polyAbcWriter::outputColors(ostream& os) 
//Summary:	outputs the colors for this polygonal mesh
//Args   :	os - an output stream to write to
//Returns:	MStatus::kSuccess if all colors were outputted
//			MStatus::kFailure otherwise
{
	unsigned int colorCount = fColorArray.length();
	if (0 == colorCount) {
		return MStatus::kFailure;
	}

	os << "Colors:  " << colorCount << "\n";
	os << HEADER_LINE;
	os << "Format:  Index|R G B A\n";
	os << LINE;
	
	unsigned int i;
	for (i = 0; i < colorCount; i++) {
		os << i << DELIMITER
		   << fColorArray[i].r << " "
		   << fColorArray[i].g << " "
		   << fColorArray[i].b << " "
		   << fColorArray[i].a << "\n";
	}
	os << "\n\n";

	return MStatus::kSuccess;
}


MStatus polyAbcWriter::outputUVs(ostream& os) 
//Summary:	for each UV Set, outputs all uv coordinates
//Args   :	os - an output stream to write to
//Returns:	MStatus::kSuccess if UV coordinates for all UV sets were outputted
//			MStatus::kFailure otherwise
{
	UVSet* currUVSet;
	unsigned int i, uvCount;
	for (currUVSet = fHeadUVSet; currUVSet != NULL; currUVSet = currUVSet->next) {
		if (currUVSet->name == fCurrentUVSetName) {
			os << "Current ";
		}

		os << "UV Set:  " << currUVSet->name << "\n";
		uvCount = currUVSet->uArray.length();
		os << "UV Count:  " << uvCount << "\n";
		os << HEADER_LINE;
		os << "Format:  Index|(u, v)\n";
		os << LINE;
		for (i = 0; i < uvCount; i++) {
			os << i << DELIMITER << "(" << currUVSet->uArray[i] << ", " << currUVSet->vArray[i] << ")\n";
		}
		os << "\n";
	}
	os << "\n";
	return MStatus::kSuccess;
}


MStatus polyAbcWriter::outputSingleSet(ostream& os, MString setName, MIntArray faces, MString textureName)
//Summary:	outputs this mesh's sets and each sets face components, and any 
//			associated texture
//Args   :	os - an output stream to write to
//Returns:	MStatus::kSuccess if set information was outputted
//			MStatus::kFailure otherwise
{
	unsigned int i;
	unsigned int faceCount = faces.length();

	os << "Set:  " << setName << "\n";
	os << HEADER_LINE;
	os << "Faces:  ";
	for (i = 0; i < faceCount; i++) { 
		os << faces[i] << " ";
	}
	os << "\n";
	if (textureName == "") {
		textureName = "none";
	} 
	os << "Texture File: " << textureName << "\n";
	os << "\n\n";
	return MStatus::kSuccess;
}
