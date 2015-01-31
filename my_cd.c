#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fcntl.h"
#include "unistd.h"
#include "errno.h"
#include <linux/ext2_fs.h>

#include "common.h"

extern int block_size;
extern int inode_size;
extern int inode_table;
extern int fid;

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Usage: %s <directory name>\n", argv[0]);
		exit(1);
	}

	char buffer[block_size];	
	char dir_name[BUF_SIZE];	
	strcpy(dir_name, argv[1]);
	
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

	if ( valid_path(dir_name) == 0 )
	{
		fid = open("/tmp/.myext2", O_RDWR | O_TRUNC) ;
		if (fid == -1)
		{
			if(errno == EACCES)
			{
				printf("[ERROR] open 'myext2' failed, permission denied\n");
			}
			else if(errno == ENONET)
			{
				printf("[ERROR] open 'myext2' failed, the file does not exist\n");
			}
			else
			{
				printf("[ERROR] open 'myext2' failed\n");
			}
			exit(1);
		}
		int res = write(fid, dir_name, strlen(dir_name));
		if (res == -1)
		{
			printf("[ERROR] write directory to 'myext2' failed\n");
			exit(1);
		}
	}
	else
	{
		printf("[ERROR] directory doesn't exist\n");
		exit(1);
	}

	return 0;
}

