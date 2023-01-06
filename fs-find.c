/*
*Cheko Mkocheko
*fs-find.c
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
void print_files_sys(struct direct *mydir, int ind, int sect); // prints all files in the file sys

// given an inode number and pointer to superblok
// return the exact addr of the inode
int inode_addr(struct fs *fp, int inode_num){
	int abs_frag = ino_to_fsba(fp, inode_num);
	// get the position of the inode in the cyl group
	int exact_ino_pos_blk = ino_to_fsbo(fp, inode_num);
	return abs_frag*fp->fs_fsize + exact_ino_pos_blk*(sizeof(struct ufs2_dinode));
}

//this function prints all files in a given directory
//it keeps track of the notion of indent to properly list files in their respective parent directory
//ino_index keeps track of an inodes data blocks being accessed in the inode db array 
void print_files_sys(struct direct *mydir, int indent, int ino_index){
    struct ufs2_dinode *curr_inode;
    u_int32_t inode_num; 
    static int go = 1; // keep track of the indent for directories
    if(flag_root){ // accessing the root inode for the first time
       inode_num = UFS_ROOTINO;
    }else{  // adccessing other non-root inodes
        inode_num = mydir->d_ino;
    }

    curr_inode = (struct ufs2_dinode *)(inode_addr(fp, inode_num) + pt_mem);
    //printf("size of inode: %lu \n", curr_inode->di_size); ?? can the size give any useful informarin


    //print a regular file with the proper indent
    if((IFREG == (curr_inode->di_mode & IFMT)) & (!flag_root)){
         for(int i = 0; i < indent; i++){
               printf(" ");
         }
         printf("%s \n", mydir->d_name);
    }
    // 
    else if(go || IFDIR == (curr_inode->di_mode & IFMT)){
       go = 0;
        if(!flag_root){
            if(strcmp(mydir->d_name, ".") == 0 || strcmp(mydir->d_name, "..") == 0){
                    return;
                    }
        }

        for(int i = 0; i < indent; i++){
            printf(" ");
        }
        if(!flag_root){
        printf("%s \n", mydir->d_name);
        }
        if(flag_root){
            flag_root = 0;
        }
        ufs2_daddr_t *inode_db_arr = curr_inode->di_db;
        ufs2_daddr_t *inode_ib_arr = curr_inode->di_ib;
        struct direct *child_dir;
        int padding = 0; //padding to add at the end of the dir to get the proper alignment of the next dir
        int not_end = 1; //flag to know when we have exchausted the entire inode block arrays
        int next_index =  0; //flag to know when to switch to the next index in the inode 
        int count = 0;      // keep track of number of inodes per indexed inode block
        // direct blocks
        while(not_end){
            if(next_index==0){
                child_dir = (struct direct *)(inode_db_arr[ino_index]*MINBSIZE + pt_mem + padding);
                padding +=child_dir->d_reclen;
                count++;
            }
            if(child_dir->d_ino > total_inodes || count > 8192){
                next_index = 1;
                ino_index++;
                padding = 0;
                count = 0;
            }
            if(next_index == 1){
                next_index = 0; 
                //going into the first layer of the indirect blocks
                if(ino_index == UFS_NDADDR){
                    int loop = 1; // we have not reached the end of the indirect blk
                   int  *indirect_blocks = (inode_ib_arr[0] * MINBSIZE  + pt_mem);
                    while(loop){
                        child_dir = (struct direct *)(indirect_blocks[0]*MINBSIZE + pt_mem + padding);
                        padding +=child_dir->d_reclen;
                        //printf("length: %d\n", child_dir->d_reclen); 
                        if(child_dir->d_reclen > UFS_MAXNAMLEN || child_dir->d_reclen == 0){
                               loop = 0;
                        }
                          else{ 
                              ino_index = 0; 
                              print_files_sys(child_dir, indent+1, ino_index);
                           }
                    }
                    break; // we have reached the end of  all the data in a given inode
                }
             //printf("Fails here after accessing indirect blks; ino_ino_index: %d\n", ino_index);
            child_dir = (struct direct *)(inode_db_arr[ino_index] * MINBSIZE + pt_mem + padding);
            padding +=child_dir->d_reclen;
            count++;
            }
            if(child_dir->d_ino == 0){
                not_end = 0;
            }
            if(child_dir->d_ino != 0){
                print_files_sys(child_dir, indent+1, ino_index);
         
            }
        }
    }
    else{
        printf("My inode: %d: \n", mydir->d_ino);
        printf("Neither file nor dir %s \n",mydir->d_name);

    }

}

void print_files(char *disk_image_path){
	int fd;
	int partition_size;
	struct stat s;
	
	fd=open(disk_image_path,O_RDONLY);
	fstat(fd, &s);
	partition_size = s.st_size;// get size of image file
	
    pt_mem = mmap(0, partition_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(pt_mem == MAP_FAILED){
        perror(NULL);
        printf("Mapping Failed");
        exit(1);

    }
	fp = (struct fs *)( pt_mem + SBLOCK_UFS2);
    total_inodes = fp->fs_ipg * fp->fs_ncg;
	// get inode address
	// get root inode
    print_files_sys(NULL, 0, 0);
   
	
	int err = munmap(pt_mem, partition_size);
	if(err!=0){
		printf("UnMapping Failed\n");
		exit(1);
	}

	close(fd);

}


int main(int argc, char **argv){
	if(argc < 2){
		printf("Usage: ./fs-find [path/filename.img]\n");
		return 1;
	}
	char *disk_image_path = argv[1];
	print_files(disk_image_path);
}
