#include <ftw.h>
#include "BatchLogic.h"

/*
 * Batch Logic skeleton:
 * 
 * 1. Query search term (Show matches, if any). Record to history. Ask to show all results if amount is high.
 *  1a. Yes, jump to 2
 *  2a. No, jump to 1
 * 2. Shift focus to results window. User selects files to delete.
 *  2a. If none are selected do nothing.
 * 3. Ask if user is sure about deletion
 *  3a. Yes, delete files. Show total size of deleted files. Record to history.
 *  3b. No, jump to 2.
 * 4. Jump to 1. 
 */ 

/* Searches for files. Returns amount of results. */
