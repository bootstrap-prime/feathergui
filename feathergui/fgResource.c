// Copyright �2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feathergui.h"

#include "fgResource.h"
#include <stdio.h>

void FG_FASTCALL fgResource_Init(fgResource* self, void* res, const CRect* uv, unsigned int color, fgFlag flags, fgChild* parent, const fgElement* element)
{
  memset(self, 0, sizeof(fgResource));
  fgChild_Init(&self->element, flags, parent, element);
  fgChild_IntMessage((fgChild*)self, FG_SETCOLOR, color, 0);
  fgChild_VoidMessage((fgChild*)self, FG_SETUV, (void*)uv);
  fgChild_VoidMessage((fgChild*)self, FG_SETRESOURCE, res);
}
void FG_FASTCALL fgResource_Destroy(fgResource* self)
{
  if(self->res) fgDestroyResource(self->res);
  fgChild_Destroy(&self->element);
}
size_t FG_FASTCALL fgResource_Message(fgResource* self, const FG_Msg* msg)
{
  assert(self != 0 && msg != 0);
  switch(msg->type)
  {
  case FG_SETUV:
    if(msg->other)
      self->uv = *((CRect*)msg->other);
    fgResource_Recalc(self);
    return 0;
  case FG_SETRESOURCE:
    if(self->res) fgDestroyResource(self->res);
    self->res = 0;
    if(msg->other) self->res = fgCloneResource(msg->other);
    fgResource_Recalc(self);
    break;
  case FG_SETCOLOR:
    self->color = msg->otherint;
    break;
  case FG_GETUV:
    return (size_t)(&self->uv);
  case FG_GETRESOURCE:
    return (size_t)self->res;
  case FG_GETCOLOR:
    return self->color;
  case FG_MOVE:
    if(!(msg->otheraux & 1) && (msg->otheraux&(2 | 4)))
      fgResource_Recalc(self);
    break;
  case FG_DRAW:
    fgDrawResource(self->res, &self->uv, self->color, (AbsRect*)msg->other, self->element.flags);
    break;
  case FG_GETCLASSNAME:
    return (size_t)"fgResource";
  }
  return fgChild_Message(&self->element, msg);
}

void* FG_FASTCALL fgCreateResourceFile(fgFlag flags, const char* file)
{
  FILE* f = fopen(file, "rb");
  if(!f) return 0;
  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  fseek(f, 0, SEEK_SET);
  char* buf = malloc(len);
  fread(buf, 1, len, f);
  fclose(f);
  void* r = fgCreateResource(flags, buf, len);
  free(buf);
  return r;
}

void FG_FASTCALL fgResource_Recalc(fgResource* self)
{
  if(self->res && (self->element.flags&(FGCHILD_EXPANDX | FGCHILD_EXPANDY)))
  {
    AbsRect area;
    ResolveRect((fgChild*)self, &area);
    fgResourceSize(self->res, &self->uv, &area, self->element.flags);
    CRect adjust = self->element.element.area;
    if(self->element.flags&FGCHILD_EXPANDX)
    {
      adjust.left.abs = area.left;
      adjust.right.abs = area.right;
    }
    if(self->element.flags&FGCHILD_EXPANDY)
    {
      adjust.top.abs = area.left;
      adjust.bottom.abs = area.right;
    }
    fgChild_VoidMessage((fgChild*)self, FG_SETAREA, &adjust);
  }
}