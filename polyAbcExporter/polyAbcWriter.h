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

#ifndef __POLYABCWRITER_H
#define __POLYABCWRITER_H

// polyAbcWriter.h

//
// *****************************************************************************
//
// CLASS:    polyAbcWriter
//
// *****************************************************************************
//
// CLASS DESCRIPTION (polyAbcWriter)
// 
// polyAbcWriter is a class derived from polyWriter.  It currently outputs in
// raw text format the following polygonal mesh data:
// - faces and their vertex components
// - vertex coordinates
// - colors per vertex
// - normals per vertex
// - current uv set and coordinates
// - component sets
// - file textures (for the current uv set)
// - other uv sets and coordinates
//
// *****************************************************************************

#include "polyWriter.h"
#include "AbcExportInterface.h"
#include <unordered_set>

#include <maya/MString.h>
#include <maya/MObjectArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MDagPathArray.h>


#include <maya/MItDependencyNodes.h>
#include <maya/MItGeometry.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MFnSingleIndexedComponent.h>
//#include <maya/MItMeshVertex.h>
//#include <maya/MItSurfaceCV.h>
//#include <maya/MItCurveCV.h>
//Used to store UV set information
//


struct UVSet {
	MFloatArray	uArray;
	MFloatArray	vArray;
	MString		name;
	UVSet*		next;
};

class polyAbcWriter : public polyWriter {

	public:
				polyAbcWriter (const MDagPath& dagPath, MStatus& status);
				~polyAbcWriter () override;
				MStatus extractGeometry () override;
				MStatus extractSkinningData(std::map<std::string, std::map<std::string, int>> &armatures);
				void mapArmatureJoint(Alembic::Abc::OObject &jointObj, std::map<std::string, int> &jointIndexMap, int jointIndex);

				MStatus writeToFile (ostream& os) override; //not gonna use this, but compiler needs it
				MStatus writeGeometryToArchive(Alembic::AbcGeom::OXform &xformObj, Alembic::Abc::OObject &mtlObject);

				friend bool operator< (const MObject &left, const MObject &right){ return true;}
	private:
		//Functions
		//
				MStatus	outputSingleSet (ostream& os, 
										 MString setName, 
										 MIntArray faces, 
										 MString textureName) override;
				MStatus outputFaces (ostream& os);
				MStatus outputVertices (Alembic::AbcGeom::OPolyMesh &polyMeshObj);
				MStatus	outputVertexInfo (ostream& os);
				MStatus	outputNormals (ostream& os);
				MStatus	outputTangents (ostream& os);
				MStatus	outputBinormals (ostream& os);
				MStatus	outputColors (ostream& os);
				MStatus	outputUVs (ostream& os);

				//bool polyAbcWriter::isSkinClusterIncluded(MObject &node);
				void populateInfluenceIndexArray(MFnSkinCluster &skinClusterFn, MIntArray &influenceIndexArray);

		//Data Member
		//for storing UV information
		//
		UVSet*	fHeadUVSet;


		MFnSkinCluster * fSkinCluster;


		MIntArray		fNumWeightsPerVertexArray;
		MIntArray		fVertexWeightsInfluenceArray;
		MFloatArray     fVertexWeightsArray;


		MDagPathArray   fInfluenceArray;
		MIntArray		influenceIndexArray;
		MObjectArray    fskinClusterArray;
		MDoubleArray    fWeightArray;
		MStringArray    fGeometryArray;
		bool            fAssignAllToSingle;

		// data structures for undo
		//MDagPathArray   fDagPathArray;
		//MObjectArray    fComponentArray;
		//MIntArray      *fInfluenceIndexArrayPtrArray;
		//MDoubleArray   *fWeightsPtrArray;
};

#endif /*__POLYABCWRITER_H*/
