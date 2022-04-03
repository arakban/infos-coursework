/*
 * The Tree Command
 * SKELETON IMPLEMENTATION TO BE FILLED IN FOR TASK 3
 */

#include <infos.h>

/* dir structure:
    export top-dir := $(CURDIR)
    export inc-dir := $(top-dir)/inc
    export crt-dir := $(top-dir)/crt
    export lib-dir := $(top-dir)/lib
    export src-dir := $(top-dir)/src
    export bin-dir := $(top-dir)/bin
    */

int main(const char *cmdline)
{   
    // get top-dir
    const char *path;
    printf('Trying to get top dir');
    
    // no command, recursively search for files  
    
    if (!cmdline || strlen(cmdline) == 0) {
		path = "/usr";
    }
    return 0;
}