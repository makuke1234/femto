#ifndef FEMTO_ERR_H
#define FEMTO_ERR_H

typedef enum fErr
{
	ferrOK,
	ferrUNKNOWN,
	ferrFILE,
	ferrWINDOW,
	ferrMEMORY,
	ferrTHREAD,

	ferrNUM_OF_ELEMS

} fErr_e;
/**
 * @brief Prints error message to stderr, adds newline character
 * 
 * @param errCode Error code, ferrOK on success
 */
void fErr_print(fErr_e errCode);


#endif
