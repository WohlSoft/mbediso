#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "j9660.h"

struct j9660_io* j9660_io_from_file(FILE* file)
{
    struct j9660_io_file* io = malloc(sizeof(struct j9660_io_file));

    if(!io)
        return NULL;

    io->tag = 1;
    io->file = file;
    io->filepos = -1;

    // eventually, figure out sector size here...
    io->buffer[0] = malloc(2048);

    if(!io->buffer[0])
    {
        free(io);
        return NULL;
    }

    io->buffer[1] = malloc(2048);

    if(!io->buffer[1])
    {
        free(io->buffer[0]);
        free(io);
        return NULL;
    }

    return (struct j9660_io*)io;
}

const uint8_t* j9660_io_read_sector(struct j9660_io* _io, uint32_t sector, bool use_secondary_buffer)
{
    if(!_io)
        return NULL;

    if(_io->tag == 1)
    {
        struct j9660_io_file* io = (struct j9660_io_file*)_io;

        uint64_t target_pos = sector * 2048;

        if(io->filepos != target_pos)
        {
            printf("seeking %lx...\n", target_pos);

            if(fseek(io->file, target_pos, SEEK_SET))
            {
                io->filepos = -1;
                return NULL;
            }
        }

        uint8_t* buffer = io->buffer[use_secondary_buffer];

        if(fread(buffer, 1, 2048, io->file) != 2048)
        {
            printf("read failed...\n");

            io->filepos = -1;
            return NULL;
        }

        io->filepos = (sector + 1) * 2048;

        return buffer;
    }

    return NULL;
}

void j9660_io_close(struct j9660_io* _io)
{
    if(!_io)
        return;

    if(_io->tag == 1)
    {
        struct j9660_io_file* io = (struct j9660_io_file*)_io;

        fclose(io->file);

        free(io->buffer[1]);
        free(io->buffer[0]);

        free(io);
    }
}
