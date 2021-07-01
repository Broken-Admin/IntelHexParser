#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

int16_t parseHexadecimalPairString(char highChar, char lowChar);
int16_t parseHexadecimalNibble(char numberChar);
//char numberToHexadecimalNibble(int16_t val);
//char* numberToHexadecimalPair(int16_t val);

int main(int argc, char* argv[]) {
    char* filename = argv[1];
    FILE* inputFile = fopen(filename, "r");
    if(inputFile == NULL) {
        if(filename != NULL) printf("File %s could not be opened. Errno %i.\n", filename, errno);
        else printf("Usage: ./intelhexparse.o filename\n");
        exit(1);
    }
    char* lineBuffer = NULL;
    int64_t lineCharacterCount = 0;
    int readSize = getline(&lineBuffer, &lineCharacterCount, inputFile);
    for(int lineCount = 1; readSize > 0; lineCount++, readSize = getline(&lineBuffer, &lineCharacterCount, inputFile)) {
        if(readSize <= 0) break;
        // Start by reading the colon on the current line
        if(lineBuffer[0] != ':') {
            printf("Line %i does not start with a colon, skipping line.\n", lineCount);
            continue;
        }

        // Assume provided a colon, that the entire line is at least valid

        // Add every byte-pair together, modulus 256, two's complement
        int16_t checksumExpected = 0;

        // Read the record length, 1 byte, amount of data bytes in line
        int16_t recordLength = parseHexadecimalPairString(lineBuffer[1], lineBuffer[2]);
        printf("Line %i contains %i bytes of data.\n", lineCount, recordLength);
        // Read the starting address, these could later maybe be used to combine together, but possibly not. 2 bytes, little endian
        int32_t startingAddress = (parseHexadecimalPairString(lineBuffer[3], lineBuffer[4]) << 8) + parseHexadecimalPairString(lineBuffer[5], lineBuffer[6]);
        // Read the record type which is assumed to be data, if not skip the line
        int16_t recordType = parseHexadecimalPairString(lineBuffer[7], lineBuffer[8]);
        if(recordType != 0) {
            printf("Line %i is not data record type, skipping line.\n", lineCount);
            continue;
        }

        // Add up all previously read bytes
        checksumExpected += recordLength + startingAddress + recordType;
        printf("Starting at hexadecimal address: %02x\n", startingAddress);

        printf("Displaying hexadecimal dump of data for line %i.\n", lineCount);
        // Simplify access to the checksum
        int nibbleCharactersRead = 0;
        // Offset of 9 for data bytes
        // Every record is 2 characters, increment 2 characters until reached end of record
        for(int i = 0; i < recordLength * 2; i+=2) {
            int16_t cValue = parseHexadecimalPairString(lineBuffer[9+i], lineBuffer[10+i]);
            printf("%c%c", lineBuffer[9+i], lineBuffer[10+i]);
            if(i + 2 != recordLength * 2) printf(" ");
            checksumExpected += cValue;
            nibbleCharactersRead += 2;
        }
        printf("\n");
        // Take two's complement of sum of all byte-pairs
        
        checksumExpected = (0xff - (checksumExpected % 256)) + 1;
        // Confirm the checksum is correct, if not rewrite it
        int16_t checksumValue = parseHexadecimalPairString(lineBuffer[9+nibbleCharactersRead], lineBuffer[10+nibbleCharactersRead]);
        if(checksumValue != checksumExpected) {
            printf("Expected checksum %02x but got %02x.\n\n", checksumExpected, checksumValue);
        } else {
            printf("Got expected checksum %02x.\n\n", checksumExpected);
        }
    }
    free(lineBuffer);
    fclose(inputFile);
}

int16_t parseHexadecimalPairString(char highChar, char lowChar) {
    int16_t highNibble = parseHexadecimalNibble(highChar);
    int16_t lowNibble = parseHexadecimalNibble(lowChar);
    return((highNibble * 16) +  lowNibble);
}

int16_t parseHexadecimalNibble(char numberChar) {
    // int16_t numberInt (int16_t)numberChar;
    if(numberChar >= 'A' && numberChar <= 'Z')
        return(
            ((int16_t)numberChar - 0x40) + 9
        );
    else return((int16_t)numberChar - 0x30);
}

// Used by unnecessary function numberToHexadecimalPair
/* char numberToHexadecimalNibble(int16_t val) {
    if(val > 9) return((val - 9) + 0x40);
    else return(val + 0x30);
} */

// This can be made unecessary by using %02x in printf
/* char* numberToHexadecimalPair(int16_t val) {
    char* pair = malloc(3);
    int16_t highNibble = val >> 4;
    int16_t lowNibble = val - (highNibble * 16);
    pair[0] = numberToHexadecimalNibble(highNibble);
    pair[1] = numberToHexadecimalNibble(lowNibble);
    // Null terminator
    pair[2] = '\0';
    return(pair);
} */