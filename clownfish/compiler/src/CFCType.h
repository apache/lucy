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

/** Clownfish::CFC::Model::Type - A variable's type.
 */

#ifndef H_CFCTYPE
#define H_CFCTYPE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCType CFCType;
struct CFCClass;
struct CFCParcel;

#define CFCTYPE_CONST       0x00000001
#define CFCTYPE_NULLABLE    0x00000002
#define CFCTYPE_VOID        0x00000004
#define CFCTYPE_INCREMENTED 0x00000008
#define CFCTYPE_DECREMENTED 0x00000010
#define CFCTYPE_OBJECT      0x00000020
#define CFCTYPE_PRIMITIVE   0x00000040
#define CFCTYPE_INTEGER     0x00000080
#define CFCTYPE_FLOATING    0x00000100
#define CFCTYPE_STRING_TYPE 0x00000200
#define CFCTYPE_VA_LIST     0x00000400
#define CFCTYPE_ARBITRARY   0x00000800
#define CFCTYPE_COMPOSITE   0x00001000

/** Generic constructor.
 *
 * @param flags Flags which apply to the Type.  Supplying incompatible flags
 * will trigger an error.
 * @param parcel A Clownfish::CFC::Model::Parcel.
 * @param specifier The C name for the type, not including any indirection or
 * array subscripts.
 * @param indirection integer indicating level of indirection. Example: the C
 * type "float**" has a specifier of "float" and indirection 2.
 */
CFCType*
CFCType_new(int flags, struct CFCParcel *parcel, const char *specifier,
            int indirection);

CFCType*
CFCType_init(CFCType *self, int flags, struct CFCParcel *parcel,
             const char *specifier, int indirection);

/** Return a Type representing an integer primitive.
 *
 * Support is limited to a subset of the standard C integer types:
 *
 *     int8_t
 *     int16_t
 *     int32_t
 *     int64_t
 *     uint8_t
 *     uint16_t
 *     uint32_t
 *     uint64_t
 *     char
 *     short
 *     int
 *     long
 *     size_t
 *
 * Many others are not supported: "signed" or "unsigned" anything, "long
 * long", "ptrdiff_t", "off_t", etc.
 *
 * The following Charmonizer typedefs are supported:
 *
 *     bool
 *
 * @param flags Allowed flags: CONST, INTEGER, PRIMITIVE.
 * @param specifier Must match one of the supported types.
 */
CFCType*
CFCType_new_integer(int flags, const char *specifier);

/** Return a Type representing a floating point primitive.
 *
 * @param flags Allowed flags: CONST, FLOATING, PRIMITIVE.
 * @param specifier Must be either 'float' or 'double'.
 */
CFCType*
CFCType_new_float(int flags, const char *specifier);

/** Create a Type representing an object.
 *
 * The supplied <code>specifier</code> must match the last component of the
 * class name -- i.e. for the class "Crustacean::Lobster" it must be
 * "Lobster".
 *
 * The Parcel's prefix will be prepended to the specifier by new_object().
 *
 * @param flags Allowed flags: OBJECT, STRING_TYPE, CONST, NULLABLE,
 * INCREMENTED, DECREMENTED.
 * @param parcel A Clownfish::CFC::Model::Parcel.
 * @param specifier Required.  Must follow the rules for
 * Clownfish::CFC::Model::Class class name components.
 * @param indirection Level of indirection.  Must be 1 if supplied.
 */
CFCType*
CFCType_new_object(int flags, struct CFCParcel *parcel, const char *specifier,
                   int indirection);

/** Constructor for a composite type which is made up of repetitions of a
 * single, uniform subtype.
 *
 * @param flags Allowed flags: COMPOSITE, NULLABLE
 * @param child The Clownfish::CFC::Model::Type which the composite is
 * comprised of.
 * @param indirection integer indicating level of indirection.  Example: the C
 * type "float**" has indirection 2.
 * @param array A string describing an array postfix.
 */
CFCType*
CFCType_new_composite(int flags, CFCType *child, int indirection,
                      const char *array);

/** Return a Clownfish::CFC::Model::Type representing a the 'void' keyword in
 * C.  It can be used either for a void return type, or in conjuction with
 * with new_composite() to support the <code>void*</code> opaque pointer type.
 *
 * @param is_const Should be true if the type is const.  (Useful in the
 * context of <code>const void*</code>).
 */
CFCType*
CFCType_new_void(int is_const);

/** Create a Type representing C's va_list, from stdarg.h.
 */
CFCType*
CFCType_new_va_list(void);

/** "Arbitrary" types are a hack that spares us from having to support C types
 * with complex declaration syntaxes -- such as unions, structs, enums, or
 * function pointers -- from within Clownfish itself.
 *
 * The only constraint is that the <code>specifier</code> must end in "_t".
 * This allows us to create complex types in a C header file...
 *
 *    typedef union { float f; int i; } floatint_t;
 *
 * ... pound-include the C header, then use the resulting typedef in a
 * Clownfish header file and have it parse as an "arbitrary" type.
 *
 *    floatint_t floatint;
 *
 * If <code>parcel</code> is supplied and <code>specifier</code> begins with a
 * capital letter, the Parcel's prefix will be prepended to the specifier:
 *
 *    foo_t         -> foo_t                # no prefix prepending
 *    Lobster_foo_t -> crust_Lobster_foo_t  # prefix prepended
 *
 * @param specifier The name of the type, which must end in "_t".
 * @param parcel A Clownfish::CFC::Model::Parcel.
 */
CFCType*
CFCType_new_arbitrary(struct CFCParcel *parcel, const char *specifier);

/** Find the actual class of an object variable without prefix.
 */
void
CFCType_resolve(CFCType *self, struct CFCClass **classes);

void
CFCType_destroy(CFCType *self);

/** Returns true if two Clownfish::CFC::Model::Type objects are equivalent.
 */
int
CFCType_equals(CFCType *self, CFCType *other);

/** Weak checking of type which allows for covariant return types.  Calling
 * this method on anything other than an object type is an error.
 */
int
CFCType_similar(CFCType *self, CFCType *other);

void
CFCType_set_specifier(CFCType *self, const char *specifier);

const char*
CFCType_get_specifier(CFCType *self);

/** Return the name of the VTable variable which corresponds to the object
 * type.  Returns NULL for non-object types.
 */
const char*
CFCType_get_vtable_var(CFCType *self);

int
CFCType_get_indirection(CFCType *self);

/* Return the parcel in which the Type is used. Note that for class types,
 * this is not neccessarily the parcel where class is defined.
 */
struct CFCParcel*
CFCType_get_parcel(CFCType *self);

/** Return the C representation of the type.
 */
const char*
CFCType_to_c(CFCType *self);

size_t
CFCType_get_width(CFCType *self);

const char*
CFCType_get_array(CFCType *self);

int
CFCType_const(CFCType *self);

void
CFCType_set_nullable(CFCType *self, int nullable);

int
CFCType_nullable(CFCType *self);

/** Returns true if the Type is incremented.  Only applicable to object Types.
 */
int
CFCType_incremented(CFCType *self);

/** Returns true if the Type is decremented.  Only applicable to object Types.
 */
int
CFCType_decremented(CFCType *self);

int
CFCType_is_void(CFCType *self);

int
CFCType_is_object(CFCType *self);

int
CFCType_is_primitive(CFCType *self);

int
CFCType_is_integer(CFCType *self);

int
CFCType_is_floating(CFCType *self);

/** Returns true if $type represents a Clownfish type which holds unicode
 * strings.
 */
int
CFCType_is_string_type(CFCType *self);

int
CFCType_is_va_list(CFCType *self);

int
CFCType_is_arbitrary(CFCType *self);

int
CFCType_is_composite(CFCType *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCTYPE */

