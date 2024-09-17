#pragma once

#include <stdint.h>
#include <stddef.h>
#include <ATA.h>

// Define constants related to FAT32
#define SECTOR_SIZE 512

// Define global variables for sectors_per_cluster
extern uint32_t sectors_per_cluster;

// FAT32 maximum cluster value
#define FAT32_MAX_CLUSTER 0x0FFFFFF6

// Function prototypes for FAT32 operations
void initialize_fat32(void); // Initializes FAT32 parameters from boot sector
void create_directory_entry(const char* filename, uint32_t starting_cluster, uint32_t file_size);
void create_file(const char* filename, uint8_t* data, uint32_t data_size);
