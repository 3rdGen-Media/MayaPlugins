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
				MStatus			exportAll(Alembic::Abc::OObject &topObject, Alembic::Abc::OObject &mtlObject);// override;
				MStatus			exportSelection(Alembic::Abc::OObject &topObject, Alembic::Abc::OObject &mtlObject);// override;

				MStatus			processPolyMesh(const MDagPath dagPath, Alembic::Abc::OObject &topObject, Alembic::Abc::OObject &mtlObject);
				Alembic::AbcGeom::OXform processMeshXform(const MDagPath meshDagPath, Alembic::Abc::OObject &topObject);


};

#endif /*__POLYEXPORTER_H*/
