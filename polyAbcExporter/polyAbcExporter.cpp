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

//polyAbcExporter.cpp
#include <maya/MFnPlugin.h>
#include <maya/MDagPath.h>
#include <maya/MIOStream.h>
#include <maya/MFStream.h>

#include <maya/MFnTransform.h>

//polyExporter.cpp
#include <maya/MItDag.h>
//#include <maya/MDagPath.h>
#include <maya/MItSelectionList.h>

//General Includes
//
//#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
//#include <maya/MIntArray.h>
//#include <maya/MFnSet.h>
//#include <maya/MItDependencyGraph.h>
//#include <maya/MDagPath.h>
//#include <maya/MFnMesh.h>
//#include <maya/MPlug.h>
//#include <maya/MIOStream.h>
//#include <time.h>


#include "polyAbcExporter.h"
#include "polyAbcWriter.h"

#include "AbcExportInterface.h"

/*
polyAbcExporter::polyAbcExporter()
{
	MGlobal::displayInfo("polyAbcExporter::Constructor");
}
*/

polyAbcExporter::~polyAbcExporter() 
{ 
//Summary:  destructor method; does nothing
//
}

     
void* polyAbcExporter::creator() 
//Summary:  allows Maya to allocate an instance of this object
{
	return new polyAbcExporter();
}


MString polyAbcExporter::defaultExtension () const 
//Summary:	called when Maya needs to know the preferred extension of this file
//			format.  For example, if the user tries to save a file called 
//			"test" using the Save As dialog, Maya will call this method and 
//			actually save it as "test.x3d". Note that the period should *not* 
//			be included in the extension.
//Returns:  "raw"
{
	return MString("abc");
}


MStatus initializePlugin(MObject obj)
//Summary:	registers the commands, tools, devices, and so on, defined by the 
//			plug-in with Maya
//Returns:	MStatus::kSuccess if the registration was successful;
//			MStatus::kFailure otherwise
{
	MStatus status;
	MFnPlugin plugin(obj, "3rdGen Multimedia", "4.5", "Any");

	// Register the translator with the system
	//
	status =  plugin.registerFileTranslator("ABCexport",
											"",
											polyAbcExporter::creator,
											"",
											"option1=1",
											true);
	if (!status) {
		status.perror("registerFileTranslator");
		return status;
	}

	MGlobal::displayInfo("polyAbcExporter::InitializePlugin");
	printf("polyAbcExporter::InitializePlugin\n");

	return status;
}


MStatus uninitializePlugin(MObject obj) 
//Summary:	deregisters the commands, tools, devices, and so on, defined by the 
//			plug-in
//Returns:	MStatus::kSuccess if the deregistration was successful;
//			MStatus::kFailure otherwise
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status =  plugin.deregisterFileTranslator("ABCexport");
	if (!status) {
		status.perror("deregisterFileTranslator");
		return status;
	}


	MGlobal::displayInfo("polyAbcExporter::UninitializePlugin");
	printf("polyAbcExporter::UninitializePlugin\n");

	return status;
}


void polyAbcExporter::writeHeader(Alembic::Abc::OArchive &archive) 
//Summary:	outputs legend information before the main data
//Args   :	os - an output stream to write to
{
	/*
	os << "Legend:\n"
	   << "Delimiter = TAB\n"
	   << "() = coordinates\n"
	   << "[] = vector\n"
	   << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";
	*/

	MGlobal::displayInfo("polyAbcExporter::writeHeader");
	printf("polyAbcExporter::writeHeader\n");

	//TO DO:  Check for geometry, because if it isn't present virtual implementation of polyExporter::writeToFile won't get called
}

MStatus polyAbcExporter::writer(const MFileObject& file,
	const MString& /*options*/,
	MPxFileTranslator::FileAccessMode mode)
	//Summary:	saves a file of a type supported by this translator by traversing
	//			the all or selected objects (depending on mode) in the current
	//			Maya scene, and writing a representation to the given file
	//Args   :	file - object containing the pathname of the file to be written to
	//			options - a string representation of any file options 
	//			mode - the method used to write the file - export, or export active
	//				   are valid values; method will fail for any other values 
	//Returns:	MStatus::kSuccess if the export was successful;
	//			MStatus::kFailure otherwise
{

	MGlobal::displayInfo("polyAbcExporter::writer");

	const MString fileName = file.expandedFullName();
	
	std::string appWriter = "3rdGen Multimedia Maya ABC Export Plugin";
	//Intialize an Alembic output archive C++ object
	//Note 1:  This will create the export file using mmap and ostream internal to Alembic
	//Note 2:  Alembic Objects must go out of scope to clean themselves up.  Writes do not occur until the object is destructed.  (i.e. The archive cannot be global or it will never go out of scope)
	Alembic::Abc::OArchive archive = Alembic::Abc::CreateArchiveWithInfo(Alembic::AbcCoreOgawa::WriteArchive(), fileName.asUTF8(), appWriter, "", Alembic::Abc::MetaData() );
	Alembic::Abc::OObject topObject = archive.getTop();// Alembic::Abc::OObject(g_archive, Alembic::Abc::kTop)

	//We are using the Alembic API to open the file stream for us
	//So we leave this here to demonstrate that we don't need to do this explicitly
	/*
	ofstream newFile(fileName.asChar(), ios::out);
	if (!newFile) {
		MGlobal::displayError(fileName + ": could not be opened for reading");
		return MS::kFailure;
	}
	newFile.setf(ios::unitbuf);
	*/

	//AbcCoreOgawa will take care of writing the Alembic Ogawa header for us using an Ogawa::OArchive
	//Note that the header is incomplete until the archive goes out of scope (because it is not known a priori where the top object will be located in the file)
	//writeHeader(g_archive.);

	//check which objects are to be exported, and invoke the corresponding
	//methods; only 'export all' and 'export selection' are allowed
	if (MPxFileTranslator::kExportAccessMode == mode) {
		if (MStatus::kFailure == exportAll(topObject)) {
			return MStatus::kFailure;
		}
	}
	else if (MPxFileTranslator::kExportActiveAccessMode == mode) {
		if (MStatus::kFailure == exportSelection(topObject)) {
			return MStatus::kFailure;
		}
	}
	else {
		return MStatus::kFailure;
	}
	
	//There is no "Footer" data for Alembic files
	//Closest equivalent are the top level indexed metadata Ogawa Nodes, but again
	//OArchive will take care of writing this stuff for us
	//writeFooter(g_archive);

	//Again, Alembic Objects clean themselves up when they go out of scope
	//Same goes for the internal file mapping and ostream
	//newFile.flush();
	//newFile.close();

	MGlobal::displayInfo("Export to " + fileName + " successful!");
	return MS::kSuccess;
}

Alembic::AbcGeom::OXform polyAbcExporter::processMeshXform(const MDagPath meshDagPath, Alembic::Abc::OObject &topObject)
{
	MStatus status;

	MDagPath * xformDagPath = new MDagPath(meshDagPath);
	xformDagPath->pop();
	MFnTransform * transform = new MFnTransform(*xformDagPath, &status);

	MGlobal::displayInfo("Populating Xform Object: " + xformDagPath->partialPathName());
	Alembic::AbcGeom::OXform xformObj(topObject, xformDagPath->partialPathName().asUTF8());

	MVector translate = transform->getTranslation(MSpace::kWorld, &status);
	double scale[3];
	transform->getScale(scale);
	double rotation[3];
	MTransformationMatrix::RotationOrder rotOrder;
	status = transform->getRotation(rotation, rotOrder);

	MString translateStr = MString("Translation: ") + translate.x + " " + translate.y + " " + translate.z;
	MGlobal::displayInfo(translateStr);
	MString scaleStr = MString("Scale: ") + scale[0] + " " + scale[1] + " " + scale[2];
	MGlobal::displayInfo(scaleStr);
	MString rotStr = MString("Rotation: ") + rotation[0] + " " + rotation[1] + " " + rotation[2];
	MGlobal::displayInfo(rotStr);

	Alembic::AbcGeom::XformOp transop(Alembic::AbcGeom::kTranslateOperation, Alembic::AbcGeom::kTranslateHint);
	Alembic::AbcGeom::XformOp scaleop(Alembic::AbcGeom::kScaleOperation, Alembic::AbcGeom::kScaleHint);
	Alembic::AbcGeom::XformOp rotateXOp(Alembic::AbcGeom::kRotateXOperation, Alembic::AbcGeom::kRotateHint);
	Alembic::AbcGeom::XformOp rotateYOp(Alembic::AbcGeom::kRotateYOperation, Alembic::AbcGeom::kRotateHint);
	Alembic::AbcGeom::XformOp rotateZOp(Alembic::AbcGeom::kRotateZOperation, Alembic::AbcGeom::kRotateHint);
	//Alembic::AbcGeom::XformOp matrixop(Alembic::AbcGeom::kMatrixOperation, Alembic::AbcGeom::kMatrixHint);

	//TO DO: figure out the bounding box associated with all geometry that this transform affects
	//Alembic::AbcGeom::OBox3dProperty childBounds = xformObj.getSchema().getChildBoundsProperty();
	//childBounds.set(Alembic::Abc::Box3d(Alembic::Abc::V3d(, , , ), Alembic::Abc::V3d(, , , )));

	Alembic::AbcGeom::XformSample xformSamp;

	//TO DO:  Figure out how to check if translate is zero, so we don't write the op in that case
	xformSamp.addOp(transop, Alembic::Abc::V3d(translate.x, translate.y, translate.z));
	//TO DO:  Figure out how to check if scale is identity, so we don't write the op in that case
	xformSamp.addOp(scaleop, Alembic::Abc::V3d(scale[0], scale[1], scale[2]));

	//Note: Maya's rotation order should be interpreted in reverse
	//So we order rotation operations similarly when writing to alembic
	//TO DO:  Figure out how to easily determine if there is a zero rotation, so we don't write the ops
	assert(rotOrder != MTransformationMatrix::RotationOrder::kInvalid && rotOrder != MTransformationMatrix::RotationOrder::kLast);
	if (rotOrder == MTransformationMatrix::RotationOrder::kXYZ)
	{
		xformSamp.addOp(rotateZOp, rotation[2]);
		xformSamp.addOp(rotateYOp, rotation[1]);
		xformSamp.addOp(rotateXOp, rotation[0]);
	}
	else if (rotOrder == MTransformationMatrix::RotationOrder::kYZX)
	{
		xformSamp.addOp(rotateXOp, rotation[2]);
		xformSamp.addOp(rotateZOp, rotation[1]);
		xformSamp.addOp(rotateYOp, rotation[0]);
	}
	else if (rotOrder == MTransformationMatrix::RotationOrder::kZXY)
	{
		xformSamp.addOp(rotateYOp, rotation[2]);
		xformSamp.addOp(rotateXOp, rotation[1]);
		xformSamp.addOp(rotateZOp, rotation[0]);
	}
	else if (rotOrder == MTransformationMatrix::RotationOrder::kXZY)
	{
		xformSamp.addOp(rotateYOp, rotation[2]);
		xformSamp.addOp(rotateZOp, rotation[1]);
		xformSamp.addOp(rotateXOp, rotation[0]);
	}
	else if (rotOrder == MTransformationMatrix::RotationOrder::kYXZ)
	{
		xformSamp.addOp(rotateZOp, rotation[2]);
		xformSamp.addOp(rotateXOp, rotation[1]);
		xformSamp.addOp(rotateYOp, rotation[0]);
	}
	else if (rotOrder == MTransformationMatrix::RotationOrder::kZYX)
	{
		xformSamp.addOp(rotateXOp, rotation[2]);
		xformSamp.addOp(rotateYOp, rotation[1]);
		xformSamp.addOp(rotateZOp, rotation[0]);
	}

	xformSamp.setInheritsXforms(true);
	xformObj.getSchema().set(xformSamp);

	delete xformDagPath;

	return xformObj;
}

MStatus polyAbcExporter::processPolyMesh(const MDagPath dagPath, Alembic::Abc::OObject &topObject)
{
	//Summary:	processes the mesh on the given dag path by extracting its geometry
	//			and writing this data to file
	//Args   :	dagPath - the current dag path whose poly mesh is to be processed
	//			os - an output stream to write to
	//Returns:	MStatus::kSuccess if the polygonal mesh data was processed fully;
	//			MStatus::kFailure otherwise

		MStatus status;

		Alembic::AbcGeom::OXform meshXform = processMeshXform(dagPath, topObject);

		//Now Write the geometry child of this node

		polyAbcWriter* pWriter = (polyAbcWriter*)createPolyWriter(dagPath, status);
		if (MStatus::kFailure == status) {
			delete pWriter;
			return MStatus::kFailure;
		}
		if (MStatus::kFailure == pWriter->extractGeometry()) {
			delete pWriter;
			return MStatus::kFailure;
		}
		if (MStatus::kFailure == pWriter->writeGeometryToArchive(meshXform)) {
			delete pWriter;
			return MStatus::kFailure;
		}
		delete pWriter;


		return MStatus::kSuccess;
}


MStatus polyAbcExporter::exportAll(Alembic::Abc::OObject &topObject)
//Summary:	finds and outputs all polygonal meshes in the DAG
//Args   :	os - an output stream to write to
//Returns:  MStatus::kSuccess if the method succeeds
//			MStatus::kFailure if the method fails
{
	MStatus status;

	//create an iterator for only the mesh components of the DAG
	//
	MItDag itDag(MItDag::kDepthFirst, MFn::kMesh, &status);

	if (MStatus::kFailure == status) {
		MGlobal::displayError("MItDag::MItDag");
		return MStatus::kFailure;
	}

	for (; !itDag.isDone(); itDag.next()) {
		//get the current DAG path
		//
		MDagPath dagPath;
		if (MStatus::kFailure == itDag.getPath(dagPath)) {
			MGlobal::displayError("MDagPath::getPath");
			return MStatus::kFailure;
		}

		MFnDagNode visTester(dagPath);

		//if this node is visible, then process the poly mesh it represents
		//
		if (isVisible(visTester, status) && MStatus::kSuccess == status) {
			if (MStatus::kFailure == processPolyMesh(dagPath, topObject)) {
				return MStatus::kFailure;
			}
		}
	}
	return MStatus::kSuccess;
}


MStatus polyAbcExporter::exportSelection(Alembic::Abc::OObject &topObject)
//Summary:	finds and outputs all selected polygonal meshes in the DAG
//Args   :	os - an output stream to write to
//Returns:  MStatus::kSuccess if the method succeeds
//			MStatus::kFailure if the method fails
{
	MStatus status;

	//create an iterator for the selected mesh components of the DAG
	//
	MSelectionList selectionList;
	if (MStatus::kFailure == MGlobal::getActiveSelectionList(selectionList)) {
		MGlobal::displayError("MGlobal::getActiveSelectionList");
		return MStatus::kFailure;
	}

	MItSelectionList itSelectionList(selectionList, MFn::kMesh, &status);
	if (MStatus::kFailure == status) {
		return MStatus::kFailure;
	}

	for (itSelectionList.reset(); !itSelectionList.isDone(); itSelectionList.next()) {
		MDagPath dagPath;

		//get the current dag path and process the poly mesh on it
		//
		if (MStatus::kFailure == itSelectionList.getDagPath(dagPath)) {
			MGlobal::displayError("MItSelectionList::getDagPath");
			return MStatus::kFailure;
		}

		if (MStatus::kFailure == processPolyMesh(dagPath, topObject)) {
			return MStatus::kFailure;
		}
	}
	return MStatus::kSuccess;
}


polyWriter* polyAbcExporter::createPolyWriter(const MDagPath dagPath, MStatus& status) 
//Summary:	creates a polyWriter for the abc export file type
//Args   :	dagPath - the current polygon dag path
//			status - will be set to MStatus::kSuccess if the polyWriter was 
//					 created successfully;  MStatus::kFailure otherwise
//Returns:	pointer to the new polyWriter object
{
	return new polyAbcWriter(dagPath, status);
}
