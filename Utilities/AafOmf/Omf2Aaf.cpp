// @doc INTERNAL
// @com This file implements the conversion of OMF files to AAF file format.
/******************************************\
*                                          *
* Advanced Authoring Format                *
*                                          *
* Copyright (c) 1998 Avid Technology, Inc. *
* Copyright (c) 1998 Microsoft Corporation *
*                                          *
\******************************************/
/***********************************************************************
 *
 *              Copyright (c) 1996 Avid Technology, Inc.
 *
 * Permission to use, copy and modify this software and to distribute
 * and sublicense application software incorporating this software for
 * any purpose is hereby granted, provided that (i) the above
 * copyright notice and this permission notice appear in all copies of
 * the software and related documentation, and (ii) the name Avid
 * Technology, Inc. may not be used in any advertising or publicity
 * relating to the software without the specific, prior written
 * permission of Avid Technology, Inc.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL AVID TECHNOLOGY, INC. BE LIABLE FOR ANY DIRECT,
 * SPECIAL, INCIDENTAL, INDIRECT, CONSEQUENTIAL OR OTHER DAMAGES OF
 * ANY KIND, OR ANY DAMAGES WHATSOEVER ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE, INCLUDING, 
 * WITHOUT  LIMITATION, DAMAGES RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, AND WHETHER OR NOT ADVISED OF THE POSSIBILITY OF
 * DAMAGE, REGARDLESS OF THE THEORY OF LIABILITY.
 *
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream.h>

// OMF Includes
namespace OMF2
{
#include "omPublic.h"
#include "omMedia.h"
#include "omEffect.h"
}

#include "AAFTypes.h"

//#include "AAFUtils.h"
AAFRESULT aafMobIDNew(aafUID_t *mobID);
AAFRESULT aafMobIDFromMajorMinor(
        aafUInt32	major,
		aafUInt32	minor,
		aafUID_t *mobID);     /* OUT - Newly created Mob ID */

//#include "aafCvt.h"

// AAF Utilities Infra-structure 
#include "UtlConsole.h"

#include "AafOmf.h"
#include "omf2aaf.h"


// Include the AAF Stored Object identifiers. These symbols are defined in aaf.lib.
#include "AAFStoredObjectIDs.h"


extern AafOmfGlobals* gpGlobals;
// ============================================================================
// Constructor
// ============================================================================
Omf2Aaf::Omf2Aaf() : pFile(NULL), pHeader(NULL), pDictionary(NULL)
{
	OMFSession = 0;
	OMFFileHdl = 0;
	OMFFileRev = OMF2::kOmfRev1x;
}

// ============================================================================
// Destructor
// ============================================================================
Omf2Aaf::~Omf2Aaf()
{
}
// ============================================================================
// ConvertFile
//			This function provides the control and logic to convert an OMF file
//			by calling all the functions inside this object
//
// ============================================================================
HRESULT Omf2Aaf::ConvertFile ()
{
	HRESULT		rc = AAFRESULT_SUCCESS;

	rc = OMFFileOpen( gpGlobals->sInFileName );
	if (SUCCEEDED(rc))
	{
		rc = OpenOutputFile();
		if (SUCCEEDED(rc))
		{
			rc = ConvertOMFHeader();
			if (SUCCEEDED(rc))
			{
				rc = OMFFileRead();
			}
		}
	}

	if (gpGlobals->bOMFFileOpen)
	{
		OMFFileClose();
		gpGlobals->bOMFFileOpen = AAFFalse;
	}

	if (gpGlobals->bAAFFileOpen)
	{
		AAFFileClose();
		gpGlobals->bAAFFileOpen = AAFFalse;
	}
	return rc;
}

// ============================================================================
// OpenOutputFile
//			This function creates the output file.
//
// ============================================================================
HRESULT Omf2Aaf::OpenOutputFile ()
{
	HRESULT		rc = AAFRESULT_SUCCESS;
	aafBool		bOverride = AAFFalse;

	if (strlen(gpGlobals->sOutFileName) == 0)
	{
		char*	pExt;
		bOverride = AAFTrue;
		strcpy(gpGlobals->sOutFileName, gpGlobals->sInFileName);
		pExt= strrchr(gpGlobals->sOutFileName, '.');
		strcpy(pExt,".aaf");
	}
	
	if (bOverride)
	{
		UTLstdprintf("Previous file: %s will be overwritten\n", gpGlobals->sOutFileName);
		deleteFile(gpGlobals->sOutFileName);
	}

	rc = AAFFileOpen(gpGlobals->sOutFileName);

	return rc;
}
// ============================================================================
// OMFFileOpen
//
//		Open an OMF File
//
// ============================================================================
HRESULT Omf2Aaf::OMFFileOpen(char * pFileName)
{
	HRESULT				rc = AAFRESULT_SUCCESS;
	aafBool				bSessionStarted = AAFFalse;
	char				szFileVersion[5];

	if (OMF2::OM_ERR_NONE == OMF2::omfsBeginSession(0, &OMFSession))
	{
		bSessionStarted = AAFTrue;
		if (OMF2::OM_ERR_NONE == OMF2::omfmInit(OMFSession))
		{
			rc = OMF2::omfsOpenFile((OMF2::fileHandleType)pFileName, OMFSession, &OMFFileHdl);

		}
		else
			rc = AAFRESULT_BADOPEN;
	}
	else 
	{
		if (bSessionStarted)
			OMF2::omfsEndSession(OMFSession);

		rc = AAFRESULT_BAD_SESSION;
	}

	OMF2::omfsFileGetRev(OMFFileHdl, &OMFFileRev);
	if (OMF2::kOmfRev2x == OMFFileRev)
	{
		strcpy(szFileVersion, "2.0");
	}
	else
	{
		strcpy(szFileVersion, "1.0");
	}
	gpGlobals->bOMFFileOpen = AAFTrue;
	if (gpGlobals->bVerboseMode)
	{
		UTLstdprintf("OMF File: %s opened succesfully\n", pFileName);
		UTLstdprintf("          File Revision %s \n", szFileVersion);
	}
	return rc;
}

// ============================================================================
// AAFFileOpen
//
//		Open an AAF File
//
// ============================================================================
HRESULT Omf2Aaf::AAFFileOpen( char* pFileName)
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfErr_t			OMFError = OMF2::OM_ERR_NONE;

	OMF2::omfObject_t		OMFHeadObject, OMFIdent;
	char					text[256];
	aafWChar*				pwFileName = NULL;
	aafInt32				nOMFIdentifications;
	aafWChar*				pwCompanyName = NULL;
	aafWChar*				pwProductName = NULL;
    aafWChar*				pwProductVersionString = NULL;
    aafWChar*				pwPlatform = NULL;
	aafBool					bAddExtraIdent = AAFFalse;
	OMF2::omfProductVersion_t OMFVersion;

	aafProductIdentification_t	ProductInfo;
	IAAFIdentification*		pIdent = NULL;

	
	UTLStrAToStrW(pFileName, &pwFileName);

	// Get Identification from OMF Header 
	// We going to use the last identification entry in the
	// Identification array.. 
	OMFError = OMF2::omfsGetHeadObject(OMFFileHdl, &OMFHeadObject);
	nOMFIdentifications = OMF2::omfsLengthObjRefArray(OMFFileHdl, OMFHeadObject, OMF2::OMHEADIdentList);
	if (nOMFIdentifications >= 1) 
	{
		OMFError = OMF2::omfsGetNthObjRefArray(OMFFileHdl, OMFHeadObject, OMF2::OMHEADIdentList,
										&OMFIdent, nOMFIdentifications);
		if (OMF2::OM_ERR_NONE == OMFError)
		{
			if(OMF2::omfsReadString(OMFFileHdl, OMFIdent, OMF2::OMIDNTCompanyName, text, sizeof(text)) != OMF2::OM_ERR_NONE)
				strcpy(text, "<Not Specified>");
			UTLStrAToStrW(text, &pwCompanyName);
			ProductInfo.companyName = pwCompanyName;
			if(OMF2::omfsReadString(OMFFileHdl, OMFIdent, OMF2::OMIDNTProductName, text, sizeof(text)) != OMF2::OM_ERR_NONE)
				strcpy(text, "<Not Specified>");
			UTLStrAToStrW(text, &pwProductName);
			ProductInfo.productName = pwProductName;
			if(OMF2::omfsReadString(OMFFileHdl, OMFIdent, OMF2::OMIDNTProductVersionString, text, sizeof(text)) != OMF2::OM_ERR_NONE)
				strcpy(text, "<Not Specified>");
			UTLStrAToStrW(text, &pwProductVersionString);
			ProductInfo.productVersionString = pwProductVersionString;
			if (OMF2::omfsReadProductVersionType(OMFFileHdl, OMFIdent, OMF2::OMIDNTProductVersion, &OMFVersion) != OMF2::OM_ERR_NONE)
			{
				OMFVersion.major = 0;
				OMFVersion.minor = 0;
				OMFVersion.tertiary = 0;
				OMFVersion.patchLevel = 0;
				OMFVersion.type = OMF2::kVersionUnknown;
			}
			ProductInfo.productVersion.major = OMFVersion.major;
			ProductInfo.productVersion.minor = OMFVersion.minor;
			ProductInfo.productVersion.tertiary = OMFVersion.tertiary;
			ProductInfo.productVersion.patchLevel = OMFVersion.type;
			ProductInfo.productVersion.type = (aafProductReleaseType_t)OMFVersion.type;

			if(OMF2::omfsReadString(OMFFileHdl, OMFIdent, OMF2::OMIDNTPlatform, text, sizeof(text)) != OMF2::OM_ERR_NONE)
				strcpy(text, "<Not Specified>");
			UTLStrAToStrW(text, &pwPlatform);
			ProductInfo.platform = pwPlatform;

			rc = AAFFileOpenNewModify(pwFileName, 0, &ProductInfo, &pFile);
			bAddExtraIdent = AAFTrue;
		}
	}
	else
	{
		ProductInfo.companyName = L"Company Name";
		ProductInfo.productName = L"AAF/OMF File Conversion";
		ProductInfo.productVersion.major = 1;
		ProductInfo.productVersion.minor = 0;
		ProductInfo.productVersion.tertiary = 0;
		ProductInfo.productVersion.patchLevel = 0;
		ProductInfo.productVersion.type = kVersionDebug;
		ProductInfo.productVersionString = NULL;
		ProductInfo.productID = 42;
		ProductInfo.platform = NULL;
		rc = AAFFileOpenNewModify(pwFileName, 0, &ProductInfo, &pFile);
	}

	gpGlobals->bAAFFileOpen = AAFTrue;
	rc = pFile->GetHeader(&pHeader);
	if (AAFRESULT_SUCCESS == rc)
	{
		rc = pHeader->GetDictionary(&pDictionary);
		if (AAFRESULT_SUCCESS == rc && bAddExtraIdent)
		{
			// Create a new Identification
			rc = pDictionary->CreateInstance(&AUID_AAFIdentification, 
											 IID_IAAFIdentification,
											 (IUnknown **)&pIdent);
			if (AAFRESULT_SUCCESS == rc)
			{
				ProductInfo.companyName = L"Company Name";
				ProductInfo.productName = L"OMF to AAF File Conversion";
				ProductInfo.productVersion.major = 1;
				ProductInfo.productVersion.minor = 0;
				ProductInfo.productVersion.tertiary = 0;
				ProductInfo.productVersion.patchLevel = 0;
				ProductInfo.productVersion.type = kVersionDebug;
				ProductInfo.productVersionString = NULL;
				ProductInfo.productID = 42;
				ProductInfo.platform = NULL;
				pIdent->SetCompanyName(ProductInfo.companyName);
				pIdent->SetProductName(ProductInfo.productName);
				pIdent->SetProductVersion(&ProductInfo.productVersion);
				rc = pHeader->AppendIdentification(pIdent);
				pIdent->Release();
				pIdent = NULL;
			}
		}
	}
	else
		rc = AAFRESULT_INTERNAL_ERROR;

	if (gpGlobals->bVerboseMode)
		UTLstdprintf("AAF File: %s Created succesfully\n", pFileName);

	// Clean up all allocated memory and return
	if(pwFileName)
		UTLMemoryFree(pwFileName);

	if(pwCompanyName)
		UTLMemoryFree(pwCompanyName);

	if(pwProductName)
		UTLMemoryFree(pwProductName);

	if(pwProductVersionString)
		UTLMemoryFree(pwProductVersionString);

	if(pwPlatform)
		UTLMemoryFree(pwPlatform);

	return rc;
}

// ============================================================================
// OMFFileRead
//
//		Here is where all the real work is done.  We recursively read all 
//		Objects inside the OMF file.
//
// ============================================================================
HRESULT Omf2Aaf::OMFFileRead()
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfErr_t			OMFError = OMF2::OM_ERR_NONE;
	aafInt32				nOMFNumMobs;
	aafInt32				nOMFMobCount;

	// OMF Variables
	OMF2::omfIterHdl_t		OMFMobIter = NULL;
	OMF2::omfMobObj_t		OMFMob;
	OMF2::omfObject_t		OMFHeader, OMFObject;
	OMF2::omfProperty_t		OMFPropertyID;

	// AAF Variables
	IAAFMob*				pMob = NULL;
	IAAFCompositionMob*		pCompMob = NULL;
	IAAFMasterMob*			pMasterMob = NULL;
	IAAFSourceMob*			pSourceMob = NULL;

	aafInt32				numMedia;
	
	OMFError = OMF2::omfiIteratorAlloc( OMFFileHdl, &OMFMobIter);
	if (OMF2::OM_ERR_NONE == OMFError)
	{
		OMFError = OMF2::omfiGetNumMobs( OMFFileHdl, OMF2::kAllMob, &nOMFNumMobs);
		if (OMF2::OM_ERR_NONE == OMFError)
		{
			if (gpGlobals->bVerboseMode)
			{
				UTLstdprintf("Found: %ld Mobs in the input file\n", nOMFNumMobs);
			}
			for (nOMFMobCount = 0; nOMFMobCount < nOMFNumMobs; nOMFMobCount++)
			{
				OMFError = OMF2::omfiGetNextMob(OMFMobIter, NULL, &OMFMob);
				if (OMF2::OM_ERR_NONE == OMFError)
				{
					pMob = NULL;
					gpGlobals->nNumOMFMobs++;
					if (OMF2::omfiIsACompositionMob(OMFFileHdl, OMFMob, &OMFError))
					{
						// Create a new Composition Mob
						rc = pDictionary->CreateInstance(&AUID_AAFCompositionMob, 
	 													 IID_IAAFCompositionMob,
														 (IUnknown **)&pCompMob);
						if (SUCCEEDED(rc))
						{
							if (gpGlobals->bVerboseMode)
								UTLstdprintf( "Created AAF Composition Mob\n");
							rc = ConvertOMFCompositionObject( OMFMob, pCompMob );
							pCompMob->QueryInterface(IID_IAAFMob, (void **)&pMob);
							pCompMob->Release();
							pCompMob = NULL;
						}
					}
					else if (OMF2::omfiIsAMasterMob(OMFFileHdl, OMFMob, &OMFError) )
					{
						// Create a Master Mob 
						rc = pDictionary->CreateInstance(&AUID_AAFMasterMob,
														 IID_IAAFMasterMob,
														 (IUnknown **)&pMasterMob);
						if (SUCCEEDED(rc))
						{
							if (gpGlobals->bVerboseMode)
								UTLstdprintf("Created AAF Master Mob\n");
							rc = ConvertOMFMasterMob(OMFMob, pMasterMob );
							pMasterMob->QueryInterface(IID_IAAFMob, (void **)&pMob);
							pMasterMob->Release();
							pMasterMob = NULL;
						}
					}
					else if ( OMF2::omfiIsASourceMob(OMFFileHdl, OMFMob, &OMFError) )
					{
						rc = pDictionary->CreateInstance(&AUID_AAFSourceMob,
														 IID_IAAFSourceMob,
														 (IUnknown **)&pSourceMob);
						if (SUCCEEDED(rc))
						{
							if (gpGlobals->bVerboseMode)
								UTLstdprintf("Created AAF Source Mob\n");
							rc = ConvertOMFSourceMob( OMFMob, pSourceMob );
							pSourceMob->QueryInterface(IID_IAAFMob, (void **)&pMob);
							pSourceMob->Release();
							pSourceMob = NULL;
						}
					}
					else
					{
						OMF2::omfClassID_t		objClass;
						char					id[5];

						OMFError = OMF2::omfsReadClassID(OMFFileHdl, OMFMob, OMF2::OMOOBJObjClass, objClass);
						if (OMF2::OM_ERR_NONE == OMFError && gpGlobals->bVerboseMode)
						{
							strncpy(id, objClass, 4);
							id[4] = '\0';
							UTLstdprintf("Unrecognized Mob Class ID: %s\n",id);
						}
					}
					if (pMob)
					{
						rc = ConvertOMFMOBObject( OMFMob, pMob);
						if (rc != AAFRESULT_SUCCESS)
							UTLerrprintf("ERROR:Unspecified error convert basic MOB data\n");
						rc = TraverseOMFMob( OMFMob, pMob);
						if (rc != AAFRESULT_SUCCESS)
							UTLerrprintf("ERROR:Unspecified error Traversing MOB\n ");
						rc = pHeader->AppendMob(pMob);
						if (rc != AAFRESULT_SUCCESS)
							UTLerrprintf("ERROR:Unspecified error appending MOB to the file\n");
						pMob->Release();
						pMob = NULL;
						gpGlobals->nNumAAFMobs++;
					}
				}
			}
		}
	}
	OMF2::omfiIteratorDispose(OMFFileHdl, OMFMobIter);
	// Now that we have read all the metadata we can get on with
	// the actual essence (media) data

	OMFError = OMF2::omfsGetHeadObject( OMFFileHdl, &OMFHeader );
	if (OMF2::kOmfRev2x == OMFFileRev)
	{
		OMFPropertyID = OMF2::OMHEADMediaData;
		numMedia = OMF2::omfsLengthObjRefArray(OMFFileHdl, OMFHeader, OMFPropertyID); 
	}
	else
	{
		OMFPropertyID = OMF2::OMMediaData;
		numMedia = OMF2::omfsLengthObjIndex(OMFFileHdl, OMFHeader, OMFPropertyID);
	}

	if (gpGlobals->bVerboseMode)
	{
		UTLstdprintf("Found: %ld Media data objects\n", numMedia);
	}
	for (int k = 1;k <= numMedia; k++)
	{
		OMF2::omfObjIndexElement_t	objIndex;
		OMF2::omfUID_t				tmpMediaID;

		if (OMF2::kOmfRev2x == OMFFileRev)
		{
			OMFError = OMF2::omfsGetNthObjRefArray(OMFFileHdl, OMFHeader, OMFPropertyID, &OMFObject, k);
			tmpMediaID.prefix = 0;
			tmpMediaID.major = 0;
			tmpMediaID.minor = 0;
		}
		else
		{
			OMFError = OMF2::omfsGetNthObjIndex(OMFFileHdl, OMFHeader, OMFPropertyID, &objIndex, k);
			OMFObject = objIndex.Mob;
			tmpMediaID = objIndex.ID;

		}
		if (OMF2::OM_ERR_NONE == OMFError)
		{
			// Process the given Class Dictionary object.
			rc = ConvertOMFMediaDataObject(OMFObject, tmpMediaID);
		}
	}

	return rc;
}
// ============================================================================
// ConvertOMFHeader
//
//			This function is invoked by the main module and is responsible for
//			reading an OMF header, copy the relevant information to the aaf 
//			header, and convert all OMF dictionary and data definitions into 
//			the AAF Class dictionary.
//
//			
// Returns: AAFRESULT_SUCCESS if Header object is converted succesfully
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFHeader( void )
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfErr_t			OMFError = OMF2::OM_ERR_NONE;

	OMF2::omfObject_t		OMFHeader, OMFObject;

	aafInt32				numDefs, numEntries;


	OMFError = OMF2::omfsGetHeadObject( OMFFileHdl, &OMFHeader );
	if (gpGlobals->bVerboseMode)
	{
		UTLstdprintf("Processing OMF Header\n");
	}
	IncIndentLevel();
	if (OMF2::OM_ERR_NONE == OMFError)
	{
		//From the OMF header we will extract all definition Objects and Class Dictionary
		// First we process the Data definitions
		numDefs = OMF2::omfsLengthObjRefArray(OMFFileHdl, OMFHeader, OMF2::OMHEADDefinitionObjects);
		if (gpGlobals->bVerboseMode)
		{
			UTLstdprintf("%sFound: %ld Data Definitions\n", gpGlobals->indentLeader, numDefs);
		}
		for (int i = 1;i <= numDefs;i++)
		{
			OMFError = OMF2::omfsGetNthObjRefArray(OMFFileHdl, OMFHeader, OMF2::OMHEADDefinitionObjects, &OMFObject, i);
			if (OMF2::OM_ERR_NONE == OMFError)
			{
				// Process the given Data Definition object.
				rc = ConvertOMFDataDefinitionObject(OMFObject);
			}
		}
		// Now we process the class dictionary
		if (OMF2::kOmfRev2x == OMFFileRev)
		{
			numEntries = OMF2::omfsLengthObjRefArray(OMFFileHdl, OMFHeader, OMF2::OMHEADClassDictionary);
		}
		else
		{
			numEntries = OMF2::omfsLengthObjRefArray(OMFFileHdl, OMFHeader, OMF2::OMClassDictionary);
		}
		if (gpGlobals->bVerboseMode)
		{
			UTLstdprintf("Found: %ld Class Definitions\n", numEntries);
		}
		for (int j = 1;j <= numEntries; j++)
		{
			OMFError = OMF2::omfsGetNthObjRefArray(OMFFileHdl, OMFHeader, OMF2::OMHEADClassDictionary, &OMFObject, j);
			if (OMF2::OM_ERR_NONE == OMFError)
			{
				// Process the given Class Dictionary object.
				rc = ConvertOMFClassDictionaryObject(OMFObject);
			}
		}
	}
	if (AAFRESULT_SUCCESS == rc && gpGlobals->bVerboseMode)
		UTLstdprintf("Converted OMF Header values to AAF\n");

	DecIndentLevel();
	return rc;
}
// ============================================================================
// ConvertOMFDataDefinitionObject
//
//			This function is called by the ConvertOMFHeader module for each 
//			Data definition found in the header, all relevant data from the
//			OMF DDEF is extracted and converted into the AAF Class dictionary.
//			
// Returns: AAFRESULT_SUCCESS if Header object is converted succesfully
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFDataDefinitionObject( OMF2::omfObject_t obj )
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfErr_t			OMFError = OMF2::OM_ERR_NONE;
	OMF2::omfProperty_t		OMFPropertyID;
	OMF2::omfClassID_t		objClass;
	OMF2::omfInt32			nameSize = 64, idSize = 64, descSize = 120;
    aafWChar*				pwCategory = NULL;
    aafWChar*				pwDesc = NULL;
    aafWChar*				pwName = NULL;
	IAAFOperationDef*			pEffectDef = NULL;
	IAAFParameterDef*		pParameterDef = NULL;

	char					id[5];

	IncIndentLevel();
	if (OMF2::kOmfRev2x == OMFFileRev)
	{
		OMFPropertyID = OMF2::OMOOBJObjClass;
	}
	else
	{
		OMFPropertyID = OMF2::OMObjID;
	}

	OMFError = OMF2::omfsReadClassID(OMFFileHdl, obj, OMFPropertyID, objClass);
	if (OMF2::OM_ERR_NONE == OMFError && gpGlobals->bVerboseMode)
	{
		strncpy(id, objClass, 4);
		id[4] = '\0';
		UTLstdprintf("%sProcessing: %s Data Definition\n", gpGlobals->indentLeader, id );
	}
	if (strcmp(id, "EDEF") == 0)
	{
		IAAFOperationDef*	pEffectDef = NULL;
		rc = ConvertOMFEffectDefinition(obj, NULL, &pEffectDef);
		pEffectDef->Release();
	}
	DecIndentLevel();
	return rc;
}
// ============================================================================
// ConvertOMFClassDictionaryObject
//
//			This function is called by the ConvertOMFHeader module for each 
//			Class Dictionary object found in the header, all relevant data from the
//			OMF Class dictionary is extracted and converted into the AAF Class
//			dictionary.
//			
// Returns: AAFRESULT_SUCCESS if Header object is converted succesfully
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFClassDictionaryObject( OMF2::omfObject_t obj )
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfProperty_t		OMFPropertyID;

	char					id[5];

	IncIndentLevel();
	memset(id, 0, sizeof(id));
	if (OMF2::kOmfRev2x == OMFFileRev)
	{
		OMFPropertyID = OMF2::OMCLSDClass;
	}
	else
	{
		OMFPropertyID = OMF2::OMCLSDClassID;
	}

	rc = OMF2::omfsReadClassID(OMFFileHdl, obj, OMFPropertyID, id);
	if ( OMF2::OM_ERR_PROP_NOT_PRESENT == rc )
		UTLerrprintf("ERROR:*** Invalid Class Id ***\n");
	else if (gpGlobals->bVerboseMode)
		UTLstdprintf("%sProcessing: %s Class Definition\n", gpGlobals->indentLeader, id);

	DecIndentLevel();
	return rc;
}

// ============================================================================
// ConvertOMFMediaDataObject
//
//			This function is called by the ConvertOMFHeader module for each 
//			media data object found in the header
//			
// Returns: AAFRESULT_SUCCESS if Header object is converted succesfully
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFMediaDataObject( OMF2::omfObject_t obj, OMF2::omfUID_t inMediaID )
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfErr_t			OMFError = OMF2::OM_ERR_NONE;
	OMF2::omfUID_t			mediaID;
	OMF2::omfProperty_t		idProperty;
	OMF2::omfProperty_t		OMFPropertyID;
	OMF2::omfDDefObj_t		datakind;
	OMF2::omfUniqueName_t	propName;
	char					id[5];

	IAAFEssenceData*		pEssenceData = NULL;
	IAAFMob*				pMob = NULL;
	IAAFSourceMob*			pSourceMob = NULL;
	aafUID_t				mobID;
	aafBool					bConvertMedia = AAFFalse;

	IncIndentLevel();
	memset(id, 0, sizeof(id));
	if (OMF2::kOmfRev2x == OMFFileRev)
	{
		OMFPropertyID = OMF2::OMOOBJObjClass;
		OMFError = OMF2::omfsReadUID(OMFFileHdl, obj, OMF2::OMMDATMobID, &mediaID);
	}
	else
	{
		OMFPropertyID = OMF2::OMObjID;
		mediaID = inMediaID;
	}
	rc = aafMobIDFromMajorMinor(mediaID.major, mediaID.minor, &mobID);
	rc = pHeader->LookupMob(&mobID, &pMob);
	if (FAILED(rc))
		return rc;

	rc = pMob->QueryInterface(IID_IAAFSourceMob, (void **)&pSourceMob);
	OMFError = OMF2::omfsReadClassID(OMFFileHdl, obj, OMFPropertyID, id);
	if (OMF2::OM_ERR_NONE == OMFError)
	{
		if (gpGlobals->bVerboseMode)
			UTLstdprintf("%sProcessing: %s Media Data\n", gpGlobals->indentLeader, id);
		strcpy(propName, "OMFI:");
		if (strncmp(id, "TIFF", 4) == 0)
		{
			// handle TIFF media data
			IAAFTIFFData*	pTIFFData = NULL;
			rc = pDictionary->CreateInstance(&AUID_AAFTIFFData,
											 IID_IAAFTIFFData,
											 (IUnknown **)&pTIFFData);
			rc = pTIFFData->QueryInterface(IID_IAAFEssenceData, (void **)&pEssenceData);
			rc = pEssenceData->SetFileMob(pSourceMob);
			rc = pHeader->AppendEssenceData(pEssenceData);

			if (OMF2::kOmfRev2x == OMFFileRev)
			{
				idProperty = OMF2::OMTIFFData;
				OMF2::omfiDatakindLookup(OMFFileHdl, "omfi:data:Picture", &datakind, (OMF2::omfErr_t *)&rc);
			}
			else
			{
				strncat(propName, id, (size_t)4);
				strcat(propName, ":Data");
			}

			pTIFFData->Release();
			bConvertMedia = AAFTrue;
		}
		else if (strncmp(id, "AIFC", 4) == 0)
		{
			// Handle Audio (Wave data)
			IAAFAIFCData*	pAIFCData = NULL;
			rc = pDictionary->CreateInstance(&AUID_AAFAIFCData,
											 IID_IAAFAIFCData,
											 (IUnknown **)&pAIFCData);
			rc = pAIFCData->QueryInterface(IID_IAAFEssenceData, (void **)&pEssenceData);
			rc = pEssenceData->SetFileMob(pSourceMob);
			rc = pHeader->AppendEssenceData(pEssenceData);
			if (OMF2::kOmfRev2x == OMFFileRev)
			{
				idProperty = OMF2::OMAIFCData;
				OMF2::omfiDatakindLookup(OMFFileHdl, "omfi:data:Sound", &datakind, (OMF2::omfErr_t *)&rc);
			}
			else
			{
				strncat(propName, id, (size_t)4);
				strcat(propName, ":Data");
			}
			pAIFCData->Release();
			bConvertMedia = AAFTrue;
		}
		else if (strncmp(id, "WAVE", 4) == 0 )
		{
			// Handle Audio (Wave data)
			IAAFWAVEData*	pWAVEData = NULL;
			rc = pDictionary->CreateInstance(&AUID_AAFWAVEData,
											 IID_IAAFWAVEData,
											 (IUnknown **)&pWAVEData);
			rc = pWAVEData->QueryInterface(IID_IAAFEssenceData, (void **)&pEssenceData);
			rc = pEssenceData->SetFileMob(pSourceMob);
			rc = pHeader->AppendEssenceData(pEssenceData);
			if (OMF2::kOmfRev2x == OMFFileRev)
			{
				idProperty = OMF2::OMWAVEData;
				OMF2::omfiDatakindLookup(OMFFileHdl, "omfi:data:Sound", &datakind, (OMF2::omfErr_t *)&rc);
			}
			else
			{
				strncat(propName, id, (size_t)4);
				strcat(propName, ":Data");
			}
			pWAVEData->Release();
			bConvertMedia = AAFTrue;
		}
		else if (strncmp(id, "JPEG", 4) == 0)
		{
			IAAFJPEGData*	pJPEGData = NULL;

			rc = pDictionary->CreateInstance(&AUID_AAFJPEGData,
											 IID_IAAFJPEGData,
											 (IUnknown **)&pJPEGData);
			rc = pJPEGData->QueryInterface(IID_IAAFEssenceData, (void **)&pEssenceData);
			rc = pEssenceData->SetFileMob(pSourceMob);
			rc = pHeader->AppendEssenceData(pEssenceData);
			if (OMF2::kOmfRev2x == OMFFileRev)
			{
				idProperty = OMF2::OMIDATImageData;
				OMF2::omfiDatakindLookup(OMFFileHdl, "omfi:data:Picture", &datakind, (OMF2::omfErr_t *)&rc);
			}
			else
			{
				strcat(propName, "IDAT");
				strcat(propName, ":Data");
			}
			pJPEGData->Release();
			bConvertMedia = AAFTrue;
		}
		else
		{
			// rest of media (essence) NOT implemented)
			IncIndentLevel();
			UTLstdprintf("%sThis conversion is not implemented yet !!\n", gpGlobals->indentLeader);
			DecIndentLevel();
		}
		if (bConvertMedia)
		{
			void*				pBuffer = NULL;
			OMF2::omfPosition_t	OMFOffset;

			aafUInt32			numBytes;
			aafUInt32			nBlockSize;
			aafUInt32			numBytesRead;
			aafUInt32			numBytesWritten;
			aafBool				bMore = AAFFalse;
			

			// find out how big the data is 
			if (OMF2::kOmfRev2x == OMFFileRev)
				numBytes = (aafUInt32)OMF2::omfsLengthDataValue(OMFFileHdl, obj, idProperty);
			else
			{
				OMF2::omfUniqueName_t	tmpPropName;
				OMF2::omfIterHdl_t		propIter = NULL;
				OMF2::omfProperty_t		tmpProp;
				OMF2::omfType_t			tmpType;

				OMF2::omfiIteratorAlloc(OMFFileHdl, &propIter);
				OMFError = OMF2::omfiGetNextProperty(propIter, obj, &tmpProp, &tmpType);		
				while (OMF2::OM_ERR_NONE == OMFError && tmpProp)
				{
					OMF2::omfiGetPropertyName(OMFFileHdl, tmpProp, OMUNIQUENAME_SIZE, tmpPropName);
					if (strcmp(tmpPropName, propName) == 0)
					{
						idProperty = tmpProp;
						break;
					}
					OMFError = OMF2::omfiGetNextProperty(propIter, obj, &tmpProp, &tmpType);		
				}
				OMF2::omfiIteratorDispose(OMFFileHdl, propIter);

				numBytes = (aafUInt32)OMF2::omfsLengthVarLenBytes(OMFFileHdl, obj, idProperty);
			}
			if (numBytes > 0)
			{
				if (numBytes > (2 * 1048576))
				{
					nBlockSize = 2 * 1048576;		// only allocate 2 Meg
					bMore = AAFTrue;			// you going to need more than one read/write
				}
				else
				{
					nBlockSize = numBytes;
				}
				rc = UTLMemoryAlloc(nBlockSize, &pBuffer);
				OMFOffset = 0;
				do 
				{
					if (OMF2::kOmfRev2x == OMFFileRev)
						OMFError = OMF2::omfsReadDataValue( OMFFileHdl, 
									 					    obj,
														    idProperty,
															datakind,
															(OMF2::omfDataValue_t)pBuffer,
															OMFOffset,
															nBlockSize,
															&numBytesRead);
					else
						OMFError = OMF2::omfsReadVarLenBytes(OMFFileHdl,
															 obj,
															 idProperty,
															 OMFOffset,
															 nBlockSize,
															 pBuffer,
															 &numBytesRead);

					// write the data
					rc = pEssenceData->SetPosition((aafPosition_t) OMFOffset);
					rc = pEssenceData->Write(numBytesRead, (aafDataBuffer_t)pBuffer, &numBytesWritten);

					// calculate next offset
					OMFOffset += numBytesRead;

				}while (numBytes > OMFOffset );
				// Free the allocated buffer 
				UTLMemoryFree(pBuffer);
			}
		}
	}
	if (pEssenceData)
	{
		pEssenceData->Release();
		pEssenceData = NULL;
	}
	DecIndentLevel();
	pMob->Release();
	pSourceMob->Release();
	return rc;
}

// ============================================================================
// ConvertUniqueNameToAUID
//
//			This function converts an OMF uinique name or id into an AAF datadef. 
//			
// Returns: AAFRESULT_SUCCESS if datakind is converted succesfully
//
// ============================================================================
HRESULT Omf2Aaf::ConvertUniqueNameToAUID(OMF2::omfUniqueName_t datakindName,
										 aafUID_t* pDatadef)
{
	HRESULT					rc = AAFRESULT_SUCCESS;

	if (strcmp(datakindName, "omfi:effect:VideoDissolve")== 0)
		*pDatadef = kAAFEffectVideoDissolve;
	else if (strcmp(datakindName, "omfi:effect:SimpleVideoDissolve")== 0)
		*pDatadef = kAAFEffectVideoDissolve;
	else if (strcmp(datakindName, "omfi:effect:MonoAudioDissolve") == 0)
		*pDatadef = kAAFEffectMonoAudioDissolve;
	else if (strcmp(datakindName, "omfi:effect:SimpleMonoAudioDissolve") == 0)
		*pDatadef = kAAFEffectMonoAudioDissolve;
	else if(strcmp(datakindName, "omfi:effect:StereoAudioDissolve") == 0)
		*pDatadef = kAAFEffectStereoAudioDissolve;
	else if(strcmp(datakindName, "omfi:effect:SimpleStereoAudioDissolve") == 0)
		*pDatadef = kAAFEffectStereoAudioDissolve;
	else if(strcmp(datakindName, "omfi:effect:SimpleStereoAudioDissoolve") == 0)
		*pDatadef = kAAFEffectStereoAudioDissolve;
	else if(strcmp(datakindName, "omfi:effect:VideoFadeToBlack") == 0)
		*pDatadef = kAAFEffectVideoFadeToBlack;
	else if(strcmp(datakindName, "omfi:effect:SMPTEVideoWipe") == 0)
		*pDatadef = kAAFEffectSMPTEVideoWipe;
	else if(strcmp(datakindName, "omfi:effect:VideoSpeedControl") == 0)
		*pDatadef = kAAFEffectVideoSpeedControl;
	else if(strcmp(datakindName, "omfi:effect:VideoRepeat") == 0)
		*pDatadef = kAAFEffectVideoRepeat;
	else if(strcmp(datakindName, "omfi:effect:VideoFrameMask") == 0)
		*pDatadef = kAAFEffectVideoFrameToMask;
	else if(strcmp(datakindName, "omfi:effect:MonoAudioPan") == 0)
		*pDatadef = kAAFEffectMonoAudioPan;
	else if(strcmp(datakindName, "omfi:effect:MonoAudioGain") == 0)
		*pDatadef = kAAFEffectMonoAudioGain;
	else if(strcmp(datakindName, "omfi:effect:StereoAudioGain") == 0)
		*pDatadef = kAAFEffectStereoAudioGain;
	else
	{
		UTLstdprintf("Unknown name :%s Found in sequence\n", datakindName);
		*pDatadef = kAAFOperationUnknown;
	}

	return rc;
}
// ============================================================================
// ConvertOMFDatakind
//
//			This function converts an OMF datakind into an AAF datadef. 
//			
// Returns: AAFRESULT_SUCCESS if datakind is converted succesfully
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFDatakind( OMF2::omfDDefObj_t datakind, 
								   aafUID_t* pDatadef)
{
	HRESULT					rc = AAFRESULT_SUCCESS;

	OMF2::omfUniqueName_t	datakindName;
	
	rc = OMF2::omfiDatakindGetName(OMFFileHdl, datakind, 64, datakindName);
	if (strncmp("omfi:data:Picture", datakindName, strlen(datakindName))== 0)
		*pDatadef = DDEF_Video;
	else if (strncmp("omfi:data:Sound", datakindName, strlen(datakindName)) == 0)
		*pDatadef = DDEF_Audio;
	else if (strncmp("omfi:data:StereoSound", datakindName, strlen(datakindName)) == 0)
		*pDatadef = DDEF_Audio;
	else if(strncmp("omfi:data:Timecode", datakindName, strlen(datakindName)) == 0)
		*pDatadef = DDEF_Timecode;
	else if(strncmp("omfi:data:Edgecode", datakindName, strlen(datakindName)) == 0)
		*pDatadef = DDEF_Edgecode;
	else if(strncmp("omfi:data:PictureWithMatte", datakindName, strlen(datakindName)) == 0)
//		*pDatadef = kAAFEffectPictureWithMate;
		*pDatadef = DDEF_Video;			// hack for now TLK

	else
	{
		UTLstdprintf("Invalid DataDef :%s Found in sequence\n", datakindName);
		UTLerrprintf("ERROR:Invalid DataDef :%s Found in sequence\n", datakindName);
		rc = AAFRESULT_INVALID_DATADEF;
	}

	return rc;
}

// ============================================================================
// ConvertOMFMOBObject
//
//			This function is converts all the mob basic data (name, MobId, etc.) 
//			
// Returns: AAFRESULT_SUCCESS if MOB object is converted succesfully
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFMOBObject( OMF2::omfObject_t obj, IAAFMob* pMob )
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	char					sMobName[32];
	aafWChar*				pwMobName = NULL;
	aafInt32				numComments;
	aafInt32				times;

	OMF2::omfUID_t			OMFMobID;
	OMF2::omfIterHdl_t		OMFIterator;

	aafUID_t				AAFMobUID;

	IncIndentLevel();
	rc = OMF2::omfiMobGetInfo(OMFFileHdl, obj, &OMFMobID, 32, sMobName, NULL, NULL);
	if (OMF2::OM_ERR_NONE == rc)
		UTLStrAToStrW(sMobName, &pwMobName);
	else
		UTLStrAToStrW("Name NOT provided", &pwMobName);

	if (gpGlobals->bVerboseMode)
		UTLstdprintf("%sMob Name: %s\n", gpGlobals->indentLeader, sMobName);

	// Set Name
	pMob->SetName(pwMobName);

	// Convert OMF MobID into AAF AUID

	rc = aafMobIDFromMajorMinor(OMFMobID.major, OMFMobID.minor, &AAFMobUID);

	// Set MobID
	pMob->SetMobID(&AAFMobUID);

	// Set comments
	rc = OMF2::omfiMobGetNumComments(OMFFileHdl, obj, &numComments);
	if (OMF2::OM_ERR_NONE == rc && numComments > 0)
	{
		// Allocate the iterator
		OMF2::omfiIteratorAlloc(OMFFileHdl, &OMFIterator);
		for (times = 0; times < numComments; times++)
		{
			char		sCommentName[64];
			aafWChar*	pwCommentName = NULL;
			char		sCommentValue[256];
			aafWChar*	pwCommentValue = NULL;

			rc = OMF2::omfiMobGetNextComment(OMFIterator, obj, sizeof(sCommentName), sCommentName, sizeof(sCommentValue), sCommentValue);
			if (OMF2::OM_ERR_NONE ==  rc)
			{
				UTLStrAToStrW(sCommentName, &pwCommentName);
				UTLStrAToStrW(sCommentValue, &pwCommentValue);
				rc = pMob->AppendComment(pwCommentName, pwCommentValue);
				if (pwCommentName)
					UTLMemoryFree(pwCommentName);
				if (pwCommentValue)
					UTLMemoryFree(pwCommentValue);
			}
		}

		// Release the iterator
		OMF2::omfiIteratorDispose(OMFFileHdl, OMFIterator);
	}

	if (pwMobName)
		UTLMemoryFree(pwMobName);

	DecIndentLevel();
	return rc;
}

// ============================================================================
// ConvertOMFCompositionObject
//
//			This function extracts all the properties of an OMF Composition MOB,
//			Creates an AAF Composition mob object, sets its properties and 
//			appends it to the AAF header. 
//
//			
// Returns: AAFRESULT_SUCCESS if MOB object is converted succesfully
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFCompositionObject(OMF2::omfObject_t obj, 
											IAAFCompositionMob* pCompMob)
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfErr_t			OMFError = OMF2::OM_ERR_NONE;
	OMF2::omfDefaultFade_t	OMFDefaultFade;

	aafDefaultFade_t		AAFDefaultFade;
	
	// get Composition mob information
	OMFError = OMF2::omfiMobGetDefaultFade(OMFFileHdl, obj, &OMFDefaultFade);
	if (OMF2::OM_ERR_NONE == OMFError && OMFDefaultFade.valid)
	{
		gpGlobals->nNumOMFProperties++;
		AAFDefaultFade.fadeLength   = OMFDefaultFade.fadeLength;
		switch(OMFDefaultFade.fadeType)
		{
			case OMF2::kFadeNone: 
				AAFDefaultFade.fadeType = kFadeNone;
				break;
			case OMF2::kFadeLinearAmp:
				AAFDefaultFade.fadeType = kFadeLinearAmp;
				break;
			case OMF2::kFadeLinearPower:
				AAFDefaultFade.fadeType = kFadeLinearPower;
				break;
		}
		AAFDefaultFade.fadeEditUnit.numerator = OMFDefaultFade.fadeEditUnit.numerator;
		AAFDefaultFade.fadeEditUnit.denominator = OMFDefaultFade.fadeEditUnit.denominator;
	}
	else
	{
		AAFDefaultFade.fadeType = kFadeNone;
		AAFDefaultFade.fadeEditUnit.numerator = 0;
		AAFDefaultFade.fadeEditUnit.denominator = 1;
		AAFDefaultFade.fadeLength = 0;
		AAFDefaultFade.valid = AAFFalse;
		rc = AAFRESULT_SUCCESS;
	}


	// Set default fade values
	if (AAFDefaultFade.valid)
	{
		pCompMob->SetDefaultFade(AAFDefaultFade.fadeLength,
								 AAFDefaultFade.fadeType,
								 AAFDefaultFade.fadeEditUnit);
		gpGlobals->nNumAAFProperties++;
	}
	if (AAFRESULT_SUCCESS == rc && gpGlobals->bVerboseMode)
		UTLstdprintf("Converted OMF Composition MOB to AAF\n");

	return rc;
}
// ============================================================================
// TraverseOMFMob
//
//			This function converts all the mob slots in the given OMF mob object 
//			
// Returns: AAFRESULT_SUCCESS if succesfully
//
// ============================================================================
HRESULT Omf2Aaf::TraverseOMFMob( OMF2::omfObject_t obj, IAAFMob* pMob )
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfErr_t			OMFError = OMF2::OM_ERR_NONE;

	aafInt32				times;
	aafInt32				numSlots;
	aafInt32				numTracks;

	OMF2::omfIterHdl_t		OMFIterator;
	OMF2::omfMSlotObj_t		OMFSlot;
	OMF2::omfMSlotObj_t		OMFSegment;
	OMF2::omfTrackID_t		OMFTrackID;
	OMF2::omfRational_t		OMFeditRate;
	OMF2::omfPosition_t		OMFOrigin;
	char					sTrackName[32];
	aafWChar*				pwTrackName = NULL;	
	aafUInt32				physicalTrackNumber = 0;			

	IAAFMobSlot*			pMobSlot = NULL;
	IAAFSegment*			pSegment = NULL;
	IAAFComponent*			pComponent = NULL;
	IAAFTimelineMobSlot*	pTimelineMobSlot = NULL;

	OMFError = OMF2::omfiMobGetNumSlots(OMFFileHdl, obj, &numSlots);
	if (OMF2::OM_ERR_NONE != OMFError)
		numSlots = 0;

	OMFError = OMF2::omfiMobGetNumTracks(OMFFileHdl, obj, &numTracks);
	if (OMF2::OM_ERR_NONE != OMFError)
		numTracks = 0;

	IncIndentLevel();
	if (gpGlobals->bVerboseMode)
	{
		UTLstdprintf("%sFound: %ld sub tracks\n", gpGlobals->indentLeader, numSlots);
	}
	OMF2::omfiIteratorAlloc(OMFFileHdl, &OMFIterator);
	for (times = 0; times < numSlots; times++)
	{
		OMFError = OMF2::omfiMobGetNextSlot(OMFIterator, obj, NULL, &OMFSlot);
		if (OMF2::OM_ERR_NONE == OMFError)
		{
			OMFError = OMF2::omfiMobSlotGetInfo(OMFFileHdl, OMFSlot, &OMFeditRate, &OMFSegment);
			if (OMF2::omfiMobSlotIsTrack(OMFFileHdl, OMFSlot, &OMFError))
			{
				OMFError = OMF2::omfiTrackGetInfo(OMFFileHdl, obj, OMFSlot, &OMFeditRate, sizeof(sTrackName),
								sTrackName, &OMFOrigin, &OMFTrackID, &OMFSegment);
				OMF2::omfiTrackGetPhysicalNum(OMFFileHdl, OMFSlot, (OMF2::omfUInt32 *)&physicalTrackNumber);
				if (OMF2::OM_ERR_NONE == OMFError)
				{
					rc = ProcessOMFComponent(OMFSegment, &pComponent);
					rc = pComponent->QueryInterface(IID_IAAFSegment, (void **)&pSegment);
					if (pSegment)
					{
						IncIndentLevel();
						if (strlen(sTrackName) > 0)
							UTLStrAToStrW(sTrackName, &pwTrackName);
						else
							UTLStrAToStrW("", &pwTrackName);
						// OMF ONLY created timeline mob slots 
						// so that is what we going to do here
						rc = pDictionary->CreateInstance(&AUID_AAFTimelineMobSlot,
														 IID_IAAFTimelineMobSlot,
														 (IUnknown **)&pTimelineMobSlot);
						rc = pTimelineMobSlot->QueryInterface(IID_IAAFMobSlot, (void **)&pMobSlot);
						pTimelineMobSlot->SetEditRate((aafRational_t *)&OMFeditRate);
						pTimelineMobSlot->SetOrigin((aafPosition_t)OMFOrigin);
						pMobSlot->SetName(pwTrackName);
						pMobSlot->SetSlotID((aafSlotID_t)OMFTrackID);
						pMobSlot->SetSegment(pSegment);
						pMobSlot->SetPhysicalNum(physicalTrackNumber);
						rc = pMob->AppendSlot( pMobSlot );
						if (gpGlobals->bVerboseMode)
						{
							UTLstdprintf("%sConverted SlotID: %d, Name: %s\n",gpGlobals->indentLeader, (int)OMFTrackID, sTrackName);
						}
						if (pTimelineMobSlot)
						{
							pTimelineMobSlot->Release();
							pTimelineMobSlot = NULL;
						}
						if (pMobSlot)
						{
							pMobSlot->Release();
							pMobSlot = NULL;
						}
						UTLMemoryFree(pwTrackName);
						pwTrackName = NULL;
						DecIndentLevel();
					}
					if (pSegment)
					{
						pSegment->Release();
						pSegment = NULL;
					}
				}
			}
			
		}
	}

	OMF2::omfiIteratorDispose(OMFFileHdl, OMFIterator);
	DecIndentLevel();
	if (pwTrackName)
		UTLMemoryFree(pwTrackName);

	return rc;
}
// ============================================================================
// ConvertOMFSelector
//
//			This function converts an OMF Selector object and all the objects it
// contains or references.
//			
// Returns: AAFRESULT_SUCCESS if succesfully
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFSelector( OMF2::omfObject_t selector, IAAFSelector* pSelector )
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfErr_t			OMFError = OMF2::OM_ERR_NONE;

	OMF2::omfLength_t		OMFLength;
	OMF2::omfDDefObj_t		OMFDatakind;
	OMF2::omfSegObj_t		OMFSelected;
	OMF2::omfNumSlots_t		numAlternates;

	IAAFComponent*			pComponent = NULL;
	IAAFSegment*			pSegment = NULL;
	aafUID_t				datadef ;

	OMFError = OMF2::omfiSelectorGetInfo( OMFFileHdl, 
									selector,
									&OMFDatakind,
									&OMFLength,
									&OMFSelected);

	if (OMF2::OM_ERR_NONE == OMFError)
	{        
		rc = ConvertOMFDatakind(OMFDatakind, &datadef);
		pSelector->QueryInterface(IID_IAAFComponent, (void **)&pComponent);
		pComponent->SetDataDef(&datadef);
		pComponent->SetLength((aafLength_t *)&OMFLength);
		pComponent->Release();
		pComponent = NULL;
		if (gpGlobals->bVerboseMode)
			UTLstdprintf("%sProcessing Selector object of length = %ld\n", gpGlobals->indentLeader, (int)OMFLength);
		rc = ProcessOMFComponent(OMFSelected, &pComponent);
		if (SUCCEEDED(rc))
		{
			rc = pComponent->QueryInterface(IID_IAAFSegment, (void **)&pSegment);
			rc = pSelector->SetSelectedSegment(pSegment);
			OMFError = OMF2::omfiSelectorGetNumAltSlots(OMFFileHdl,
												  selector,
												  &numAlternates);
			if (OMF2::OM_ERR_NONE == OMFError && numAlternates > 0)
			{
				OMF2::omfIterHdl_t		OMFIterator;
				int						i;
				OMF2::omfSegObj_t		OMFAltSelected;

				IAAFComponent*			pAltComponent = NULL;
				IAAFSegment*			pAltSegment = NULL;

				OMF2::omfiIteratorAlloc(OMFFileHdl, &OMFIterator);
				IncIndentLevel();
				for (i = 0; i <(int)numAlternates;i++)
				{
					OMFError = OMF2::omfiSelectorGetNextAltSlot(OMFIterator,
														  selector,
														  NULL, 
														  &OMFAltSelected);
					rc = ProcessOMFComponent(OMFAltSelected, &pAltComponent);
					if (SUCCEEDED(rc))
					{
						rc = pAltComponent->QueryInterface(IID_IAAFSegment, (void **)&pAltSegment);
						rc = pSelector->AppendAlternateSegment(pAltSegment);
						pAltSegment->Release();
					}
					pAltComponent->Release();
				}
				DecIndentLevel();
				OMF2::omfiIteratorDispose(OMFFileHdl, OMFIterator);
			}
			pSegment->Release();
			pComponent->Release();
		}
	}
	return rc;
}
// ============================================================================
// ProcessOMFComponent
//
//			This function will :
//				1. Identify type of OMF Component
//				2. Create the equivalent AAF object
//				3. Convert the OMF datakind to AAF datadef of the object
//				4. Traverse the component (if needed) into its objects
//				5. return the AAF segment 
//
//	INPUTS:		OMFSegment	OMF object to be processed.
//				pSegment	Pointer to a AAF Segment Interface pointer
//
//	OUTPUTS:	pSegment	new AAF Segment	
//			
// Returns: AAFRESULT_SUCCESS if MOB object is converted succesfully
//
// ============================================================================
HRESULT Omf2Aaf::ProcessOMFComponent(OMF2::omfObject_t OMFSegment, IAAFComponent** ppComponent)
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfErr_t			OMFError = OMF2::OM_ERR_NONE;

	OMF2::omfEdgecode_t		OMFEdgecode;
	OMF2::omfLength_t		OMFLength;
	OMF2::omfDDefObj_t		OMFDatakind;
	OMF2::omfTimecode_t		OMFTimecode;
	OMF2::omfEffObj_t		OMFEffect;
	OMF2::omfPosition_t		OMFCutPoint;

	IAAFSequence*			pSequence = NULL;
	IAAFSourceClip*			pSourceClip = NULL;
	IAAFTimecode*			pTimecode = NULL;
	IAAFEdgecode*			pEdgecode = NULL;
	IAAFFiller*				pFiller = NULL;
	IAAFTransition*			pTransition = NULL;
	IAAFSelector*			pSelector = NULL;
	IAAFOperationGroup*		pEffect = NULL;
	IAAFNestedScope*		pNestedScope = NULL;
	IAAFScopeReference*		pScopeRef = NULL;
	aafEdgecode_t			edgecode;
	aafTimecode_t			timecode;
	aafUID_t				datadef ;


	// First get sequence information
	IncIndentLevel();
	if (OMF2::omfiIsASequence(OMFFileHdl, OMFSegment, &OMFError) )
	{
		rc = pDictionary->CreateInstance(&AUID_AAFSequence,
										 IID_IAAFSequence,
										 (IUnknown **)&pSequence);
		pSequence->QueryInterface(IID_IAAFComponent, (void **)ppComponent);
		if (gpGlobals->bVerboseMode)
			UTLstdprintf("%sProcessing Sequence\n", gpGlobals->indentLeader);

		ConvertOMFSequence(OMFSegment, pSequence);
		TraverseOMFSequence(OMFSegment, pSequence);
		pSequence->Release();
	}

	else if (OMF2::omfiIsASourceClip(OMFFileHdl, OMFSegment, &OMFError) )
	{
		rc = pDictionary->CreateInstance(&AUID_AAFSourceClip,
										 IID_IAAFSourceClip,
										 (IUnknown **)&pSourceClip);
		rc = ConvertOMFSourceClip(OMFSegment, pSourceClip);
		pSourceClip->QueryInterface(IID_IAAFComponent, (void **)ppComponent);
		if (gpGlobals->bVerboseMode)
			UTLstdprintf("%sProcessing SourceClip\n", gpGlobals->indentLeader);
	}
	else if (OMF2::omfiIsATimecodeClip(OMFFileHdl, OMFSegment, &OMFError) )
	{
		// Get Timecode information
		OMFError = OMF2::omfiTimecodeGetInfo(OMFFileHdl, OMFSegment, &OMFDatakind, &OMFLength, &OMFTimecode);
		timecode.startFrame = OMFTimecode.startFrame;
		timecode.drop = (aafDropType_t)OMFTimecode.drop;
		timecode.fps  = OMFTimecode.fps;
		if (gpGlobals->bVerboseMode)
		{
			UTLstdprintf("%sProcessing Timecode\n", gpGlobals->indentLeader);
			IncIndentLevel();
			UTLstdprintf("%slength\t\t: %ld\n", gpGlobals->indentLeader, (int)OMFLength);
			UTLstdprintf("%sstart Frame\t: %ld\n", gpGlobals->indentLeader, timecode.startFrame);
			if (timecode.drop == AAFTrue)
				UTLstdprintf("%sdrop\t\t: True\n", gpGlobals->indentLeader);
			else
				UTLstdprintf("%sdrop\t\t: False\n", gpGlobals->indentLeader);
			UTLstdprintf("%sFrames/second\t: %ld\n", gpGlobals->indentLeader, timecode.fps);     
			DecIndentLevel();				
		}

		rc = pDictionary->CreateInstance(&AUID_AAFTimecode,
										 IID_IAAFTimecode,
										 (IUnknown **)&pTimecode);

		pTimecode->Initialize((aafLength_t)OMFLength, &timecode);
		pTimecode->QueryInterface(IID_IAAFComponent, (void **)ppComponent);
		pTimecode->Release();
	}
	else if (OMF2::omfiIsAnEdgecodeClip(OMFFileHdl, OMFSegment, &OMFError) )
	{
		// Get edgecode data
		IAAFComponent*	pTempComp;
		OMFError = OMF2::omfiEdgecodeGetInfo(OMFFileHdl, OMFSegment, &OMFDatakind, &OMFLength, &OMFEdgecode);
		edgecode.startFrame = OMFEdgecode.startFrame;
		edgecode.filmKind = (aafFilmType_t)OMFEdgecode.filmKind;
		edgecode.codeFormat = (aafEdgeType_t)OMFEdgecode.codeFormat;
		for (int ii=0;ii<sizeof(edgecode.header);ii++)
			edgecode.header[ii] = OMFEdgecode.header[ii];

		if (gpGlobals->bVerboseMode)
		{
			UTLstdprintf("%sProcessing Edgecode\n ", gpGlobals->indentLeader);
			IncIndentLevel();
			UTLstdprintf("%slength\t\t: %ld\n", gpGlobals->indentLeader, (int)OMFLength);
			UTLstdprintf("%sstart Frame\t: %ld\n", gpGlobals->indentLeader, edgecode.startFrame);
			DecIndentLevel();				
		}
		ConvertOMFDatakind(OMFDatakind, &datadef);
		rc = pDictionary->CreateInstance(&AUID_AAFEdgecode,
										 IID_IAAFEdgecode,
										 (IUnknown **)&pEdgecode);

		pEdgecode->Create((aafLength_t)OMFLength, edgecode);
		pEdgecode->QueryInterface(IID_IAAFComponent, (void **)ppComponent);
		pEdgecode->Release();
		pTempComp = *ppComponent;
		pTempComp->SetDataDef(&datadef);
//		pTempComp->Release();
	}
	else if (OMF2::omfiIsAFiller(OMFFileHdl, OMFSegment, &OMFError) )
	{
		OMFError = OMF2::omfiFillerGetInfo(OMFFileHdl, OMFSegment, &OMFDatakind, &OMFLength);
		if (OMFLength > 0 || gpGlobals->bConvertAllObjects)
		{
			rc = ConvertOMFDatakind( OMFDatakind, &datadef);
			if (SUCCEEDED(rc))
			{
				if (gpGlobals->bVerboseMode)
				{
					UTLstdprintf("%sProcessing Filler of length: %ld\n ", gpGlobals->indentLeader, (int)OMFLength);
				}
				rc = pDictionary->CreateInstance(&AUID_AAFFiller,
												  IID_IAAFFiller,
												  (IUnknown **) &pFiller);
				rc = pFiller->Initialize( &datadef, (aafLength_t)OMFLength);
				rc = pFiller->QueryInterface(IID_IAAFComponent, (void **)ppComponent);
				pFiller->Release();
				pFiller = NULL;
			}
		}
		else
		{
			UTLstdprintf("%sZero length Filler ignored !!\n", gpGlobals->indentLeader);
		}
	}
	else if (OMF2::omfiIsAnEffect(OMFFileHdl, OMFSegment, &OMFError) )
	{
		rc = pDictionary->CreateInstance(&AUID_AAFOperationGroup, IID_IAAFOperationGroup, (IUnknown **) &pEffect);
		if (SUCCEEDED(rc))
		{
			rc = ConvertOMFEffects(OMFSegment, pEffect);
			rc = pEffect->QueryInterface(IID_IAAFComponent, (void **)ppComponent);
		}
		pEffect->Release();
		pEffect = NULL;
	}
	else if (OMF2::omfiIsATransition(OMFFileHdl, OMFSegment, &OMFError) )
	{
		IAAFOperationGroup*		pEffect = NULL;

		OMFError = OMF2::omfiTransitionGetInfo(OMFFileHdl, 
										 OMFSegment,
										 &OMFDatakind, 
										 &OMFLength, 
										 &OMFCutPoint,
										 &OMFEffect);
		if (OMF2::OM_ERR_NONE == OMFError)
		{
			if (gpGlobals->bVerboseMode)
			{
				UTLstdprintf("%sProcessing Transition of length: %ld\n ", gpGlobals->indentLeader, (int)OMFLength);
			}
			rc = ConvertOMFDatakind( OMFDatakind, &datadef);
			if (OMF2::kOmfRev2x == OMFFileRev)
			{
				rc = pDictionary->CreateInstance(&AUID_AAFTransition, IID_IAAFTransition, (IUnknown **)&pTransition);
				rc = pDictionary->CreateInstance(&AUID_AAFOperationGroup, IID_IAAFOperationGroup, (IUnknown **) &pEffect);
				rc = ConvertOMFEffects(OMFEffect, pEffect);
				if (SUCCEEDED(rc))
				{
					pTransition->Create(&datadef, (aafLength_t)OMFLength, (aafPosition_t)OMFCutPoint, pEffect);
					rc = pTransition->QueryInterface(IID_IAAFComponent, (void **)ppComponent);
					pEffect->Release();
				}
				pTransition->Release();
			}
			else
			{
				// NOTE - 1x Transitions are really effects so we process them
				// accordingly.
				OMF2::omfUniqueName_t	EffectID1x;
				OMFError = OMF2::omfsReadString(OMFFileHdl, OMFSegment, OMF2::OMCPNTEffectID, EffectID1x, OMUNIQUENAME_SIZE);
				if (OMF2::OM_ERR_NONE == OMFError)
				{
					if ( (strcmp(EffectID1x, "Blend:Dissolve") == 0) && 
						(OMF2::omfiIsSoundKind(	OMFFileHdl,	OMFDatakind, OMF2::kExactMatch, &OMFError)))
					{
						// Create a Mono Audio Dissolve Effect
						if (gpGlobals->bVerboseMode)
						{
							UTLstdprintf("%sReplacing 1.x Transition with a Audio Dissolve Effect!\n ", gpGlobals->indentLeader);
						}
					}
					else if ( (strcmp(EffectID1x, "Blend:Dissolve") == 0) && 
							  (OMF2::omfiIsPictureWithMatteKind( OMFFileHdl, OMFDatakind, OMF2::kConvertTo, &OMFError)))
					{
						// Create a Video Dissolve Effect
						if (gpGlobals->bVerboseMode)
						{
							UTLstdprintf("%sReplacing 1.x Transition with a Video Dissolve Effect!\n ", gpGlobals->indentLeader);
						}
					}
					else if ( strncmp(EffectID1x, "Wipe:SMPTE:", 11) == 0)
					{
						// Get SMPTE Vide Wipe Info and 
						// Create a SMPTE Video Wipe Effect !!
						OMF2::omfInt32	wipeNumber = 0;
						OMFError = OMF2::omfeSMPTEVideoWipeGetInfo(OMFFileHdl, OMFSegment, NULL, NULL, NULL, NULL, &wipeNumber, NULL);
						if (gpGlobals->bVerboseMode)
						{
							UTLstdprintf("%sReplacing 1.x Transition with a SMPTE Video Wipe Effect!\n ", gpGlobals->indentLeader);
						}
					}
					else
					{
						// Unknown transition - convert it to a filler
						if (gpGlobals->bVerboseMode)
						{
							UTLstdprintf("%sConverting UNKNOWN 1.x Transition to a Filler !! \n ", gpGlobals->indentLeader);
						}
						rc = pDictionary->CreateInstance(&AUID_AAFFiller,
														  IID_IAAFFiller,
														  (IUnknown **) &pFiller);
						rc = pFiller->Initialize( &datadef, (aafLength_t)OMFLength);
						rc = pFiller->QueryInterface(IID_IAAFComponent, (void **)ppComponent);
						pFiller->Release();
						pFiller = NULL;
					}
				}
				else if (OMF2::OM_ERR_PROP_NOT_PRESENT == OMFError)
				{
					// Transition has no Effect ID - cannot convert !
					if (gpGlobals->bVerboseMode)
					{
						UTLstdprintf("%sConverting UNKNOWN 1.x Transition to a Filler !! \n ", gpGlobals->indentLeader);
					}
					rc = pDictionary->CreateInstance(&AUID_AAFFiller,
													  IID_IAAFFiller,
													  (IUnknown **) &pFiller);
					rc = pFiller->Initialize( &datadef, (aafLength_t)OMFLength);
					rc = pFiller->QueryInterface(IID_IAAFComponent, (void **)ppComponent);
					pFiller->Release();
					pFiller = NULL;
				}
			}
		}
	}
	else if (OMF2::omfiIsANestedScope(OMFFileHdl, OMFSegment, &OMFError) )
	{
		rc = pDictionary->CreateInstance(&AUID_AAFNestedScope,
										  IID_IAAFNestedScope,
										  (IUnknown **) &pNestedScope);
		rc = ConvertOMFNestedScope(OMFSegment, pNestedScope);
		rc = pNestedScope->QueryInterface(IID_IAAFComponent, (void **)ppComponent);
		pNestedScope->Release();
		pNestedScope = NULL;
	}
	else if (OMF2::omfiIsAScopeRef(OMFFileHdl, OMFSegment, &OMFError) )
	{
		rc = pDictionary->CreateInstance(&AUID_AAFScopeReference,
										  IID_IAAFScopeReference,
										  (IUnknown **) &pScopeRef);
		rc = ConvertOMFScopeRef(OMFSegment, pScopeRef);
		rc = pScopeRef->QueryInterface(IID_IAAFComponent, (void **)ppComponent);
		pScopeRef->Release();
		pScopeRef = NULL;
	}
	else if (OMF2::omfiIsASelector(OMFFileHdl, OMFSegment, &OMFError) )
	{
		rc = pDictionary->CreateInstance(&AUID_AAFSelector,
										  IID_IAAFSelector,
										  (IUnknown **) &pSelector);
		IncIndentLevel();
		rc = ConvertOMFSelector(OMFSegment, pSelector);
		rc = pSelector->QueryInterface(IID_IAAFComponent, (void **)ppComponent);
		pSelector->Release();
		pSelector = NULL;
		DecIndentLevel();
	}
	else if (OMF2::omfiIsAMediaGroup(OMFFileHdl, OMFSegment, &OMFError) )
	{
		if (gpGlobals->bVerboseMode)
		{
			UTLstdprintf("%sProcessing Media Group\n ", gpGlobals->indentLeader);
		}
	}
	else
	{
		char	classID[5];

		OMFError = OMF2::omfsReadClassID(OMFFileHdl, OMFSegment, OMF2::OMOOBJObjClass,classID);
		gpGlobals->nNumUndefinedOMFObjects++;
		classID[4] = '\0';
		if (gpGlobals->bVerboseMode)
		{
			UTLerrprintf("%sERROR:*** UNKNOWN OBJECT : %s\n", gpGlobals->indentLeader, classID);
		}
	}

	DecIndentLevel();
	return rc;
}
// ============================================================================
// ConvertOMFSequence
//
//			This function converts all the properties of an OMF Sequence, into 
//			their AAF  equivalent.
//			
// Returns: AAFRESULT_SUCCESS 
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFSequence(OMF2::omfObject_t sequence, 
								   IAAFSequence* pSequence )
{
	HRESULT					rc = AAFRESULT_SUCCESS;

	OMF2::omfDDefObj_t		datakind = NULL;
	OMF2::omfLength_t		sequLength = 0;

	IAAFComponent*			pComponent = NULL;
	aafUID_t				datadef;
	
	aafBool					bValid = AAFFalse;

	// Get a pointer to a component interface
	rc = pSequence->QueryInterface(IID_IAAFComponent, (void **)&pComponent);
	if (SUCCEEDED(rc))
	{
		// Get Sequence data kind 
		rc = OMF2::omfiSequenceGetInfo(OMFFileHdl, sequence, &datakind, &sequLength);
		rc = ConvertOMFDatakind(datakind, &datadef);
		if (SUCCEEDED(rc))
			pComponent->SetDataDef(&datadef);
		pComponent->SetLength((aafLength_t *)&sequLength);
		// Next - get all properties
//		rc = ConvertOMFComponentProperties(sequence, pComponent);
		pComponent->Release();
	}

	return rc;
}
// ============================================================================
// ConvertOMFSourceClip
//
//			This function converts all the properties of an OMF Source Clip into 
//			their AAF  equivalent.
//			
// Returns: AAFRESULT_SUCCESS 
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFSourceClip(OMF2::omfObject_t sourceclip, 
									 IAAFSourceClip* pSourceClip )
{
	HRESULT					rc = AAFRESULT_SUCCESS;

	OMF2::omfDDefObj_t		datakind = NULL;
	OMF2::omfLength_t		clipLength = 0;
	OMF2::omfSourceRef_t	OMFSourceRef;
	OMF2::omfFadeType_t		OMFFadeinType;
	OMF2::omfFadeType_t		OMFFadeoutType;
	OMF2::omfBool			fadeinPresent, fadeoutPresent;

	IAAFComponent*			pComponent = NULL;
	aafUID_t				datadef;
	aafSourceRef_t			sourceRef;
	aafFadeType_t			fadeinType, fadeoutType;
	aafInt32				fadeinLen, fadeoutLen;


	rc = OMF2::omfiSourceClipGetInfo(OMFFileHdl, sourceclip, &datakind, &clipLength, &OMFSourceRef);
	if (OMF2::OM_ERR_NONE != rc)
		return rc;
	rc = OMF2::omfiSourceClipGetFade(OMFFileHdl, sourceclip, 
									 &fadeinLen, &OMFFadeinType, &fadeinPresent,
									 &fadeoutLen, &OMFFadeoutType, &fadeoutPresent);
	if (OMF2::OM_ERR_NONE == rc)
	{
		ConvertOMFDatakind(datakind, &datadef);
		rc = aafMobIDFromMajorMinor(OMFSourceRef.sourceID.major,
									OMFSourceRef.sourceID.minor,
									&sourceRef.sourceID);
		sourceRef.sourceSlotID = OMFSourceRef.sourceTrackID;
		sourceRef.startTime    = OMFSourceRef.startTime;
		fadeinType = (aafFadeType_t)OMFFadeinType;
		fadeoutType = (aafFadeType_t)OMFFadeoutType;
		rc = pSourceClip->Initialize(&datadef, (aafLength_t *)&clipLength, sourceRef);
		if (fadeinPresent || fadeoutPresent)
			rc = pSourceClip->SetFade(fadeinLen, fadeinType, fadeoutLen, fadeoutType);
	}	
	return rc;
}
// ============================================================================
// ConvertOMFComponentProperties
//
//			This function converts all the properties of an OMF Component, into 
//			their AAF  equivalent.
//			
// Returns: AAFRESULT_SUCCESS 
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFComponentProperties(OMF2::omfObject_t component, 
											 IAAFComponent* pComponent )
{
	HRESULT					rc = AAFRESULT_SUCCESS;

	OMF2::omfProperty_t		Property;
	OMF2::omfUniqueName_t	propertyName;
	OMF2::omfIterHdl_t		propertyIterator = NULL;
	OMF2::omfType_t			propertyType = OMF2::OMNoType;
	OMF2::omfDDefObj_t		datakind = NULL;

	IncIndentLevel();
	OMF2::omfiIteratorAlloc(OMFFileHdl, &propertyIterator);
	OMF2::omfiGetNextProperty(propertyIterator, component, &Property, &propertyType);
	while((OMF2::OM_ERR_NONE == rc) &&Property)
	{
		switch (Property)
		{
			case OMF2::OMOOBJObjClass:
			case OMF2::OMCPNTLength:
			case OMF2::OMCPNTDatakind:
			case OMF2::OMSEQUComponents:
				gpGlobals->nNumOMFProperties++;
				gpGlobals->nNumAAFProperties++;
				break;
			case OMF2::OMCPNTTrackKind:
			case OMF2::OMCPNTEditRate:
			case OMF2::OMCPNTName:
			case OMF2::OMCPNTEffectID:
			case OMF2::OMCPNTAttributes:
			default:
				gpGlobals->nNumOMFProperties++;
				OMF2::omfiGetPropertyName(OMFFileHdl, Property, 64, propertyName);
				UTLstdprintf("%sComponent Property NOT converted : %s\n", gpGlobals->indentLeader, propertyName);
				break;
		}
		rc = OMF2::omfiGetNextProperty(propertyIterator, component, &Property, &propertyType);
	}
	rc = OMF2::omfiIteratorDispose(OMFFileHdl, propertyIterator);
	DecIndentLevel();
	return rc;
}
// ============================================================================
// TraverseOMFSequence
//
//			This function reads all components the of an OMF Sequence,
//			sets the equivalent AAF properties 
//			
// Returns: AAFRESULT_SUCCESS if MOB object is converted succesfully
//
// ============================================================================
HRESULT Omf2Aaf::TraverseOMFSequence(OMF2::omfObject_t sequence, IAAFSequence* pSequence )
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfErr_t			OMFError = OMF2::OM_ERR_NONE;

	OMF2::omfIterHdl_t		componentIterator = NULL;
	OMF2::omfPosition_t		sequPos;
	OMF2::omfCpntObj_t		sequComponent;

	IAAFComponent*			pComponent = NULL;

	aafInt32				numComponents = 0;
	aafInt32				cpntCount;

	OMFError = OMF2::omfiSequenceGetNumCpnts(OMFFileHdl, sequence, &numComponents);
	if (numComponents > 0)
	{
		OMFError = OMF2::omfiIteratorAlloc(OMFFileHdl, &componentIterator);
		if (OMF2::OM_ERR_NONE == OMFError)
		{
			for (cpntCount = 0; cpntCount < numComponents; cpntCount++)
			{   
				OMFError = OMF2::omfiSequenceGetNextCpnt(componentIterator, sequence, NULL, 
												   &sequPos, &sequComponent); 
				if (OMF2::OM_ERR_NONE == OMFError)
				{
					rc = ProcessOMFComponent(sequComponent, &pComponent);
					if (pComponent)
					{
						rc = pSequence->AppendComponent(pComponent);
						pComponent->Release();
						pComponent = NULL;
					}
				}
			}
		}
		OMF2::omfiIteratorDispose(OMFFileHdl, componentIterator);
	}

	if (pComponent)
		pComponent->Release();
	return rc;
}
// ============================================================================
// ConvertOMFMasterMob
//
//			This function extracts all the properties of an OMF Master MOB,
//			sets the equivalent AAF properties and 
//			
// Returns: AAFRESULT_SUCCESS if MOB object is converted succesfully
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFMasterMob(OMF2::omfObject_t obj,
									IAAFMasterMob* pMasterMob )
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	
	if (gpGlobals->bVerboseMode)
		UTLstdprintf("Converting OMF Master MOB to AAF\n");
	return rc;
}
// ============================================================================
// ConvertOMFLocator
//
//			This function extracts all the properties of an OMF Locator,
//			sets the equivalent AAF properties
//			
// Returns: AAFRESULT_SUCCESS if MOB object is converted succesfully
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFLocator(OMF2::omfObject_t obj,
									IAAFEssenceDescriptor* pEssenceDesc)
{
	HRESULT					rc = AAFRESULT_SUCCESS;

	OMF2::omfObject_t		OMFLocator;
	OMF2::omfIterHdl_t		locatorIter;
	OMF2::omfClassID_t		locType;
	char					locatorPath[128];

    aafWChar*				pwLocatorPath;
	IAAFNetworkLocator*		pNetworkLocator = NULL;
	IAAFLocator*			pLocator = NULL;

	rc = OMF2::omfiIteratorAlloc(OMFFileHdl, &locatorIter);
	rc = OMF2::omfmMobGetNextLocator(locatorIter, obj, &OMFLocator);
	while((rc == OMF2::OM_ERR_NONE) &&(OMFLocator != NULL))
	{
		rc = OMF2::omfmLocatorGetInfo(OMFFileHdl, OMFLocator, locType, 128, locatorPath);
		if (SUCCEEDED(rc))
		{
			UTLStrAToStrW(locatorPath, &pwLocatorPath);
			rc = pDictionary->CreateInstance(&AUID_AAFNetworkLocator,
											 IID_IAAFNetworkLocator,
											 (IUnknown **)&pNetworkLocator);
			rc = pNetworkLocator->QueryInterface(IID_IAAFLocator, (void **)&pLocator);
			if (SUCCEEDED(rc))
			{
				rc = pLocator->SetPath(pwLocatorPath);
				rc = pEssenceDesc->AppendLocator(pLocator);
				pNetworkLocator->Release();
				pNetworkLocator = NULL;
				pLocator->Release();
				pLocator = NULL;
				if (gpGlobals->bVerboseMode)
					UTLstdprintf("%sAdded a Network locator to the Essence Descriptor\n", gpGlobals->indentLeader);
			}
			UTLMemoryFree(pwLocatorPath);
		}
		rc = OMF2::omfmMobGetNextLocator(locatorIter, obj, &OMFLocator);
	}

	rc = OMF2::omfiIteratorDispose(OMFFileHdl, locatorIter);

	if (pLocator)
		pLocator->Release();

	return rc;
}
// ============================================================================
// ConvertOMFCDCIDescriptorLocator
//
//			This function extracts all the properties of an CDCI Image Descriptor,
//			sets the equivalent AAF properties
//			
// Returns: AAFRESULT_SUCCESS if MOB object is converted succesfully
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFCDCIDescriptorLocator(OMF2::omfObject_t mediaDescriptor,
									IAAFCDCIDescriptor* pAAFDescriptor)
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfFrameLayout_t	frameLayout;
	OMF2::omfProperty_t		omCDCIComponentWidth;
	OMF2::omfProperty_t		omCDCIHorizontalSubsampling;
	OMF2::omfProperty_t		omCDCIColorSiting;
	OMF2::omfProperty_t		omCDCIBlackReferenceLevel;
	OMF2::omfProperty_t		omCDCIWhiteReferenceLevel;
	OMF2::omfProperty_t		omCDCIColorRange;
	OMF2::omfProperty_t		omCDCIPaddingBits;
	OMF2::omfPosition_t		zeroPos = 0;
	OMF2::omfPosition_t		fourPos = 0;

	IAAFDigitalImageDescriptor*	pDigImageDesc = NULL;

	aafUInt32			Height = 0;
	aafUInt32			Width = 0;
	aafInt32			XOffset = 0;
	aafInt32			YOffset = 0;
	aafInt32			alphaTransparency = 0;
	aafInt32			alignmentFactor;

	aafFrameLayout_t	AAFFrameLayout;
	aafRational_t		gamma;
	aafRational_t		aspectRatio;

	aafInt32			componentWidth = 0;
	aafUInt32			horizontalSubsampling = 0;
	aafColorSiting_t	colorSiting = kCoSiting;
	aafUInt32			blackReferenceLevel = 0;
	aafUInt32			whiteReferenceLevel = 0;
	aafUInt32			colorRange = 0;
	aafInt16			paddingBits = 0;
	aafInt32			videoLineMap[2];
						
	rc = pAAFDescriptor->QueryInterface(IID_IAAFDigitalImageDescriptor, (void **)&pDigImageDesc);
	if (FAILED(rc))
		return rc;
	// Get Digital Image properties and set them
	rc = OMF2::omfsReadUInt32(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDStoredHeight, &Height);
	if (rc != AAFRESULT_SUCCESS)
	{
		rc = OMF2::omfsReadInt32(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDStoredHeight, (OMF2::omfInt32 *)&Height);
		if (rc != AAFRESULT_SUCCESS)
		{
			Height = 0;
		}
	}
	else
		Height = 0;
	rc = OMF2::omfsReadUInt32(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDStoredWidth, &Width);
	if (rc != AAFRESULT_SUCCESS)
	{
		rc = OMF2::omfsReadInt32(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDStoredWidth, (OMF2::omfInt32 *)&Width);
		if (rc != AAFRESULT_SUCCESS)
		{
			Width = 0;
		}
	}
	else
		Width = 0;
	rc = pDigImageDesc->SetStoredView(Height, Width);

	rc = OMF2::omfsReadUInt32(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDSampledHeight, &Height);
	if (rc != AAFRESULT_SUCCESS)
	{
		rc = OMF2::omfsReadInt32(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDSampledHeight, (OMF2::omfInt32 *)&Height);
		if (rc != AAFRESULT_SUCCESS)
		{
			Height = 0;
		}
	}
	else
		Height = 0;
	rc = OMF2::omfsReadUInt32(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDSampledWidth, &Width);
	if (rc != AAFRESULT_SUCCESS)
	{
		rc = OMF2::omfsReadInt32(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDSampledWidth, (OMF2::omfInt32 *)&Width);
		if (rc != AAFRESULT_SUCCESS)
		{
			Width = 0;
		}
	}
	else
		Width = 0;
	rc = OMF2::omfsReadInt32(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDSampledXOffset, &XOffset);
	rc = OMF2::omfsReadInt32(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDSampledYOffset, &YOffset);
	rc = pDigImageDesc->SetSampledView(Height, Width, XOffset, YOffset);

	rc = OMF2::omfsReadUInt32(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDDisplayHeight, &Height);
	if (rc != AAFRESULT_SUCCESS)
	{
		rc = OMF2::omfsReadInt32(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDDisplayHeight, (OMF2::omfInt32 *)&Height);
		if (rc != AAFRESULT_SUCCESS)
		{
			Height = 0;
		}
	}
	else
		Height = 0;
	rc = OMF2::omfsReadUInt32(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDDisplayWidth, &Width);
	if (rc != AAFRESULT_SUCCESS)
	{
		rc = OMF2::omfsReadInt32(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDDisplayWidth, (OMF2::omfInt32 *)&Width);
		if (rc != AAFRESULT_SUCCESS)
		{
			Width = 0;
		}
	}
	else
		Width = 0;
	rc = OMF2::omfsReadInt32(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDDisplayXOffset, &XOffset);
	rc = OMF2::omfsReadInt32(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDDisplayYOffset, &YOffset);
	rc = pDigImageDesc->SetDisplayView(Height, Width, XOffset, YOffset);

	rc = OMF2::omfsReadLayoutType(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDFrameLayout, &frameLayout);
	if (frameLayout > 0)
		AAFFrameLayout = (aafFrameLayout_t)(frameLayout-1);
	else
		AAFFrameLayout = (aafFrameLayout_t)0;

	rc = pDigImageDesc->SetFrameLayout(AAFFrameLayout);
	rc = OMF2::omfsReadInt32(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDAlphaTransparency, &alphaTransparency);
	rc = pDigImageDesc->SetAlphaTransparency((aafAlphaTransparency_t)alphaTransparency);

	rc = OMF2::omfsReadRational(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDGamma, (OMF2::omfRational_t *)&gamma);
	rc = pDigImageDesc->SetGamma(gamma);

	rc = OMF2::omfsReadRational(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDImageAspectRatio, (OMF2::omfRational_t *)&aspectRatio);
	rc = pDigImageDesc->SetImageAspectRatio(aspectRatio);

	rc = OMF2::omfsReadInt32(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDFieldAlignment, &alignmentFactor);
	rc = pDigImageDesc->SetImageAlignmentFactor(alignmentFactor);

	rc  = OMF2::OMReadProp(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDVideoLineMap, 
						   zeroPos, OMF2::kSwabIfNeeded, OMF2::OMInt32Array,
						   sizeof(aafInt32), &videoLineMap[0]); 

	omfsCvtInt32toPosition(sizeof(aafInt32), fourPos);
	rc  = OMF2::OMReadProp(OMFFileHdl, mediaDescriptor, OMF2::OMDIDDVideoLineMap, 
						   fourPos, OMF2::kSwabIfNeeded, OMF2::OMInt32Array,
						   sizeof(aafInt32), &videoLineMap[1]); 

	rc = pDigImageDesc->SetVideoLineMap( (sizeof(videoLineMap)/sizeof(aafInt32)), videoLineMap);
	pDigImageDesc->Release();
	pDigImageDesc = NULL;

	// To get the CDCI codec related properties we first reister them in OMF
	rc = OMF2::omfsRegisterDynamicProp(OMFSession, OMF2::kOmfTstRevEither, 
									   "ComponentWidth", OMClassCDCI, 
									   OMF2::OMVersionType, OMF2::kPropRequired, 
									   &omCDCIComponentWidth);
	rc = OMF2::omfsRegisterDynamicProp(OMFSession, OMF2::kOmfTstRevEither, 
									   "HorizontalSubsampling", OMClassCDCI, 
									   OMF2::OMBoolean, OMF2::kPropRequired, 
									   &omCDCIHorizontalSubsampling);
	rc = OMF2::omfsRegisterDynamicProp(OMFSession, OMF2::kOmfTstRevEither, 
									   "ColorSiting", OMClassCDCI, 
									   OMF2::OMBoolean, OMF2::kPropRequired, 
									   &omCDCIColorSiting);
	rc = OMF2::omfsRegisterDynamicProp(OMFSession, OMF2::kOmfTstRevEither, 
									   "BlackReferenceLevel", OMClassCDCI, 
									   OMF2::OMInt32, OMF2::kPropRequired, 
									   &omCDCIBlackReferenceLevel);
	rc = OMF2::omfsRegisterDynamicProp(OMFSession, OMF2::kOmfTstRevEither, 
									   "WhiteReferenceLevel", OMClassCDCI, 
									   OMF2::OMInt32, OMF2::kPropRequired, 
									   &omCDCIWhiteReferenceLevel);
	rc = OMF2::omfsRegisterDynamicProp(OMFSession, OMF2::kOmfTstRevEither, 
									   "ColorRange", OMClassCDCI, 
									   OMF2::OMInt32, OMF2::kPropRequired, 
									   &omCDCIColorRange);
	rc = OMF2::omfsRegisterDynamicProp(OMFSession, OMF2::kOmfTstRevEither, 
									   "PaddingBits", OMClassCDCI, 
									   OMF2::OMInt32, OMF2::kPropRequired, 
									   &omCDCIPaddingBits);
	// Next we read the values
	rc  = OMF2::omfsReadInt32(OMFFileHdl, mediaDescriptor, omCDCIComponentWidth, &componentWidth); 
	rc  = OMF2::omfsReadUInt32(OMFFileHdl, mediaDescriptor, omCDCIHorizontalSubsampling, &horizontalSubsampling); 
	rc = OMF2::OMReadProp(OMFFileHdl, mediaDescriptor, omCDCIHorizontalSubsampling, 
						  zeroPos, OMF2::kSwabIfNeeded, OMF2::OMColorSitingType,
						  sizeof(colorSiting), (void *)&(colorSiting));
	rc  = OMF2::omfsReadUInt32(OMFFileHdl, mediaDescriptor, omCDCIBlackReferenceLevel, &blackReferenceLevel); 
	rc  = OMF2::omfsReadUInt32(OMFFileHdl, mediaDescriptor, omCDCIWhiteReferenceLevel, &whiteReferenceLevel); 
	rc  = OMF2::omfsReadUInt32(OMFFileHdl, mediaDescriptor, omCDCIColorRange, &colorRange); 
	rc  = OMF2::OMReadProp(OMFFileHdl, mediaDescriptor, omCDCIPaddingBits, 
						   zeroPos, OMF2::kSwabIfNeeded, OMF2::OMInt16,
						   sizeof(paddingBits), &paddingBits); 
	// Now set the EssenceDescriptor
	rc = pAAFDescriptor->SetComponentWidth(componentWidth);
	rc = pAAFDescriptor->SetHorizontalSubsampling(horizontalSubsampling);
	rc = pAAFDescriptor->SetColorSiting(colorSiting);
	rc = pAAFDescriptor->SetBlackReferenceLevel(blackReferenceLevel);
	rc = pAAFDescriptor->SetWhiteReferenceLevel(whiteReferenceLevel);
	rc = pAAFDescriptor->SetColorRange(colorRange);
	rc = pAAFDescriptor->SetPaddingBits(paddingBits);

	return rc;
}
// ============================================================================
// ConvertOMFSourceMob
//
//			This function extracts all the properties of an OMF Source MOB,
//			sets the equivalent AAF properties and 
//			
// Returns: AAFRESULT_SUCCESS if MOB object is converted succesfully
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFSourceMob(OMF2::omfObject_t obj,
									IAAFSourceMob* pSourceMob)
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfErr_t			OMFError = OMF2::OM_ERR_NONE;

	OMF2::omfObject_t		mediaDescriptor;
	OMF2::omfInt32			numLocators = 0;

	IAAFEssenceDescriptor*	pEssenceDesc = NULL;
	IAAFTapeDescriptor*		pTapeDesc = NULL;
	IAAFFileDescriptor*		pFileDesc = NULL;
	IAAFTIFFDescriptor*		pTiffDesc = NULL;
	IAAFWAVEDescriptor*		pWAVEDesc = NULL;
	IAAFAIFCDescriptor*		pAifcDesc = NULL;
	IAAFCDCIDescriptor*		pCDCIDesc = NULL;

	aafLength_t				length = 0;
	aafRational_t			sampleRate;


	if (gpGlobals->bVerboseMode)
		UTLstdprintf("Converting OMF Source MOB to AAF\n");


	IncIndentLevel();
	OMFError = OMF2::omfmMobGetMediaDescription(OMFFileHdl, obj, &mediaDescriptor);
	if (OMFError == OMF2::OM_ERR_NONE)
	{
		if ( OMF2::omfsIsTypeOf(OMFFileHdl, mediaDescriptor, OMClassMDFL, &OMFError))
		{
			// File Source Mob
			if ( OMF2::omfsIsTypeOf(OMFFileHdl, mediaDescriptor, OMClassTIFD, &OMFError) )
			{
				// TIFF File Descriptor
				OMF2::omfBool			IsContiguous, IsUniform;
				OMF2::omfInt32			leadingLines, trailingLines;
				char					summary[1024];
				OMF2::omfJPEGTableID_t	data;
				OMF2::omfDDefObj_t		datakind;
				
				aafUInt32				bytesRead = 0;

				rc = pDictionary->CreateInstance(&AUID_AAFTIFFDescriptor,
												 IID_IAAFTIFFDescriptor,
												 (IUnknown **)&pTiffDesc);
				if (SUCCEEDED( rc) )
				{

					OMFError = OMF2::omfsReadBoolean( OMFFileHdl,
												mediaDescriptor,
												OMF2::OMTIFDIsContiguous, &IsContiguous);
					OMFError = OMF2::omfsReadBoolean( OMFFileHdl,
												mediaDescriptor,
												OMF2::OMTIFDIsUniform,
												&IsUniform);
					OMFError = OMF2::omfsReadInt32(OMFFileHdl,
											 mediaDescriptor,
											 OMF2::OMTIFDLeadingLines, 
											 &leadingLines);
					OMFError = OMF2::omfsReadInt32(OMFFileHdl,
											 mediaDescriptor,
											 OMF2::OMTIFDTrailingLines, 
											 &trailingLines);
					OMFError = OMF2::omfsReadJPEGTableIDType( OMFFileHdl,
														mediaDescriptor,
														OMF2::OMTIFDJPEGTableID, 
														&data);
					if (OMF2::kOmfRev2x == OMFFileRev)
					{
						OMF2::omfiDatakindLookup(OMFFileHdl, "omfi:data:Picture", &datakind, (OMF2::omfErr_t *)&rc);
						OMFError = OMF2::omfsReadDataValue(OMFFileHdl,
													 mediaDescriptor,
													 OMF2::OMTIFDSummary,
													 datakind,
													 summary,
													 0,
													 sizeof(summary),
													 &bytesRead);
					}
					else
					{
						OMFError = OMF2::omfsReadVarLenBytes(OMFFileHdl,
													   mediaDescriptor,
													   OMF2::OMTIFDSummary,
													   0,
													   sizeof(summary),
													   summary,
													   &bytesRead);
					}

					rc = pTiffDesc->QueryInterface(IID_IAAFEssenceDescriptor, (void **)&pEssenceDesc);
					pTiffDesc->SetIsUniform((aafBool)IsUniform);
					pTiffDesc->SetIsContiguous((aafBool)IsContiguous);
					pTiffDesc->SetLeadingLines((aafInt32)leadingLines);
					pTiffDesc->SetTrailingLines((aafInt32)trailingLines);
					pTiffDesc->SetJPEGTableID((aafJPEGTableID_t)data);
					if (bytesRead > 0)
						pTiffDesc->SetSummary((aafUInt32)bytesRead, (aafDataValue_t) summary);
					pSourceMob->SetEssenceDescriptor(pEssenceDesc);
					if (gpGlobals->bVerboseMode)
						UTLstdprintf("%sAdded a TIFF Essence Descriptor to a Source MOB\n", gpGlobals->indentLeader);
					if (pTiffDesc)
					{
						pTiffDesc->Release();
						pTiffDesc = NULL;
					}
				}
			}
			else if ( OMF2::omfsIsTypeOf(OMFFileHdl, mediaDescriptor, OMClassWAVD, &OMFError) )
			{
				// WAVE File Descriptor
				void*					pSummary = NULL;
				OMF2::omfDDefObj_t		datakind;
				
				aafUInt32				bytesRead;
				aafUInt32				numBytes;

				rc = pDictionary->CreateInstance(&AUID_AAFWAVEDescriptor,
												 IID_IAAFWAVEDescriptor,
												 (IUnknown **)&pWAVEDesc);
				if (SUCCEEDED( rc) )
				{
					OMF2::omfiDatakindLookup(OMFFileHdl, "omfi:data:Sound", &datakind, &OMFError);
					numBytes = (aafUInt32)OMF2::omfsLengthDataValue(OMFFileHdl, mediaDescriptor, OMF2::OMWAVDSummary);
					UTLMemoryAlloc(numBytes, &pSummary);
					OMFError = OMF2::omfsReadDataValue(OMFFileHdl,
												 mediaDescriptor,
												 OMF2::OMWAVDSummary,
												 datakind,
												 (OMF2::omfDataValue_t)pSummary,
												 0,
												 numBytes,
												 &bytesRead);
					rc = pWAVEDesc->SetSummary(bytesRead, (aafDataValue_t) pSummary);

					rc = pWAVEDesc->QueryInterface(IID_IAAFEssenceDescriptor, (void **)&pEssenceDesc);
					pSourceMob->SetEssenceDescriptor(pEssenceDesc);
					if (gpGlobals->bVerboseMode)
						UTLstdprintf("%sAdded a Wave Essence Descriptor to a Source MOB\n", gpGlobals->indentLeader);
					if (pWAVEDesc)
					{
						pWAVEDesc->Release();
						pWAVEDesc = NULL;
					}
					UTLMemoryFree(pSummary);
				}
			}
			else if ( OMF2::omfsIsTypeOf(OMFFileHdl, mediaDescriptor, OMClassAIFD, &OMFError) )
			{
				// AIFC Audio File Descriptor
				char					summary[1024];
				OMF2::omfDDefObj_t		datakind;
				
				aafUInt32				bytesRead;
				aafUInt32				numBytes;

				rc = pDictionary->CreateInstance(&AUID_AAFAIFCDescriptor,
												 IID_IAAFAIFCDescriptor,
												 (IUnknown **)&pAifcDesc);
				if (SUCCEEDED( rc) )
				{
					if (OMF2::kOmfRev2x == OMFFileRev)
					{
						OMF2::omfiDatakindLookup(OMFFileHdl, "omfi:data:Sound", &datakind, &OMFError);
						numBytes = (aafUInt32)OMF2::omfsLengthDataValue(OMFFileHdl, mediaDescriptor, OMF2::OMAIFDSummary);
						OMFError = OMF2::omfsReadDataValue(OMFFileHdl,
													 mediaDescriptor,
													 OMF2::OMAIFDSummary,
													 datakind,
													 (OMF2::omfDataValue_t)summary,
													 0,
													 numBytes,
													 &bytesRead);
					}
					else
					{
						OMFError = OMF2::omfsReadVarLenBytes(OMFFileHdl,
													   mediaDescriptor,
													   OMF2::OMAIFDSummary,
													   0,
													   sizeof(summary),
													   summary,
													   &bytesRead);
					}
					rc = pAifcDesc->SetSummary(bytesRead, (aafDataValue_t) summary);

					rc = pAifcDesc->QueryInterface(IID_IAAFEssenceDescriptor, (void **)&pEssenceDesc);
					pSourceMob->SetEssenceDescriptor(pEssenceDesc);
					if (gpGlobals->bVerboseMode)
						UTLstdprintf("%sAdded a AIFC Essence Descriptor to a Source MOB\n", gpGlobals->indentLeader);
					if (pAifcDesc)
					{
						pAifcDesc->Release();
						pAifcDesc = NULL;
					}
				}
			}
			else if ( OMF2::omfsIsTypeOf(OMFFileHdl, mediaDescriptor, OMClassCDCI, &OMFError) )
			{
				rc = pDictionary->CreateInstance(&AUID_AAFCDCIDescriptor,
												 IID_IAAFCDCIDescriptor,
												 (IUnknown **)&pCDCIDesc);
				if (SUCCEEDED( rc) )
				{
					rc = ConvertOMFCDCIDescriptorLocator(mediaDescriptor, pCDCIDesc);
					rc = pCDCIDesc->QueryInterface(IID_IAAFEssenceDescriptor, (void **)&pEssenceDesc);
					pSourceMob->SetEssenceDescriptor(pEssenceDesc);
					if (pCDCIDesc)
					{
						pCDCIDesc->Release();
						pCDCIDesc = NULL;
					}
				}
			}
			else
			{
				OMF2::omfClassID_t	objClass;
				char				id[5];

				OMFError = OMF2::omfsReadClassID(OMFFileHdl, mediaDescriptor, OMF2::OMOOBJObjClass, objClass);
				strncpy(id, objClass, 4);
				id[4] = '\0';
				if (gpGlobals->bVerboseMode)
					UTLstdprintf("%sCannot translate this Media File Descriptor: %s\n", gpGlobals->indentLeader, id);
				UTLerrprintf("%sERROR:Cannot translate this Media File Descriptor: %s\n", gpGlobals->indentLeader, id) ;
				gpGlobals->nNumUndefinedOMFObjects++;
			}
			// Retrieve and set generic File Descriptor properties.
			rc = pEssenceDesc->QueryInterface(IID_IAAFFileDescriptor, (void **) &pFileDesc);
			if (SUCCEEDED(rc))
			{
				OMFError = OMF2::omfsReadRational(OMFFileHdl, mediaDescriptor, OMF2::OMMDFLSampleRate, (OMF2::omfRational_t *)&sampleRate);
				if (OMFError == OMF2::OM_ERR_NONE)
					rc = pFileDesc->SetSampleRate(&sampleRate);

				OMFError = OMF2::omfsReadLength(OMFFileHdl, mediaDescriptor, OMF2::OMMDFLLength, (OMF2::omfLength_t *)&length);
				if (OMFError == OMF2::OM_ERR_NONE)
					rc = pFileDesc->SetLength(length);

				pFileDesc->Release();
				pFileDesc = NULL;
			}
		}
		else
		{
			// Physical Source Mob
			if ( OMF2::omfsIsTypeOf(OMFFileHdl, mediaDescriptor, OMClassMDTP, &OMFError) ||
				 OMF2::omfsIsTypeOf(OMFFileHdl, mediaDescriptor, OMClassMDES, &OMFError) )
			{
				// Tape Media descriptor
				OMF2::omfTapeCaseType_t		formFactor;
				OMF2::omfVideoSignalType_t	videoSignal;
				OMF2::omfTapeFormatType_t	tapeFormat;
				OMF2::omfLength_t			length;
				char						manufacturer[64], model[64];
				OMF2::omfInt32				manuSize, modelSize;

				aafWChar*					pwManufacturer = NULL;
				aafWChar*					pwModel = NULL;

				rc = pDictionary->CreateInstance(&AUID_AAFTapeDescriptor,
												 IID_IAAFTapeDescriptor,
												 (IUnknown **)&pTapeDesc);
				if (SUCCEEDED( rc) )
				{
					manuSize = modelSize = 64;
					OMFError = OMF2::omfmTapeMobGetDescriptor(OMFFileHdl, 
															obj,
															&formFactor,
															&videoSignal,
															&tapeFormat,
															&length,
															manuSize, manufacturer,
															modelSize, model);
					if (OMF2::OM_ERR_NONE == OMFError)
					{
						if (strlen(manufacturer) > 0)
						{
							UTLStrAToStrW(manufacturer, &pwManufacturer);
						}
						if (strlen(model) > 0)
						{
							UTLStrAToStrW(model, &pwModel);
						}
					}
					else
					{
						UTLStrAToStrW("Not provided", &pwManufacturer);
						UTLStrAToStrW("Not provided", &pwModel);
						formFactor = OMF2::kTapeCaseNull;
						videoSignal = OMF2::kVideoSignalNull;
						tapeFormat = OMF2::kTapeFormatNull;
						length = 0;
					}
					pTapeDesc->SetTapeModel(pwModel);
					pTapeDesc->SetTapeManufacturer(pwManufacturer);
					pTapeDesc->SetTapeFormFactor((aafTapeCaseType_t)formFactor);
					pTapeDesc->SetSignalType((aafVideoSignalType_t)videoSignal);
					pTapeDesc->SetTapeFormat((aafTapeFormatType_t)tapeFormat);
					pTapeDesc->SetTapeLength((aafLength_t)length);
					rc = pTapeDesc->QueryInterface(IID_IAAFEssenceDescriptor, (void **)&pEssenceDesc);
					pSourceMob->SetEssenceDescriptor(pEssenceDesc);
					if (gpGlobals->bVerboseMode)
						UTLstdprintf("%sAdded a Tape Essence Descriptor to a Source MOB\n", gpGlobals->indentLeader);
					if (pTapeDesc)
					{
						pTapeDesc->Release();
						pTapeDesc = NULL;
					}
				}
			}
			else if ( OMF2::omfsIsTypeOf(OMFFileHdl, mediaDescriptor, OMClassMDFM, &OMFError))
			{
				// Film Media descriptor
				if (gpGlobals->bVerboseMode)
					UTLstdprintf("%sFilm media descriptor NOT Implemented yet\n", gpGlobals->indentLeader);
				UTLerrprintf("%sERROR:Film media descriptor NOT Implemented yet\n", gpGlobals->indentLeader);
			}
			else
			{
				OMF2::omfClassID_t	objClass;
				OMF2::omfClassID_t	objSuperClass;
				OMF2::omfProperty_t	propertyID;
				OMF2::omfBool		found;
				char				id[5];
				char				superID[5];

				if (OMF2::kOmfRev2x == OMFFileRev)
				{
					propertyID = OMF2::OMOOBJObjClass;
				}
				else
				{
					propertyID = OMF2::OMObjID;
				}
				OMFError = OMF2::omfsReadClassID(OMFFileHdl, mediaDescriptor, propertyID, objClass);
				strncpy(id, objClass, 4);
				id[4] = '\0';
				if (gpGlobals->bVerboseMode)
					UTLstdprintf("%sCannot translate this Physical Media Descriptor: %s\n", gpGlobals->indentLeader, id);
				UTLerrprintf("%sERROR:Cannot translate this Physical Media Descriptor: %s\n", gpGlobals->indentLeader, id) ;
				gpGlobals->nNumUndefinedOMFObjects++;
				// Here we could check for a Dictionary entry to translate this class !!
				// For now lets try to figure out what kind of descriptor can we do.
				rc = OMF2::omfsClassFindSuperClass(OMFFileHdl, objClass, objSuperClass, &found);
				if (SUCCEEDED(rc) && found)
				{
					// we found the class 
					strncpy(superID, objSuperClass, 4);
					superID[4] = '\0';
					if (gpGlobals->bVerboseMode)
						UTLstdprintf("%s %s ---> %s\n", gpGlobals->indentLeader, id, superID);
				}
			}
		}
		// Retrieve any locator info from the Source mob
		OMFError = OMF2::omfmMobGetNumLocators(OMFFileHdl, obj, &numLocators);
		if ((OMF2::OM_ERR_NONE == OMFError) &&(numLocators > 0))
		{
			rc = ConvertOMFLocator(obj, pEssenceDesc);
			if (SUCCEEDED(rc))
			{
//				pSourceMob->SetEssenceDescriptor(pEssenceDesc);
				if (gpGlobals->bVerboseMode)
					UTLstdprintf("%sConverted %ld Locators\n", gpGlobals->indentLeader, (int)numLocators);
			}
			else
			{
				if (gpGlobals->bVerboseMode)
					UTLstdprintf("%sCannot convert Locator\n", gpGlobals->indentLeader);
				UTLerrprintf("%sERROR:Cannot convert Locator\n", gpGlobals->indentLeader);
			}
		}
	}
	if (pEssenceDesc)
		pEssenceDesc->Release();


	DecIndentLevel();
	return rc;
}
// ============================================================================
// ConvertOMFConstValue
//
//			This function extracts all the properties of an OMF Constant Value 
//			segment, and converts it into an AAF Constant Value object.
//			
// Returns: AAFRESULT_SUCCESS if MOB object is converted succesfully
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFConstValue(OMF2::omfSegObj_t segment,
									  IAAFConstValue* pConstValue)
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfErr_t			OMFError = OMF2::OM_ERR_NONE;
	
	OMF2::omfDDefObj_t	cvDatakind;
	OMF2::omfLength_t	cvLength;
	OMF2::omfInt64		cvValueSize;
	OMF2::omfInt64		cvBytesRead;
	void *				pcvBuffer = NULL;

	aafUInt32			valueSize;
	IAAFParameter*		pParameter;
	aafUID_t			datadef;

	OMFError = OMF2::omfiDataValueGetSize(OMFFileHdl, segment, &cvValueSize);
	OMF2::omfsTruncInt64toUInt32(cvValueSize, &valueSize);
	if (valueSize > 0)
	{
		UTLMemoryAlloc(valueSize, &pcvBuffer);
		rc = pConstValue->QueryInterface(IID_IAAFParameter, (void **)&pParameter);
		OMFError = OMF2::omfiConstValueGetInfo(OMFFileHdl, segment, 
							&cvDatakind, &cvLength, cvValueSize, &cvBytesRead, pcvBuffer);
		if (gpGlobals->bVerboseMode)
		{
			UTLstdprintf("%sProcessing Constant Value of length = %ld\n ", gpGlobals->indentLeader, (int)cvLength);
		}
		ConvertOMFDatakind(cvDatakind, &datadef);
		rc = pConstValue->SetValue(valueSize, (unsigned char *)pcvBuffer);
		
	}
	if (pcvBuffer)
		UTLMemoryFree(pcvBuffer);
	if (pParameter)
		pParameter->Release();
	return rc;
}
// ============================================================================
// ConvertOMFVaryingValue
//
//			This function extracts all the properties of an OMF Varying Value 
//			segment, and converts it into an AAF Constant Value object.
//			
// Returns: AAFRESULT_SUCCESS if MOB object is converted succesfully
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFVaryingValue(OMF2::omfSegObj_t segment,
										IAAFVaryingValue* pVaryingValue)
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfErr_t			OMFError = OMF2::OM_ERR_NONE;

	OMF2::omfIterHdl_t		OMFIterator;
	OMF2::omfInt32			numPoints;
	OMF2::omfDDefObj_t		vvDatakind;
	OMF2::omfCntlPtObj_t	control;
	OMF2::omfDDefObj_t		cpDatakind;
	OMF2::omfLength_t		vvLength;
	OMF2::omfInterpKind_t	vvInterpolation;
	OMF2::omfRational_t		time;
	OMF2::omfEditHint_t		editHint;
	OMF2::omfInt64			cpValueSize;
	OMF2::omfInt32			bytesRead;
	void *					pCPBuffer = NULL;

	IAAFParameter*			pParameter = NULL;
	IAAFControlPoint*		pControlPoint = NULL;
	aafUID_t				datadef;
	aafRational_t			AAFCPTime;
	aafEditHint_t			AAFCPEditHint;
	aafUInt32				valueSize;

	IncIndentLevel();
	OMFError = OMF2::omfiVaryValueGetInfo(OMFFileHdl, segment, &vvDatakind, &vvLength, &vvInterpolation);
	OMFError = OMF2::omfiVaryValueGetNumPoints(OMFFileHdl, segment, &numPoints);

	if (gpGlobals->bVerboseMode)
	{
		UTLstdprintf("%sProcessing Varying Value of length = %ld and %ld Control Points\n ", gpGlobals->indentLeader, (int)vvLength, (int)numPoints);
	}
	ConvertOMFDatakind(vvDatakind, &datadef);
	if (numPoints > 0)
	{
		OMF2::omfiIteratorAlloc(OMFFileHdl, &OMFIterator);
		for (int i = 0; i < numPoints;i++)
		{
			OMFError = OMF2::omfiVaryValueGetNextPoint(OMFIterator, segment, NULL, &control);
			if (OMF2::OM_ERR_NONE == OMFError)
			{
				pDictionary->CreateInstance(&AUID_AAFControlPoint, IID_IAAFControlPoint, (IUnknown **)&pControlPoint);
				OMFError = OMF2::omfiDataValueGetSize(OMFFileHdl, control, &cpValueSize);
				OMF2::omfsTruncInt64toUInt32(cpValueSize, &valueSize);
				UTLMemoryAlloc(valueSize, &pCPBuffer);
				OMFError = OMF2::omfiControlPtGetInfo(OMFFileHdl, control, &time, &editHint, &cpDatakind, 
										valueSize, &bytesRead, pCPBuffer);
				AAFCPTime.numerator = time.numerator;
				AAFCPTime.denominator = time.denominator;
				AAFCPEditHint = (aafEditHint_t)editHint;
				pControlPoint->SetTime(AAFCPTime);
				pControlPoint->SetEditHint(AAFCPEditHint);
				pControlPoint->SetValue((aafUInt32)valueSize, (unsigned char *)pCPBuffer);
				pVaryingValue->AddPoint(pControlPoint);
			}
			if (pCPBuffer)
				UTLMemoryFree(pCPBuffer);
			if (pControlPoint)
				pControlPoint->Release();
			pControlPoint = NULL;
		}
		OMF2::omfiIteratorDispose(OMFFileHdl, OMFIterator);
	}

	if (pParameter)
		pParameter->Release();
	DecIndentLevel();
	return rc;
}
// ============================================================================
// ConvertOMFNestedScope
//
//			This function extracts all the properties of an OMF Nested scope
//			segment, and converts it into an AAF Nested scope object.
//			
// Returns: AAFRESULT_SUCCESS if MOB object is converted succesfully
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFNestedScope(OMF2::omfSegObj_t segment,
									   IAAFNestedScope* pNestedScope)
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfErr_t			OMFError = OMF2::OM_ERR_NONE;

	OMF2::omfIterHdl_t		OMFIterator;
	OMF2::omfNumSlots_t		numSlots;
	OMF2::omfDDefObj_t		nsDatakind;
	OMF2::omfLength_t		nsLength;
	OMF2::omfSegObj_t		scopeSlot;

	aafUID_t				datadef;
	IAAFComponent*			pSlotComp = NULL;
	IAAFComponent*			pSegmentComp = NULL;
	IAAFSegment*			pSegment = NULL;

	IncIndentLevel();
	OMFError = OMF2::omfiNestedScopeGetInfo(OMFFileHdl, segment, &nsDatakind, &nsLength);
	OMFError = OMF2::omfiNestedScopeGetNumSlots(OMFFileHdl, segment, &numSlots);
	if (gpGlobals->bVerboseMode)
	{
		UTLstdprintf("%sProcessing Nested Scope of length = %ld and %ld slots\n ", gpGlobals->indentLeader, (int)nsLength, (int) numSlots);
	}
	// Set Nested Scope Component properties.
	ConvertOMFDatakind(nsDatakind, &datadef);
	pNestedScope->QueryInterface(IID_IAAFComponent, (void **)&pSegmentComp);
	pSegmentComp->SetDataDef(&datadef);
	pSegmentComp->SetLength((aafLength_t *)&nsLength);
	pSegmentComp->Release();
	pSegmentComp = NULL;

	if (numSlots > 0)
	{
		OMF2::omfiIteratorAlloc(OMFFileHdl, &OMFIterator);
		for (int i = 0; i < numSlots;i++)
		{
			OMFError = OMF2::omfiNestedScopeGetNextSlot(OMFIterator, segment, NULL, &scopeSlot);
			rc = ProcessOMFComponent(scopeSlot, &pSlotComp);
			pSlotComp->QueryInterface(IID_IAAFSegment, (void **)&pSegment);
			pNestedScope->AppendSegment(pSegment);
			pSlotComp->Release();
			pSlotComp = NULL;
			pSegment->Release();
			pSegment = NULL;
		}
		OMF2::omfiIteratorDispose(OMFFileHdl, OMFIterator);
	}

	DecIndentLevel();
	return rc;
}
// ============================================================================
// ConvertOMFScopeRef
//
//			This function extracts all the properties of an OMF Scope reference
//			segment, and converts it into an AAF ScopeReference object.
//			
// Returns: AAFRESULT_SUCCESS if MOB object is converted succesfully
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFScopeRef(OMF2::omfSegObj_t segment,
									IAAFScopeReference* pScopeRef)
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfErr_t			OMFError = OMF2::OM_ERR_NONE;
	OMF2::omfDDefObj_t		srDatakind;
	OMF2::omfLength_t		srLength;
	OMF2::omfUInt32			relScope;
	OMF2::omfUInt32			relSlot;

	IAAFComponent*			pSegmentComp = NULL;
	aafUID_t				datadef;

	IncIndentLevel();
	OMFError = OMF2::omfiScopeRefGetInfo(OMFFileHdl, segment, &srDatakind, &srLength, &relScope, &relSlot);
	if (OMF2::OM_ERR_NONE == OMFError)
	{
		if (gpGlobals->bVerboseMode)
		{
			UTLstdprintf("%sProcessing Scope reference of length = %ld\n ", gpGlobals->indentLeader, (int)srLength);
		}
		ConvertOMFDatakind(srDatakind, &datadef);
		pScopeRef->QueryInterface(IID_IAAFComponent, (void **)&pSegmentComp);
		pSegmentComp->SetDataDef(&datadef);
		pSegmentComp->SetLength((aafLength_t *)&srLength);
		pSegmentComp->Release();
		pSegmentComp = NULL;

		rc = pScopeRef->Create((aafUInt32)relScope, (aafUInt32)relSlot);

	}

	DecIndentLevel();
	return rc;
}
// ============================================================================
// OMFFileClose
//
//			This function closes the input OMF file
//			
// Returns: None
//
// ============================================================================
void Omf2Aaf::OMFFileClose()
{
	OMF2::omfsCloseFile(OMFFileHdl);
	OMF2::omfsEndSession(OMFSession);
}

// ============================================================================
// AAFFileClose
//
//			This function closes the output AAF file
//			
// Returns: None
//
// ============================================================================
void Omf2Aaf::AAFFileClose( )
{
	if (pDictionary)
	{
		pDictionary->Release();
		pDictionary = NULL;
	}

	if (pHeader)
	{
		pHeader->Release();
		pHeader = NULL;
	}

	if (pFile)
	{
		pFile->Save();
		pFile->Close();
		pFile->Release();
		pFile = NULL;
	}
}
static aafUID_t	zeroID = { 0 };

// ============================================================================
// SetEffectOptionalProperties
//
//			This function sets the optional properties of an Effect to default 
//			values. 
//	NOTE:	This is a temporary hack to allow OM to succesfully write the AAF
//			file. Once optional properties are supported this function should be
//			removed or modified to handle the properties correctly and not just
//			making them from thin air!!! 
//			
// Returns: None
//
// ============================================================================
HRESULT Omf2Aaf::SetEffectOptionalProperties(OMF2::omfEffObj_t	effect,
											 IAAFOperationGroup*		pEffect,
											 aafLength_t		effectLength,
											 aafUID_t			effectDatadef)
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfSegObj_t		OMFSourceClip;		
	OMF2::omfArgIDType_t	bypassOverride;
	OMF2::omfNumSlots_t		numSlots;

	IAAFSourceReference*	pSourceRef= NULL;
	IAAFSegment*			pFiller = NULL;
	IAAFComponent*			pComponent = NULL;
	IAAFSourceClip*			pSourceClip = NULL;
	IAAFParameter*			pParameter = NULL;
	IAAFParameterDef*		pParameterDef = NULL;
	aafSourceRef_t			sourceRef;

	// Create a source clip objet and Get Render
	rc = pDictionary->CreateInstance(&AUID_AAFSourceClip, IID_IAAFSourceClip, (IUnknown **)&pSourceClip);
	if (SUCCEEDED(rc))
	{
		rc = OMF2::omfiEffectGetFinalRender(OMFFileHdl, effect, &OMFSourceClip);
		if (OMF2::OM_ERR_NONE == rc)
		{
			rc = ConvertOMFSourceClip(OMFSourceClip, pSourceClip);
		}
		else
		{
			sourceRef.sourceID = zeroID;
			sourceRef.sourceSlotID = 0;
			sourceRef.startTime = 0;
			pSourceClip->Initialize (&effectDatadef, &effectLength, sourceRef);
		}
	}
	pSourceClip->QueryInterface (IID_IAAFSourceReference, (void **)&pSourceRef);
	pEffect->SetRender(pSourceRef);
	
	// Get bypassOverride
	rc = OMF2::omfiEffectGetBypassOverride(OMFFileHdl, effect, &bypassOverride);
	if (rc != OMF2::OM_ERR_NONE )
		bypassOverride = 1;
	pEffect->SetBypassOverride((aafUInt32) bypassOverride);

	rc = OMF2::omfiEffectGetNumSlots(OMFFileHdl, effect, &numSlots);
	if (numSlots > 0)
	{
		OMF2::omfIterHdl_t		OMFIterator;
		OMF2::omfESlotObj_t		OMFEffectSlot;
		OMF2::omfArgIDType_t	argID;
		OMF2::omfSegObj_t		argValue;

		OMF2::omfiIteratorAlloc(OMFFileHdl, &OMFIterator);
		for (int i = 0;i < numSlots;i++)
		{
			rc = OMF2::omfiEffectGetNextSlot(OMFIterator, effect, NULL, &OMFEffectSlot);
			if (OMF2::OM_ERR_NONE == rc)
			{
				OMF2::omfiEffectSlotGetInfo(OMFFileHdl, OMFEffectSlot, &argID, &argValue);

			}
		}
		OMF2::omfiIteratorDispose(OMFFileHdl, OMFIterator);
	}
	else
	{
		// this code is necesary while optional properties must be present !
		// tlk
		pDictionary->CreateInstance(&AUID_AAFFiller, IID_IAAFSegment, (IUnknown **)&pFiller);
		pFiller->QueryInterface(IID_IAAFComponent, (void **)&pComponent);
		pComponent->SetLength(&effectLength);
		pComponent->SetDataDef(&effectDatadef);
		pEffect->AppendNewInputSegment(pFiller);
	}

	pDictionary->CreateInstance(&AUID_AAFParameter, IID_IAAFParameter, (IUnknown **) &pParameter);
//	pParameter->SetParameterDefinition(pParameterDef);

	pEffect->AddNewParameter(pParameter);

	pComponent->Release();
	pFiller->Release();
	pSourceRef->Release();
	pSourceClip->Release();
	pParameter->Release();
	return rc;
}
// ============================================================================
// ConvertOMFEffects
//
//			This function reads an OMF effect object, converts its properties
//			to AAF, updates the AAF Effect object and, if necessary creates the 
//			effect definition by Calling ConvertOMFEffectDefinition. 
//			
// Returns: AAFRESULT_SUCCESS if object is converted.
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFEffects(OMF2::omfEffObj_t	effect,
								   IAAFOperationGroup*	pEffect)
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfErr_t			OMFError = OMF2::OM_ERR_NONE;

	OMF2::omfDDefObj_t		effectDatakind;
	OMF2::omfSegObj_t		effectSegment = NULL;
	OMF2::omfSegObj_t		inputSegmentA ;
	OMF2::omfSegObj_t		inputSegmentB ;
	OMF2::omfSegObj_t		levelSegment;
	OMF2::omfSegObj_t		renderClip = NULL;
	OMF2::omfESlotObj_t		effectSlot = NULL;
	OMF2::omfLength_t		effectLength;
	OMF2::omfDDefObj_t		effectDef;
	OMF2::omfInt32			nameSize = 64, idSize = 64;
	OMF2::omfUniqueName_t	effectID;
	char					effectDefName[64];
	OMF2::omfRational_t		speedRatio;
	OMF2::omfUInt32			phaseOffset = 0;
	OMF2::omfUInt32			mask = 0;
	OMF2::omfFxFrameMaskDirection_t addOrDrop = OMF2::kOmfFxFrameMaskDirNone;
	OMF2::omfArgIDType_t	bypassOverride;

	IAAFOperationDef*		pEffectDef = NULL;
	IAAFComponent*			pEffectSegment = NULL;
	IAAFComponent*			pRenderSegment = NULL;
	IAAFSourceReference*	pSourceRef= NULL;
	IAAFSegment*			pSegment = NULL;
	IAAFParameter*			pParameter = NULL;

	aafUID_t				effectAUID ;
	aafUID_t				effectDefAUID ;

	if (OMF2::kOmfRev2x == OMFFileRev)
	{
		OMFError = OMF2::omfiEffectGetInfo(OMFFileHdl, effect, &effectDatakind, &effectLength, &effectDef);

		if (gpGlobals->bVerboseMode)
		{
			UTLstdprintf("%sProcessing Effect of length = %ld\n ", gpGlobals->indentLeader, (unsigned int)effectLength);
		}

		OMFError = OMF2::omfiEffectDefGetInfo(OMFFileHdl, effectDef, idSize, effectID, nameSize, effectDefName,
									0, NULL, NULL, NULL);
		OMFError = OMF2::omfiEffectGetBypassOverride(OMFFileHdl, effect, &bypassOverride);
		if (OMF2::OM_ERR_PROP_NOT_PRESENT)
			bypassOverride = 0;
		if (strcmp(effectID, "omfi:effect:VideoSpeedControl") == 0)
		{
			OMFError = OMF2::omfeVideoSpeedControlGetInfo(OMFFileHdl, effect, &effectLength, &inputSegmentA, &speedRatio, &phaseOffset);
			if(OMF2::OM_ERR_NONE == OMFError)
			{
				if (inputSegmentA)
				{
					rc = ProcessOMFComponent(inputSegmentA, &pEffectSegment);
					pEffectSegment->QueryInterface(IID_IAAFSegment, (void **)&pSegment);
					pEffect->AppendNewInputSegment(pSegment);
					pSegment->Release();
					pEffectSegment->Release();
					pSegment = NULL;
					pEffectSegment = NULL;
				}
			}
		}
		else if (strcmp(effectID, "omfi:effect:VideoFrameMask") == 0)
		{
			OMFError = OMF2::omfeVideoFrameMaskGetInfo(OMFFileHdl, effect, &effectLength, &inputSegmentA, &mask, &addOrDrop, &phaseOffset);
			if(OMF2::OM_ERR_NONE == OMFError)
			{
				if (inputSegmentA)
				{
					rc = ProcessOMFComponent(inputSegmentA, &pEffectSegment);
					pEffectSegment->QueryInterface(IID_IAAFSegment, (void **)&pSegment);
					pEffect->AppendNewInputSegment(pSegment);
					pSegment->Release();
					pEffectSegment->Release();
					pSegment = NULL;
					pEffectSegment = NULL;
				}
			}
		}
		else if (strcmp(effectID, "omfi:effect:VideoRepeat") == 0)
		{
			OMFError = OMF2::omfeVideoRepeatGetInfo(OMFFileHdl, effect, &effectLength, &inputSegmentA, &phaseOffset);
			if(OMF2::OM_ERR_NONE == OMFError)
			{
				if (inputSegmentA)
				{
					rc = ProcessOMFComponent(inputSegmentA, &pEffectSegment);
					pEffectSegment->QueryInterface(IID_IAAFSegment, (void **)&pSegment);
					pEffect->AppendNewInputSegment(pSegment);
					pSegment->Release();
					pEffectSegment->Release();
					pSegment = NULL;
					pEffectSegment = NULL;
				}
			}
		}
		else if (strcmp(effectID, "omfi:effect:VideoDissolve") == 0)
		{
			OMFError = OMF2::omfeVideoDissolveGetInfo(OMFFileHdl, effect, &effectLength, &inputSegmentA, &inputSegmentB, &levelSegment);
			if(OMF2::OM_ERR_NONE == OMFError)
			{
				if (inputSegmentA)
				{
					rc = ProcessOMFComponent(inputSegmentA, &pEffectSegment);
					pEffectSegment->QueryInterface(IID_IAAFSegment, (void **)&pSegment);
					pEffect->AppendNewInputSegment(pSegment);
					pSegment->Release();
					pEffectSegment->Release();
					pSegment = NULL;
					pEffectSegment = NULL;
				}
				if (inputSegmentB)
				{
					rc = ProcessOMFComponent(inputSegmentB, &pEffectSegment);
					pEffectSegment->QueryInterface(IID_IAAFSegment, (void **)&pSegment);
					pEffect->AppendNewInputSegment(pSegment);
					pSegment->Release();
					pEffectSegment->Release();
					pSegment = NULL;
					pEffectSegment = NULL;
				}
				if (levelSegment)
				{
					if (OMF2::omfiIsAConstValue(OMFFileHdl, levelSegment, &OMFError))
					{
						IAAFConstValue* pConstantValue = NULL;
			
						rc = pDictionary->CreateInstance(&AUID_AAFConstValue, IID_IAAFConstValue, (IUnknown **)&pConstantValue);
						if (SUCCEEDED(rc))
						{
							rc = ConvertOMFConstValue(levelSegment, pConstantValue);
							rc = pConstantValue->QueryInterface(IID_IAAFParameter, (void **)&pParameter);
							pEffect->AddNewParameter(pParameter);
							pParameter->Release();
							pParameter = NULL;
							pConstantValue->Release();
							pConstantValue = NULL;
						}
					}
					else if (OMF2::omfiIsAVaryValue(OMFFileHdl, levelSegment, &OMFError))
					{
						IAAFVaryingValue* pVaryingValue = NULL;

						rc = pDictionary->CreateInstance(&AUID_AAFVaryingValue, IID_IAAFVaryingValue, (IUnknown **)&pVaryingValue);
						if (SUCCEEDED(rc))
						{
							rc = ConvertOMFVaryingValue(levelSegment, pVaryingValue);
							rc = pVaryingValue->QueryInterface(IID_IAAFParameter, (void **)&pParameter);
							pEffect->AddNewParameter(pParameter);
							pParameter->Release();
							pParameter = NULL;
							pVaryingValue->Release();
							pVaryingValue = NULL;
						}
					}
				}
			}
		}
		else if (strcmp(effectID, "omfi:effect:SMPTEVideoWipe") == 0)
		{
			OMFError = OMF2::omfeSMPTEVideoWipeGetInfo(OMFFileHdl, effect, &effectLength, &inputSegmentA, &inputSegmentB, &levelSegment, NULL, NULL);
			if(OMF2::OM_ERR_NONE == OMFError)
			{
				if (inputSegmentA)
				{
					rc = ProcessOMFComponent(inputSegmentA, &pEffectSegment);
					pEffectSegment->QueryInterface(IID_IAAFSegment, (void **)&pSegment);
					pEffect->AppendNewInputSegment(pSegment);
					pSegment->Release();
					pEffectSegment->Release();
					pSegment = NULL;
					pEffectSegment = NULL;
				}
				if (inputSegmentB)
				{
					rc = ProcessOMFComponent(inputSegmentB, &pEffectSegment);
					pEffectSegment->QueryInterface(IID_IAAFSegment, (void **)&pSegment);
					pEffect->AppendNewInputSegment(pSegment);
					pSegment->Release();
					pEffectSegment->Release();
					pSegment = NULL;
					pEffectSegment = NULL;
				}
			}
		}
		else if (strcmp(effectID, "omfi:effect:MonoAudioDissolve") == 0)
		{
			OMFError = OMF2::omfeMonoAudioDissolveGetInfo(OMFFileHdl, effect, &effectLength, &inputSegmentA, &inputSegmentB, &levelSegment);
			if(OMF2::OM_ERR_NONE == OMFError)
			{
				if (inputSegmentA)
				{
					rc = ProcessOMFComponent(inputSegmentA, &pEffectSegment);
					pEffectSegment->QueryInterface(IID_IAAFSegment, (void **)&pSegment);
					pEffect->AppendNewInputSegment(pSegment);
					pSegment->Release();
					pEffectSegment->Release();
					pSegment = NULL;
					pEffectSegment = NULL;
				}
				if (inputSegmentB)
				{
					rc = ProcessOMFComponent(inputSegmentB, &pEffectSegment);
					pEffectSegment->QueryInterface(IID_IAAFSegment, (void **)&pSegment);
					pEffect->AppendNewInputSegment(pSegment);
					pSegment->Release();
					pEffectSegment->Release();
					pSegment = NULL;
					pEffectSegment = NULL;
				}
				if (levelSegment)
				{
					if (OMF2::omfiIsAConstValue(OMFFileHdl, levelSegment, &OMFError))
					{
						IAAFConstValue* pConstantValue = NULL;
			
						rc = pDictionary->CreateInstance(&AUID_AAFConstValue, IID_IAAFConstValue, (IUnknown **)&pConstantValue);
						if (SUCCEEDED(rc))
						{
							rc = ConvertOMFConstValue(levelSegment, pConstantValue);
							rc = pConstantValue->QueryInterface(IID_IAAFParameter, (void **)&pParameter);
							pEffect->AddNewParameter(pParameter);
							pParameter->Release();
							pParameter = NULL;
							pConstantValue->Release();
							pConstantValue = NULL;
						}
					}
					else if (OMF2::omfiIsAVaryValue(OMFFileHdl, levelSegment, &OMFError))
					{
						IAAFVaryingValue* pVaryingValue = NULL;

						rc = pDictionary->CreateInstance(&AUID_AAFVaryingValue, IID_IAAFVaryingValue, (IUnknown **)&pVaryingValue);
						if (SUCCEEDED(rc))
						{
							rc = ConvertOMFVaryingValue(levelSegment, pVaryingValue);
							rc = pVaryingValue->QueryInterface(IID_IAAFParameter, (void **)&pParameter);
							pEffect->AddNewParameter(pParameter);
							pParameter->Release();
							pParameter = NULL;
							pVaryingValue->Release();
							pVaryingValue = NULL;
						}
					}
				}
			}
		}
		else
		{
			// Generic OMF 2.x Effect processing !!! 
			OMF2::omfIterHdl_t		OMFIterator;
			OMF2::omfESlotObj_t		OMFEffectSlot;
			OMF2::omfArgIDType_t	argID;
			OMF2::omfSegObj_t		argValue;
			OMF2::omfNumSlots_t		numSlots;

			if (gpGlobals->bVerboseMode)
			{
				UTLstdprintf("%sGeneric OMF Effect = %s\n ", gpGlobals->indentLeader, effectID);
			}
			OMFError = OMF2::omfiEffectGetNumSlots(OMFFileHdl, effect, &numSlots);;
			if (numSlots > 0)
			{
				OMF2::omfiIteratorAlloc(OMFFileHdl, &OMFIterator);
				OMFError = OMF2::omfiEffectGetNextSlot(OMFIterator, effect, NULL, &OMFEffectSlot);
				while(OMFEffectSlot != NULL)
				{
					OMFError = OMF2::omfiEffectSlotGetInfo(OMFFileHdl, OMFEffectSlot, &argID, &argValue);
					OMFError = OMF2::omfiEffectGetNextSlot(OMFIterator, effect, NULL, &OMFEffectSlot);
				}
				OMFError = OMF2::omfiIteratorDispose(OMFFileHdl, OMFIterator);
			}
		}
	}
	else
	{
		if (gpGlobals->bVerboseMode)
		{
			UTLstdprintf("%sGeneric 1.x OMF Effect\n ", gpGlobals->indentLeader);
		}
	}

	if (OMF2::OM_ERR_NONE == OMFError )
	{
		ConvertUniqueNameToAUID(effectID, &effectDefAUID);
		ConvertOMFDatakind(effectDatakind, &effectAUID);
		rc = pDictionary->LookupOperationDefinition(&effectDefAUID, &pEffectDef);
		pEffect->Initialize(&effectAUID, (aafLength_t)effectLength, pEffectDef);
		pEffect->SetBypassOverride(bypassOverride);

		OMFError = OMF2::omfiEffectGetFinalRender(OMFFileHdl, effect, &renderClip);
		if (OMF2::OM_ERR_NONE == OMFError)
		{
			rc = ProcessOMFComponent(renderClip, &pRenderSegment);
			rc = ConvertOMFDatakind( effectDatakind, &effectAUID);
			if (pRenderSegment)
			{
				pRenderSegment->QueryInterface(IID_IAAFSourceReference, (void **)&pSourceRef);
				pEffect->SetRender(pSourceRef);
				pSourceRef->Release();
			}
			 
		}
		else if (OMF2::OM_ERR_PROP_NOT_PRESENT == OMFError)
		{
			// we need to add this code hear until optional arguments are implemented !!!
			IAAFSourceReference*	pNULLSourceRef= NULL;
			IAAFSourceClip*			pNULLSourceClip = NULL;
			aafSourceRef_t			sourceRef;

			rc = pDictionary->CreateInstance(&AUID_AAFSourceClip, IID_IAAFSourceClip, (IUnknown **)&pNULLSourceClip);
			sourceRef.sourceID = zeroID;
			sourceRef.sourceSlotID = 0;
			sourceRef.startTime = 0;
			pNULLSourceClip->Initialize (&effectAUID, &effectLength, sourceRef);
			pNULLSourceClip->QueryInterface (IID_IAAFSourceReference, (void **)&pNULLSourceRef);
			pEffect->SetRender(pNULLSourceRef);
			pNULLSourceRef->Release();
			pNULLSourceRef = NULL;
			pNULLSourceClip->Release();
			pNULLSourceClip = NULL;
		}
//		rc = SetEffectOptionalProperties(effect, pEffect, (aafLength_t)effectLength, effectAUID);
	}

	if (pEffectDef)
		pEffectDef->Release();

	if (pEffectSegment)
		pEffectSegment->Release();

	if (pRenderSegment)
		pRenderSegment->Release();

	return rc;
}
// ============================================================================
// ConvertOMFEffectDefinition
//
//			This function Creates and sets the optional properties of an Effect
//			definition.  It creates also a Parameter Definition object. 
//			
// Returns: None
//
// ============================================================================
HRESULT Omf2Aaf::ConvertOMFEffectDefinition(OMF2::omfDDefObj_t	effectDef,
											aafUID_t*			pEffectAUID,
											IAAFOperationDef**		ppEffectDef)
{
	HRESULT					rc = AAFRESULT_SUCCESS;
	OMF2::omfArgIDType_t	bypassOverride;
	OMF2::omfBool			isTimeWarp;
	OMF2::omfInt32			nameSize = 64, idSize = 64, descSize = 120;
	OMF2::omfUniqueName_t	effectID;
	char					effectName[64], descBuffer[120];

    aafWChar*				pwCategory = NULL;
	aafWChar*				pwDesc = NULL;
	aafWChar*				pwName = NULL;
	aafUID_t				effectDefAUID ;
	IAAFDefObject*			pDefObject = NULL;

	rc = OMF2::omfiEffectDefGetInfo(OMFFileHdl,
									effectDef,
									idSize, effectID,
									nameSize, effectName,
									descSize, descBuffer,
									&bypassOverride,
									&isTimeWarp);
	if (OMF2::OM_ERR_NONE == rc)
	{
		UTLStrAToStrW("Not Specified", &pwCategory);
		UTLStrAToStrW(descBuffer, &pwDesc);
		UTLStrAToStrW(effectName, &pwName);
		ConvertUniqueNameToAUID(effectID, &effectDefAUID);

		// Look in the dictionary to find if the effect Definition exists
		// if it exists use it.
		rc = pDictionary->LookupOperationDefinition(&effectDefAUID, ppEffectDef);
		if (AAFRESULT_SUCCESS != rc)
		{
			pDictionary->CreateInstance(&AUID_AAFOperationDef, IID_IAAFOperationDef, (IUnknown **) ppEffectDef);

			(*ppEffectDef)->QueryInterface(IID_IAAFDefObject, (void **)&pDefObject);
			pDefObject->Init(&effectDefAUID, pwName, pwDesc);
			pDefObject->Release();
			pDefObject = NULL;
			pDictionary->RegisterOperationDefinition(*ppEffectDef);
			(*ppEffectDef)->SetIsTimeWarp((aafBool)isTimeWarp);
			(*ppEffectDef)->SetCategory(pwName);
			(*ppEffectDef)->SetBypass((aafUInt32 )bypassOverride);
			if (memcmp((const void *)&effectDefAUID, (const void*)&kAAFEffectVideoDissolve, sizeof(aafUID_t)) == 0)
				(*ppEffectDef)->SetNumberInputs((aafInt32)2);
			else
				(*ppEffectDef)->SetNumberInputs((aafInt32)0);
			CreateParameterDefinition(*ppEffectDef);
			rc = AAFRESULT_SUCCESS;
		}
		if (pEffectAUID != NULL)
			(*ppEffectDef)->SetDataDefinitionID(pEffectAUID); 

	}
	if (pwName)
		UTLMemoryFree(pwName);
	if (pwDesc)
		UTLMemoryFree(pwDesc);
	if (pwCategory)
		UTLMemoryFree(pwCategory);

	if (pDefObject)
		pDefObject->Release();

	return rc;
}
// ============================================================================
// CreateParameterDefinition
//
//			This function Creates and sets the properties for a Paramter 
//			Definition object.
//			
// Returns: None
//
// ============================================================================
void Omf2Aaf::CreateParameterDefinition(IAAFOperationDef*	pEffectDef)
{
	HRESULT				rc = AAFRESULT_SUCCESS;

//	aafUID_t			typeDefinition;
    aafWChar*			pwCategory = NULL;
	aafWChar*			pwDesc = NULL;
	aafWChar*			pwName = NULL;
	aafUID_t			parameterDefAUID ;
	aafUInt32			bufferLength;
	aafWChar*			pwDisplayUnits = NULL;
	IAAFParameterDef*	pParameterDef = NULL;
	IAAFDefObject*		pDefObject = NULL;

	// We are going to use the effect definitions def object values 
	// since OMF does NOT have a Parameter definition object !
	pEffectDef->QueryInterface(IID_IAAFDefObject, (void **)&pDefObject);
	pDefObject->GetAUID(&parameterDefAUID);
	pDefObject->GetNameBufLen(&bufferLength);
	UTLMemoryAlloc(bufferLength, (void **)&pwName);
	pDefObject->GetName(pwName, bufferLength);
	pDefObject->GetDescriptionBufLen(&bufferLength);
	UTLMemoryAlloc(bufferLength, (void **)&pwDesc);
	pDefObject->GetDescription(pwDesc, bufferLength);
	pDefObject->Release();
	pDefObject = NULL;
	rc = pDictionary->CreateInstance(&AUID_AAFParameterDef, IID_IAAFParameterDef, (IUnknown **) &pParameterDef);
	if (AAFRESULT_SUCCESS == rc)
	{
		pParameterDef->QueryInterface(IID_IAAFDefObject, (void **)&pDefObject);
		pDefObject->Init(&parameterDefAUID, pwName, pwDesc);
		pDefObject->Release();
		pDefObject = NULL;
		pDictionary->RegisterParameterDefinition(pParameterDef);
		UTLStrAToStrW("Not Specified", &pwDisplayUnits);
		pParameterDef->SetDisplayUnits(pwDisplayUnits);
		UTLMemoryFree(pwDisplayUnits);
	}
	pEffectDef->AddParameterDefs(pParameterDef);

	pParameterDef->Release();
	if (pwName)
		UTLMemoryFree(pwName);
	if (pwDesc)
		UTLMemoryFree(pwDesc);
}
										
