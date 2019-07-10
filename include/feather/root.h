// Copyright (c)2019 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feather.h"

#ifndef FG__ROOT_H
#define FG__ROOT_H

#include "backend.h"
#include "outline.h"
#include "data.h"
#include <stdio.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct
{
  float x, y; // Last known position of the mouse
  unsigned char buttons; // Last known configuration of mouse buttons
  unsigned char state;
  struct FG__DOCUMENT_NODE* capture;
} fgPointerState;

typedef struct
{
  size_t sz;
  fgBehaviorFunction fn;
  fgResolver resolver;
} fgBehaviorDefinition;

typedef struct
{
  size_t sz;
  fgLayout fn;
  fgResolver resolver;
} fgLayoutDefinition;

typedef struct
{
  void* obj;
  void(*f)(struct FG__ROOT*, void*, const void*, unsigned int, unsigned int);
} fgDataListener;

KHASH_DECLARE(component, const char*, fgBehaviorDefinition);
KHASH_DECLARE(layout, const char*, fgLayoutDefinition);
KHASH_DECLARE(data, const void*, fgDataListener);

typedef struct FG__ROOT
{
  fgBackend* backend;
  FILE* log;
  FG_LOGLEVEL level;
  int keys[256 / 32];
  fgPointerState* pointers;
  int n_pointers;
  fgDocumentNode* keyfocus;
  fgGetField getfield;
  fgGetIndex getindex;
  fgSetData setdata;
  fgSetRange setrange;
  void* data; // Root data pointer
  kh_function_t* operators; // Registered functions to use for evaluating expressions
  kh_layout_t* layouts;
  kh_component_t* components;
  kh_data_t* listeners;
} fgRoot;

FG_COMPILER_DLLEXPORT void fgLog(fgRoot* root, FG_LOGLEVEL level, const char* format, ...);
FG_COMPILER_DLLEXPORT fgMessageResult fgInject(fgRoot* root, struct FG__DOCUMENT_NODE* node, fgMessage* msg); // node can be NULL depending on message type
FG_COMPILER_DLLEXPORT bool fgGetKey(fgRoot* root, unsigned char key);
FG_COMPILER_DLLEXPORT fgDataField fgGetData(struct FG__ROOT* root, void* data, const char* accessor);
FG_COMPILER_DLLEXPORT void fgUpdateData(struct FG__ROOT* root, const void* obj);
FG_COMPILER_DLLEXPORT void fgUpdateDataRange(struct FG__ROOT* root, const void* obj, unsigned int offset, unsigned int count);
FG_COMPILER_DLLEXPORT void fgInitialize(fgRoot* root);
FG_COMPILER_DLLEXPORT void fgTerminate(fgRoot* root);
FG_COMPILER_DLLEXPORT fgError fgProcessMessages(fgRoot* root);

#ifdef  __cplusplus
}
#endif

#endif