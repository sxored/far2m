#include <all_far.h>

#include "Int.h"

BOOL SayNotReadedTerminates(LPCSTR fnm, BOOL &SkipAll)
{
	LPCSTR MsgItems[] = {FMSG(MError), FMSG(MCannotCopyHost), fnm,
			/*0*/ FMSG(MCopySkip),
			/*1*/ FMSG(MCopySkipAll),
			/*2*/ FMSG(MCopyCancel)};

	if (SkipAll)
		return FALSE;

	switch (FMessage(FMSG_WARNING | FMSG_DOWN | FMSG_ERRORTYPE, NULL, MsgItems, ARRAYSIZE(MsgItems), 3)) {
		case 1:
			SkipAll = TRUE;
		case 0:
			break;
		default:
			return TRUE;
	}

	return FALSE;
}

int FTP::PutHostsFiles(struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode)
{
	BOOL SkipAll = FALSE;
	String DestPath, SrcFile, DestName, CurName;
	struct stat src_stat = {0};
	int n;
	FTPHost h;
	FP_SizeItemList il;
	DestPath = HostsPath;
	AddEndSlash(DestPath, '/');

	if (!ExpandList(PanelItem, ItemsNumber, &il, FALSE))
		return 0;

	for (n = 0; n < il.Count(); n++) {
		if (CheckForEsc(FALSE))
			return -1;

		h.Init();
		CurName = FTP_FILENAME(&il.List[n]);

		if (sdc_stat(CurName.c_str(), &src_stat) < 0)
			continue;

		if ((src_stat.st_mode & S_IFMT) == S_IFDIR) {
			h.Folder = TRUE;
			StrCpy(h.Host, CurName.c_str(), ARRAYSIZE(h.Host));
			h.Write(HostsPath);
			continue;
		}

		if (CurName.Chr('/') == -1) {
			SrcFile.printf("./%s", CurName.c_str());
			DestName = DestPath;
		} else {
			SrcFile = CurName;
			DestName = DestPath;
			DestName.Add(CurName);
			DestName.SetLength(DestName.RChr('/'));
		}

		if (!h.ReadINI(SrcFile.c_str())) {
			if (!IS_SILENT(OpMode))
				if (SayNotReadedTerminates(SrcFile.c_str(), SkipAll))
					return -1;

			continue;
		}

		Log(("Write new host [%s] -> [%s]", SrcFile.c_str(), DestName.c_str()));
		h.Write(DestName.c_str());

		if (Move) {
			// WINPORT(SetFileAttributes)(CurName.c_str(),0);
			remove(CurName.c_str());
			// WINPORT(DeleteFile)(CurName.c_str());
		}

		if (n < ItemsNumber)
			PanelItem[n].Flags&= ~PPIF_SELECTED;
	}

	if (Move) {
		for (n = il.Count() - 1; n >= 0; n--) {
			if (CheckForEsc(FALSE))
				return -1;

			if (IS_FLAG(il.List[n].FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
				if (rmdir(FTP_FILENAME(&il.List[n])) == 0)
					if (n < ItemsNumber)
						PanelItem[n].Flags&= ~PPIF_SELECTED;
		}
	}

	return 1;
}
