" Licensed to the Apache Software Foundation (ASF) under one or more
" contributor license agreements.  See the NOTICE file distributed with
" this work for additional information regarding copyright ownership.
" The ASF licenses this file to You under the Apache License, Version 2.0
" (the "License"); you may not use this file except in compliance with
" the License.  You may obtain a copy of the License at
"
"     http://www.apache.org/licenses/LICENSE-2.0
"
" Unless required by applicable law or agreed to in writing, software
" distributed under the License is distributed on an "AS IS" BASIS,
" WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
" See the License for the specific language governing permissions and
" limitations under the License.
"
" Vim syntax file
" Language:     Clownfish
" Maintainer:   Apache Lucy Developers
" URL:          http://lucy.apache.org

if exists("b:current_syntax")
    finish
endif

syn keyword cfishType int8_t  int16_t  int32_t  int64_t  bool_t
syn keyword cfishType uint8_t uint16_t uint32_t uint64_t
syn keyword cfishType size_t
syn keyword cfishType float double
syn keyword cfishType char short int long
syn keyword cfishType va_list
syn keyword cfishType void
syn keyword cfishType class
syn keyword cfishType parcel

syn keyword cfishAttribute inherits nickname

syn keyword cfishModifier public private
syn keyword cfishModifier abstract inert const final
syn keyword cfishModifier incremented decremented nullable

syn keyword cfishConstant NULL true false

syn match  cfishNumber  "\<\d\+\>"
syn match  cfishNumber  "\<0[xX]\x\+\>"
syn match  cfishNumber  "\<\d\+\.\d\+\>"

syn region cfishString start=+"+ skip=+\\\\\|\\"+ end=+"+

syn keyword cfishTodo TODO XXX
syn region cfishComment start="/\*" end="\*/" contains=cfishTodo
syn match cfishLineComment "//.*" contains=cfishTodo
syn region cfishDocuComment start="/\*\*" end="\*/" contains=cfishDocuCommentBrief,cfishDocuCommentTags,cfishTodo
syn region cfishDocuCommentBrief contained matchgroup=cfishDocuComment start="/\*\*" matchgroup=cfishDocuCommentBrief keepend end="\.$" end="\.\s\+" end="\*/"
syn match cfishDocuCommentTags contained "@param\s\+\S\+" contains=cfishDocuCommentParam
syn match cfishDocuCommentParam contained "\s\S\+"

hi def link cfishType               Type
hi def link cfishConstant           Constant
hi def link cfishModifier           Operator
hi def link cfishExposure           Operator
hi def link cfishAttribute          Statement
hi def link cfishNumber             Number
hi def link cfishString             String
hi def link cfishTodo               Todo
hi def link cfishDocuCommentBrief   SpecialComment
hi def link cfishDocuComment        Comment
hi def link cfishComment            Comment
hi def link cfishLineComment        Comment
hi def link cfishDocuCommentTags    Special
hi def link cfishDocuCommentParam   Function

let b:current_syntax = "clownfish"

