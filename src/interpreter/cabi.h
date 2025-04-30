// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file cabi.h
 * @brief <TBA>
 * @warning This file contains experimental features
 */
#ifndef VIA_HAS_HEADER_CABI_H
#define VIA_HAS_HEADER_CABI_H

#include <common.h>

#define __cmov(dst, src)     asm volatile("mov %0,%1\n" : : "r"(src), "r"(dst));
#define __cmovrdst(dst, src) asm volatile("mov %0, %%" #dst "\n" : : "r"(src) : #dst)
#define __cmovrsrc(dst, src) asm volatile("mov %%" #src ",%0\n" : : "r"(dst) : #src)
#define __cmovrr(dst, src)   asm volatile("mov %%" #src ", %%" #dst "\n" : : : #dst, #src)

#define __ccall(addr)
#define __ccallr(reg) asm volatile("call *%%" #reg "\n" : : : #reg)

#endif
