/*
 * The Tree Command
 * SKELETON IMPLEMENTATION TO BE FILLED IN FOR TASK 3
 */

#include <infos.h>


/*
 * concatenates a second string to the end of the first str 
 */
char *strcat(char *first, const char *second)
{
    char *start = first;
    while (*first){first++;} //traverse to end of first guy
    while (*first++ = *second++); //
    return start;
}

/*
 * copies a string to an empty char[] given 
 */
char * strcpy(char *target, const char *guy)
{
    char *start = target;
    while((*target++=*guy++) != '\0'); //copy the char until end
    return start;
}



void fill(char *arr,int depth,bool is_last_dir){
    const char* bar = "|   ";
    const char* empty = "    ";
    const char* dash =  "|---";
    
    if(depth == 0){arr = strcat(arr,dash);}

    else if(is_last_dir){arr = strcat(arr,empty);
        for(int i=0;i<depth-1;i++){
            arr = strcat(arr,bar);
        }
        arr = strcat(arr,dash);
    } 

    else{arr = strcat(arr,bar);
        for(int i=0;i<depth-1;i++){
            arr = strcat(arr,bar);
        }
        arr = strcat(arr,dash);
    }
    

}
int strCmp(const char* s1, const char* s2)
{
    while(*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}
bool parse_dir(char * path,int depth,bool is_last_dir,int* count){

    char curr[256]; 
    strcpy(curr,path); // get current path 
    char* curr_dir = strcat(curr,"/");  // make "path" -> "path/"
    
    // opening current directory, next_dir is for checking if we reached the last guy in a directory.
    HDIR dir = opendir(curr, 0);
    HDIR next_dir = opendir(curr, 0);
    struct dirent guy;
    struct dirent next_guy;


	if (is_error(dir)) { return 0 ;} //if the current guy is either an empty dir or a file, end recursive call.


    char header[(4*(depth+1))+1]; 
    fill(header,depth,is_last_dir);

    int isReading = 1;
    readdir(next_dir, &next_guy);

	while (isReading) {
        isReading = readdir(dir, &guy);
        readdir(next_dir, &next_guy);

        if(!isReading){break;} // done reading directory
        if(strCmp(guy.name,next_guy.name) == 0){is_last_dir = true;} 

        printf("%s %s\n",header,guy.name);

        char next_path[256];
        strcpy(next_path,curr_dir);
        strcat(next_path,guy.name);
        if(parse_dir(next_path,depth+1, is_last_dir,count)){
            *(count) += 1;  
        }
        else{
            *(count+1) += 1;
        }
	}

    header[0] = 0;
    is_last_dir = false;
	closedir(dir);
    if(depth == 0){
        printf("%d directories, %d files\n",*(count),*(count+1));
    }
    return 1; 
} 

int main(const char *cmdline)
{

	const char *path;
	if (!cmdline || strlen(cmdline) == 0) {
		path = "/usr";
	} else {
		path = cmdline;
	}
	
	HDIR dir = opendir(path, 0);
	if (is_error(dir)) {
		printf("Unable to open directory '%s' for reading.\n", path);
		return 1;
	}
	
	printf("Directory Listing of '%s':\n", path);

    
    char next_path[256];
    int depth = 0;
    int count[2];
    int *ptr;
    ptr = count;
    bool is_last_dir = false;
    strcpy(next_path,path);
    printf(".\n");
    parse_dir(next_path,depth,is_last_dir, ptr);

	return 0;
}