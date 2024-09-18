#include <NSFS.h>

void *memoryCopy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = static_cast<uint8_t *>(dest);
    const uint8_t *psrc = static_cast<const uint8_t *>(src);

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

#include <stddef.h>

char *string_num_copy(char *dest, const char *src, size_t n) {
    size_t i;
    
    // Copy up to n characters from src to dest
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    
    // If we copied less than n characters, null-terminate the dest
    for (; i < n; i++) {
        dest[i] = '\0'; // Fill the rest with null bytes
    }
    
    return dest; // Return the destination pointer
}

void *setMemory(void *s, int c, size_t n) {
    uint8_t *p = static_cast<uint8_t *>(s);

    for (size_t i = 0; i < n; i++) {
        p[i] = static_cast<uint8_t>(c);
    }

    return s;
}

size_t GetStrLen(const char* str) {
    size_t length = 0;

    // Iterate through the string until the null terminator is found
    while (str[length] != '\0') {
        length++;
    }

    return length;
}

const uint64_t PossibleSectors = 9593-5003;
const uint64_t startSector     = 5003;
const uint64_t endSector       = 9593;
uint64_t SectorIndex    = 0;
uint64_t WrittenSectors = 0;

void CreateFile(char* filename, uint8_t* data) {
    NSFS_File_Entry_t nsfs_file_entry;
    
    // Copy the filename into the file entry (up to 8 characters)
    for (int i = 0; i < 8; i++) {
        nsfs_file_entry.name[i] = (i < GetStrLen(filename)) ? filename[i] : '\0';
    }

    uint8_t buffer[512];
    
    const char* tag = "NSFSFILE:TAG:";
    size_t tag_len = sizeof("NSFSFILE:TAG:") - 1; // Length excluding null terminator
    memoryCopy(buffer, tag, tag_len);
    
    // Copy the filename into the buffer
    string_num_copy((char*)&buffer[tag_len], nsfs_file_entry.name, sizeof(nsfs_file_entry.name));
    
    size_t offset = tag_len + sizeof(nsfs_file_entry.name);
    
    // Get the length of the data
    size_t data_len = GetStrLen((const char*)data);
    
    // Ensure we do not write past the buffer's end
    if (data_len > 512 - offset) {
        data_len = 512 - offset; // Adjust if too long
    }

    // Copy the data into the buffer
    string_num_copy((char*)&buffer[offset], (const char*)data, data_len);
    
    // Write the buffer to sector 0
    ata_write_sector(startSector+SectorIndex++, buffer);
    WrittenSectors++;
}

int CompareFileData(const char *s1, const char *s2, size_t n) {
    while (n > 0) {
        if (*s1 != *s2) {
            return (unsigned char)(*s1) - (unsigned char)(*s2); // Compare the characters
        }
        if (*s1 == '\0') {
            return 0; // Strings are equal up to this point
        }
        s1++;
        s2++;
        n--;
    }
    return 0; // All characters are equal up to n characters
}

uint8_t* ReadFile(char* filename) {
    for (int i = startSector; i <= startSector+WrittenSectors; i++) {
        uint8_t buffer_temp[512];
        ata_read_sector(i, buffer_temp);
        if (CompareFileData((char*)&buffer_temp, "NSFSFILE:TAG:", sizeof("NSFSFILE:TAG:")-1) == 0) {
            if (CompareFileData((char*)&buffer_temp[sizeof("NSFSFILE:TAG:")-1], filename, 8) == 0) {
                uint8_t data[491];
                memoryCopy((char*)data, (char*)&buffer_temp[512-491], 491);
                return data;
            }
        }
    }
}