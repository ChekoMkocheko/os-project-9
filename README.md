Fast File System
Reading from a raw disk


This program enables the user to read files from a raw disk image that 
was formated using the fast file system. 

To compile the program: run make
To print the list of files in the disk: run ./fs-find [path/filename.img]
                                        for example: ./fs-find ~/partition.img

To print the content of a specific file
 in the raw disk if it exists: run ./fs-cat [ partition-disk path/filename]
                             : for eample  ./fs-cat ~/partition.img ./sam.txt
Nothing it printed if the file does not exist.


For purspose of validating that the above program work, I used 
shell scripts to create a disk partition in *** freebsd *** and added files that I 
could parse during development. 

The script.sh file contains commands to partition a raw disk using gpart and writes 
files in it. The programs to produce files can be found in the gen_files folder. 
You do not need to do anything in genfiles because all the commands are in script.sh.
The only thing to change is the path where the new partition will be mounted. 
In this case, you can modify "cheko" to your username.

mntd.sh contains commands to unmount the disk and dump its raw content in a file
that can be accessed by fs-cat and fs-find

remove_files.sh contains commands to delete all partitions and associated files. 

The scripts are divided in three parts to allow for various manual
modifications in between commands. For instance, one might want to put specific
text into files after running the script.sh file before unmounting. The order
helps for learning about the steps too. 

