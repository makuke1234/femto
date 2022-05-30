#include "femto.h"
#include "fArg.h"
#include "fSyntax.h"

// This variable has to be static because it is used by the exit handler
// which is run after returning from wmain
static fData_t editor;

int wmain(int argc, const wchar * argv[])
{
	femto_exitHandlerSetVars(&editor);
	if (!fData_reset(&editor))
	{
		fErr_print(ferrMEMORY);
		return 1;
	}

	// Initialise profiler, if applicable
	fProf_init();

	// Limit the scope of errCode
	{
		fErr_e errCode = fSettings_cmdLine(&editor.settings, argc, argv);
		if (errCode != ferrOK)
		{
			if (errCode == ferrUNKNOWN)
			{
				// Get last error
				wchar errMsg[FEMTO_SETTINGS_ERR_MAX];
				fSettings_lastError(&editor.settings, errMsg, FEMTO_SETTINGS_ERR_MAX);
				wprintf(L"[Settings]: %S\n", errMsg);
				femto_printHelpClue(argv[0]);
				return 2;
			}
			else
			{
				fErr_print(errCode);
				return 2;
			}
		}
		else if (editor.settings.bHelpRequest || (argc == 1))
		{
			femto_printHelp(argv[0]);
			return 0;
		}
	}


	if (!fData_openTab(&editor, editor.settings.fileName))
	{
		fErr_print(ferrFILE);
		return 3;
	}

	if (!fData_init(&editor))
	{
		fErr_print(ferrWINDOW);
		return 4;
	}

	{
		wchar tempstr[MAX_STATUS];
		const wchar * res;
		if ((res = fFile_read(editor.files[editor.fileIdx])) != NULL)
		{
			wcscpy_s(tempstr, MAX_STATUS, res);
		}
		else
		{
			swprintf_s(
				tempstr, MAX_STATUS,
				L"File loaded successfully! %s%s EOL sequences; Settings file: %s; Syntax: %S",
				(editor.files[editor.fileIdx]->eolSeq & eolCR) ? L"CR" : L"",
				(editor.files[editor.fileIdx]->eolSeq & eolLF) ? L"LF" : L"",
				(editor.settings.settingsFileName != NULL) ? editor.settings.settingsFileName : L"-",
				fStx_name(editor.files[editor.fileIdx]->syntax)
			);
		}
		fData_statusMsg(&editor, tempstr, NULL);
	}

	fData_refreshEdit(&editor);

	if (!femto_asyncDrawInit(&editor))
	{
		fErr_print(ferrTHREAD);
		return 5;
	}

	while (femto_loop(&editor));

	femto_asyncDrawStop(&editor);

	return 0;
}
