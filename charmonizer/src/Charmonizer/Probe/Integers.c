/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define CHAZ_USE_SHORT_NAMES

#include "Charmonizer/Core/HeaderChecker.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/Integers.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Determine endian-ness of this machine.
 */
static chaz_bool_t
S_machine_is_big_endian();

static char sizes_code[] =
    QUOTE(  #include "_charm.h"                       )
    QUOTE(  int main () {                             )
    QUOTE(      Charm_Setup;                          )
    QUOTE(      printf("%d ", (int)sizeof(char));     )
    QUOTE(      printf("%d ", (int)sizeof(short));    )
    QUOTE(      printf("%d ", (int)sizeof(int));      )
    QUOTE(      printf("%d ", (int)sizeof(long));     )
    QUOTE(      printf("%d ", (int)sizeof(void*));    )
    QUOTE(      return 0;                             )
    QUOTE(  }                                         );

static char type64_code[] =
    QUOTE(  #include "_charm.h"                       )
    QUOTE(  int main()                                )
    QUOTE(  {                                         )
    QUOTE(      Charm_Setup;                          )
    QUOTE(      printf("%%d", (int)sizeof(%s));       )
    QUOTE(      return 0;                             )
    QUOTE(  }                                         );

static char literal64_code[] =
    QUOTE(  #include "_charm.h"                       )
    QUOTE(  #define big 9000000000000000000%s         )
    QUOTE(  int main()                                )
    QUOTE(  {                                         )
    QUOTE(      Charm_Setup;                          )
    QUOTE(      int truncated = (int)big;             )
    QUOTE(      printf("%%d\n", truncated);           )
    QUOTE(      return 0;                             )
    QUOTE(  }                                         );

void
Integers_run(void) {
    char *output;
    size_t output_len;
    int sizeof_char       = -1;
    int sizeof_short      = -1;
    int sizeof_int        = -1;
    int sizeof_ptr        = -1;
    int sizeof_long       = -1;
    int sizeof_long_long  = -1;
    int sizeof___int64    = -1;
    chaz_bool_t has_8     = false;
    chaz_bool_t has_16    = false;
    chaz_bool_t has_32    = false;
    chaz_bool_t has_64    = false;
    chaz_bool_t has_long_long = false;
    chaz_bool_t has___int64   = false;
    chaz_bool_t has_inttypes  = HeadCheck_check_header("inttypes.h");
    chaz_bool_t has_stdint    = HeadCheck_check_header("stdint.h");
    char i32_t_type[10];
    char i32_t_postfix[10];
    char u32_t_postfix[10];
    char i64_t_type[10];
    char i64_t_postfix[10];
    char u64_t_postfix[10];
    char code_buf[1000];

    ConfWriter_start_module("Integers");

    /* Document endian-ness. */
    if (S_machine_is_big_endian()) {
        ConfWriter_append_conf("#define CHY_BIG_END\n");
    }
    else {
        ConfWriter_append_conf("#define CHY_LITTLE_END\n");
    }

    /* Record sizeof() for several common integer types. */
    output = CC_capture_output(sizes_code, strlen(sizes_code), &output_len);
    if (output != NULL) {
        char *end_ptr = output;

        sizeof_char  = strtol(output, &end_ptr, 10);
        output       = end_ptr;
        sizeof_short = strtol(output, &end_ptr, 10);
        output       = end_ptr;
        sizeof_int   = strtol(output, &end_ptr, 10);
        output       = end_ptr;
        sizeof_long  = strtol(output, &end_ptr, 10);
        output       = end_ptr;
        sizeof_ptr   = strtol(output, &end_ptr, 10);
    }

    /* Determine whether long longs are available. */
    sprintf(code_buf, type64_code, "long long");
    output = CC_capture_output(code_buf, strlen(code_buf), &output_len);
    if (output != NULL) {
        has_long_long    = true;
        sizeof_long_long = strtol(output, NULL, 10);
    }

    /* Determine whether the __int64 type is available. */
    sprintf(code_buf, type64_code, "__int64");
    output = CC_capture_output(code_buf, strlen(code_buf), &output_len);
    if (output != NULL) {
        has___int64 = true;
        sizeof___int64 = strtol(output, NULL, 10);
    }

    /* Figure out which integer types are available. */
    if (sizeof_char == 1) {
        has_8 = true;
    }
    if (sizeof_short == 2) {
        has_16 = true;
    }
    if (sizeof_int == 4) {
        has_32 = true;
        strcpy(i32_t_type, "int");
        strcpy(i32_t_postfix, "");
        strcpy(u32_t_postfix, "U");
    }
    else if (sizeof_long == 4) {
        has_32 = true;
        strcpy(i32_t_type, "long");
        strcpy(i32_t_postfix, "L");
        strcpy(u32_t_postfix, "UL");
    }
    if (sizeof_long == 8) {
        has_64 = true;
        strcpy(i64_t_type, "long");
    }
    else if (sizeof_long_long == 8) {
        has_64 = true;
        strcpy(i64_t_type, "long long");
    }
    else if (sizeof___int64 == 8) {
        has_64 = true;
        strcpy(i64_t_type, "__int64");
    }

    /* Probe for 64-bit literal syntax. */
    if (has_64 && sizeof_long == 8) {
        strcpy(i64_t_postfix, "L");
        strcpy(u64_t_postfix, "UL");
    }
    else if (has_64) {
        sprintf(code_buf, literal64_code, "LL");
        output = CC_capture_output(code_buf, strlen(code_buf), &output_len);
        if (output != NULL) {
            strcpy(i64_t_postfix, "LL");
        }
        else {
            sprintf(code_buf, literal64_code, "i64");
            output = CC_capture_output(code_buf, strlen(code_buf),
                                       &output_len);
            if (output != NULL) {
                strcpy(i64_t_postfix, "i64");
            }
            else {
                Util_die("64-bit types, but no literal syntax found");
            }
        }
        sprintf(code_buf, literal64_code, "ULL");
        output = CC_capture_output(code_buf, strlen(code_buf), &output_len);
        if (output != NULL) {
            strcpy(u64_t_postfix, "ULL");
        }
        else {
            sprintf(code_buf, literal64_code, "Ui64");
            output = CC_capture_output(code_buf, strlen(code_buf), &output_len);
            if (output != NULL) {
                strcpy(u64_t_postfix, "Ui64");
            }
            else {
                Util_die("64-bit types, but no literal syntax found");
            }
        }
    }

    /* Write out some conditional defines. */
    if (has_inttypes) {
        ConfWriter_append_conf("#define CHY_HAS_INTTYPES_H\n");
    }
    if (has_stdint) {
        ConfWriter_append_conf("#define CHY_HAS_STDINT_H\n");
    }
    if (has_long_long) {
        ConfWriter_append_conf("#define CHY_HAS_LONG_LONG\n");
    }
    if (has___int64) {
        ConfWriter_append_conf("#define CHY_HAS___INT64\n");
    }

    /* Write out sizes. */
    ConfWriter_append_conf("#define CHY_SIZEOF_CHAR %d\n",  sizeof_char);
    ConfWriter_append_conf("#define CHY_SIZEOF_SHORT %d\n", sizeof_short);
    ConfWriter_append_conf("#define CHY_SIZEOF_INT %d\n",   sizeof_int);
    ConfWriter_append_conf("#define CHY_SIZEOF_LONG %d\n",  sizeof_long);
    ConfWriter_append_conf("#define CHY_SIZEOF_PTR %d\n",   sizeof_ptr);
    if (has_long_long) {
        ConfWriter_append_conf("#define CHY_SIZEOF_LONG_LONG %d\n",
                               sizeof_long_long);
    }
    if (has___int64) {
        ConfWriter_append_conf("#define CHY_SIZEOF___INT64 %d\n",
                               sizeof___int64);
    }

    /* Write affirmations, typedefs and maximums/minimums. */
    ConfWriter_append_conf("typedef int chy_bool_t;\n");
    if (has_stdint) {
        ConfWriter_append_conf("#include <stdint.h>\n");
    }
    else {
        /* we support only the following subset of stdint.h
         *   int8_t
         *   int16_t
         *   int32_t
         *   int64_t
         *   uint8_t
         *   uint16_t
         *   uint32_t
         *   uint64_t
         */
        if (has_8) {
            ConfWriter_append_conf(
                "typedef signed char int8_t;\n"
                "typedef unsigned char uint8_t;\n"
            );
        }
        if (has_16) {
            ConfWriter_append_conf(
                "typedef short int16_t;\n"
                "typedef unsigned short uint16_t;\n"
            );
        }
        if (has_32) {
            ConfWriter_append_conf(
                "typedef %s int32_t;\n", i32_t_type
            );
            ConfWriter_append_conf(
                "typedef unsigned %s uint32_t;\n", i32_t_type
            );
        }
        if (has_64) {
            ConfWriter_append_conf(
                "typedef %s int64_t;\n", i64_t_type
            );
            ConfWriter_append_conf(
                "typedef unsigned %s uint64_t;\n", i64_t_type
            );
        }
    }
    if (has_8) {
        ConfWriter_append_conf(
            "#define CHY_HAS_I8_T\n"
            "typedef signed char chy_i8_t;\n"
            "typedef unsigned char chy_u8_t;\n"
            "#define CHY_I8_MAX 0x7F\n"
            "#define CHY_I8_MIN (-I8_MAX - 1)\n"
            "#define CHY_U8_MAX (I8_MAX * 2 + 1)\n"
        );
    }
    if (has_16) {
        ConfWriter_append_conf(
            "#define CHY_HAS_I16_T\n"
            "typedef short chy_i16_t;\n"
            "typedef unsigned short chy_u16_t;\n"
            "#define CHY_I16_MAX 0x7FFF\n"
            "#define CHY_I16_MIN (-I16_MAX - 1)\n"
            "#define CHY_U16_MAX (I16_MAX * 2 + 1)\n"
        );
    }
    if (has_32) {
        ConfWriter_append_conf("#define CHY_HAS_I32_T\n");
        ConfWriter_append_conf("typedef %s chy_i32_t;\n", i32_t_type);
        ConfWriter_append_conf("typedef unsigned %s chy_u32_t;\n",
                               i32_t_type);
        ConfWriter_append_conf("#define CHY_I32_MAX 0x7FFFFFFF%s\n",
                               i32_t_postfix);
        ConfWriter_append_conf("#define CHY_I32_MIN (-I32_MAX - 1)\n");
        ConfWriter_append_conf("#define CHY_U32_MAX (I32_MAX * 2%s + 1%s)\n",
                               u32_t_postfix, u32_t_postfix);
    }
    if (has_64) {
        ConfWriter_append_conf("#define CHY_HAS_I64_T\n");
        ConfWriter_append_conf("typedef %s chy_i64_t;\n", i64_t_type);
        ConfWriter_append_conf("typedef unsigned %s chy_u64_t;\n",
                               i64_t_type);
        ConfWriter_append_conf("#define CHY_I64_MAX 0x7FFFFFFFFFFFFFFF%s\n",
                               i64_t_postfix);
        ConfWriter_append_conf("#define CHY_I64_MIN (-I64_MAX - 1%s)\n",
                               i64_t_postfix);
        ConfWriter_append_conf("#define CHY_U64_MAX (I64_MAX * 2%s + 1%s)\n",
                               u64_t_postfix, u64_t_postfix);
    }

    /* Create the I64P and U64P printf macros. */
    if (has_64) {
        int i;
        char *options[] = {
            "ll",
            "l",
            "L",
            "q",  /* Some *BSDs */
            "I64", /* Microsoft */
            NULL,
        };

        /* Buffer to hold the code, and its start and end. */
        static char format_64_code[] =
            QUOTE(  #include "_charm.h"                           )
            QUOTE(  int main() {                                  )
            QUOTE(      Charm_Setup;                              )
            QUOTE(      printf("%%%su", 18446744073709551615%s);  )
            QUOTE(      return 0;                                 )
            QUOTE( }                                              );

        for (i = 0; options[i] != NULL; i++) {
            /* Try to print 2**64-1, and see if we get it back intact. */
            sprintf(code_buf, format_64_code, options[i], u64_t_postfix);
            output = CC_capture_output(code_buf, strlen(code_buf),
                                       &output_len);

            if (output_len != 0
                && strcmp(output, "18446744073709551615") == 0
               ) {
                ConfWriter_append_conf("#define CHY_I64P \"%sd\"\n",
                                       options[i]);
                ConfWriter_append_conf("#define CHY_U64P \"%su\"\n",
                                       options[i]);
                break;
            }
        }

    }

    /* Write out the 32-bit and 64-bit literal macros. */
    if (has_32) {
        if (strcmp(i32_t_postfix, "") == 0) {
            ConfWriter_append_conf("#define CHY_I32_C(n) n\n");
            ConfWriter_append_conf("#define CHY_U32_C(n) n##%s\n",
                                   u32_t_postfix);
        }
        else {
            ConfWriter_append_conf("#define CHY_I32_C(n) n##%s\n",
                                   i32_t_postfix);
            ConfWriter_append_conf("#define CHY_U32_C(n) n##%s\n",
                                   u32_t_postfix);
        }
    }
    if (has_64) {
        ConfWriter_append_conf("#define CHY_I64_C(n) n##%s\n", i64_t_postfix);
        ConfWriter_append_conf("#define CHY_U64_C(n) n##%s\n", u64_t_postfix);
    }

    /* Create macro for promoting pointers to integers. */
    if (has_64) {
        if (sizeof_ptr == 8) {
            ConfWriter_append_conf("#define CHY_PTR_TO_I64(ptr) "
                                   "((chy_i64_t)(chy_u64_t)(ptr))\n");
        }
        else {
            ConfWriter_append_conf("#define CHY_PTR_TO_I64(ptr) "
                                   "((chy_i64_t)(chy_u32_t)(ptr))\n");
        }
    }

    /* True and false. */
    ConfWriter_append_conf(
        "#ifndef true\n"
        "  #define true 1\n"
        "#endif\n"
        "#ifndef false\n"
        "  #define false 0\n"
        "#endif\n"
    );

    /* Shorten. */
    ConfWriter_start_short_names();
    if (S_machine_is_big_endian()) {
        ConfWriter_shorten_macro("BIG_END");
    }
    else {
        ConfWriter_shorten_macro("LITTLE_END");
    }
    ConfWriter_shorten_macro("SIZEOF_CHAR");
    ConfWriter_shorten_macro("SIZEOF_SHORT");
    ConfWriter_shorten_macro("SIZEOF_LONG");
    ConfWriter_shorten_macro("SIZEOF_INT");
    ConfWriter_shorten_macro("SIZEOF_PTR");
    if (has_long_long) {
        ConfWriter_shorten_macro("HAS_LONG_LONG");
        ConfWriter_shorten_macro("SIZEOF_LONG_LONG");
    }
    if (has___int64) {
        ConfWriter_shorten_macro("HAS___INT64");
        ConfWriter_shorten_macro("SIZEOF___INT64");
    }
    if (has_inttypes) {
        ConfWriter_shorten_macro("HAS_INTTYPES_H");
    }
    ConfWriter_shorten_typedef("bool_t");
    if (has_8) {
        ConfWriter_shorten_macro("HAS_I8_T");
        ConfWriter_shorten_typedef("i8_t");
        ConfWriter_shorten_typedef("u8_t");
        ConfWriter_shorten_macro("I8_MAX");
        ConfWriter_shorten_macro("I8_MIN");
        ConfWriter_shorten_macro("U8_MAX");
    }
    if (has_16) {
        ConfWriter_shorten_macro("HAS_I16_T");
        ConfWriter_shorten_typedef("i16_t");
        ConfWriter_shorten_typedef("u16_t");
        ConfWriter_shorten_macro("I16_MAX");
        ConfWriter_shorten_macro("I16_MIN");
        ConfWriter_shorten_macro("U16_MAX");
    }
    if (has_32) {
        ConfWriter_shorten_macro("HAS_I32_T");
        ConfWriter_shorten_typedef("i32_t");
        ConfWriter_shorten_typedef("u32_t");
        ConfWriter_shorten_macro("I32_MAX");
        ConfWriter_shorten_macro("I32_MIN");
        ConfWriter_shorten_macro("U32_MAX");
        ConfWriter_shorten_macro("I32_C");
        ConfWriter_shorten_macro("U32_C");
    }
    if (has_64) {
        ConfWriter_shorten_macro("HAS_I64_T");
        ConfWriter_shorten_typedef("i64_t");
        ConfWriter_shorten_typedef("u64_t");
        ConfWriter_shorten_macro("I64_MAX");
        ConfWriter_shorten_macro("I64_MIN");
        ConfWriter_shorten_macro("U64_MAX");
        ConfWriter_shorten_macro("I64P");
        ConfWriter_shorten_macro("U64P");
        ConfWriter_shorten_macro("I64_C");
        ConfWriter_shorten_macro("U64_C");
        ConfWriter_shorten_macro("PTR_TO_I64");
    }
    ConfWriter_end_short_names();

    ConfWriter_end_module();
}

static chaz_bool_t
S_machine_is_big_endian() {
    long one = 1;
    return !(*((char*)(&one)));
}


