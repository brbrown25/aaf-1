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
/************************************************************************
 *
 * AafOmf.h Describes structures and prototypes for the OMF to AAF 
 *          conversion utility.
 *
 ************************************************************************/


  
#if defined(_MAC) || defined(macintosh)
#include <initguid.h> // define all of the AAF guids.
#include "AAF.h"
#else
#include "AAF.h"
// TODO: This should not be here, I added them for now to get a good link.
const CLSID CLSID_AAFFile = { 0x9346ACD2, 0x2713, 0x11d2, { 0x80, 0x35, 0x00, 0x60, 0x08, 0x14, 0x3E, 0x6F } };

#endif

// Include the defintions for the AAF Stored Object identifiers.
#define INIT_AUID
#include "AAFStoredObjectIDs.h"


const int MAX_INDENT = 8;


// ============================================================================
// simple helper class to initialize and cleanup COM library.
// ============================================================================
struct CComInitialize
{
	CComInitialize()
	{
		CoInitialize(NULL);
	}

	~CComInitialize()
	{
		CoUninitialize();
	}
};

// ============================================================================
// simple helper class to run the main application.
// ============================================================================
class AafOmf
{
public:
	AafOmf();
	~AafOmf();

public:
	void Usage( void );
	void IncIndentLevel( void );
	void DecIndentLevel( void );
	HRESULT GetUserInput(int argc, char* argv[]);
	HRESULT OpenInputFile( void );
	HRESULT OpenOutputFile( void );
	aafBool IsOMFFile(FILE* pStream);
	HRESULT OMFFileOpen( char* pFileName );
	HRESULT	TOCFileCreate( void ); 
	void OMFFileClose( void );
	HRESULT AAFFileOpen( char* pFileName );
	HRESULT AAFDefinitionFileOpen( void );
	void AAFFileClose(void );
	HRESULT OMFFileRead( void );
	HRESULT ConvertOMFHeader( void );
	HRESULT ConvertOMFDataDefinitionObject( OMF2::omfObject_t obj);
	HRESULT ConvertOMFClassDictionaryObject( OMF2::omfObject_t obj);
	HRESULT ConvertOMFMediaDataObject( OMF2::omfObject_t obj);
	HRESULT ConvertOMFDatakind(OMF2::omfDDefObj_t datakind, aafUID_t* pDatadef);
	HRESULT ConvertOMFMOBObject( OMF2::omfObject_t obj, IAAFMob* pMob );
	HRESULT ConvertOMFCompositionObject( OMF2::omfObject_t obj,IAAFCompositionMob* pCompMob );
	HRESULT ConvertOMFMasterMob( OMF2::omfObject_t obj, IAAFMasterMob* pMasterMob );
	HRESULT ConvertOMFSourceMob( OMF2::omfObject_t obj, IAAFSourceMob* pSourceMob );
	HRESULT TraverseOMFMob( OMF2::omfObject_t obj, IAAFMob* pMob );
	HRESULT ProcessOMFComponent( OMF2::omfObject_t obj, IAAFComponent** ppComponent );
	HRESULT TraverseOMFSequence( OMF2::omfObject_t obj, IAAFSequence* pSequence );
	HRESULT	ConvertOMFSequence( OMF2::omfObject_t sequence, IAAFSequence* pSequence);
	HRESULT ConvertOMFSourceClip( OMF2::omfObject_t sourceclip, IAAFSourceClip* pSourceClip);
	HRESULT ConvertOMFComponentProperties(OMF2::omfObject_t sequence, IAAFComponent* pComponent);
	HRESULT ConvertOMFTransition(OMF2::omfObject_t transition, IAAFTransition* pTransition);
	HRESULT ConvertOMFSelector(OMF2::omfObject_t selector, IAAFSelector* pSelector);

	char*					pProgramName;

private:
	aafBool					bVerboseMode;
	aafBool					bCreateTOCFile;
	aafBool					bConvertAllObjects;
	aafBool					bOMFFileOpen;
	aafBool					bAAFFileOpen;
	aafBool					bLogFile;
	aafBool					bDefFile;
	char					sInFileName[256];
	char					sTOCFileName[256];
	char					sDefinitionFileName[256];
	char					sOutFileName[256];
	char					sLogFileName[256];

	// For Statistical summary
	aafInt32				nNumOMFObjects;
	aafInt32				nNumAAFObjects;
	aafInt32				nNumOMFProperties;
	aafInt32				nNumAAFProperties;
	aafInt32				nNumUndefinedOMFObjects;
	aafInt32				nNumUndefinedOMFProperties;

	// For display and print purposes
	aafInt16				numIndents;
	char					indentLeader[MAX_INDENT+1];	

    OMF2::omfSessionHdl_t	OMFSession;
	OMF2::omfHdl_t			OMFFileHdl;
	OMF2::omfFileRev_t		OMFFileRev;

	IAAFFile*				pFile;
	IAAFHeader*				pHeader;
	IAAFDictionary*			pDictionary;
};
