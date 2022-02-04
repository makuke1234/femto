#include "femto.h"
#include "femtoArg.h"


static femtoData_t editor;

int wmain(int argc, const wchar_t * argv[])
{
	femto_exitHandlerSetVars(&editor);
	if (!femtoData_reset(&editor))
	{
		femto_printErr(femtoErr_memory);
		return 2;
	}

	// Initialise profiler, if applicable
	initProfiler();

	if (!femtoSettings_populate(&editor.settings, argc, argv))
	{
		// Get last error
		wchar_t errMsg[FEMTO_SETTINGS_ERR_MAX];
		femtoSettings_getLastError(&editor.settings, errMsg, FEMTO_SETTINGS_ERR_MAX);
		fputws(errMsg, stdout);
		femto_printHelpClue(argv[0]);
		return 1;
	}
	else if (editor.settings.helpRequested)
	{
		femto_printHelp(argv[0]);
		return 1;
	}


	if (!femtoFile_open(editor.file, editor.settings.fileName, false))
	{
		femto_printErr(femtoErr_file);
		return 2;
	}
	femtoFile_close(editor.file);

	// Set console title
	femtoFile_setConTitle(editor.file);

	if (!femtoData_init(&editor))
	{
		femto_printErr(femtoErr_window);
		return 3;
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
				L"File loaded successfully! %s%s EOL sequences",
				(editor.file->eolSeq & EOL_CR) ? L"CR" : L"",
				(editor.file->eolSeq & EOL_LF) ? L"LF" : L""
			);
		}
		femtoData_statusDraw(&editor, tempstr, NULL);
	}

	femtoData_refresh(&editor);
	while (femto_loop(&editor));

	return 0;
}
