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

#ifndef __POLYWRITER_H
#define __POLYWRITER_H

// polyWriter.h

// *****************************************************************************
//
// CLASS:    polyWriter
//
// *****************************************************************************
//
// CLASS DESCRIPTION (polyWriter)
// 
// polyWriter is a class used for creating polygonal mesh exporter plugins.  Its
// purpose is to output polygonal mesh data in the format required.  
// 
// To use this class, derive a new class and begin by adding the following *.h 
// files:
//    #include <maya/MFStream.h>
//
// The following functions must be implemented:
// constructor - which takes in MDagPath and MStatus object addresses
// destructor - which destroys any objects created in the constructor
// writeToFile() - which performs the actual data export
// outputSingleSet() - which performs the export of a particular polygonal set 
//					   on the mesh
//
// The extractGeometry() function may be overridden to extract more data that
// it is doing currently, but be sure to call this class' extractGeometry() 
// method as its first operation so that essential data is extracted.
//
// It is recommended that smaller helper functions are added to any derived
// classes, to export and format specific data about the mesh.  
//
// Once the derived class has been defined, create and return a new object of 
// this type in the createPolyWriter() function that must be defined in any
// class derived from the polyExporter class.
//
// For examples, see the classes polyRawWriter and polyX3DWriter
//
// *****************************************************************************

#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatArray.h>

#include <vector>

class polyWriter {

	public:
							polyWriter (MDagPath dagPath, MStatus& status);
		virtual				~polyWriter ();
		virtual MStatus		extractGeometry ();
		virtual MStatus		writeToFile (ostream & os) = 0;

	protected:
		//Methods
		//
				MObject		findShader (const MObject& setNode);
		virtual MStatus		outputSets (ostream& os);
		virtual	MStatus		outputSingleSet (ostream& os, 
											 MString setName, 
											 MIntArray faces, 
											 MString textureName) = 0;
		static	void		outputTabs (ostream & os, unsigned int tabCount);
	
		//Data Members
		//

		//the current UV set's name
		//
		MString				fCurrentUVSetName;

		//for storing general mesh information
		//
		MPointArray			fVertexArray;
		MColorArray			fColorArray;
		MFloatVectorArray	fNormalArray;
		MFloatVectorArray	fTangentArray;
		MFloatVectorArray	fBinormalArray;

		//for storing DAG objects
		//
		std::vector<MFnTransform*>	fTransformArray;
		std::vector<MFnMesh*>		fMeshArray;

		MFnMesh*			fMesh;
		MDagPath*			fDagPath;
		MObjectArray		fPolygonSets;
		MObjectArray		fPolygonComponents;
};

#endif /*__POLYWRITER_H*/
