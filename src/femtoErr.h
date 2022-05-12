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
} femtoErr_t;
/**
 * @brief Prints error message to stderr, adds newline character
 * 
 * @param errCode Error code
 */
void femto_printErr(femtoErr_t errCode);


#endif
