/* fs.c - Functions to init and deal with file system
 * vim:ts=4 noexpandtab
 */

#include "fs.h"
#include "lib.h"

/*
* int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
*   Inputs: fname -- Name of the file that needs to be read from.
*			dentry -- Dentry structure that needs to be populated.
*   Return Value: int32_t (0 if success, -1 if failure)
*	Function: updates the dentry struct to the values of the filename passed in.
*/
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
{
	if (strncmp((int8_t*)fname, (int8_t*)"", 32) == 0)
		return -1;
	int d = 0;
	while (strncmp(bblock->dentry[d].file_name, (int8_t*)fname, 32))	//compare the value of the filename passed in and the filename in the bootblock
	{
		if (strncmp(bblock->dentry[d].file_name, (int8_t*)"", 32) == 0)	//check if all files are checked
			return -1;
		d++;
	}
	strcpy(dentry->file_name, bblock->dentry[d].file_name);	//update the dentry values when the file is found
	dentry->file_type = bblock->dentry[d].file_type;
	dentry->inode_num = bblock->dentry[d].inode_num;
	return 0;
}

/*
* int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
*   Inputs: index -- Index node of the file that needs to be read from.
*			dentry -- Dentry structure that needs to be populated.
*   Return Value: int32_t (0 if success, -1 if failure)
*	Function: updates the dentry struct to the values of the index nodes passed in.
*/
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
{
	int d = 0;
	while (bblock->dentry[d].inode_num != index || bblock->dentry[d].file_type != 2)	//compare the value of the index node passed in and the inode in the bootblock & ignore all entries except for regular files
	{
		if (d >= MAX_FILES - 1)	//check if all files are checked
			return -1;
		d++;
	}
	strcpy(dentry->file_name, bblock->dentry[d].file_name);	//update the dentry values when the file is found
	dentry->file_type = bblock->dentry[d].file_type;
	dentry->inode_num = bblock->dentry[d].inode_num;
	return 0;
}

/*
* int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
*   Inputs: inode -- Index node of the file that needs to be read from.
*			offset -- Offset bytes into the file and read from there.
*			buf -- buffer to store values read from file.
*			length -- number of bytes to read.
*   Return Value: int32_t (number of bytes read from file, 0 if none were read and -1 if there is an error)
*	Function: reads data of length bytes from the file referred to by inode starting from offset and stores it into the buf
*/
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	/* the first block to start reading from */
	uint32_t start_block;
	/* how many blocks need to be read from (including start_block itself) */
	uint32_t num_blocks_left;
	/* the number of the block currently being read */
	uint32_t cur_block_num;
	/* how many bytes copied into the buffer */
	uint32_t bytes_copied = 0;

	int i, j;
	uint8_t * ptr;

	inode_t * in = GET_INODE_ADRS(inode);

	/* is inode number in range? */
	if (inode >= bblock->inode_cnt) {
		/* DEBUG print statement */
		// printf("read_data: inode number %u out of range 0 to %u\n", inode, bblock->inode_cnt - 1);
		return -1;
	}

	/* does offset refer to a valid block number? */
	if (offset > in->length) {
		/* DEBUG print statement */
		// printf("read_data: offset %u out of range\n", offset);
		return -1;
	}

	if (offset == in->length) {
		return 0;
	}

	/* is length valid? */
	if (length == 0) {
		/* DEBUG print statement */
		// printf("read_data: number of bytes to read is 0\n");
		return 0;
	}

	if ((in->length - offset) < length){
		length = in->length-offset;
	}

	start_block = offset / BYTES_PER_BLOCK;
	num_blocks_left = length / BYTES_PER_BLOCK + 1;

	for (i = 0; i < num_blocks_left; i++) {
		if (i == 0) {
			/* calculate the offset from the start of start_block instead of
			 * from the start of the file
			*/
			offset = offset % BYTES_PER_BLOCK;
		} else {
			/* start reading from the beginning of the block */
			offset = 0;
		}

		cur_block_num = in->data_block[start_block + i];
		ptr = GET_DATABLOCK_ADRS(cur_block_num);
		/* if end of file reached */
		if (cur_block_num == 0) {
			/* DEBUG print statement */
			// printf("read_data: end of file reached\n");
			return bytes_copied;
		}

		for (j = offset; j < BYTES_PER_BLOCK; j++) {
			if (bytes_copied == length || bytes_copied == in->length) {
				/* DEBUG print statement */
				// printf("read_data: copied all bytes\n");
				return bytes_copied;
			}

			buf[bytes_copied] = ptr[j];
			bytes_copied++;
		}
	}

	/* DEBUG print statement */
	// printf("read_data: copied all blocks\n");
	return bytes_copied;

}

/*
* int32_t reg_file_open(const uint8_t* filename)
*   Inputs: filename -- Name of the file that needs to be opened.
*   Return Value: int32_t (0 if success, -1 if failure)
*	Function: Sets up the file system struct in the PCB for the file to be opened.
*/
int32_t reg_file_open(const uint8_t* filename)
{
	return 0;
}

/*
* int32_t reg_file_read (file_struct_t * fs_ptr, void* buf, int32_t nbytes)
*   Inputs: fs_ptr -- Pointer to the file system struct of the file to read.
*			buf -- Buffer to put read data on to.
*			nbytes -- Number of bytes to read from file
*   Return Value: int32_t (number of bytes read from file or -1 if failure)
*	Function: Reads nbytes from a file refered to by fs_ptr and stores them into buf.
*/
int32_t reg_file_read (int32_t fd, void* buf, int32_t nbytes)
{
	if(pcb->file_array[fd].flags == (TYPE_FILE | OPEN))	//check if the file is open
	{
		int ret_val = read_data(pcb->file_array[fd].inode, pcb->file_array[fd].file_position, buf, nbytes);	//read nbytes using the read_data function
		if (ret_val > 0)
		{
			pcb->file_array[fd].file_position += ret_val;		//move the file position to the current position in the file
		}
		else
		{
			// printf("read_data returned value %d\n", ret_val);
		}
		return ret_val;
	}
	// printf("reg_file_read failed because file is not open!\n");
	return -1;
}

/*
* int32_t reg_file_write (file_struct_t * fs_ptr, void* buf, int32_t nbytes)
*   Inputs: fs_ptr -- Pointer to the file system struct of the file to write.
*			buf -- Buffer with data that needs to be written to file.
*			nbytes -- Number of bytes to write to file.
*   Return Value: int32_t (number of bytes written to file or -1 if failure)
*	Function: Writes nbytes from the buf to the file.
*/
int32_t reg_file_write (int32_t fd, const void* buf, int32_t nbytes)
{
	return -1;
}

/*
* int32_t reg_file_close(int32_t fd)
*   Inputs: fd -- File descriptor of file to close.
*   Return Value: int32_t (0 if success, -1 if failure)
*	Function: Closes the file in the PCB array and makes it available.
*/
int32_t reg_file_close(int32_t fd)
{
	return 0;
}

/*
* int32_t dir_open(const uint8_t* dir_name)
*   Inputs: dir_name -- Name of the directory that needs to be opened.
*   Return Value: int32_t (0 if success, -1 if failure)
*	Function: Sets up the file system struct in the PCB for the directory to be opened.
*/
int32_t dir_open(const uint8_t* dir_name)
{
	return 0;
}

/*
* int32_t dir_read (file_struct_t * fs_ptr, void* buf, int32_t nbytes)
*   Inputs: fs_ptr -- Pointer to the file system struct of the directory to read
*			buf -- Buffer to put read filenames in to.
*			nbytes -- Number of bytes to read from file.
*   Return Value: int32_t (number of bytes read from directory or -1 if failure)
*	Function: Reads nbytes of a filename in a directory refered to by fs_ptr and stores them into buf.
*/
int32_t dir_read (int32_t fd, void* buf, int32_t nbytes)
{
	if(pcb->file_array[fd].flags == (TYPE_DIR | OPEN))
	{
		if (strncmp("", bblock->dentry[pcb->file_array[fd].file_position].file_name, nbytes) == 0) //check if reached end of directory
		{
			return 0;
		}
		else
		{
			strncpy(buf, bblock->dentry[pcb->file_array[fd].file_position].file_name, nbytes);	//store filename up to nbytes into the buffer
			pcb->file_array[fd].file_position++;
			return nbytes;
		}
	}
	// printf("reg_file_read failed because directory is not open!\n");
	return -1;
}

/*
* int32_t dir_write (file_struct_t * fs_ptr, void* buf, int32_t nbytes)
*   Inputs: fs_ptr -- Pointer to the file system struct of the directory to write.
*			buf -- Buffer with data that needs to be written to directory.
*			nbytes -- Number of bytes to write to the directory.
*   Return Value: int32_t (number of bytes written to file or -1 if failure)
*	Function: Writes nbytes from the buf to the directory.
*/
int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes)
{
	return -1;
}

/*
* int32_t dir_close(int32_t fd)
*   Inputs: fd -- File descriptor of directory to close.
*   Return Value: int32_t (0 if success, -1 if failure)
*	Function: Closes the directory in the PCB array and makes it available.
*/
int32_t dir_close(int32_t fd)
{
	return 0;
}
