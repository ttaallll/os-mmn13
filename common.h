
#include <linux/ext2_fs.h>

#define BUF_SIZE 2048

int read_block(int block_number, char *buffer);
int read_inode(int inode_number, char *buffer);
int get_inode(struct ext2_inode* inode_pointer, int inode_number);
int get_dir_entry( struct ext2_dir_entry_2 *dir_pointer, char* entry_name, int inode_number);
int split_path(char* path, char*** result);
int valid_path(char path[]);
