#pragma once

#include <stddef.h>
#include <stdint.h>
#include <ATA.h>

#define BLOCKSIZE          512
#define START_BLOCK_SECTOR 4'500
#define END_BLOCK_SECTOR   4'697
#define TOTAL_BLOCKS       34
#define MAX_BYTES          17'408

typedef struct NSFS_Super_Block_t {
    char magic[4];
    uint32_t block_size;
    uint32_t total_blocks;
    uint32_t free_blocks;
    char volume_name[8];
};

typedef struct NSFS_Directory_Entry_t {
    char name[8];
    uint32_t startBlock;
    uint32_t blocks;
    char* path;
};

typedef struct NSFS_File_Entry_t {
    char name[8];
    uint32_t startBlock;
    uint32_t blocks;
    NSFS_Directory_Entry_t parentDirectory;
    char* path;
};

void CreateFile(char* filename, uint8_t* data);
uint8_t* ReadFile(char* filename);