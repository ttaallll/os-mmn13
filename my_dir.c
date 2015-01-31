#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fcntl.h"
#include "unistd.h"
#include "errno.h"
#include <linux/ext2_fs.h>
#include <time.h>

#include "common.h"

extern int block_size;
extern int inode_size;
extern int inode_table;
extern int fid;

int get_files_in_path(char* path)
{
	char** sub_directories;
	char tmp_path[BUF_SIZE];
	strcpy(tmp_path , path);
	int i, inode_number = 1;

	if(split_path(tmp_path, &sub_directories) == -1)
	{
		printf("[ERROR] split_path failed\n");
		return -1;
	}

	char badPath = 0;
	struct ext2_dir_entry_2 current_entry;
	for(i=0; sub_directories[i] != NULL; i++)
	{
		if (get_dir_entry(&current_entry, sub_directories[i], inode_number) == -1)
		{
			badPath = 1;
			break;
		}
		inode_number = current_entry.inode - 1;
	}

	// we found bad path
	if (badPath) inode_number = 1;

	struct ext2_inode inode, temp_inode;
	int j, db_length;

	if(get_inode(&inode, inode_number)==-1)
	{
		printf("[ERROR] get_inode failed\n");
		return -1;

	}
	char dir_data[block_size];
	char temp_name[block_size];
	char temp_date[block_size];
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
			if( memcpy( &current_entry, dir_data + j, sizeof( struct ext2_dir_entry_2)) == NULL)
				return -1;

			if( memcpy(temp_name, current_entry.name, current_entry.name_len) == NULL)
				return -1;

			temp_name[current_entry.name_len] = 0;

			/* get the date */
			if(get_inode(&temp_inode, current_entry.inode - 1)==-1)
			{
				printf("[ERROR] get_inode failed\n");
				return -1;
			}
			struct tm* ts = localtime(((time_t*)(&temp_inode.i_mtime)));
			strftime(temp_date, sizeof(temp_date), " %d-%b-%Y %H:%M", ts);

			printf("%s ", temp_date);
			printf("%s\n", temp_name);

			j += current_entry.rec_len;

		}
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("Usage: %s \n", argv[0]);
		exit(1);
	}

	char buffer[block_size];
	char dir_name[BUF_SIZE];

	/* ---- SuperBlock ---- */
	int fd = read_block(1, buffer);
	if (fd == -1)
	{
		printf("[ERROR] read super block failed\n");
		exit(1);
	}

	struct ext2_super_block sb;
	if (memcpy((void*)&sb, buffer, sizeof(struct ext2_super_block)) == NULL)
	{
		printf("[ERROR] memcpy super block failed\n");
		exit(1);
	}

	block_size = (int)pow(2, sb.s_log_block_size) * 1024; /*Block size*/
	inode_size = sb.s_inode_size; /*Size of on-disk inode structure*/

	/* ---- GroupDescriptor ---- */
	fd = read_block(2, buffer);
	if (fd == -1)
	{
		printf("[ERROR] read group descriptor block failed\n");
		exit(1);
	}

	struct ext2_group_desc gd;
	if(memcpy((void*)&gd, buffer, sizeof(struct ext2_group_desc)) == NULL)
	{
		printf("[ERROR] memcpy group descriptor block failed\n");
		exit(1);
	}

	inode_table = gd.bg_inode_table; /*Block number of first inode table block*/


	fid = open("/tmp/.myext2", O_RDONLY);
	if (fid == -1)
	{
		if(errno == EACCES)
		{
			printf("[ERROR] open 'myext2' failed, permission denied\n");
			exit(1);
		}
		else
		{
			printf("open 'myext2' failed, the file does not exist\n");
			printf("create 'myext2'\n");
			fid = open("/tmp/.myext2", O_RDWR | O_TRUNC);
			if (fid == -1)
			{
				printf("couldnt create 'myext2'\n");
				exit(1);
			}
		}
	}

	int res = read(fid, dir_name, BUF_SIZE);
	if (res == -1)
	{
		printf("[ERROR] read directory from 'myext2' failed\n");
		exit(1);
	}

	if ( get_files_in_path(dir_name) != 0 ) {
		printf("[ERROR] couldn't get files in path\n");
		exit(1);
	}

	printf("\n");

	return 0;
}
