#include <FAT32.h>

// Constants
#define SECTOR_SIZE 512
#define CLUSTER_SIZE (SECTOR_SIZE * sectors_per_cluster) // Define sectors_per_cluster properly
#define FAT32_MAX_CLUSTER 0x0FFFFFF6
#define OFFSET_SECTORS_PER_CLUSTER 0x0D
#define OFFSET_ROOT_CLUSTER 0x2C

// Define your FAT32 directory entry structure
typedef struct {
    char filename[11]; // FAT32 short name (8.3 format)
    uint32_t starting_cluster;
    uint32_t file_size;
    uint8_t attributes;
} FAT32DirEntry;

// Define global variables
uint8_t boot_sector[SECTOR_SIZE];
uint32_t sectors_per_cluster;
uint32_t root_dir_cluster;
uint32_t fat_starting_sector = 5842; // Starting sector for clusters

// Function to copy memory
void *memoryCopy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

// Function to get sectors per cluster from the boot sector
uint8_t get_sectors_per_cluster(const uint8_t* boot_sector) {
    return boot_sector[OFFSET_SECTORS_PER_CLUSTER];
}

// Initialize sectors_per_cluster and root_dir_cluster
void initialize_fat32() {
    read_boot_sector();
    sectors_per_cluster = get_sectors_per_cluster(boot_sector);
    root_dir_cluster = *(uint32_t*)(boot_sector + OFFSET_ROOT_CLUSTER);
}

// Function to find a free cluster in the FAT table
uint32_t find_free_cluster() {
    uint32_t cluster = 2; // Starting cluster for FAT32
    uint8_t fat_sector[SECTOR_SIZE];

    // Iterate through FAT table sectors
    while (cluster < FAT32_MAX_CLUSTER) {
        uint32_t fat_sector_number = (cluster * 4) / SECTOR_SIZE;
        uint32_t offset = (cluster * 4) % SECTOR_SIZE;

        // Read FAT sector
        ata_read_sector(fat_sector_number + fat_starting_sector, fat_sector);

        // Check if cluster is free
        uint32_t fat_entry = *(uint32_t*)(fat_sector + offset);
        if (fat_entry == 0) {
            return cluster;
        }

        cluster++;
    }

    return 0; // No free cluster found
}

// Function to allocate a cluster in the FAT table
void allocate_cluster(uint32_t cluster) {
    uint8_t fat_sector[SECTOR_SIZE];
    uint32_t fat_sector_number = (cluster * 4) / SECTOR_SIZE;
    uint32_t offset = (cluster * 4) % SECTOR_SIZE;

    // Read FAT sector
    ata_read_sector(fat_sector_number + fat_starting_sector, fat_sector);

    // Mark cluster as used (simple example, not handling chain)
    *(uint32_t*)(fat_sector + offset) = FAT32_MAX_CLUSTER; // End of file marker

    // Write FAT sector
    ata_write_sector(fat_sector_number + fat_starting_sector, fat_sector);
}

char *string_n_copy(char *dest, const char *src, size_t n) {
    size_t i;
    
    // Copy characters from src to dest, up to n characters
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    
    // If src is shorter than n, pad the rest of dest with '\0'
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    
    return dest;
}

uint32_t sector_to_cluster(uint32_t sector_number) {
    // Calculate the cluster index (zero-based)
    uint32_t cluster_number = (sector_number - fat_starting_sector) / sectors_per_cluster + 2;

    return cluster_number;
}

uint32_t cluster_to_sector(uint32_t cluster_number) {
    // Adjust cluster_number to be zero-based
    uint32_t zero_based_cluster = cluster_number - 2;

    // Calculate the starting sector of the cluster
    uint32_t sector_number = fat_starting_sector + zero_based_cluster * sectors_per_cluster;

    return sector_number;
}

// Function to create a directory entry for the file
void create_directory_entry(const char* filename, uint32_t starting_cluster, uint32_t file_size) {
    uint8_t root_dir_sector[SECTOR_SIZE];
    uint32_t root_dir_sector_number = cluster_to_sector(root_dir_cluster);

    ata_read_sector(root_dir_sector_number, root_dir_sector);

    // Create a directory entry
    FAT32DirEntry* entry = (FAT32DirEntry*)root_dir_sector;
    // Fill in the entry details (name, starting cluster, size, etc.)
    string_n_copy(entry->filename, filename, sizeof(entry->filename) - 1);
    entry->filename[sizeof(entry->filename) - 1] = '\0'; // Ensure null termination
    entry->starting_cluster = starting_cluster;
    entry->file_size = file_size;
    entry->attributes = 0x20; // Regular file attribute

    // Write back the updated root directory sector
    ata_write_sector(root_dir_sector_number, root_dir_sector);
}

// Function to create a file
void create_file(const char* filename, uint8_t* data, uint32_t data_size) {
    // Find a free cluster
    uint32_t starting_cluster = find_free_cluster();
    if (starting_cluster == 0) {
        return;
    }

    // Allocate cluster(s)
    allocate_cluster(starting_cluster);

    // Create a directory entry for the file
    create_directory_entry(filename, starting_cluster, data_size);

    // Write file data to clusters
    uint8_t cluster_data[CLUSTER_SIZE];
    uint32_t clusters_needed = (data_size + CLUSTER_SIZE - 1) / CLUSTER_SIZE;

    for (uint32_t i = 0; i < clusters_needed; i++) {
        // Copy data to cluster_data
        uint32_t offset = i * CLUSTER_SIZE;
        uint32_t to_copy = (data_size - offset) < CLUSTER_SIZE ? (data_size - offset) : CLUSTER_SIZE;
        memoryCopy(cluster_data, data + offset, to_copy);

        // Write data to the cluster
        ata_write_sector(cluster_to_sector(starting_cluster + i), cluster_data);
    }
}
