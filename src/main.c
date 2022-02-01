#include "femto.h"


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

	const wchar_t * fileName = femto_getFileName(argc, argv);
	if (fileName == NULL)
	{
		femto_printHelp(argv[0]);
		return 1;
	}

	if (!femtoFile_open(editor.file, fileName, false))
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

	const wchar_t * res;
	writeProfiler("wmain", "Starting to read file...");
	if ((res = femtoFile_read(editor.file)) != NULL)
	{
		femtoData_statusDraw(&editor, res, NULL);
	}
	else
	{
		wchar_t tempstr[MAX_STATUS];
		swprintf_s(
			tempstr,
			MAX_STATUS,
			L"File loaded successfully! %s%s line endings.",
			(editor.file->eolSeq & EOL_CR) ? L"CR" : L"",
			(editor.file->eolSeq & EOL_LF) ? L"LF" : L""
		);
		femtoData_statusDraw(&editor, tempstr, NULL);
	}

	femtoData_refresh(&editor);
	while (femto_loop(&editor));

	return 0;
}
