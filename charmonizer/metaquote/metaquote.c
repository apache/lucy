#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

/* Replace text between paired METAQUOTE tags with concatenated C string
 * literals.
 */
char*
metaquote(char *source, size_t source_len, size_t *dest_len);

/* Print a message to stderr and exit.
 */
void
die(char *format, ...);

/* Verify fseek and ftell.  Verify fread, too, but weakly.
 */
#define Check_IO_Op(op) \
    if ((op) == -1) \
        die("IO operation failed at line %d: %s", __LINE__, strerror(errno))

int main(int argc, char **argv) 
{
    FILE   *in_fh, *out_fh;
    char   *source, *dest;
    size_t  source_len, dest_len;
    int     chars_read;

    /* die unless both input and output filenames were supplied */
    if (argc != 3)
        die("Usage: metaquote INPUT_FILEPATH OUTPUT_FILEPATH");

    /* open the input file */
    in_fh = fopen(argv[1], "r");
    if (in_fh == NULL)
        die("Can't open file '%s' for reading: %s", argv[1], strerror(errno));

    /* get the length of the file */
    Check_IO_Op( fseek(in_fh, 0, SEEK_END) );
    Check_IO_Op( source_len = ftell(in_fh) );
    Check_IO_Op( fseek(in_fh, 0, SEEK_SET) );

    /* slurp input file and metaquote the contents */
    source = (char*)malloc(source_len * sizeof(char));
    Check_IO_Op( fread(source, sizeof(char), source_len, in_fh) );
    dest = metaquote(source, source_len, &dest_len);
    
    /* blast out the metaquoted text */
    out_fh = fopen(argv[2], "w");
    if (out_fh == NULL)
        die("Can't open file '%s' for writing: %s", argv[2], strerror(errno));
    if (fwrite(dest, sizeof(char), dest_len, out_fh) != dest_len)
        die("Error writing to '%s': %s", argv[2], strerror(errno));

    /* clean up */
    if (fclose(in_fh))
        die("Error closing file '%s': %s", argv[1], strerror(errno));
    if (fclose(out_fh))
        die("Error closing file '%s': %s", argv[2], strerror(errno));
    free(source);
    free(dest);

    return 0;
}

char*
metaquote(char *source, size_t source_len, size_t *dest_len) 
{
    char *source_limit = source + source_len;
    char *dest = (char*)malloc(100);
    char *dest_start = dest;
    char *dest_limit = dest + (100);
    int   inside_metaquote = 0;

    while (source < source_limit) {
        size_t chars_left = source_limit - source;

        /* reallocate dest if necessary */
        if (dest_limit - dest < 20) {
            size_t cur_len = dest - dest_start;
            dest_start = (char*)realloc(dest_start, cur_len + 100 );
            dest = dest_start + cur_len;
            dest_limit = dest_start + cur_len + 100;
        }

        /* see if we're starting or ending a METAQUOTE */
        if (chars_left >= 9 && strncmp(source, "METAQUOTE", 9) == 0) {
            inside_metaquote = inside_metaquote == 0 ? 1 : 0;
            *dest++ = '"';
            source += 9;
            continue;
        }
        /* if not inside a METAQUOTE, just copy source to dest */
        else if (!inside_metaquote) {
            *dest++ = *source++;
            continue;
        }

        /* inside a METAQUOTE, so render as a series of C string literals */
        else {
            switch(*source) {
            case '\\':
                *dest++ = '\\';
                *dest++ = '\\';
                break;

            case '"':
                *dest++ = '\\';
                *dest++ = '"';
                break;

            case '\r':
                if ((source + 1 < source_limit) && (*(source+1) == '\n'))
                    source++;
                /* fall through */

            case '\n':
                strncpy(dest, "\\n\"\n    \"", 9);
                dest += 9;
                break;

            default:
                *dest++ = *source;
            }

            source++;
        }
    }

    *dest_len = dest - dest_start;

    return dest_start;
}

void 
die(char* format, ...) 
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(1);
}

/**
 * Copyright 2006 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

