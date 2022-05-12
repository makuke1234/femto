#include "femto.h"
#include "femtoArg.h"
#include "femtoSyntax.h"


static femtoData_t editor;

int wmain(int argc, const wchar_t * argv[])
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
				wchar_t errMsg[FEMTO_SETTINGS_ERR_MAX];
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


	if (!femtoFile_open(editor.file, editor.settings.fileName, false))
	{
		femto_printErr(ferrFILE);
		return 3;
	}
	femtoFile_close(editor.file);

	// Set console title
	femtoFile_setConTitle(editor.file);

	if (!femtoData_init(&editor))
	{
		femto_printErr(ferrWINDOW);
		return 4;
	}

	{
		wchar_t tempstr[MAX_STATUS];
		const wchar_t * res;
		if ((res = femtoFile_read(editor.file)) != NULL)
		{
			wcscpy_s(tempstr, MAX_STATUS, res);
		}
		else
		{
			swprintf_s(
				tempstr,
				MAX_STATUS,
				L"File loaded successfully! %s%s EOL sequences; Settings file: %s; Syntax: %S",
				(editor.file->eolSeq & eolCR) ? L"CR" : L"",
				(editor.file->eolSeq & eolLF) ? L"LF" : L"",
				(editor.settings.settingsFileName != NULL) ? editor.settings.settingsFileName : L"-",
				fSyntaxName(editor.file->syntax)
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
