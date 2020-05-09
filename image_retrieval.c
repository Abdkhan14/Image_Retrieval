#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <float.h>
#include "worker.h"
//ABDULLAH FINAL

int main(int argc, char **argv) {

	char ch;
	char path[PATHLENGTH];
	char *startdir = ".";
        char *image_file = NULL;

	while((ch = getopt(argc, argv, "d:")) != -1) {
		switch (ch) {
			case 'd':
			startdir = optarg;
			break;
			default:
			fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME] FILE_NAME\n");
			exit(1);
		}
	}

        if (optind != argc-1) {
	     fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME] FILE_NAME\n");
        } else
             image_file = argv[optind];

	// Open the directory provided by the user (or current working directory)

	Image *img = read_image(image_file);
	if(!img){
        exit(1);
	}
    //////////////////////////////////////////////
	DIR *dirp;
	if((dirp = opendir(startdir)) == NULL) {
		perror("opendir");
		exit(1);
	}

    ////////////////////////////////////////////////////////////////////////
	struct dirent *dp;
    CompRecord CRec;
    strcpy(CRec.filename,"");
    CRec.distance = FLT_MAX;
    int i=0;
    int size = 0;
    ////////////////////////////////////////////////////////////////////////
 	while((dp = readdir(dirp)) != NULL) {

		if(strcmp(dp->d_name, ".") == 0 ||
		   strcmp(dp->d_name, "..") == 0 ||
		   strcmp(dp->d_name, ".svn") == 0){
			continue;
		}
		strncpy(path, startdir, PATHLENGTH);
		strncat(path, "/", PATHLENGTH - strlen(path) - 1);
		strncat(path, dp->d_name, PATHLENGTH - strlen(path) - 1);
		struct stat sbuf;
		if(stat(path, &sbuf) == -1) {
			perror("stat");
			exit(1);
		}
		if(S_ISDIR(sbuf.st_mode)) {
                 size++;
		}
	}//while ends here
	//printf("Size is %d\n", size);
    int fd[size][2];
    ///////////////////////////////////////////////////////////////////////
    DIR *dirp1;
	if((dirp1 = opendir(startdir)) == NULL) {
		perror("opendir");
		exit(1);
	}

	while((dp = readdir(dirp1)) != NULL) {

		if(strcmp(dp->d_name, ".") == 0 ||
		   strcmp(dp->d_name, "..") == 0 ||
		   strcmp(dp->d_name, ".svn") == 0){
			continue;
		}
		strncpy(path, startdir, PATHLENGTH);
		strncat(path, "/", PATHLENGTH - strlen(path) - 1);
		strncat(path, dp->d_name, PATHLENGTH - strlen(path) - 1);

		struct stat sbuf;
		if(stat(path, &sbuf) == -1) {
			perror("stat");
			exit(1);
		}

		// Only call process_dir if it is a directory
		// Otherwise ignore it.
		if(S_ISDIR(sbuf.st_mode)) {
                       // printf("Processing all images in directory: %s \n", path);
                         if(pipe(fd[i])==-1){
                            perror("pipe");
                            exit(1);
                         }
                         int result = fork();
                         if(result<0){
                            perror("fork");
                            exit(1);
                         }else if(result==0){
                             if(close(fd[i][0])==-1){
                                perror("close");
                                exit(1);
                             }
                             int c;
                             for(c=1;c<i;c++){
                                if(close(fd[c][0])==-1){
                                    perror("close");
                                    exit(1);
                                }
                             }
                             process_dir(path,img,fd[i][1]);
                         }else{

                            if(close(fd[i][1])==-1){
                                perror("close");
                                exit(1);
                            }
                         }
                         i++;
		}
	}//while ends here


	CompRecord cnt;
	for(int j = 0;j<size;j++){
        if(read(fd[j][0],&cnt,sizeof(CompRecord))!= sizeof(CompRecord) ){
                perror("read");
                exit(1);

        }

        if(cnt.distance<CRec.distance){
            strcpy(CRec.filename,cnt.filename);
            CRec.distance = cnt.distance;

        }
	}

        printf("The most similar image is %s with a distance of %f\n", CRec.filename, CRec.distance);

        free(img->p);
        free(img);

	return 0;
}


