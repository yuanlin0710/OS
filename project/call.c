// Name: YUAN Lin
// sid: 1155141399
// CSCI3150 Project


#include "call.h"
const char *HD = "HD";

superblock *read_superblock(int fd) {
    superblock *sb = (superblock *)malloc(sizeof(superblock));
    pread(fd, sb, sizeof(superblock), S_OFFSET);
    return sb;
}



int open_t(char *pathname)
{
	int inode_number;

	// open hd
	int fd = open(HD, O_RDONLY);
    if (fd == -1) {
        perror("Error opening HD file");
        return -1;
    }

    // read superblock
    superblock *sb = read_superblock(fd);
    if (!sb) {
        close(fd);
        return -1;
    }
	
	// if the pathname is root 
    if (strcmp(pathname, "/") == 0) {
        // Clean up
        free(sb);
        close(fd);
        return 0; 
    }

	char *tokens[25]; // assume maximum 25 levels
    int n = 0;


	char* pathname_var;
	pathname_var = (char*)malloc(MAX_COMMAND_LENGTH);
	strcpy(pathname_var, pathname);




    char *token = strtok(pathname_var, "/");
    while (token != NULL) {
        tokens[n++] = token;
        token = strtok(NULL, "/");
	}
	// char* tokens will be the path


	// traverse the path
	int current_inode_number = 0;
	for(int i = 0; i < n; i++){


		// read current inode
		inode *current_inode = (inode *)malloc(sizeof(inode));
    	pread(fd, current_inode, sizeof(inode), sb->i_offset + current_inode_number * sizeof(inode));

		if (current_inode->f_type != DIR) {
            // Current inode is not a directory, then should be error
            free(current_inode);
            free(sb);
			free(pathname_var);
            close(fd);
            return -1;
        }

		
		DIR_NODE* p_block = (DIR_NODE*)malloc(sb->blk_size);
		// Considering that SFS only supports at most 100 inodes so that only direct_blk[0] will be used
		
		int blk_num = current_inode->direct_blk[0];

		int currpos=lseek(fd, sb->d_offset + blk_num * sb->blk_size, SEEK_SET);
		read(fd, p_block, sb->blk_size);



		bool found = false;
		for (int j = 0; j < current_inode->sub_f_num; j++) {
            if (strcmp(p_block[j].f_name, tokens[i]) == 0) {
                current_inode_number = p_block[j].i_number;
                found = true;
                break;
            }
        }
		// compare each f_name

		free(p_block);
		free(current_inode);



        if (!found) {
            free(sb);
            close(fd);
			free(pathname_var);
            return -1;
        }
		        

	}


	inode_number = current_inode_number;
	// free malloc and close fd
	free(sb);
    close(fd);
	free(pathname_var);

	return inode_number;
}






int read_t(int i_number, int offset, void *buf, int count){

	int read_bytes = 0;
	char *buffer_pt = (char *)buf;

	// open HD
	int fd = open(HD, O_RDONLY);
    if (fd < 0) {
        perror("Error opening HD");
        return -1;
    }

    // read superblock
    superblock *sb = read_superblock(fd);
    if (sb == NULL) {
        close(fd);
        return -1;
    }

	// read inode
	inode *node = (inode *)malloc(sizeof(inode));
    pread(fd, node, sizeof(inode), sb->i_offset + i_number * sizeof(inode));

	if (node == NULL || node->i_number != i_number) {
		free(node);
        free(sb);
        close(fd);
        return -1;
    }
	
	if (offset >= node->f_size) {
		// check whether out-of-range
        free(node);
        free(sb);
        close(fd);
		//printf("offset out of range");
        return 0; // End of file
    }

	if (offset + count > node->f_size) {
		// check EOF issue
		// if read count > EOF, then change count = EOF position offset
        count = node->f_size - offset;
    }


	// consider read from 2 direct blocks first
	if(offset< 2*sb->blk_size){
		int start_block = offset / sb->blk_size;
		
		for(int i = start_block; i<2 && count > 0; i++){
			int block_offset = offset % sb->blk_size;		
			int read_size = count;
			if(sb->blk_size - block_offset < count){
				read_size = sb->blk_size - block_offset;
			}
		
			pread(fd, buffer_pt, read_size, sb->d_offset + node->direct_blk[i] * sb->blk_size + block_offset);

			//update offset in file for next time to read
			offset += read_size;
			// update offset in buffer for next time to load in
			buffer_pt += read_size;
			// update remaining byte to read
			count -= read_size;
			// update already read byte number
			read_bytes += read_size;

		}

	}
	

	// if still has sth to read, read from indirect blocks
	if(count > 0 && node->indirect_blk != -1){
		int* indirect_blk_pts = (int*)malloc(sb->blk_size);
		pread(fd, indirect_blk_pts, sb->blk_size, sb->d_offset + node->indirect_blk * sb->blk_size);

		int start_block = (offset - sb->blk_size*2) / sb->blk_size;
		for(int i = start_block; count>0 && i < (sb->blk_size / sizeof(int));i++ ){
			// read each indirect block until count<0 or finish all the indirect block
			int block_offset = offset % sb->blk_size;
			int read_size = count;
			if(sb->blk_size - block_offset < count){
				read_size = sb->blk_size - block_offset;
			}
			pread(fd, buffer_pt, read_size, sb->d_offset + indirect_blk_pts[i] * sb->blk_size + block_offset);

			//update offset in file for next time to read
			offset += read_size;
			// update offset in buffer for next time to load in
			buffer_pt += read_size;
			// update remaining byte to read
			count -= read_size;
			// update already read byte number
			read_bytes += read_size;


		}
		// finish reading, so free malloc indirect pointer
		free(indirect_blk_pts);

	}




	// free malloc and close fd
	free(sb);
	free(node);
	close(fd);
	return read_bytes; 
}

// you are allowed to create any auxiliary functions that can help your implementation. But only "open_t()" and "read_t()" are allowed to call these auxiliary functions.