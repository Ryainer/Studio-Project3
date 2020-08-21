#pragma once
#include <iostream>
#include <string.h>
#include "..\\irrKlang\includes\irrKlang.h"

using namespace irrklang;

#pragma comment(lib, "irrKlang.lib")

class CMyFileFactory : public irrklang::IFileFactory
{
public:
	virtual irrklang::IFileReader* createFileReader(const ik_c8* filename);

protected:

	class CMyReadFile : public irrklang::IFileReader
	{
	public:
		CMyReadFile(FILE* openedFile, const ik_c8* filename);
		~CMyReadFile();

		ik_s32 read(void* buffer, ik_u32 sizetoRead);
		bool seek(ik_s32 finalPos, bool relativeMovement);
		ik_s32 getSize();
		ik_s32 getPos();
		const ik_c8* getFileName();

		FILE* File;
		char Filename[1024];
		ik_s32 FileSize;

	};

};

