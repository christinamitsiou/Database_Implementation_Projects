#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "hp_file.h"
#include "record.h"

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}

int HP_CreateFile(char *fileName){

    // Use BF_CreateFile to create the file with the same name as the parameter given
    BF_ErrorCode error = BF_CreateFile(fileName);

    // If creating the file was unsuccesful return -1
    if (error != BF_OK)
      return -1;
    
    int file_descriptor;

    // Open the BF file
    error = BF_OpenFile(fileName, &file_descriptor);

    // If opening the file was unsuccesful return -1
    if (error != BF_OK)
      return -1;

    // Initialize the first block of the file containing the metadata we need
    BF_Block* block;
    BF_Block_Init(&block);

    // Allocate a new block for the metadata of the file which also gets pinned
    error = BF_AllocateBlock(file_descriptor,block);
    void* data = BF_Block_GetData(block);  
    HP_info* hp_file_metadata = data;
    HP_block_info* block_info =  data + BF_BLOCK_SIZE - sizeof(*block_info) -1;

    // Block id for metadata block is 0 and the blocks with records have ids >=1
    block_info->block_id = 0;
    block_info->number_of_records = 0;
    
    // The metadata block is the last block since it's the only block in the file at the moment
    hp_file_metadata->id_of_last_block = 0;
    
    // Each record has a size of 74 bytes meaning that each block can have up to 6 records (because lower_bound(512/74) = 6)
    hp_file_metadata->number_of_records_per_block = 6;

    // Set the meta data block to dirty in order to save its contents once it is unpinned 
    BF_Block_SetDirty(block);

    // Unpin the block so it doesn't take space in the memory once the file is closed
    error = BF_UnpinBlock(block);

    // If unpinning the block was unsuccesful return -1
    if (error != BF_OK)
      return -1;
    
    // Close BF the file
    error = BF_CloseFile(file_descriptor);

    // If closing the file was unsuccesful return -1
    if (error != BF_OK)
      return -1;  

    // Destroy the block
    BF_Block_Destroy(&block);
    
    // If destroying the block was unsuccesful return -1
    if (error != BF_OK)
      return -1;

    // If everything worked return 0
    return 0;

}

HP_info* HP_OpenFile(char *fileName, int *file_desc){

    HP_info* hpInfo;

    // Initialize a block
    BF_Block* block; 
    BF_Block_Init(&block);

    // Open the BF file 
    BF_ErrorCode error = BF_OpenFile(fileName,file_desc) ;

    // Load the block with the file's metadata (block 0)
    error = BF_GetBlock(*file_desc, 0 ,block);

    // Get the data from the block
    void* data = BF_Block_GetData(block);
    hpInfo = data;

    // Unpin and destroy block
    error = BF_UnpinBlock(block);
    BF_Block_Destroy(&block);

    // Return the file's matedata
    return hpInfo;
}


int HP_CloseFile(int file_desc,HP_info* hp_info ){
    
    // Initialize a block
    BF_Block* block;
    BF_Block_Init(&block);

    BF_ErrorCode error;

    int count = 0;
    int total_num_of_blocks;
    error = BF_GetBlockCounter(file_desc,&total_num_of_blocks);

    // Unpin every pinned block of the file from the memory
    while ( count < total_num_of_blocks) {

      error = BF_GetBlock(file_desc,count,block);
      error = BF_UnpinBlock(block);
      count++;
    }

    // Close the BF file
    error = BF_CloseFile(file_desc);
    
    // Destroy the block
    BF_Block_Destroy(&block);

    // If the BF file closed properly return 0, if there was an error return -1
    if (error != BF_OK)
      return -1;
    return 0;
}

int HP_InsertEntry(int file_desc,HP_info* hp_info, Record record){
  
    // Load the metadata block and pin it 
    BF_Block* metadata_block;
    BF_Block_Init(&metadata_block);
    BF_ErrorCode error = BF_GetBlock(file_desc,0,metadata_block);

    // If loading the block was unsuccesful return -1
    if (error != BF_OK)
        return -1;

    // Initialize a block
    BF_Block* block;
    BF_Block_Init(&block);

    // Get the id of the last block in order to see if we can insert the new Record in it
    int last_block_id = hp_info->id_of_last_block;

    // Load the block 
    error = BF_GetBlock(file_desc,last_block_id,block);

    // If loading the block was unsuccesful return -1
    if (error != BF_OK)
      return -1;

    // Initialize a pointer to the data which will be used later
    void* data  = BF_Block_GetData(block);
    HP_block_info* block_info = data + BF_BLOCK_SIZE - sizeof(*block_info) -1;

    // If we have more than just the metadata block in the file and the last block of the file is not full we store the record in the last block
    if ( (last_block_id > 0) && (block_info->number_of_records < hp_info->number_of_records_per_block) ) {

        int index = block_info->number_of_records;
        Record* rec = data;

        // Copy the data from the record to the block in the buffer
        rec[index].id = record.id;
        memcpy(rec[index].record,record.record,strlen(record.record)+1);
        memcpy(rec[index].name,record.name,strlen(record.name) + 1);
        memcpy(rec[index].surname,record.surname,strlen(record.surname) + 1);
        memcpy(rec[index].city,record.city,strlen(record.city)+1);
        
        // Increase the number of records in the block
        block_info->number_of_records += 1;

        //printf("Last block insertion succesful , block_id = %d, number_of_records_in_block=%d \n",block_info->block_id,block_info->number_of_records);
    }
    // If we either don't have any blocks other than the metadata block or the last block is full, create a new block to store the new record in
    else {

      // Unpin the old last block and set it dirty
      BF_Block_SetDirty(block);
      error = BF_UnpinBlock(block);

      // If setting the block dirty was unsuccesful return -1
      if (error != BF_OK)
        return -1;

      // Allocate a new block in the file to store the new record
      error = BF_AllocateBlock(file_desc,block);

      // If allocating a new block was unsuccesful return -1
      if (error != BF_OK)
        return -1;

      // Get a pointer to the new block's data
      data = BF_Block_GetData(block);
      Record* rec = data;

      // Copy the data from the record to the block in the buffer
      rec[0].id = record.id;
      memcpy(rec[0].record,record.record,strlen(record.record)+1);
      memcpy(rec[0].name,record.name,strlen(record.name) + 1);
      memcpy(rec[0].surname,record.surname,strlen(record.surname) + 1);
      memcpy(rec[0].city,record.city,strlen(record.city)+1);

      // Get a pointer to the (area of) the block where the metadata will be stored and initialize the block's metadata ???
      block_info = data + BF_BLOCK_SIZE - sizeof(*block_info) -1;

      // Give an id to the new block
      block_info->block_id = hp_info->id_of_last_block + 1;

      // Initialize the number of records, which is 1 (the one we just added)
      block_info->number_of_records = 1;

      // Update the file's metadata
      hp_info->id_of_last_block = block_info->block_id;

      // Set the metadata block of the file dirty 
      BF_Block_SetDirty(metadata_block);

      //printf("New block insertion succesful , block_id = %d, number_of_records_in_block=%d\n ",block_info->block_id,block_info->number_of_records);
    }

    // In both cases set the block that we inserted the record in dirty and unpin it from the memory
    BF_Block_SetDirty(block);
    error = BF_UnpinBlock(block);

    // If unpinning the block was unsuccesful return -1
    if (error != BF_OK)
      return -1;

    // Unpin the metadata block
    error = BF_UnpinBlock(metadata_block);

    // Destroy both the metadata block and the block in which we inserted the new record
    BF_Block_Destroy(&metadata_block);
    BF_Block_Destroy(&block);

    // Return the id of the last block 
    return hp_info->id_of_last_block;
}

int HP_GetAllEntries(int file_desc,HP_info* hp_info, int value){

    // Initialize a block
    BF_Block* block;
    BF_Block_Init(&block);

    BF_ErrorCode error;

    // Initialize pointers for each block's data, records and block_info
    void* data;
    Record* record;
    Record rec;
    HP_block_info* block_info;

    int count = 1;
    int total_num_of_blocks;

    // Load the number of the file's available blocks in total_num_of_blocks
    error = BF_GetBlockCounter(file_desc,&total_num_of_blocks);

    // For each block check the records that it contains
    while( count < total_num_of_blocks ){

      // Load the block
      error = BF_GetBlock(file_desc,count,block);

      // Access the data of the block
      data = BF_Block_GetData(block);
      record = data;

      // Find how many records are stored in the block
      block_info = data + BF_BLOCK_SIZE - sizeof(*block_info) -1;

      // For each one of the block's records check if id == value and if it is, print said record
      for ( int i = 0; i < block_info->number_of_records; i++ ){

        if (record[i].id == value){
          printf("\n");
          printRecord(record[i]);
        }
      }
      
      // Unpin the block we just searched
      error = BF_UnpinBlock(block);
      
      count++;
    }

    // Destroy the block
    BF_Block_Destroy(&block);

    // Return the number of blocks in the file (not including the first meta data block since we didn't search it)
    return count - 1;
}