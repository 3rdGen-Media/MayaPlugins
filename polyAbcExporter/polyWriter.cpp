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

//polyWriter.cpp

//General Includes
//
#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MFnSet.h>
#include <maya/MDagPath.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MPlug.h>

//Iterator Includes
//
#include <maya/MItMeshPolygon.h>

//Header File
//
#include "polyWriter.h"


polyWriter::polyWriter(MDagPath dagPath, MStatus& status)
//Summary:	Constructor - creates the MDagPath and MFnMesh objects necessary
//			for extracting the data
//Args   :	dagPath - the dagPath where the mesh is located=
//			status - will be set to MStatus::kSuccess if the creation of the 
//			MFnMesh is successful; MStatus::kFailure otherwise
{
	//initial dagPath
	fDagPath = new MDagPath(dagPath);
	fMesh = new MFnMesh(*fDagPath, &status);
	MGlobal::displayInfo("polyWriter::initialDagPath->fullPathName = " + fDagPath->fullPathName(NULL));

	//find top dag path for export all
	//MDagPath * topDagPath = new MDagPath(dagPath);
	//while (topDagPath->pop() == MStatus::kSuccess);

	/*
	//build array of MFnMesh objects, depending on 'All' vs 'Selection' option
	if (this->fileAccessMode() != kExportActiveAccessMode)
	{

	}
	*/

	//fMeshArray.push_back(fMesh);
	
	/*
	MDagPath * parentDagPath = new MDagPath(dagPath);
	parentDagPath->pop();
	MGlobal::displayInfo("polyWriter::pDagPath->fullPathName = " + parentDagPath->fullPathName(NULL));

	parentDagPath->pop();

	MGlobal::displayInfo("polyWriter::pDagPath->fullPathName = " + parentDagPath->fullPathName(NULL));
	*/
	//MFnArray
}


polyWriter::~polyWriter()
//Summary:	Destructor - deletes all constructor created objects
{
	if (NULL != fDagPath) delete fDagPath;
	if (fMeshArray.size() > 0)
	{
		for (int meshIndex = 0; meshIndex < fMeshArray.size(); meshIndex++)
			delete fMeshArray[meshIndex];
		//fMeshArray = NULL;
		fMeshArray.clear();
	}

	if (NULL != fMesh) delete fMesh;
}

MStatus polyWriter::extractGeometry() 
//Summary:	extracts the main geometry (vertices, vertex colours, vertex normals) 
//			of this polygonal mesh.  
//Returns:  MStatus::kSuccess if the method succeeds
//			MStatus::kFailure if the method fails
{

	if (MStatus::kFailure == fMesh->getPoints(fVertexArray, MSpace::kWorld)) {
		MGlobal::displayError("MFnMesh::getPoints"); 
		return MStatus::kFailure;
	}

	/*
	if (MStatus::kFailure == fMesh->getFaceVertexColors(fColorArray)) {
		MGlobal::displayError("MFnMesh::getFaceVertexColors"); 
		return MStatus::kFailure;
	}
	*/

	if (MStatus::kFailure == fMesh->getNormals(fNormalArray, MSpace::kWorld)) {
		MGlobal::displayError("MFnMesh::getNormals"); 
		return MStatus::kFailure;
	}
	if (MStatus::kFailure == fMesh->getCurrentUVSetName(fCurrentUVSetName)) {
		MGlobal::displayError("MFnMesh::getCurrentUVSetName"); 
		return MStatus::kFailure;
	}

	if (MStatus::kFailure == fMesh->getTangents(fTangentArray, MSpace::kWorld, &fCurrentUVSetName)) {
		MGlobal::displayError("MFnMesh::getTangents"); 
		return MStatus::kFailure;
	}

	if (MStatus::kFailure == fMesh->getBinormals(fBinormalArray, MSpace::kWorld, &fCurrentUVSetName)) {
		MGlobal::displayError("MFnMesh::getBinormals"); 
		return MStatus::kFailure;
	}

	//Have to make the path include the shape below it so that
	//we can determine if the underlying shape node is instanced.
	//By default, dag paths only include transform nodes.
	//
	fDagPath->extendToShape();

	//If the shape is instanced then we need to determine which
	//instance this path refers to.
	//
	int instanceNum = 0;
	if (fDagPath->isInstanced())
		instanceNum = fDagPath->instanceNumber();

	//Get the connected sets and members - these will be used to determine texturing of different
	//faces
	//
	if (!fMesh->getConnectedSetsAndMembers(instanceNum, fPolygonSets, fPolygonComponents, true)) {
		MGlobal::displayError("MFnMesh::getConnectedSetsAndMembers"); 
		return MStatus::kFailure;
	}

	return MStatus::kSuccess;
}


void polyWriter::outputTabs(ostream& os, unsigned int tabCount) 
//Summary:	outputs tab spacing
//Args   :	os - an output stream to write to
//			tabCount - the number of tabs to print
//Returns:  MStatus::kSuccess if the method succeeds
//			MStatus::kFailure if the method fails
{
	unsigned int i;
	for (i = 0; i < tabCount; i++) {
		os << "\t";
	}
}



MObject polyWriter::findShader(const MObject& setNode) 
//Summary:	finds the shading node for the given shading group set node
//Args   :	setNode - the shading group set node
//Returns:  the shader node for setNode if found;
//			MObject::kNullObj otherwise
{
	MStatus status;
	MFnDependencyNode fnNode(setNode);
	MPlug shaderPlug = fnNode.findPlug("surfaceShader", false, &status);
			
	if (!shaderPlug.isNull()) {			
		MPlugArray connectedPlugs;

		//get all the plugs that are connected as the destination of this 
		//surfaceShader plug so we can find the surface shaderNode
		//
		MStatus status;
		shaderPlug.connectedTo(connectedPlugs, true, false, &status);
		if (MStatus::kFailure == status) {
			MGlobal::displayError("MPlug::connectedTo");
			return MObject::kNullObj;
		}

		if (1 != connectedPlugs.length()) {
			MGlobal::displayError("Error getting shader for: " + fMesh->partialPathName());
		} else {
			return connectedPlugs[0].node();
		}
	}
	
	return MObject::kNullObj;
}




MStatus polyWriter::outputSets(ostream& os)
//Summary:	outputs this mesh's sets and each sets face components, and any 
//			associated texture
//Args   :	os - an output stream to write to
//Returns:	MStatus::kSuccess if set information was outputted
//			MStatus::kFailure otherwise
{
	MStatus status;

	//if there is more than one set, the last set simply consists of all 
	//polygons, so we won't include it
	//
	unsigned int setCount = fPolygonSets.length();
	if (setCount > 1) {
		setCount--;
	}

	MIntArray faces;

	unsigned int i;
	for (i = 0; i < setCount; i++ ) {
		MObject set = fPolygonSets[i];
		MObject comp = fPolygonComponents[i];
		MFnSet fnSet(set, &status);
		if (MS::kFailure == status) {
			MGlobal::displayError("MFnSet::MFnSet");
            continue;
        }

        //Make sure the set is a polygonal set.  If not, continue.
		MItMeshPolygon itMeshPolygon(*fDagPath, comp, &status);

		if ((MS::kFailure == status)) {
			MGlobal::displayError("MItMeshPolygon::MItMeshPolygon");
            continue;
		}

		//add the current set's face indices to the faces array
		//
		faces.setLength(itMeshPolygon.count());

		unsigned int j = 0;
		for (itMeshPolygon.reset(); !itMeshPolygon.isDone(); itMeshPolygon.next()) {
			faces[j++] = itMeshPolygon.index();
		}

		//Find the texture that is applied to this set.  First, get the
		//shading node connected to the set.  Then, if there is an input
		//attribute called "color", search upstream from it for a texture
		//file node.
		MObject shaderNode = findShader(set);
		if (MObject::kNullObj == shaderNode) {
			continue;
		}

		MPlug colorPlug = MFnDependencyNode(shaderNode).findPlug("color", false, &status);
		if (MS::kFailure == status) {
			MGlobal::displayError("MFnDependencyNode::findPlug");
			continue;
		}

		MItDependencyGraph itDG(colorPlug, MFn::kFileTexture,
								MItDependencyGraph::kUpstream, 
								MItDependencyGraph::kBreadthFirst,
								MItDependencyGraph::kNodeLevel, 
								&status);

		if (MS::kFailure == status) {
			MGlobal::displayError("MItDependencyGraph::MItDependencyGraph");
			continue;
		}

		//disable automatic pruning so that we can locate a specific plug 
		itDG.disablePruningOnFilter();

		//If no texture file node was found, pass in an empty string as the texture filename 
		//so that color information is outputted instead
		//
		MString textureName("");
		if (itDG.isDone()) {
			if (MStatus::kFailure == outputSingleSet(os, MString(fnSet.name()), faces, textureName)) {
				return MStatus::kFailure;
			}
		//otherwise retrieve the filename and pass it in to output texture information
		//
		} else {
			MObject textureNode = itDG.currentItem();// thisNode();
			MPlug filenamePlug = MFnDependencyNode(textureNode).findPlug("fileTextureName", false, &status);
			filenamePlug.getValue(textureName);
			if (MStatus::kFailure == outputSingleSet(os, MString(fnSet.name()), faces, textureName)) {
				return MStatus::kFailure;
			}

		}
	}
	return MStatus::kSuccess;
}
