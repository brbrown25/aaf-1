// @doc INTERNAL
// @com This file implements the module test for CAAFContentStorage
/***********************************************************************
 *
 *              Copyright (c) 1998-1999 Avid Technology, Inc.
 *
 * Permission to use, copy and modify this software and accompanying 
 * documentation, and to distribute and sublicense application software
 * incorporating this software for any purpose is hereby granted, 
 * provided that (i) the above copyright notice and this permission
 * notice appear in all copies of the software and related documentation,
 * and (ii) the name Avid Technology, Inc. may not be used in any
 * advertising or publicity relating to the software without the specific,
 * prior written permission of Avid Technology, Inc.
 *
 * THE SOFTWARE IS PROVIDED AS-IS AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL AVID TECHNOLOGY, INC. BE LIABLE FOR ANY DIRECT,
 * SPECIAL, INCIDENTAL, PUNITIVE, INDIRECT, ECONOMIC, CONSEQUENTIAL OR
 * OTHER DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE AND
 * ACCOMPANYING DOCUMENTATION, INCLUDING, WITHOUT LIMITATION, DAMAGES
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, AND WHETHER OR NOT
 * ADVISED OF THE POSSIBILITY OF DAMAGE, REGARDLESS OF THE THEORY OF
 * LIABILITY.
 *
 ************************************************************************/


#include "AAF.h"

#include <iostream.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "AAFStoredObjectIDs.h"
#include "AAFResult.h"
#include "ModuleTest.h"
#include "AAFDefUIDs.h"

#include "CAAFBuiltinDefs.h"

const aafMobID_t kTestMobID1 = { 0x06, 0x0E, 0x2B, 0x34, 0x01, 0x01, 0x01, 0x01,
								 0x01, 0x01, 0x04, 0x02, 0x13, 0x00, 0x00, 0x00,
								 0x464AA9D8, 0x34AC, 0x11d3, { 0x80, 0xB4, 0x00, 0x60, 0x08, 0x14, 0x3E, 0x6F } };
const aafMobID_t kTestMobID2 = { 0x06, 0x0E, 0x2B, 0x34, 0x01, 0x01, 0x01, 0x01,
								 0x01, 0x01, 0x04, 0x02, 0x13, 0x00, 0x00, 0x00,
								 0x464AA9D9, 0x34AC, 0x11d3, { 0x80, 0xB4, 0x00, 0x60, 0x08, 0x14, 0x3E, 0x6F } };


// Utility class to implement the test.
struct ContentStorageTest
{
	ContentStorageTest();
	~ContentStorageTest();
	
	void createFile(wchar_t *pFileName);
	void openFile(wchar_t *pFileName);
	
	void createFileMob(aafMobID_constref newMobID);
	void createEssenceData(IAAFSourceMob *pSourceMob);
	void openEssenceData();
	
	void writeEssenceData(IAAFEssenceData *pEssenceData,
		const aafDataBuffer_t data,
		aafUInt32 dataSize);
	void readEssenceData(IAAFEssenceData *pEssenceData,
		const aafDataBuffer_t data,
		aafUInt32 dataSize);
	
	void cleanupReferences();
	void setBufferSize(aafUInt32 bufferSize);
	inline void check(HRESULT hr);
	inline void checkExpression(bool expression, HRESULT hr);
	const char * convert(const wchar_t* pwcName);
	
	// Shared member data:
	HRESULT _hr;
	aafProductVersion_t _productVersion;
  aafProductIdentification_t _productInfo;
	IAAFFile *_pFile;
	bool _bFileOpen;
	IAAFContentStorage *_pStorage;
	IAAFDictionary *_pDictionary;
	
	IAAFMob *_pMob;
	IAAFSourceMob *_pSourceMob;
	IAAFEssenceDescriptor *_pEssenceDescriptor;
	IAAFFileDescriptor *_pFileDescriptor;
	IEnumAAFEssenceData *_pEnumEssenceData;
	IAAFEssenceData *_pEssenceData;
	aafDataBuffer_t _buffer;
	aafUInt32 _bufferSize;
	
	static const char _smiley[];
	static const char _frowney[];
};

extern "C" HRESULT CAAFContentStorage_test(testMode_t mode);
extern "C" HRESULT CAAFContentStorage_test(testMode_t mode)
{
	HRESULT hr = AAFRESULT_SUCCESS;
	wchar_t *fileName = L"AAFContentStorageTest.aaf";
	ContentStorageTest edt;
	
	try
	{
		if(mode == kAAFUnitTestReadWrite)
			edt.createFile(fileName);
		edt.openFile(fileName);
	}
	catch (HRESULT& ehr)
	{
		// thrown by ContentStorageTest_check() method.
		hr = ehr;
	}
	catch (...)
	{
		cerr << "CAAFContentStorage_test...Caught general C++"
			 << " exception!" << endl; 
		hr = AAFRESULT_TEST_FAILED;
	}
	
	// Cleanup our object if it exists.
//	if (SUCCEEDED(hr))
//	{
//		cout << "The following IAAFContentStorage methods have not been tested:" << endl;       
//		cout << "     GetMobs" << endl; 
//		hr = AAFRESULT_TEST_PARTIAL_SUCCESS;
//	}
	
	return hr;
}


const char ContentStorageTest::_smiley[] =        /* 16x16 smiley face */
  "      ****      "
  "    ********    "
  "   **********   "
  "  ************  "
  " ***  ****  *** "
  " ***  ****  *** "
  "****************"
  "****************"
  "****************"
  "****************"
  " ** ******** ** "
  " *** ****** *** "
  "  ***  **  ***  "
  "   ****  ****   "
  "    ********    "
  "      ****     ";

const char ContentStorageTest::_frowney[] =        /* 16x16 frowney face */
  "      ****      "
  "    ********    "
  "   **********   "
  "  ************  "
  " ***  ****  *** "
  " ***  ****  *** "
  "****************"
  "****************"
  "*******  *******"
  "*****  **  *****"
  " *** ****** *** "
  " ** ******** ** "
  "  ************  "
  "   **********   "
  "    ********    "
  "      ****     ";



ContentStorageTest::ContentStorageTest():
_hr(AAFRESULT_SUCCESS),
_pFile(NULL),
_bFileOpen(false),
_pStorage(NULL),
_pDictionary(NULL),
_pMob(NULL),
_pSourceMob(NULL),
_pEssenceDescriptor(NULL),
_pFileDescriptor(NULL),
_pEnumEssenceData(NULL),
_pEssenceData(NULL),
_buffer(NULL),
_bufferSize(0)
{
  _productVersion.major = 1;
  _productVersion.minor = 0;
  _productVersion.tertiary = 0;
  _productVersion.patchLevel = 0;
  _productVersion.type = kAAFVersionUnknown;
  _productInfo.companyName = L"AAF Developers Desk";
  _productInfo.productName = L"AAFContentStorage Test";
  _productInfo.productVersion = &_productVersion;
  _productInfo.productVersionString = NULL;
  _productInfo.productID = UnitTestProductID;
  _productInfo.platform = NULL;
}

ContentStorageTest::~ContentStorageTest()
{
	cleanupReferences();
}

void ContentStorageTest::cleanupReferences()
{
	
	if (NULL != _buffer)
	{
		delete _buffer;
		_buffer = NULL;
	}
	
	if (NULL != _pEssenceData)
	{
		_pEssenceData->Release();
		_pEssenceData = NULL;
	}
	
	if (NULL != _pEnumEssenceData)
	{
		_pEnumEssenceData->Release();
		_pEnumEssenceData = NULL;
	}
		
	if (NULL != _pFileDescriptor)
	{
		_pFileDescriptor->Release();
		_pFileDescriptor = NULL;
	}
	
	if (NULL != _pEssenceDescriptor)
	{
		_pEssenceDescriptor->Release();
		_pEssenceDescriptor = NULL;
	}
	
	if (NULL != _pSourceMob)
	{
		_pSourceMob->Release();
		_pSourceMob = NULL;
	}
	
	if (NULL != _pMob)
	{
		_pMob->Release();
		_pMob = NULL;
	}
	
	if (NULL != _pDictionary)
	{
		_pDictionary->Release();
		_pDictionary = NULL;
	}
	
	if (NULL != _pStorage)
	{
		_pStorage->Release();
		_pStorage = NULL;
	}
	
	if (NULL != _pFile)
	{
		if (_bFileOpen)
			_pFile->Close();
		_pFile->Release();
		_pFile = NULL;
	}
}

void ContentStorageTest::setBufferSize(aafUInt32 bufferSize)
{
	// Allocate the buffer.
	if (NULL != _buffer && bufferSize > _bufferSize)
	{
		delete _buffer;
		_buffer = NULL;
	}
	
	// Allocate the buffer.
	if (NULL == _buffer)
	{
		_bufferSize = bufferSize;
		_buffer = (aafDataBuffer_t)new char[ _bufferSize ];
		checkExpression(NULL != _buffer, AAFRESULT_NOMEMORY);
	}
}

inline void ContentStorageTest::check(HRESULT hr)
{
	if (AAFRESULT_SUCCESS != (_hr = hr))
		throw hr;
}

inline void ContentStorageTest::checkExpression(bool expression, HRESULT hr)
{
	if (!expression)
		throw hr;
}

// local conversion utility.
const char * ContentStorageTest::convert(const wchar_t* pwcName)
{
	checkExpression(NULL != pwcName, E_INVALIDARG);
	const size_t kBufferSize = 256;
	static char ansiName[kBufferSize];

	size_t convertedBytes = wcstombs(ansiName, pwcName, kBufferSize);
	checkExpression(0 < convertedBytes, E_INVALIDARG);
	
	return ansiName;
}



void ContentStorageTest::createFile(wchar_t *pFileName)
{
	IAAFHeader	*hdr;
		
		// Delete the test file if it already exists.
	remove(convert(pFileName));
	
	check(AAFFileOpenNewModify(pFileName, 0, &_productInfo, &_pFile));
	_bFileOpen = true;
	check(_pFile->GetHeader(&hdr));
	check(hdr->GetDictionary(&_pDictionary));
	check(hdr->GetContentStorage(&_pStorage));
	
	createFileMob(kTestMobID1);
	createFileMob(kTestMobID2);
	
	check(_pFile->Save());
	
	cleanupReferences();

	hdr->Release();
}

void ContentStorageTest::openFile(wchar_t *pFileName)
{
	aafUInt32		readNumEssenceData;
	aafNumSlots_t	readNumMobs;
	IAAFMob			*testMob = NULL;
	IAAFHeader		*pHdr = NULL;
	IEnumAAFMobs	*pEnum = NULL;
	aafMobID_t		uid, readID;
	aafBool			testBool;

	check(AAFFileOpenExistingRead(pFileName, 0, &_pFile));
	_bFileOpen = true;
	check(_pFile->GetHeader(&pHdr));
	check(pHdr->GetContentStorage(&_pStorage));
	
	openEssenceData();

	
	check(_pStorage->CountEssenceData(&readNumEssenceData));
	checkExpression(2 == readNumEssenceData, AAFRESULT_TEST_FAILED);
	/***/
	uid = kTestMobID1;
	check(_pStorage->LookupMob(uid, &testMob));
	check(testMob->GetMobID(&readID));
	checkExpression(memcmp(&uid, &readID, sizeof(readID)) == 0, AAFRESULT_TEST_FAILED);
	testMob->Release();
	testMob = NULL;
	/***/
	uid = kTestMobID2;
	check(_pStorage->LookupMob(uid, &testMob));
	check(testMob->GetMobID(&readID));
	checkExpression(memcmp(&uid, &readID, sizeof(readID)) == 0, AAFRESULT_TEST_FAILED);
	testMob->Release();
	testMob = NULL;
	/***/
	uid.material.Data1 = 0;	// Invalidate the mobID
	uid.material.Data2 = 0;
	uid.material.Data3 = 0;
	checkExpression(_pStorage->LookupMob(uid, &testMob) != AAFRESULT_SUCCESS, AAFRESULT_TEST_FAILED);
	/***/
	check(_pStorage->CountMobs(kAAFFileMob, &readNumMobs));
	checkExpression(2 == readNumMobs, AAFRESULT_TEST_FAILED);
	check(_pStorage->CountMobs(kAAFMasterMob, &readNumMobs));
	checkExpression(0 == readNumMobs, AAFRESULT_TEST_FAILED);
	/***/
	uid = kTestMobID1;
	check(_pStorage->GetMobs(NULL, &pEnum));		// !! Add tests for criteria
	check(pEnum->NextOne(&testMob));
	check(testMob->GetMobID(&readID));
	checkExpression(memcmp(&uid, &readID, sizeof(readID)) == 0, AAFRESULT_TEST_FAILED);
	testMob->Release();
	testMob = NULL;
	/***/
	uid = kTestMobID2;
    check(_pStorage->IsEssenceDataPresent (uid, kAAFEssence, &testBool));
	checkExpression(kAAFTrue == testBool, AAFRESULT_TEST_FAILED);
	/***/
	uid = kTestMobID2;
 	uid.material.Data1 = 0;	// Invalidate the mobID
	uid.material.Data2 = 0;
	uid.material.Data3 = 0;
	check(_pStorage->IsEssenceDataPresent (uid, kAAFEssence, &testBool));
	checkExpression(kAAFFalse == testBool, AAFRESULT_TEST_FAILED);

	/***/
	pEnum->Release();
	pEnum = NULL;
	cleanupReferences();
}

void ContentStorageTest::createFileMob(aafMobID_constref newMobID)
{
	assert(_pFile && _pStorage && _pDictionary);
	assert(NULL == _pSourceMob);
	assert(NULL == _pMob);
	assert(NULL == _pFileDescriptor);
	assert(NULL == _pEssenceDescriptor);
	assert(NULL == _pSourceMob);
	
	CAAFBuiltinDefs defs (_pDictionary);

	// Create a Mob
	check(defs.cdSourceMob()->
		  CreateInstance(IID_IAAFSourceMob, 
						 (IUnknown **)&_pSourceMob));
	
	check(_pSourceMob->QueryInterface (IID_IAAFMob, (void **)&_pMob));
	
	check(_pMob->SetMobID(newMobID));
	check(_pMob->SetName(L"ContentStorageTest File Mob"));
	
	// instantiate a concrete subclass of FileDescriptor
	check(defs.cdAIFCDescriptor()->
		  CreateInstance(IID_IAAFFileDescriptor, 
						 (IUnknown **)&_pFileDescriptor));
	
	IAAFAIFCDescriptor*			pAIFCDesc = NULL;
	check(_pFileDescriptor->QueryInterface (IID_IAAFAIFCDescriptor, (void **)&pAIFCDesc));
	check(pAIFCDesc->SetSummary (5, (unsigned char*)"TEST"));
	pAIFCDesc->Release();
	pAIFCDesc = NULL;

	check(_pFileDescriptor->QueryInterface (IID_IAAFEssenceDescriptor,
		(void **)&_pEssenceDescriptor));
	check(_pSourceMob->SetEssenceDescriptor (_pEssenceDescriptor));
	
	check(_pStorage->AddMob(_pMob));
	
	createEssenceData(_pSourceMob);
	
	// Cleanup instance data for reuse...
	_pEssenceDescriptor->Release();
	_pEssenceDescriptor = NULL;
	
	_pFileDescriptor->Release();
	_pFileDescriptor = NULL;
	
	_pMob->Release();
	_pMob = NULL;
	
	_pSourceMob->Release();
	_pSourceMob = NULL;
}

void ContentStorageTest::createEssenceData(IAAFSourceMob *pSourceMob)
{
	assert(_pFile && _pStorage && _pDictionary);
	assert(pSourceMob);
	assert(NULL == _pEssenceData);
	
	
	CAAFBuiltinDefs defs (_pDictionary);
	
	// Attempt to create an AAFEssenceData.
	check(defs.cdEssenceData()->
		  CreateInstance(IID_IAAFEssenceData,
						 (IUnknown **)&_pEssenceData));
	
	check(_pEssenceData->SetFileMob(pSourceMob));
	check(_pStorage->AddEssenceData(_pEssenceData));
	
	writeEssenceData(_pEssenceData, (aafDataBuffer_t)_smiley, sizeof(_smiley));
	writeEssenceData(_pEssenceData, (aafDataBuffer_t)_frowney, sizeof(_frowney));
	
	// Read back the data before saving and/or closing the file
	check(_pEssenceData->SetPosition(0));
	aafPosition_t dataPosition = 0;
	check(_pEssenceData->GetPosition(&dataPosition));
	checkExpression(dataPosition == 0, AAFRESULT_TEST_FAILED);
	
	// Validate the current essence size.
	aafLength_t essenceSize = 0;
	aafLength_t expectedEssenceSize = sizeof(_smiley) + sizeof(_frowney);
	check(_pEssenceData->GetSize(&essenceSize));
	checkExpression(essenceSize == expectedEssenceSize, AAFRESULT_TEST_FAILED);
	
	
	readEssenceData(_pEssenceData, (aafDataBuffer_t)_smiley, sizeof(_smiley));
	readEssenceData(_pEssenceData, (aafDataBuffer_t)_frowney, sizeof(_frowney));
	
	_pEssenceData->Release();
	_pEssenceData = NULL;
}

void ContentStorageTest::openEssenceData()
{
	assert(_pFile && _pStorage);
	assert(NULL == _pEnumEssenceData);
	assert(NULL == _pEssenceData);
	assert(NULL == _pSourceMob);
	
	
	check(_pStorage->EnumEssenceData(&_pEnumEssenceData));
	while (AAFRESULT_SUCCESS == (_hr = _pEnumEssenceData->NextOne(&_pEssenceData)))
	{
		// Validate the essence.
		// Validate the current essence size.
		aafLength_t essenceSize = 0;
		aafLength_t expectedEssenceSize = sizeof(_smiley) + sizeof(_frowney);
		check(_pEssenceData->GetSize(&essenceSize));
		checkExpression(essenceSize == expectedEssenceSize, AAFRESULT_TEST_FAILED);
		
		// Validate that the correct data can be read back.
		readEssenceData(_pEssenceData, (aafDataBuffer_t)_smiley, sizeof(_smiley));
		readEssenceData(_pEssenceData, (aafDataBuffer_t)_frowney, sizeof(_frowney));
		
		// Make sure that the essence data still references
		// a valid mob.
		check(_pEssenceData->GetFileMob(&_pSourceMob));
		
		_pSourceMob->Release();
		_pSourceMob = NULL;
		
		_pEssenceData->Release();
		_pEssenceData = NULL;
	}
	
	// It is ok to run out of objects.
	checkExpression (AAFRESULT_NO_MORE_OBJECTS == _hr, _hr);
	_hr = AAFRESULT_SUCCESS;
	
	_pEnumEssenceData->Release();
	_pEnumEssenceData = NULL;
}

void ContentStorageTest::writeEssenceData(IAAFEssenceData *pEssenceData,
				       const aafDataBuffer_t data,
				       aafUInt32 dataSize)
{
	assert(NULL != pEssenceData);
	
	// NOTE: The following code assumes that the compiler supports 64 bit
	// numbers.
	
	aafPosition_t startingPosition = 0;
	aafPosition_t expectedPosition = 0;
	aafPosition_t dataPosition = 0;
	aafUInt32 bytesWritten = 0;
	aafUInt32 offset = 0;
	
	
	
	// Save the starting position.
	check(pEssenceData->GetPosition(&startingPosition));
	
	for (offset = 0; offset < dataSize; ++offset)
	{   
		// Reset the position to test SetPosition.
		dataPosition = startingPosition + offset; 
		check(pEssenceData->SetPosition(dataPosition));
		
		bytesWritten = 0;
		check(pEssenceData->Write(dataSize - offset, data + offset, &bytesWritten));
		// Make sure that we wrote the expected number of bytes.
		checkExpression(bytesWritten == (dataSize - offset), AAFRESULT_TEST_FAILED);
		
		// Make sure the position is supported correctly.
		expectedPosition = startingPosition + dataSize;
		dataPosition = 0;
		check(pEssenceData->GetPosition(&dataPosition));
		checkExpression(dataPosition == expectedPosition, AAFRESULT_TEST_FAILED);
	}
}

void ContentStorageTest::readEssenceData(IAAFEssenceData *pEssenceData,
				      const aafDataBuffer_t data,
				      aafUInt32 dataSize)
{
	assert(NULL != pEssenceData);
	
	// NOTE: The following code assumes that the compiler supports 64 bit
	// numbers.
	
	aafPosition_t startingPosition = 0;
	aafPosition_t expectedPosition = 0;
	aafPosition_t dataPosition = 0;
	aafUInt32 bytesRead = 0;
	aafUInt32 offset = 0;
	
	
	
	// Save the starting position.
	check(pEssenceData->GetPosition(&startingPosition));
	
	// Make sure the buffer is large enough to hold the data.
	setBufferSize(dataSize);
	
	for (offset = 0; offset < dataSize; ++offset)
	{   
		// Reset the position to test SetPosition.
		dataPosition = startingPosition + offset; 
		check(pEssenceData->SetPosition(dataPosition));
		
		// Read dataSize bytes from the essence data.
		bytesRead = 0;
		check(pEssenceData->Read(dataSize - offset, _buffer, &bytesRead));
		// Make sure that we read the expected number of bytes.
		checkExpression(bytesRead == (dataSize - offset), AAFRESULT_TEST_FAILED);
		
		// Make sure the position is supported correctly.
		expectedPosition = startingPosition + dataSize;
		dataPosition = 0;
		check(pEssenceData->GetPosition(&dataPosition));
		checkExpression(dataPosition == expectedPosition, AAFRESULT_TEST_FAILED);
		
		
		// Compare the bytes we read to the given input data.
		// (this is assumed to match the original data written
		// to the essence data)
		checkExpression (0 == memcmp(data + offset, (char *)_buffer, dataSize - offset), AAFRESULT_TEST_FAILED);
	}
}


