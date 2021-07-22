#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    unsigned char ByteCount;
    unsigned short Address;
    unsigned char Type;
    unsigned char *Data;
    unsigned char Checksum;
} dtHexLine;

unsigned char CharToHex(char ch)
{
    unsigned char ret = 0;
    if((ch >= '0') && (ch <= '9')) ret = ch -'0';
    if((ch >= 'A') && (ch <= 'F')) ret = ch -'7';
    if((ch >= 'a') && (ch <= 'f')) ret = ch -'W';
    return ret;
}

unsigned char* Append(unsigned char newCh, unsigned char *buff, int size)
{
    unsigned char* t = (unsigned char*) calloc(size+1, sizeof(unsigned char));
    if(buff != 0)
    {
        memcpy(t,buff, size);
        free(buff);
    }
    t[size] = newCh;
    return t;
}

int CheckEnd(unsigned char *Bytes)
{
    int ret = 0;
    if((Bytes[-4] == 0) && (Bytes[-3] == 0) && (Bytes[-2] == 0) && (Bytes[-1] == 1) && (Bytes[0] == -1)) ret = 1;
    //printf("Checkend: %x %x %x %x %x \n", Bytes[-4],Bytes[-3],Bytes[-2],Bytes[-1],Bytes[0]);
    return ret;
}

int CheckBin(unsigned char *Bytes, int Length)
{
	int ret = 0;
    dtHexLine line;
    int i = 0;
    unsigned char Checksum = 0;
    while(i < Length && Checksum ==0)
	{
		Checksum = 0;
		line.ByteCount = Bytes[i++];
		line.Address = Bytes[i++]<<8;
		line.Address |= Bytes[i++];
		line.Type = Bytes[i++];
		line.Data = &Bytes[i];
		i+=line.ByteCount;
		line.Checksum = Bytes[i++];

		Checksum += line.ByteCount;
		Checksum += line.Address>>8;
		Checksum += line.Address&0xFF;
		Checksum += line.Type;
		for(int j=0;j<line.ByteCount; j++) Checksum += (unsigned char)line.Data[j];
		Checksum += line.Checksum;
		printf("Checksum: %x\n", Checksum);

		printf("%x %x %x ", line.ByteCount, line.Address, line.Type);
		for(int j=0;j<line.ByteCount; j++) printf("%x ", (unsigned char)(line.Data[j]));
		printf("%x\n", (unsigned char)line.Checksum);
	}
    return ret;
}

int main(int argc, char **argv)
{
    if(argc == 1)
    {
        printf("No input file\n");
        return -1;
    }
    else
    {
        FILE *HexFile=fopen(argv[1], "rb");
        if(HexFile == 0)
        {
            printf("Cannot open the HEX file\n}");
            return -2;
        }
        else
        {
            printf("HEX opening is successful\n");
            int length = 0;
            char *HexFileBytes = 0;
            unsigned char* BinFile = 0;
            int BinFileCntr = 0;

            /* Getting the length of the file */
            fseek(HexFile, 0, SEEK_END);
            length = ftell(HexFile);
            fseek(HexFile, 0, SEEK_SET);
            printf("Size of the HEX file: %d\n", length);

            HexFileBytes = (char*)calloc(length+1, sizeof(char));
            HexFileBytes[length] = 0;
            if(fread(HexFileBytes, sizeof(char), length, HexFile) == length)
            {
                int looper = 0;
                unsigned char Temp = 0;
                char DigitCntr = 0;
                char Eof = 0;
                while(Eof == 0)
                {
                    /* Getting to the beginning of a line */
                    while(HexFileBytes[looper] != ':') looper++;
                    looper++;

                    DigitCntr = 0;

                    /* Convert the digits to binary data */
                    while((HexFileBytes[looper] != '\n') && (looper < length))
                    {
                        if(DigitCntr++ == 0) Temp += CharToHex(HexFileBytes[looper]) << 4;
                        else
                        {
                            Temp += CharToHex(HexFileBytes[looper]);
                            BinFile = Append(Temp, BinFile, BinFileCntr);
                            BinFileCntr++;
                            DigitCntr = 0;
                            Temp = 0;
                        }
                        looper++;
                    }
                    /* Check if the previous line was EOF */
                    if(BinFileCntr >= 5) Eof = CheckEnd(&BinFile[BinFileCntr-1]);
                    printf("%d\n", looper);
                }

                printf("BinFile: ");
                for(int looper = 0; looper < 30; looper ++) printf("%x ", (unsigned char)BinFile[looper]&0xff);
                printf("\n");

                CheckBin(BinFile, BinFileCntr);
                printf("Length: %d\n", BinFileCntr);
            }
            else
            {
                printf("Error while reading the file\n");
            }
            free(HexFileBytes);
            fclose(HexFile);
        }
    }
    return 0;
}
