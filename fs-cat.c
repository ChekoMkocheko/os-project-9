/*
* Cheko Mkocheko
*fs-cat.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <ufs/ufs/dinode.h>
#include <ufs/ufs/dir.h>
#include <ufs/ffs/fs.h>
#include <sys/types.h>
#include <sys/stat.h>

char *pt_mem; //memory offeset of partitioned image
struct fs *fp; //image of partitioned file sysstem
u_int32_t total_inodes; //number of all inodes in the file system
int flag_root = 1; //flag for indicating when we are accessing the root for the first time


void print_file_content(struct ufs2_dinode *ptr);
int  get_inode_num(int inode_num, char *file_name);
int inode_addr(int inode_num);


// this returns the address of an inode number
// it takes superblk and inode number as inputs
int inode_addr(int inode_num){
	// get exact fragment of inode
	int abs_frag = ino_to_fsba(fp, inode_num);
	// get the position of the inode in the cyl group
	int exact_ino_pos_blk = ino_to_fsbo(fp, inode_num);
	return abs_frag*fp->fs_fsize + exact_ino_pos_blk*(sizeof(struct ufs2_dinode));
}

// given a file name, find the inode number for that file
// takes in an inode number of the parent directory and returns
// the inode number of the file we are looking for if it is in the directory
// Returns zero if no inode is found
int get_inode_num(int inode_num, char *file_name){
    int file_inode_num = 0; 
	struct ufs2_dinode *curr_inode;
	curr_inode = (struct ufs2_dinode *)(inode_addr(inode_num)+ pt_mem);
	ufs2_daddr_t *inode_db_arr = curr_inode->di_db; //direct blks
	ufs2_daddr_t *inode_ib_arr = curr_inode->di_ib; //indirect blks
	int padding = 0;
	int not_end = 1;// not end of data
	while(not_end) {
	struct direct *next_dir = (struct direct *)(inode_db_arr[0] * MINBSIZE + pt_mem + padding);
	if(strcmp(next_dir->d_name, file_name) == 0){
		curr_inode = (struct ufs2_dinode *)(inode_addr(next_dir->d_ino) + pt_mem);	
		file_inode_num = next_dir->d_ino;
		break; //found the inode for the file, break;
	}
	//go to the next struct direct, if there are no more directs then break the loop
	if(next_dir->d_ino == 0){
			not_end = 0;
		}
	// add offest to the next struct direct
	padding += next_dir->d_reclen;
	}
	return file_inode_num;
}

// given an inode address (file's inode address) print, find its data blocks 
// and print the content of the data block
// Print "FILE NOT FOUND" if the address is invalid, 
void print_file_content(struct ufs2_dinode *curr_file){
    printf("size of curr_file %lu: \n", curr_file->di_size); 
	ufs2_daddr_t *file_db_arr = curr_file->di_db; //direct blks
	ufs2_daddr_t *file_ib_arr = curr_file->di_ib; //indirect blks
	char *data = (char *) (file_db_arr[0] * MINBSIZE + pt_mem);
	//printf("%s\n", data);
	if (strlen(data) == 0) {
		printf("FILE IS EMPTY\n");
	}
	else{
	 	printf("%s\n", data);

	}
}


void print_files(char *disk_image_path,char *file_name){
	int fd;
	int partition_size;
	struct stat s;

	fd=open(disk_image_path,O_RDONLY);
	fstat(fd, &s);
	partition_size = s.st_size;
	pt_mem = mmap(0, partition_size, PROT_READ, MAP_PRIVATE, fd, 0);

	if(pt_mem == MAP_FAILED){
		perror(NULL);
		printf("Mapping Failed\n");
		exit(1);
	}

	fp = (struct fs *)( pt_mem + SBLOCK_UFS2);

	
	int parent_inode_num = 2;
	char *fname_copy;
	int slash_dlmt = 1; // forward slah delimeter
	if('/' != file_name[0]){ // check if file name is path or just name
	slash_dlmt = 0; 
	}
	fname_copy = malloc(sizeof(char) * (strlen(file_name) + 1));
	strcpy(fname_copy, file_name);
	char *curr_fname = NULL;
	int file_len = strlen(fname_copy);
	if(slash_dlmt){// entered file path and name at the end, extract name of file
	    while(0 < file_len && fname_copy[--file_len] !='/'){
        //;
		    if(fname_copy[file_len] == '/'){
			    curr_fname = fname_copy + file_len + 1;
			    fname_copy[file_len] = '\0';
		    }	
        }
    }else{ // entered file name without path, file at root
		curr_fname = fname_copy;
	}
    

	 const char str[2] = "/";
     char *token = strtok(file_name,str);
	 int file_inode_num = 0;
	 struct ufs2_dinode *file_addr;
	 // traverse the file path
        while(token != NULL){
		file_inode_num = get_inode_num(parent_inode_num,token);
        printf("File inode %d\n", file_inode_num);
		if(file_inode_num == 0){
			printf("FILE NOT FOUND\n");
			break;
		}
		// get inode inode num for file address
		file_addr = (struct ufs2_dinode *)(inode_addr(file_inode_num)+ pt_mem);
		ufs2_daddr_t *db_arr = file_addr->di_db; // access data blocks of the file
		struct direct *dir = (struct direct *)(db_arr[0] * MINBSIZE + pt_mem);
		parent_inode_num = dir->d_ino;
		// reached end of path, try to print file
		if(strcmp(curr_fname, token)==0){
		print_file_content(file_addr);
		}
		token = strtok(NULL, str);
         }
	
	int err = munmap(pt_mem, partition_size);
	if(err!=0){
		printf("UnMapping Failed\n");
		exit(1);
	}
	close(fd);
}


int main(int argc, char **argv){
	if(argc < 3){
		printf("Usage: ./fs-cat [ partition-disk path/filename.img]\n");
		return 1;
	}
	char *disk_image = argv[1];
	char *file_path = argv[2];
	///cheko/cheko_one/text_one/sample_one.txt
	print_files(disk_image, file_path);

	
}
