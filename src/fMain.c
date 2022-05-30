#include "femto.h"
#include "fArg.h"
#include "fSyntax.h"


static femtoData_t editor;

int wmain(int argc, const wchar * argv[])
{
	femto_exitHandlerSetVars(&editor);
	if (!femtoData_reset(&editor))
	{
		femto_printErr(ferrMEMORY);
		return 1;
	}

	// Initialise profiler, if applicable
	initProfiler();

	// Limit the scope of errCode
	{
		femtoErr_e errCode = femtoSettings_populate(&editor.settings, argc, argv);
		if (errCode != ferrOK)
		{
			if (errCode == ferrUNKNOWN)
			{
				// Get last error
				wchar errMsg[FEMTO_SETTINGS_ERR_MAX];
				femtoSettings_getLastError(&editor.settings, errMsg, FEMTO_SETTINGS_ERR_MAX);
				wprintf(L"[Settings]: %S\n", errMsg);
				femto_printHelpClue(argv[0]);
				return 2;
			}
			else
			{
				femto_printErr(errCode);
				return 2;
			}
		}
		else if (editor.settings.bHelpRequest || (argc == 1))
		{
			femto_printHelp(argv[0]);
			return 0;
		}
	}


	if (!femtoData_openTab(&editor, editor.settings.fileName))
	{
		femto_printErr(ferrFILE);
		return 3;
	}

	if (!femtoData_init(&editor))
	{
		femto_printErr(ferrWINDOW);
		return 4;
	}

	{
		wchar tempstr[MAX_STATUS];
		const wchar * res;
		if ((res = femtoFile_read(editor.files[editor.fileIdx])) != NULL)
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
				fSyntaxName(editor.files[editor.fileIdx]->syntax)
			);
		}
		femtoData_statusDraw(&editor, tempstr, NULL);
	}

	femtoData_refresh(&editor);

	if (!femto_loopDraw_createThread(&editor))
	{
		femto_printErr(ferrTHREAD);
		return 5;
	}

	while (femto_loop(&editor));

	femto_loopDraw_closeThread(&editor);

	return 0;
}
