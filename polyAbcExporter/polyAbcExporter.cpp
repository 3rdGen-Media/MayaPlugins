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


#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MFnSet.h>
#include <maya/MDagPath.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MPlug.h>
#include <maya/MFnAttribute.h>

#include <maya/MPlugArray.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MMatrix.h>
#include <maya/MFnMatrixData.h>
//#include <maya/MFnDependencyNode.h>
//#include <maya/MFnAttribute.h>

#include <maya/MIOStream.h>

//#include <maya/MIOStream.h>
//#include <time.h>



//Determining the time range
//The MAnimControl class can be used to find the time range for the animation.This then simply outputs the start and end time for the animation in frames(25fps).
#include<maya/MAnimControl.h>
#include<maya/MTime.h>
#include<maya/MAnimUtil.h>

#include "polyAbcExporter.h"
#include "polyAbcWriter.h"

#include "AbcExportInterface.h"

// *****************************************************************************

// HELPER METHODS

// *****************************************************************************

#ifndef min
static inline double
min(double a, double b)
{
	return (a < b ? a : b);
}
#endif

#ifndef max
static inline double
max(double a, double b)
{
	return (a > b ? a : b);
}
#endif


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
	Alembic::AbcMaterial::OMaterial mtlObject(topObject, "materials"); // TO DO:  consider making this a collection?
	Alembic::AbcCollection::OArmatureCollection armatureObject(topObject, "armatures");

	//Initialize a hash to store a unique list hypershade objects used by all our processed meshes
	//std::unordered_set<MObject*> hypershadeMaterials;

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
		MGlobal::displayInfo("polyAbcExporter::ExportAll");
		if (MStatus::kFailure == exportAll(topObject, mtlObject, armatureObject)) {
			return MStatus::kFailure;
		}
	}
	else if (MPxFileTranslator::kExportActiveAccessMode == mode) {
		MGlobal::displayInfo("polyAbcExporter::ExportSelection");
		if (MStatus::kFailure == exportSelection(topObject, mtlObject, armatureObject)) {
			return MStatus::kFailure;
		}
	}
	else {
		return MStatus::kFailure;
	}

	/*
	char buf[256];
	_itoa_s(hypershadeMaterials.size(), buf, 10);
	MString mtlSetSize(buf);
	MGlobal::displayInfo("Hypershade Material Set Size:  " + mtlSetSize + "\n");

	for (auto it = hypershadeMaterials.begin(); it != hypershadeMaterials.end(); ++it)
	{

		MObject * shaderNode = *it;
		MFnDependencyNode shaderDependencyNode(*shaderNode);
		//MGlobal::displayInfo("shaderNode.apiType:  " + apiTypeStr + "\n");
		MGlobal::displayInfo("Hypershade Node.name:  " + shaderDependencyNode.name() + "\n");

		//We will store per mtl mesh face groupings as an ArbGeomParam,
		//While we will define arbitrary face set partitions groupings using AbcGeom OFaceSet
		//std::string mtlName = std::string(shaderDependencyNode.name().asUTF8());
	}
	*/

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


void polyAbcExporter::writeDagPoseInfo(MObject& dagPoseNode, unsigned index, Alembic::AbcGeom::OXform & jointXformObj, int recursionLevel)
//
// Description:
//   Given a dagPose and an index corresponding to a joint, print out
//   the matrix info for the joint.
// Return:
//       None.
//
{

	//double matrixArray[16];
	Alembic::Abc::M44d matrixObj;
	matrixObj.makeIdentity();


	MFnDependencyNode nDagPose(dagPoseNode);
	int i = 0;
	//for (i = 0; i<recursionLevel; i++) { fprintf(file, "\t"); }
	//fprintf(file, "%s\n", nDagPose.name().asChar());

	// construct plugs for this joints world and local matrices
	//
	MObject aWorldMatrix = nDagPose.attribute("worldMatrix");
	MPlug pWorldMatrix(dagPoseNode, aWorldMatrix);
	pWorldMatrix.selectAncestorLogicalIndex(index, aWorldMatrix);

	MObject aMatrix = nDagPose.attribute("xformMatrix");
	MPlug pMatrix(dagPoseNode, aMatrix);
	pMatrix.selectAncestorLogicalIndex(index, aMatrix);

	// get and print the world matrix data
	//
	MObject worldMatrix, xformMatrix;
	MStatus status = pWorldMatrix.getValue(worldMatrix);
	if (MS::kSuccess != status) {
		MGlobal::displayError("Problem retrieving world matrix.");
	}
	else {
		bool foundMatrix = 0;
		MFnMatrixData dMatrix(worldMatrix);
		MMatrix wMatrix = dMatrix.matrix(&status);
		if (MS::kSuccess == status) {
			foundMatrix = 1;
			unsigned jj, kk;

			//for (i = 0; i<recursionLevel; i++) { fprintf(file, "\t"); }
			//fprintf(file, "worldMatrix\n");

			for (jj = 0; jj < 4; ++jj) {
				//for (i = 0; i<recursionLevel; i++) { fprintf(file, "\t"); }
				for (kk = 0; kk < 4; ++kk) {
					double val = wMatrix(jj, kk);
					//fprintf(file, "%f ", val);
				}
				//fprintf(file, "\n");
			}
		}
		if (!foundMatrix) {
			MGlobal::displayError("Error getting world matrix data.");
		}
	}

	// get and print the local matrix data
	//
	status = pMatrix.getValue(xformMatrix);
	if (MS::kSuccess != status) {
		MGlobal::displayError("Problem retrieving xform matrix.");
	}
	else {
		bool foundMatrix = 0;
		MFnMatrixData dMatrix(xformMatrix);
		if (dMatrix.isTransformation()) {
			MTransformationMatrix xform = dMatrix.transformation(&status);
			if (MS::kSuccess == status) {
				foundMatrix = 1;
				MMatrix xformAsMatrix = xform.asMatrix();
				unsigned rowIndex, colIndex;
				int arrIndex = 0;
				//for (i = 0; i<recursionLevel; i++) { fprintf(file, "\t"); }
				//fprintf(file, "localMatrix\n");
				for (rowIndex = 0; rowIndex < 4; ++rowIndex) {
					//for (i = 0; i<recursionLevel; i++) { fprintf(file, "\t"); }
					for (colIndex = 0; colIndex < 4; ++colIndex) {
						double val = xformAsMatrix(rowIndex, colIndex);
						matrixObj.x[rowIndex][colIndex] = val;
						//fprintf(file, "%f ", val);
					}
					//fprintf(file, "\n");
				}

				Alembic::AbcGeom::XformOp matrixOp(Alembic::AbcGeom::kMatrixOperation, Alembic::AbcGeom::kMatrixHint);
	
				//TO DO: figure out the bounding box associated with all geometry that this transform affects
				//Alembic::AbcGeom::OBox3dProperty childBounds = xformObj.getSchema().getChildBoundsProperty();
				//childBounds.set(Alembic::Abc::Box3d(Alembic::Abc::V3d(, , , ), Alembic::Abc::V3d(, , , )));

				Alembic::AbcGeom::XformSample xformSamp;
				xformSamp.addOp(matrixOp, matrixObj);
				xformSamp.setInheritsXforms(false);
				jointXformObj.getSchema().set(xformSamp);
			}
		}
		if (!foundMatrix) {
			MGlobal::displayError("Error getting local matrix data.");
		}
	}
}


bool polyAbcExporter::exportDagPose(MObject* jointNode, Alembic::AbcGeom::OXform &jointXformObject, int recursionLevel)
//
// Description:
//   Given a joint, check for connected dag pose nodes.
//   For each pose found, write out the pose info.
// Return:
//       If one or more poses is found, return true, else return false.
//
{
	bool rtn = 0; // return 1 if we find a pose
	int i = 0;
	MStatus status;
	MFnDependencyNode fnJoint(*jointNode);

	//MObject aDagPose = fnJoint.attribute("dagPose", 
	MObject aBindPose = fnJoint.attribute("bindPose", &status);

	if (MS::kSuccess == status) 
	{
		unsigned connLength = 0;
		MPlugArray connPlugs;
		MPlug pBindPose(*jointNode, aBindPose);
		pBindPose.connectedTo(connPlugs, false, true);
		connLength = connPlugs.length();
		for (unsigned ii = 0; ii < connLength; ++ii) {
			
			//find all dag pose connections on the joint
			if (connPlugs[ii].node().apiType() == MFn::kDagPose) 
			{
				MObject aMember = connPlugs[ii].attribute();
				MFnAttribute fnAttr(aMember);
				if (fnAttr.name() == "worldMatrix") {
					unsigned jointIndex = connPlugs[ii].logicalIndex();

					for (i = 0; i<recursionLevel; i++) { fprintf(file, "\t"); }
					fprintf(file, "%s (%u)\n", fnJoint.name().asChar(), jointIndex);

					MGlobal::displayInfo(fnJoint.name());
					MObject jointObject = connPlugs[ii].node();
					writeDagPoseInfo(jointObject, jointIndex, jointXformObject, recursionLevel);
					rtn = 1;
				}
			}
		}
	}
	return rtn;
}

static unsigned int totalJoints = 0;


void polyAbcExporter::exportJointAnimationTransform(MFnDagNode& jointDagNode, FILE * animFile, int recursionLevel)
{
	int i;
	MStatus status;
	MDagPath jointDagPath;
	jointDagNode.getPath(jointDagPath);
	// attach the function set to the object
	MFnTransform transform(jointDagPath);


	MVector translate = transform.getTranslation(MSpace::kWorld, &status);
	double scale[3];
	transform.getScale(scale);
	double rotation[3];
	MTransformationMatrix::RotationOrder rotOrder;
	status = transform.getRotation(rotation, rotOrder);


	MMatrix jointPoseMatrix = transform.transformation().asMatrix();
	//MString translateStr = MString("Translation: ") + translate.x + " " + translate.y + " " + translate.z;
	//MGlobal::displayInfo(translateStr);
	//MString scaleStr = MString("Scale: ") + scale[0] + " " + scale[1] + " " + scale[2];
	//MGlobal::displayInfo(scaleStr);
	//MString rotStr = MString("Rotation: ") + rotation[0] + " " + rotation[1] + " " + rotation[2];
	//MGlobal::displayInfo(rotStr);


	unsigned rowIndex, colIndex;
	int arrIndex = 0;
	//for (i = 0; i<recursionLevel; i++) { printf("\t"); }
	//printf("localMatrix\n");
	for (rowIndex = 0; rowIndex < 4; ++rowIndex) {
		//for (i = 0; i<recursionLevel; i++) { fprintf(animFile, "\t"); }
		for (colIndex = 0; colIndex < 4; ++colIndex) {
			float val = (float)jointPoseMatrix(rowIndex, colIndex);
			//matrixObj.x[colIndex][rowIndex] = val;
			//printf("%f ", val);
			fwrite(&val, sizeof(float), 1, animFile);
		}

		//printf("\n");
	}


	


}


static int animJointIndex = 0;
bool polyAbcExporter::processArmaturePose(MFnDagNode& jointDagNode, MObject *jointObjectNode, FILE * animFile, std::map<std::string, int> &jointIndexMap, int recursionLevel)
{
	MStatus status;
	int i;
	//MDagPath * jointDagPath = new MDagPath(dagPath);
	//jointDagPath->pop();
	//MFnTransform * transform = new MFnTransform(*jointDagPath, &status);
	//MObject * jointObject = new MObject(*jointDagPath, &status);

	//Get the Maya API MObject associated with the joint
	MObject * currentJointNode = jointObjectNode;
	MFnDependencyNode fnJoint(*currentJointNode);

	//Create an Abc Archive Xform node to hold the joint's inverse bind pose matrix
	//Alembic::AbcGeom::OXform jointXformObj(jointXformParentObj, fnJoint.name().asUTF8());

	std::string jointNameStr = fnJoint.name().asChar();
	//jointIndexMap.insert(std::make_pair(jointNameStr, (*jointIndex)++));


	for (i = 0; i<recursionLevel; i++) { printf("\t"); }
	printf("Anim Pose %s (%u)\n", fnJoint.name().asChar(), animJointIndex++);


	auto jointIndexIt = jointIndexMap.find(jointNameStr);
	if (jointIndexIt != jointIndexMap.end())
	{
		int mapJointIndex = jointIndexIt->second;
		for (i = 0; i<recursionLevel; i++) { printf("\t"); }
		printf("Joint Map Index %s (%u)\n", fnJoint.name().asChar(), mapJointIndex);

	}


	//read the current maya api joint node's transform matrix(es)
	//export the current joint bind transform (ie add its bind pose to the skeletal hierarchy of joints in the abc archive)
	exportJointAnimationTransform(jointDagNode, animFile, recursionLevel);

	//iterate and recursively process all ancestors of this joint before returning
	for (int childIndex = 0; childIndex < jointDagNode.childCount(); childIndex++)
	{
		currentJointNode = &(jointDagNode.child(childIndex));
		MFnDagNode childDagNode(*currentJointNode);
		//only process joints, not constraints
		if (currentJointNode->apiType() == MFn::kJoint)
			processArmaturePose(childDagNode, currentJointNode, animFile, jointIndexMap, recursionLevel + 1);
	}


	totalJoints += 1;


	//delete jointDagPath;

	return true;
}


bool polyAbcExporter::processJointNode(MFnDagNode& jointDagNode,  MObject *jointObjectNode, Alembic::Abc::OObject &jointXformParentObj, int recursionLevel, std::map<std::string, int> &jointIndexMap, int *jointIndex)
//bool polyAbcExporter::processJointNode(const MDagPath dagPath, Alembic::Abc::OObject &topObject, int recursionLevel)
{
	MStatus status;
	int i;
	//MDagPath * jointDagPath = new MDagPath(dagPath);
	//jointDagPath->pop();
	//MFnTransform * transform = new MFnTransform(*jointDagPath, &status);
	//MObject * jointObject = new MObject(*jointDagPath, &status);

	//Get the Maya API MObject associated with the joint
	MObject * currentJointNode = jointObjectNode;
	MFnDependencyNode fnJoint(*currentJointNode);

	//Create an Abc Archive Xform node to hold the joint's inverse bind pose matrix
	Alembic::AbcGeom::OXform jointXformObj(jointXformParentObj, fnJoint.name().asUTF8());
	
	std::string jointNameStr = fnJoint.name().asChar();

	for (i = 0; i<recursionLevel; i++) { fprintf(file, "\t"); }
	fprintf(file, "%s (%u)\n", fnJoint.name().asChar(), *jointIndex);

	jointIndexMap.insert(std::make_pair(jointNameStr, (*jointIndex)++));

	//read the current maya api joint node's transform matrix(es)
	//export the current joint bind transform (ie add its bind pose to the skeletal hierarchy of joints in the abc archive)
	exportDagPose(currentJointNode, jointXformObj, recursionLevel);

	//iterate and recursively process all ancestors of this joint before returning
	for (int childIndex = 0; childIndex < jointDagNode.childCount(); childIndex++)
	{
		currentJointNode = &(jointDagNode.child(childIndex));
		MFnDagNode childDagNode(*currentJointNode);
		//only process joints, not constraints
		if(currentJointNode->apiType() == MFn::kJoint )
			processJointNode(childDagNode, currentJointNode, jointXformObj, recursionLevel+1, jointIndexMap, jointIndex);
	}


	totalJoints += 1;


	//delete jointDagPath;

	return true;
}


MStatus polyAbcExporter::processPolyMesh(const MDagPath dagPath, Alembic::Abc::OObject &topObject, Alembic::Abc::OObject &mtlObject, Alembic::Abc::OObject &armatureObject)
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

		//read joints if there are any
		if (MStatus::kFailure == pWriter->extractSkinningData(armaturesMap)) {
			delete pWriter;
			return MStatus::kFailure;
		}

		if (MStatus::kFailure == pWriter->writeGeometryToArchive(meshXform, mtlObject)) {
			delete pWriter;
			return MStatus::kFailure;
		}
		delete pWriter;


		return MStatus::kSuccess;
}

/*
MStatus polyAbcExporter::processPolyMesh(const MDagPath dagPath, Alembic::Abc::OObject &topObject, Alembic::Abc::OObject &mtlObject)
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
	if (MStatus::kFailure == pWriter->writeGeometryToArchive(meshXform, mtlObject)) {
		delete pWriter;
		return MStatus::kFailure;
	}
	delete pWriter;


	return MStatus::kSuccess;
}
*/

/*
bool OutputBoneAnimation(MDagPath& bone, int sf, int ef)
{

	// loop through all frames in this animation cycle.
	//
	for (int i = sf; i<ef; ++i) {


		// set the current frame
		MAnimControl::setCurrentTime(MTime(i, MTime::kPALFrame));

		// attach the function set to the object
		MFnTransform fn(bone);

		// get the rotation for the bone this frame
		MQuaternion Rotation;

		fn.getRotation(Rotation);

		// write out the animation key
		// you may want more animation data, in which case
		// have a look at the transform section.
		cout << "rotation "
			<< Rotation.x << " "
			<< Rotation.y << " "
			<< Rotation.z << " "
			<< Rotation.w << endl;
	}

}
*/



MStatus polyAbcExporter::processRootJoint(const MDagPath rootJointDagPath, MObject& rootNode, Alembic::Abc::OObject &topObject, Alembic::Abc::OObject &armatureObject)
//MStatus polyAbcExporter::processRootJoint(MObject& jointNode, Alembic::Abc::OObject &topObject, Alembic::Abc::OObject &mtlObject)
{
	//Summary:	processes the mesh on the given dag path by extracting its geometry
	//			and writing this data to file
	//Args   :	dagPath - the current dag path whose poly mesh is to be processed
	//			os - an output stream to write to
	//Returns:	MStatus::kSuccess if the polygonal mesh data was processed fully;
	//			MStatus::kFailure otherwise

	MStatus status;

	
	file = NULL;
	//create a file for debugging joint matrix output
	file = fopen("C:\\Development\\svn\\CoreRender\\PlugIns\\Maya\\bin\\x64\\Joints.txt", "wb");
	if (!file) {
		MString openError("Could not open: ");
		openError += "Joints.txt";
		MGlobal::displayError(openError);
		status = MS::kFailure;
		return status;
	}
	
	totalJoints = 0;
	int recursionLevel = 0;

	MFnDagNode jointDagNode(rootJointDagPath);
	MObject * jointObjectNode = &rootNode;
	std::map< std::string, int> jointIndexMap;
	int jointIndex = 0;
	processJointNode(jointDagNode, jointObjectNode, armatureObject, recursionLevel, jointIndexMap, &jointIndex);


	MFnDependencyNode fnJoint(*jointObjectNode);
	std::string jointNameStr = fnJoint.name().asChar();
	armaturesMap.insert(std::make_pair(jointNameStr, jointIndexMap));

	fprintf(file, "\n\nTotal Joints = %d\n", totalJoints);

	fclose(file);

	file = NULL;
	//create a file for debugging joint matrix output

	std::string animFileName = "C:\\Development\\svn\\CoreRender\\PlugIns\\Maya\\bin\\x64\\idle.animation";
	//animFileName.append(jointNameStr);
	//animFileName.append("\\idle.animation");
	file = fopen(animFileName.c_str(), "wb");
	if (!file) {
		MString openError("Could not open: ");
		openError += "idle.animation";
		MGlobal::displayError(openError);
		status = MS::kFailure;
		return status;
	}


	jointIndex = 0;
	recursionLevel = 0;

	MTime Start = MAnimControl::animationStartTime();
	MTime End = MAnimControl::animationEndTime();

	double startTime = Start.as(Start.unit());
	double endTime = End.as(End.unit());
	double duration = endTime - startTime;
	double framerate = 60.;
	static const unsigned int numKeyFrames = 3;
	unsigned int keyframes[3] = { 0, 30, 59 };

	//fprintf(file, "FPS unit = %d\n", Start.unit());
	//fprintf(file, "Start Frame = %g\n", Start.as(Start.unit()));
	//fprintf(file, "End Frame = %g\n", End.as(End.unit()));
	//fprintf(file, "Num Joint Transforms = %d\n", totalJoints);
	//fprintf(file, "Num Keyframes = %d\n", numKeyFrames);
	//fprintf(file, "Keyframes = ");
	//for (int i = 0; i < numKeyFrames; i++) 	fprintf(file, "%d, ", keyframes[i]);


	//write the animation duration as double to the file first
	fwrite(&duration, sizeof(double), 1, file);
	//write the framerate that the animation duration was created at
	fwrite(&framerate, sizeof(double), 1, file);
	//next write the number of joint transforms in the armature/in each keyframe pose
	fwrite(&totalJoints, sizeof(unsigned int), 1, file);
	//then write the number of keyframes followed by each keyframe index
	fwrite(&numKeyFrames, sizeof(unsigned int), 1, file);
	
	
	for (int i = 0; i < numKeyFrames; i++) fwrite(&(keyframes[i]), sizeof(unsigned int), 1, file);
	//then write the transforms for each keyframe sequentially
	for (int i = 0; i < numKeyFrames; i++)
	{
		int keyFrame = keyframes[i];
		MAnimControl::setCurrentTime(MTime(keyFrame, Start.unit()));
		processArmaturePose(jointDagNode, jointObjectNode, file, jointIndexMap, recursionLevel);

	}

	fclose(file);

	return MStatus::kSuccess;
}

MStatus polyAbcExporter::exportAll(Alembic::Abc::OObject &topObject, Alembic::Abc::OObject &mtlObject, Alembic::Abc::OObject &armatureObject)
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
			if (MStatus::kFailure == processPolyMesh(dagPath, topObject, mtlObject, armatureObject)) {
				return MStatus::kFailure;
			}
		}
	}
	return MStatus::kSuccess;
}


MStatus polyAbcExporter::exportSelection(Alembic::Abc::OObject &topObject, Alembic::Abc::OObject &mtlObject, Alembic::Abc::OObject &armatureObject)
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


	/*
	MItSelectionList itSelectionList(selectionList, MFn::kInvalid, &status);

	if (MStatus::kFailure == status) {
		return MStatus::kFailure;
	}
	*/
	/*
	//Iterate over the selection list
	//If we don't explicitly filter the selection list,
	//We expect the user selected nodes to be of kTransform type nodes, 
	//and we need to find the child kMesh shape nodes underneath these
	while (!(itSelectionList.isDone()))//for (itSelectionList.reset(); !itSelectionList.isDone(); );// itSelectionList.next())
	{
		//Get the the Directed Acyclic Graph Path for the selected node
		MDagPath dagPath;
		if (MStatus::kFailure == itSelectionList.getDagPath(dagPath)) {
			MGlobal::displayError("MItSelectionList::getDagPath");
			return MStatus::kFailure;
		}

		itSelectionList.next();


		
		if (dagPath.apiType() == MFn::kTransform)
		{

			//MObject depNode;
			//itSelectionList.getDependNode(depNode);


			MDagPath * xformDagPath = new MDagPath(dagPath);
			xformDagPath->pop();
			MFnTransform * transform = new MFnTransform(dagPath, &status);


			for (int childIndex = 0; childIndex < transform->childCount(); childIndex++)
			{
				MObject xformChild = transform->child(childIndex, &status);

				if (xformChild.apiType() == MFn::kMesh)
				{
					if( xformChild.hasFn(MFn::kDagNode) )
					{
						MDagPath meshDagPath;
						xformDagPath->getAPathTo(xformChild, meshDagPath);
						MGlobal::displayInfo("xformChild mesh.name:  " + meshDagPath.fullPathName() + "\n");
					}


				}
			}


		}
		

		if (dagPath.apiType() == MFn::kMesh)
		{

			if (!(itSelectionList.isDone()))
			{
				//get the next item in the list, to check if it is the joint of an armature skeleton
				MDagPath nextDagPath;
				if (MStatus::kFailure == itSelectionList.getDagPath(nextDagPath)) {
					MGlobal::displayError("MItSelectionList::getDagPath");
					return MStatus::kFailure;
				}

				//get the current Mobject node for the joint dag path
				MObject nextDepNode;
				itSelectionList.getDependNode(nextDepNode);
				MString typeStr = MString(nextDepNode.apiTypeStr());
				MGlobal::displayInfo("nextDepNode.apiType:  " + typeStr + "\n");


				//If the selected node following a selected shape node of type MFn::Mesh is of type MFn::kJoint
				//Then we assume it is the root node of a skeletal armature hierarchy for export to a run-time animation system
				if ( nextDepNode.apiType() == MFn::kJoint )
				{

					if (MStatus::kFailure == processRootJoint(nextDagPath, nextDepNode, topObject, mtlObject)) {
						return MStatus::kFailure;
					}

	

				}
			}

			if (MStatus::kFailure == processPolyMesh(dagPath, topObject, mtlObject)) {
				return MStatus::kFailure;
			}

		}
		

	}
	*/
	
	

	//Retrieve all nodes of type MFn:kJoint from the selected list of nodes
	MItSelectionList jointSelectionList(selectionList, MFn::kJoint, &status);

	if (MStatus::kFailure != status)
	{
		for (jointSelectionList.reset(); !jointSelectionList.isDone(); jointSelectionList.next())
		{
			//get the current dag path and process the poly mesh on it
			MDagPath jointDagPath;
			if (MStatus::kFailure == jointSelectionList.getDagPath(jointDagPath)) {
				MGlobal::displayError("MItSelectionList::getDagPath");
				return MStatus::kFailure;
			}

			//this if statement is redundant
			if (jointDagPath.apiType() == MFn::kJoint)
			{
				//get the current Mobject node for the joint dag path
				MObject jointDepNode;
				jointSelectionList.getDependNode(jointDepNode);
				MString typeStr = MString(jointDepNode.apiTypeStr());
				MGlobal::displayInfo("jointDepNode.apiType:  " + typeStr + "\n");

				if (MStatus::kFailure == processRootJoint(jointDagPath, jointDepNode, topObject, armatureObject)) {
					return MStatus::kFailure;
				}
			}

		}

		//Iterate over joints again to calculate the animation poses at the desired keyframes
	}
	else
	{
		MGlobal::displayInfo("AbcExport -- No Joints Found in Selection\n");
	}


	//Retrieve all shape nodes of type MFn:kMesh from the selected list of nodes
	MItSelectionList meshSelectionList(selectionList, MFn::kMesh, &status);
	if (MStatus::kFailure == status) {
		MGlobal::displayInfo("AbcExport -- Error:  No Mesh Geometry Shape Nodes Found in Selection.  Aborting.\n");
		return MStatus::kFailure;
	}

	for (meshSelectionList.reset(); !meshSelectionList.isDone(); meshSelectionList.next())
	{
		//Get the the Directed Acyclic Graph Path for the selected node
		MDagPath dagPath;
		if (MStatus::kFailure == meshSelectionList.getDagPath(dagPath)) {
			MGlobal::displayError("MItSelectionList::getDagPath");
			return MStatus::kFailure;
		}

		meshSelectionList.next();

		//this is a redundant if statement
		if (dagPath.apiType() == MFn::kMesh)
		{
			if (MStatus::kFailure == processPolyMesh(dagPath, topObject, mtlObject, armatureObject)) 
			{
				MGlobal::displayInfo("AbcExport -- Error:  polyAbcExporter::processPolyMesh failed.  Aborting.\n");
				return MStatus::kFailure;
			}

		}

	}


	/*
	bool hasJoints = true;
	if (MStatus::kFailure == status) {

		hasJoints = false;
		//return MStatus::kFailure;
	}
	

	for (itSelectionList.reset(); !itSelectionList.isDone(); itSelectionList.next()) {
		MDagPath dagPath;

		//get the current dag path and process the poly mesh on it
		//
		if (MStatus::kFailure == itSelectionList.getDagPath(dagPath)) {
			MGlobal::displayError("MItSelectionList::getDagPath");
			return MStatus::kFailure;
		}

		MObject depNode;
		itSelectionList.getDependNode(depNode);
		MString typeStr = MString( depNode.apiTypeStr() );
		MGlobal::displayInfo("dagPath.apiType:  " + typeStr + "\n");

		if (dagPath.apiType() == MFn::kMesh)
		{
			//First, check if there is a skeletal joint system associated with this mesh selection
			//If so, populate the tree of joint nodes, their names and bind pose transforms
			if (hasJoints && !(jointSelectionList.isDone()))
			{
				//Get the next joint from the selection list

				//get the current dag path and process the poly mesh on it
				MDagPath jointDagPath;
				if (MStatus::kFailure == jointSelectionList.getDagPath(jointDagPath)) {
					MGlobal::displayError("MItSelectionList::getJointDagPath failed");
					return MStatus::kFailure;
				}

				//get the current Mobject node for the joint dag path
				MObject jointDepNode;
				jointSelectionList.getDependNode(jointDepNode);
				MString typeStr = MString(jointDepNode.apiTypeStr());
				MGlobal::displayInfo("jointDepNode.apiType:  " + typeStr + "\n");

				if (jointDepNode.apiType() == MFn::kJoint)
				{
					if (MStatus::kFailure == processRootJoint(jointDagPath, jointDepNode, topObject, mtlObject)) {
						return MStatus::kFailure;
					}

					jointSelectionList.next();
				}

			}

			if (MStatus::kFailure == processPolyMesh(dagPath, topObject, mtlObject)) {
				return MStatus::kFailure;
			}

			
		}
		else
		{
			MGlobal::displayInfo("MItSelectionList::Unknown MObject Selection");
		}
	}
	*/
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
