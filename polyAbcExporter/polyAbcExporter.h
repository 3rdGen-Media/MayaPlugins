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

#ifndef __POLYABCEXPORTER_H
#define __POLYABCEXPORTER_H

// polyAbcExporter.h

// *****************************************************************************
//
// CLASS:    polyAbcExporter
//
// *****************************************************************************
//
// CLASS DESCRIPTION (polyAbcExporter)
// 
// polyAbcExporter is a class derived from polyExporter.  It allows the export
// of polygonal mesh data in Alembic Extended Material Format format.  The file extension for this type
// is ".abc".
//
// *****************************************************************************

#include "polyExporter.h"
#include "AbcExportInterface.h"
#include <unordered_set>



class polyAbcExporter : public polyExporter {

	public:
		polyAbcExporter(){}
		~polyAbcExporter() override;

		static	void*			creator();
				MString			defaultExtension () const override;
				MStatus			initializePlugin(MObject obj);
				MStatus			uninitializePlugin(MObject obj);


	private:	
				//file open invocation method
				MStatus			writer(const MFileObject& file,
										const MString& optionsString,
										MPxFileTranslator::FileAccessMode mode) override;

				polyWriter*		createPolyWriter(const MDagPath dagPath, MStatus& status) override;
				void			writeHeader(Alembic::Abc::OArchive &archive);// override;
				//void			writeFooter(ostream& os) override;
				MStatus			exportAll(Alembic::Abc::OObject &topObject, Alembic::Abc::OObject &mtlObject, Alembic::Abc::OObject &armatureObject);// override;
				MStatus			exportSelection(Alembic::Abc::OObject &topObject, Alembic::Abc::OObject &mtlObject, Alembic::Abc::OObject &armatureObject);// override;



				//Process Joint/Skeletal Armatures
				MStatus			processRootJoint(const MDagPath rootJointDagPath, MObject& rootNode, Alembic::Abc::OObject &topObject, Alembic::Abc::OObject &armatureObject);
				bool			processJointNode(MFnDagNode& jointDagNode, MObject *jointObjectNode, Alembic::Abc::OObject &jointXformParentObj, int recursionLevel, std::map<std::string, int> &jointIndexMap, int *jointIndex);
				//Export Joint Armature Bind Pose(s)
				bool			exportDagPose(MObject* jointNode, Alembic::AbcGeom::OXform &jointXformObject, int recursionLevel);
				//Print Joint Armature Bind Pose(s)
				void polyAbcExporter::writeDagPoseInfo(MObject& dagPoseNode, unsigned index, Alembic::AbcGeom::OXform & jointXformObj, int recursionLevel);

				//Process/export joint/skeletal armature animations
				bool			processArmaturePose(MFnDagNode& jointDagNode, MObject *jointObjectNode, FILE * animFile, std::map<std::string, int> &jointIndexMap, int recursionLevel);
				void			exportJointAnimationTransform(MFnDagNode& jointDagNode, FILE * animFile, int recursionLevel);

				//Process/Export Geometry
				Alembic::AbcGeom::OXform processMeshXform(const MDagPath meshDagPath, Alembic::Abc::OObject &topObject);
				MStatus			processPolyMesh(const MDagPath dagPath, Alembic::Abc::OObject &topObject, Alembic::Abc::OObject &mtlObject, Alembic::Abc::OObject &armatureObject);


			

				//Private Debug Vars
				FILE*           file;
				std::map<std::string, std::map<std::string, int>> armaturesMap;

};

#endif /*__POLYEXPORTER_H*/
