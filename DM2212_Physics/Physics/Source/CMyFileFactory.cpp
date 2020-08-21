#include "CMyFileFactory.h"

irrklang::IFileReader* CMyFileFactory::createFileReader(const ik_c8* filename)
{
	FILE* file = fopen(filename, "rb");
	if (!file)
		return 0;

	return new CMyReadFile(file, filename);
}

CMyFileFactory::CMyReadFile::CMyReadFile(FILE* openedFile, const ik_c8* filename)
{
	
		File = openedFile;
		strcpy_s(Filename, filename);

		fseek(File, 0, SEEK_END);
		FileSize = ftell(File);
		fseek(File, 0, SEEK_SET);
	
}

CMyFileFactory::CMyReadFile::~CMyReadFile()
{
	fclose(File);
}

ik_s32 CMyFileFactory::CMyReadFile::read(void* buffer, ik_u32 sizetoRead)
{
	return fread(buffer, 1, sizetoRead, File);
}

bool CMyFileFactory::CMyReadFile::seek(ik_s32 finalPos, bool relativeMovement)
{
	return fseek(File, finalPos, relativeMovement ? SEEK_CUR : SEEK_SET) == 0;
}

ik_s32 CMyFileFactory::CMyReadFile::getSize()
{
	return FileSize;
}

ik_s32 CMyFileFactory::CMyReadFile::getPos()
{
	return ftell(File);
}

const ik_c8* CMyFileFactory::CMyReadFile::getFileName()
{
	return Filename;
}
