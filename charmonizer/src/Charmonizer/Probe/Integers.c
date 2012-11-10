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
static int
chaz_Integers_machine_is_big_endian(void);

static const char chaz_Integers_sizes_code[] =
    CHAZ_QUOTE(  #include "_charm.h"                       )
    CHAZ_QUOTE(  int main () {                             )
    CHAZ_QUOTE(      Charm_Setup;                          )
    CHAZ_QUOTE(      printf("%d ", (int)sizeof(char));     )
    CHAZ_QUOTE(      printf("%d ", (int)sizeof(short));    )
    CHAZ_QUOTE(      printf("%d ", (int)sizeof(int));      )
    CHAZ_QUOTE(      printf("%d ", (int)sizeof(long));     )
    CHAZ_QUOTE(      printf("%d ", (int)sizeof(void*));    )
    CHAZ_QUOTE(      return 0;                             )
    CHAZ_QUOTE(  }                                         );

static const char chaz_Integers_type64_code[] =
    CHAZ_QUOTE(  #include "_charm.h"                       )
    CHAZ_QUOTE(  int main()                                )
    CHAZ_QUOTE(  {                                         )
    CHAZ_QUOTE(      Charm_Setup;                          )
    CHAZ_QUOTE(      printf("%%d", (int)sizeof(%s));       )
    CHAZ_QUOTE(      return 0;                             )
    CHAZ_QUOTE(  }                                         );

static const char chaz_Integers_literal64_code[] =
    CHAZ_QUOTE(  #include "_charm.h"                       )
    CHAZ_QUOTE(  #define big 9000000000000000000%s         )
    CHAZ_QUOTE(  int main()                                )
    CHAZ_QUOTE(  {                                         )
    CHAZ_QUOTE(      int truncated = (int)big;             )
    CHAZ_QUOTE(      Charm_Setup;                          )
    CHAZ_QUOTE(      printf("%%d\n", truncated);           )
    CHAZ_QUOTE(      return 0;                             )
    CHAZ_QUOTE(  }                                         );

static const char chaz_Integers_u64_to_double_code[] =
    CHAZ_QUOTE(  #include "_charm.h"                       )
    CHAZ_QUOTE(  int main()                                )
    CHAZ_QUOTE(  {                                         )
    CHAZ_QUOTE(      unsigned __int64 int_num = 0;         )
    CHAZ_QUOTE(      double float_num;                     )
    CHAZ_QUOTE(      Charm_Setup;                          )
    CHAZ_QUOTE(      float_num = (double)int_num;          )
    CHAZ_QUOTE(      printf("%%f\n", float_num);           )
    CHAZ_QUOTE(      return 0;                             )
    CHAZ_QUOTE(  }                                         );

void
chaz_Integers_run(void) {
    char *output;
    size_t output_len;
    int sizeof_char       = -1;
    int sizeof_short      = -1;
    int sizeof_int        = -1;
    int sizeof_ptr        = -1;
    int sizeof_long       = -1;
    int sizeof_long_long  = -1;
    int sizeof___int64    = -1;
    int has_8             = false;
    int has_16            = false;
    int has_32            = false;
    int has_64            = false;
    int has_long_long     = false;
    int has___int64       = false;
    int has_inttypes      = chaz_HeadCheck_check_header("inttypes.h");
    int has_stdint        = chaz_HeadCheck_check_header("stdint.h");
    int can_convert_u64_to_double = true;
    char i32_t_type[10];
    char i32_t_postfix[10];
    char u32_t_postfix[10];
    char i64_t_type[10];
    char i64_t_postfix[10];
    char u64_t_postfix[10];
    char code_buf[1000];
    char scratch[50];

    chaz_ConfWriter_start_module("Integers");

    /* Document endian-ness. */
    if (chaz_Integers_machine_is_big_endian()) {
        chaz_ConfWriter_add_def("BIG_END", NULL);
    }
    else {
        chaz_ConfWriter_add_def("LITTLE_END", NULL);
    }

    /* Record sizeof() for several common integer types. */
    output = chaz_CC_capture_output(chaz_Integers_sizes_code, &output_len);
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
    sprintf(code_buf, chaz_Integers_type64_code, "long long");
    output = chaz_CC_capture_output(code_buf, &output_len);
    if (output != NULL) {
        has_long_long    = true;
        sizeof_long_long = strtol(output, NULL, 10);
    }

    /* Determine whether the __int64 type is available. */
    sprintf(code_buf, chaz_Integers_type64_code, "__int64");
    output = chaz_CC_capture_output(code_buf, &output_len);
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
        sprintf(code_buf, chaz_Integers_literal64_code, "LL");
        output = chaz_CC_capture_output(code_buf, &output_len);
        if (output != NULL) {
            strcpy(i64_t_postfix, "LL");
        }
        else {
            sprintf(code_buf, chaz_Integers_literal64_code, "i64");
            output = chaz_CC_capture_output(code_buf, &output_len);
            if (output != NULL) {
                strcpy(i64_t_postfix, "i64");
            }
            else {
                chaz_Util_die("64-bit types, but no literal syntax found");
            }
        }
        sprintf(code_buf, chaz_Integers_literal64_code, "ULL");
        output = chaz_CC_capture_output(code_buf, &output_len);
        if (output != NULL) {
            strcpy(u64_t_postfix, "ULL");
        }
        else {
            sprintf(code_buf, chaz_Integers_literal64_code, "Ui64");
            output = chaz_CC_capture_output(code_buf, &output_len);
            if (output != NULL) {
                strcpy(u64_t_postfix, "Ui64");
            }
            else {
                chaz_Util_die("64-bit types, but no literal syntax found");
            }
        }
    }

    /* Determine whether conversion of unsigned __int64 to double works */
    if (has___int64) {
        if (!chaz_CC_test_compile(chaz_Integers_u64_to_double_code)) {
            can_convert_u64_to_double = false;
        }
    }

    /* Write out some conditional defines. */
    if (has_inttypes) {
        chaz_ConfWriter_add_def("HAS_INTTYPES_H", NULL);
    }
    if (has_stdint) {
        chaz_ConfWriter_add_def("HAS_STDINT_H", NULL);
    }
    if (has_long_long) {
        chaz_ConfWriter_add_def("HAS_LONG_LONG", NULL);
    }
    if (has___int64) {
        chaz_ConfWriter_add_def("HAS___INT64", NULL);
    }

    /* Write out sizes. */
    sprintf(scratch, "%d", sizeof_char);
    chaz_ConfWriter_add_def("SIZEOF_CHAR", scratch);
    sprintf(scratch, "%d", sizeof_short);
    chaz_ConfWriter_add_def("SIZEOF_SHORT", scratch);
    sprintf(scratch, "%d", sizeof_int);
    chaz_ConfWriter_add_def("SIZEOF_INT", scratch);
    sprintf(scratch, "%d", sizeof_long);
    chaz_ConfWriter_add_def("SIZEOF_LONG", scratch);
    sprintf(scratch, "%d", sizeof_ptr);
    chaz_ConfWriter_add_def("SIZEOF_PTR", scratch);
    if (has_long_long) {
        sprintf(scratch, "%d", sizeof_long_long);
        chaz_ConfWriter_add_def("SIZEOF_LONG_LONG", scratch);
    }
    if (has___int64) {
        sprintf(scratch, "%d", sizeof___int64);
        chaz_ConfWriter_add_def("SIZEOF___INT64", scratch);
    }

    /* Write affirmations, typedefs and maximums/minimums. */
    chaz_ConfWriter_add_typedef("int", "bool_t");
    if (has_stdint) {
        chaz_ConfWriter_add_sys_include("stdint.h");
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
            chaz_ConfWriter_add_global_typedef("signed char", "int8_t");
            chaz_ConfWriter_add_global_typedef("unsigned char", "uint8_t");
        }
        if (has_16) {
            chaz_ConfWriter_add_global_typedef("signed short", "int16_t");
            chaz_ConfWriter_add_global_typedef("unsigned short", "uint16_t");
        }
        if (has_32) {
            chaz_ConfWriter_add_global_typedef(i32_t_type, "int32_t");
            sprintf(scratch, "unsigned %s", i32_t_type);
            chaz_ConfWriter_add_global_typedef(scratch, "uint32_t");
        }
        if (has_64) {
            chaz_ConfWriter_add_global_typedef(i64_t_type, "int64_t");
            sprintf(scratch, "unsigned %s", i64_t_type);
            chaz_ConfWriter_add_global_typedef(scratch, "uint64_t");
        }
    }
    if (has_8) {
        chaz_ConfWriter_add_def("HAS_I8_T", NULL);
        chaz_ConfWriter_add_typedef("signed char", "i8_t");
        chaz_ConfWriter_add_typedef("unsigned char", "u8_t");
        /* FIXME: use integer literals. */
        chaz_ConfWriter_add_def("I8_MAX", "0x7F");
        chaz_ConfWriter_add_def("I8_MIN", "(-I8_MAX - 1)");
        chaz_ConfWriter_add_def("U8_MAX", "(I8_MAX * 2 + 1)");
    }
    if (has_16) {
        chaz_ConfWriter_add_def("HAS_I16_T", NULL);
        chaz_ConfWriter_add_typedef("short", "i16_t");
        chaz_ConfWriter_add_typedef("unsigned short", "u16_t");
        /* FIXME: use integer literals. */
        chaz_ConfWriter_add_def("I16_MAX", "0x7FFF");
        chaz_ConfWriter_add_def("I16_MIN", "(-I16_MAX - 1)");
        chaz_ConfWriter_add_def("U16_MAX", "(I16_MAX * 2 + 1)");
    }
    if (has_32) {
        chaz_ConfWriter_add_def("HAS_I32_T", NULL);
        chaz_ConfWriter_add_typedef(i32_t_type, "i32_t");
        sprintf(scratch, "unsigned %s", i32_t_type);
        chaz_ConfWriter_add_typedef(scratch, "u32_t");
        /* FIXME: use integer literals. */
        sprintf(scratch, "0x7FFFFFFF%s", i32_t_postfix);
        chaz_ConfWriter_add_def("I32_MAX", scratch);
        chaz_ConfWriter_add_def("I32_MIN", "(-I32_MAX - 1)");
        sprintf(scratch, "(I32_MAX * 2%s + 1%s)", u32_t_postfix,
                u32_t_postfix);
        chaz_ConfWriter_add_def("U32_MAX", scratch);
    }
    if (has_64) {
        chaz_ConfWriter_add_def("HAS_I64_T", NULL);
        chaz_ConfWriter_add_typedef(i64_t_type, "i64_t");
        sprintf(scratch, "unsigned %s", i64_t_type);
        chaz_ConfWriter_add_typedef(scratch, "u64_t");
        /* FIXME: use integer literals. */
        sprintf(scratch, "0x7FFFFFFFFFFFFFFF%s", i64_t_postfix);
        chaz_ConfWriter_add_def("I64_MAX", scratch);
        sprintf(scratch, "(-I64_MAX - 1%s)", i64_t_postfix);
        chaz_ConfWriter_add_def("I64_MIN", scratch);
        sprintf(scratch, "(I64_MAX * 2%s + 1%s)", u64_t_postfix,
                u64_t_postfix);
        chaz_ConfWriter_add_def("U64_MAX", scratch);
    }

    /* Create the I64P and U64P printf macros. */
    if (has_64) {
        int i;
        const char *options[] = {
            "ll",
            "l",
            "L",
            "q",  /* Some *BSDs */
            "I64", /* Microsoft */
            NULL,
        };

        /* Buffer to hold the code, and its start and end. */
        static const char format_64_code[] =
            CHAZ_QUOTE(  #include "_charm.h"                           )
            CHAZ_QUOTE(  int main() {                                  )
            CHAZ_QUOTE(      Charm_Setup;                              )
            CHAZ_QUOTE(      printf("%%%su", 18446744073709551615%s);  )
            CHAZ_QUOTE(      return 0;                                 )
            CHAZ_QUOTE( }                                              );

        for (i = 0; options[i] != NULL; i++) {
            /* Try to print 2**64-1, and see if we get it back intact. */
            sprintf(code_buf, format_64_code, options[i], u64_t_postfix);
            output = chaz_CC_capture_output(code_buf, &output_len);

            if (output_len != 0
                && strcmp(output, "18446744073709551615") == 0
               ) {
                sprintf(scratch, "\"%sd\"", options[i]);
                chaz_ConfWriter_add_def("I64P", scratch);
                sprintf(scratch, "\"%su\"", options[i]);
                chaz_ConfWriter_add_def("U64P", scratch);
                break;
            }
        }

    }

    /* Write out the 32-bit and 64-bit literal macros. */
    if (has_32) {
        if (strcmp(i32_t_postfix, "") == 0) {
            chaz_ConfWriter_add_def("I32_C(n)", "n");
            sprintf(scratch, "n##%s", u32_t_postfix);
            chaz_ConfWriter_add_def("U32_C(n)", scratch);
        }
        else {
            sprintf(scratch, "n##%s", i32_t_postfix);
            chaz_ConfWriter_add_def("I32_C(n)", scratch);
            sprintf(scratch, "n##%s", u32_t_postfix);
            chaz_ConfWriter_add_def("U32_C(n)", scratch);
        }
    }
    if (has_64) {
        sprintf(scratch, "n##%s", i64_t_postfix);
        chaz_ConfWriter_add_def("I64_C(n)", scratch);
        sprintf(scratch, "n##%s", u64_t_postfix);
        chaz_ConfWriter_add_def("U64_C(n)", scratch);
    }

    /* Create macro for promoting pointers to integers. */
    if (has_64) {
        if (sizeof_ptr == 8) {
            chaz_ConfWriter_add_def("PTR_TO_I64(ptr)",
                                    "((chy_i64_t)(chy_u64_t)(ptr))");
        }
        else {
            chaz_ConfWriter_add_def("PTR_TO_I64(ptr)",
                                    "((chy_i64_t)(chy_u32_t)(ptr))");
        }
    }

    /* Create macro for converting uint64_t to double. */
    if (can_convert_u64_to_double) {
        chaz_ConfWriter_add_def("U64_TO_DOUBLE(num)",
                                "((double)(num))");
    }
    else {
        chaz_ConfWriter_add_def(
            "U64_TO_DOUBLE(num)",
            "((num) & CHY_U64_C(0x8000000000000000) ? "
            "(double)(int64_t)((num) & CHY_U64_C(0x7FFFFFFFFFFFFFFF)) + "
            "9223372036854775808.0 : "
            "(double)(int64_t)(num))");
    }

    /* True and false. */
    chaz_ConfWriter_append_conf(
        "#ifndef true\n"
        "  #define true 1\n"
        "#endif\n"
        "#ifndef false\n"
        "  #define false 0\n"
        "#endif\n"
    );

    chaz_ConfWriter_end_module();
}

static int
chaz_Integers_machine_is_big_endian(void) {
    long one = 1;
    return !(*((char*)(&one)));
}


