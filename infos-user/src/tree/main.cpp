/*
 * The Tree Command
 * SKELETON IMPLEMENTATION TO BE FILLED IN FOR TASK 3
 */


#include <infos.h>

bool isEnd(char next_dir, *ptr, int n_files, int n_dir) {
    //we need to search if there is a / for next dir
    char cur_dir[256];
}


int main(const char *cmdline)
{
	const char *path;
	char next_dir[256];
	bool end;
	int *ptr; 
	
	//keep count of total no. of files and directories, and tree structure
	int depth = 0;
	int n_files = 0;
	int n_dir = 0; 

	if (!cmdline || strlen(cmdline) == 0) {
		path = "/usr";
	} else {
		path = cmdline;
	}
	
	HDIR cur_dir = opendir(path, 0);
	if (is_error(cur_dir)) {
		printf("Unable to open current top directory '%s' for reading.\n", path);
		return 1;
	}
	
	printf("Directory Listing of '%s':\n", path);
	
	end = false;
	
	// strcpy(next_dir,path);
	printf(".\n");
	// parse_dir(next_dir, depth, end, ptr);

	return 0;
}
