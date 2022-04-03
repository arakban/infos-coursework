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
    // get pointer to top-dir
    const char *path;
    
    if (!cmdline || strlen(cmdline) == 0) {
      // no arg, recursively search for files in usr dir  
      printf('Trying to get top dir /usr then re');
		  path = "/usr";
    } else {
      //get this path and run functions to parse it
	    path = cmdline;
	  }

    //now open root dir
    HDIR dir = opendir(path, 0);
	  if (is_error(dir)) {
		    printf("Unable to open directory '%s' for reading.\n", path);
		    return 1;
	  }

    printf("Directory Listing of '%s':\n", path);

    // struct dirent child;
	  // while (readdir(dir, &child)) {
    //   printf("Going to next guy");
	  // }
	  // closedir(dir);

    return 0;
}