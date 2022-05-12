#ifndef FEMTOERR_H
#define FEMTOERR_H

typedef enum femtoErr
{
	ferrOK,
	ferrUNKNOWN,
	ferrFILE,
	ferrWINDOW,
	ferrMEMORY,
	ferrTHREAD,

	ferrNUM_OF_ELEMS

} femtoErr_e;
/**
 * @brief Prints error message to stderr, adds newline character
 * 
 * @param errCode Error code, ferrOK on success
 */
void femto_printErr(femtoErr_e errCode);


#endif
