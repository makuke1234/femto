#include "femtoSettings.h"


void femtoSettings_reset(femtoSettings_t * restrict self)
{
	assert(self != NULL);
	*self = (femtoSettings_t){
		.helpRequested = false,
		.fileName      = NULL,


		
		.lastErr       = { 0 }
	};
}

bool femtoSettings_getLastError(femtoSettings_t * restrict self, wchar_t * restrict errArr, int32_t errMax)
{
	assert(self != NULL);
	assert(errArr != NULL);
	if (self->lastErr[0] == L'\0')
	{
		errArr[0] = L'\0';
		return false;
	}
	uint32_t copyLen = (errMax == -1) ? (uint32_t)wcslen(self->lastErr) + 1 : u32Min((uint32_t)wcslen(self->lastErr) + 1, (uint32_t)errMax);
	memcpy(errArr, self->lastErr, sizeof(wchar_t) * copyLen);
	self->lastErr[copyLen - 1] = L'\0';
	return true;
}

bool femtoSettings_populate(femtoSettings_t * restrict self, int argc, const wchar_t ** restrict argv)
{
	assert(self != NULL);
	assert(argv != NULL);


	return true;
}
