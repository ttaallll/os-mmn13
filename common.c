#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <linux/ext2_fs.h>
#include <linux/types.h>
#include "fcntl.h"
#include "unistd.h"
#include "errno.h"

int fid; /* global variable set by the open() function */
int block_size = 1024; /* bytes per sector from disk geometry */
int inode_table;
int inode_size;

int read_block(int block_number, char *buffer)
{
	fid = open ("/dev/fd0", O_RDWR);
	if (fid == -1)
	{
		printf("[ERROR] open failed\n");
		return -1;
	}
	int dest, len;
	dest = lseek(fid, block_number * block_size, SEEK_SET);
	if (dest != block_number * block_size)
	{
		printf("[ERROR] lseek failed\n");
		close(fid);
		return -1;
	}

	len = read(fid, buffer, block_size);
	if (len != block_size)
	{
		printf("[ERROR] read block failed\n");
		close(fid);
		return -1;
	}
	close(fid);
	buffer[1023]= '\0';
	return len;
}

int read_inode(int inode_number, char *buffer)
{
	fid = open ("/dev/fd0", O_RDWR);
	if (fid == -1)
	{
		printf("[ERROR] open failed\n");
		return -1;
	}
	int dest, len;
	dest = lseek(fid, inode_table * block_size + inode_number * inode_size, SEEK_SET);
	if (dest != inode_table * block_size + inode_number * inode_size)
	{
		printf("[ERROR] lseek failed\n");
		close(fid);
		return -1;
	}

	len = read(fid, buffer, inode_size);
	if (len != inode_size)
	{
		printf("[ERROR] read block failed\n");
		close(fid);
		return -1;
	}
	close(fid);
	return len;
}

int get_inode(struct ext2_inode* inode_pointer, int inode_number)
{
	char inode[inode_size];
	int res = read_inode(inode_number, inode);
	if (res == -1)
	{
		printf("[ERROR] read inode %d failed\n", inode_number);
		return -1;
	}
     if( memcpy((void*)inode_pointer, inode, sizeof(struct ext2_inode))== NULL)
     {
    	 printf("[ERROR] memcpy to inode struct failed\n");
    	 return -1;
     }
     return 0;
}

int get_dir_entry( struct ext2_dir_entry_2 *dir_pointer, char* entry_name, int inode_number)
{
	struct ext2_inode inode;
    int i, j, db_length;

	if(get_inode(&inode, inode_number)==-1)
	{
		printf("[ERROR] get_inode failed\n");
		return -1;

	}
    char dir_data[block_size];
	for ( i = 0; i < 12 && (inode.i_block[i]!=0); i++ )
	{
		db_length = 0;
		db_length = read_block(inode.i_block[i], dir_data); /*first data block*/
		if(db_length == -1)
		{
			printf("[ERROR] get_inode failed\n");
			return -1;

		}
		j = 0;
		while( j < db_length )
		{
			if( memcpy( dir_pointer, dir_data + j, sizeof( struct ext2_dir_entry_2))!= NULL)
			{
				if(!strcmp(dir_pointer->name,entry_name) && dir_pointer->file_type == 2)
				{
					return 0;
				}
			   j += dir_pointer->rec_len;
			}
		}
	}
    return -1; /*the entry wasn't found*/
}

int split_path(char* path, char*** result)
{
	int len;

	char** sub_directories;

	len = strlen(path);
	if (path[len - 1] == '\n')
		path[len - 1] = 0;

	char* entry_name= strtok(path, "/");
	int entries_number = 0;
	sub_directories = (char**)malloc(sizeof(char*) * 1);
	while(entry_name)
	{
		sub_directories = realloc(sub_directories, sizeof(char*) * (++entries_number + 1));
		if (sub_directories == NULL)
		{
			printf("[ERROR] memory allocation (using realloc) failed\n");
			return -1;
		}
		sub_directories[entries_number -1] = entry_name;
		entry_name= strtok(NULL, "/");
	}
	/*realloc one more entry for the NULL*/
	sub_directories = realloc(sub_directories, sizeof(char*) * (entries_number+1));
	sub_directories[entries_number] = NULL;

	*result = sub_directories;
	return 0;
}

int valid_path(char path[])
{
	struct ext2_dir_entry_2 inner_dir;
	char** sub_directories;
	char tmp_path[BUF_SIZE];
	strcpy(tmp_path , path);
	int i, inode_number = 1;

	if(split_path(tmp_path, &sub_directories) == -1)
	{
		printf("[ERROR] split_path failed\n");
		return -1;
	}

	struct ext2_dir_entry_2 current_entry;
	for(i=0; sub_directories[i] != NULL; i++)
	{
		if (get_dir_entry(&current_entry, sub_directories[i], inode_number) == -1)
		{
			return -1;
		}
		inode_number = current_entry.inode - 1;
	}
	if( memcpy( &inner_dir, &current_entry, sizeof( struct ext2_dir_entry_2 ) ) == NULL) /*the last one is the most inner one*/
	{
		printf("[ERROR] memcpy failed\n");
		return -1;
	}
	return 0;
}
