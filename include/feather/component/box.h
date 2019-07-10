// Copyright (c)2019 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feather.h"

#ifndef FG__BOX_H
#define FG__BOX_H

#include "../message.h"

#ifdef  __cplusplus
extern "C" {
#endif

// Contains additional outline information for a "box" behavior function
typedef struct
{
  const fgRect corners;
  fgColor fillColor;
  float border;
  fgColor borderColor;
  float blur;
} fgBoxData;

fgMessageResult fgBoxBehavior(const struct FG__ROOT* root, struct FG__DOCUMENT_NODE* node, const fgMessage* msg);

#ifdef  __cplusplus
}
#endif

#endif