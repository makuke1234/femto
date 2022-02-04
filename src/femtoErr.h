#ifndef FEMTOERR_H
#define FEMTOERR_H

typedef enum femtoErr
{
	femtoErr_ok,
	femtoErr_unknown,
	femtoErr_file,
	femtoErr_window,
	femtoErr_memory,

	femtoErr_num_of_elems
} femtoErr_t;
/**
 * @brief Prints error message to stderr, adds newline character
 * 
 * @param errCode Error code
 */
void femto_printErr(femtoErr_t errCode);


#endif
