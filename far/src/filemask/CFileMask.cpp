/*
CFileMask.cpp

Основной класс для работы с масками файлов. Использовать нужно именно его.
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"

#include "CFileMask.hpp"
#include "FileMasksProcessor.hpp"
#include "FileMasksWithExclude.hpp"
#include "lang.hpp"
#include "message.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "KeyFileHelper.h"

CFileMask::CFileMask():
	FileMask(nullptr)
{
}

void CFileMask::Free()
{
	if (FileMask)
		delete FileMask;

	FileMask=nullptr;
}

/*
 Инициализирует список масок. Принимает список, разделенных запятой или точкой
 с запятой. Разрешается указывать маски исключения, отделив их от основных
 символом '|' Возвращает FALSE при неудаче (например, одна из длина одной из
 масок равна 0).
*/

bool CFileMask::Set(const wchar_t *Masks, DWORD Flags)
{
	Free();
	bool Result=false;
	int Silent=Flags & FMF_SILENT;

	FARString strMask(Masks);
	RemoveTrailingSpaces(strMask);

	if (!strMask.IsEmpty())
	{
		bool Error=false;
		//	auto pStart=strMask.CPtr();
		//
		//	if (*pStart == L'<')
		//	{
		//		auto pEnd = wcschr(pStart+1, L'>');
		//		if (pEnd && pEnd[1]==0)
		//		{
		//			Error=true;
		//			KeyFileReadSection kfh(InMyConfig("settings/masks.ini"), "Masks");
		//
		//			if (kfh.SectionLoaded())
		//			{
		//				FARString strKey(pStart+1, pEnd-pStart-1);
		//				const std::string& strValue = kfh.GetString(Wide2MB(strKey));
		//
		//				if (!strValue.empty())
		//				{
		//					strMask = strValue;
		//					Error=false;
		//				}
		//			}
		//		}
		//	}
		if (!Error)
		{
			Masks = strMask.CPtr();
			if (FileMasksWithExclude::IsExcludeMask(Masks))
			{
				FileMask=new FileMasksWithExclude;
			}
			else
			{
				FileMask=new FileMasksProcessor;
			}

			if (FileMask)
			{
				DWORD flags = (Flags & FMF_ADDASTERISK) ? FMPF_ADDASTERISK : 0;
				Result=FileMask->Set(Masks, flags);
			}
		}

		if (!Result)
			Free();
	}

	if (!Silent && !Result)
		Message(MSG_WARNING,1,Msg::Warning,Msg::IncorrectMask, Msg::Ok);

	return Result;
}

// Возвращает TRUE, если список масок пустой
bool CFileMask::IsEmpty()const
{
	return FileMask?FileMask->IsEmpty():true;
}

/* сравнить имя файла со списком масок
   Возвращает TRUE в случае успеха.
*/
bool CFileMask::Compare(const wchar_t *FileName, bool SkipPath) const
{
	return FileMask ? FileMask->Compare(SkipPath ? PointToName(FileName):FileName) : false;
}