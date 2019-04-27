#ifndef ABC_EXPORT_INTERFACE_H
#define ABC_EXPORT_INTERFACE_H

#include <Alembic/AbcCoreOgawa/All.h>
#include <Alembic/Abc/All.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcMaterial/All.h>

static Alembic::Abc::OArchive g_archive;


static const char * kAlembicAlbedoTexture =				"map_Ka";
static const char * kAlembicDiffuseTexture	=			"map_Kd";
static const char * kAlembicAmbientOcclusionTexture =	"map_Ao";

static const char * kAlembicNormalTexture	=			"map_Norm";
static const char * kAlembicSpecularTexture =			"map_Ks";
static const char * kAlembicGlossTexture =				"map_Ga";
static const char * kAlembicMetalTexture =				"map_Ms";
static const char * kAlembicRoughnessTexture =			"map_Ra";

static const char * kAlembicEmissivityTexture =			"map_Em";

//typedef struct 


#endif
