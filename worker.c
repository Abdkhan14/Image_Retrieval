#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <math.h>
#include <float.h>
#include "worker.h"



/*
 * Read an image from a file and create a corresponding
 * image struct
 */
 //ABDULLAH FINAL

Image* read_image(char *filename)
{   char buffer[3];
    Image *img=NULL;
    FILE *fp = fopen(filename,"r");
    if(!fp){
        fprintf(stderr, "Didn't open file for reading\n");
        exit(1);
    }

    img = (Image *)malloc(sizeof(Image));
    if(!img){
        fprintf(stderr,"size cudn't be allocated for img\n");

    }


    fscanf(fp,"%s",buffer);
    //printf("%c%c\n",buffer[0],buffer[1]);
    if(buffer[0]!='P' || buffer[1] != '3'){
       // printf("INVALID PPM FORMAT\n");
        return NULL;
    }

    fscanf(fp,"%d %d %d",&img->width,&img->height,&img->max_value);
    int r,b,g;
    int i=0;
    img->p = (Pixel *)malloc(sizeof(Pixel)*(img->width)*(img->height));
    while(fscanf(fp,"%d %d %d",&r,&b,&g)==3){
        Pixel *p = (Pixel *)malloc(sizeof(Pixel));
        if(!p){
            fprintf(stderr, "memory not allocated for p\n");
        }
        p->red = r;
        p->blue = b;
        p->green = g;
        img->p[i] = *p;
        free(p);
        i++;

    }


    fclose(fp);
    return img;
}

/*
 * Print an image based on the provided Image struct
 */

void print_image(Image *img){
        printf("P3\n");
        printf("%d %d\n", img->width, img->height);
        printf("%d\n", img->max_value);

        for(int i=0; i<img->width*img->height; i++)
           printf("%d %d %d  ", img->p[i].red, img->p[i].blue, img->p[i].green);
        printf("\n");
}

/*
 * Compute the Euclidian distance between two pixels
 */
float eucl_distance (Pixel p1, Pixel p2) {
        return sqrt( pow(p1.red - p2.red,2 ) + pow( p1.blue - p2.blue, 2) + pow(p1.green - p2.green, 2));

}

/*
 * Compute the average Euclidian distance between the pixels
 * in the image provided by img1 and the image contained in
 * the file filename
 */

float compare_images(Image *img1, char *filename) {

    Image *img2 = read_image(filename);
    if(!img2){
        return FLT_MAX;
    }
    if((img2->width!=img1->width)|| (img2->height!=img1->height)){
        //printf("TWO IMAGES DON'T HAVE THE SAME DIMENSIONS!!\n");
        return FLT_MAX;
    }

    Pixel *p1 = img1->p;
    Pixel *p2 = img2->p;

    int size = img1->width*img2->height;

    float sum = 0.00;
    int i;
    for(i=0;i<size;i++){

        sum+=eucl_distance(p1[i],p2[i]);

    }
    free(img2->p);
    free(img2);
return sum/(float)i;
}

/* process all files in one directory and find most similar image among them
* - open the directory and find all files in it
* - for each file read the image in it
* - compare the image read to the image passed as parameter
* - keep track of the image that is most similar
* - write a struct CompRecord with the info for the most similar image to out_fd
*/
CompRecord process_dir(char *dirname, Image *img, int out_fd){
    float max = FLT_MAX;
    char path[PATHLENGTH];
    CompRecord CRec;
    CRec.distance = max;
    DIR *dirp;
	if((dirp = opendir(dirname)) == NULL) {
		perror("opendir");
		exit(1);
	}
	struct dirent *dp;
	while((dp = readdir(dirp)) != NULL) {

 //   if(strstr(dp->d_name,"ppm")!=NULL|| strstr(dp->d_name,"txt")!=NULL){

        if(strcmp(dp->d_name, ".") == 0 ||
		   strcmp(dp->d_name, "..") == 0 ||
		   strcmp(dp->d_name, ".svn") == 0){
			continue;
		}
		strncpy(path, dirname, PATHLENGTH);
		strncat(path, "/", PATHLENGTH - strlen(path) - 1);
		strncat(path, dp->d_name, PATHLENGTH - strlen(path) - 1);

		struct stat sbuf;
		if(stat(path, &sbuf) == -1) {
			//This should only fail if we got the path wrong
			// or we don't have permissions on this entry.
			printf("Failed stat\n");
			exit(1);
		}

         if(S_ISREG(sbuf.st_mode)){
		    float result= compare_images(img,path);

		    if(result<max){
                max = result;
                CRec.distance = result;
                strcpy(CRec.filename,dp->d_name);
		    }

         }


		//}

	}

    if(out_fd!=-459){
                             //
                             if(write(out_fd,&CRec,sizeof(CompRecord))==-1){
                                perror("write");
                                exit(1);
                             }
                             //printf("JUST WROTE inside process_dir\n");
                             close(out_fd);
                             exit(0);

	}


        return CRec;
}






