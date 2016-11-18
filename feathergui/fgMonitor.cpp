// Copyright �2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feathergui.h"

#include "fgMonitor.h"
#include "fgRoot.h"
#include "feathercpp.h"

void FG_FASTCALL fgMonitor_Init(fgMonitor* BSS_RESTRICT self, fgFlag flags, fgRoot* BSS_RESTRICT parent, fgMonitor* BSS_RESTRICT prev, const AbsRect* coverage, size_t dpi)
{
  FABS scale = (!dpi || !parent->dpi) ? (FABS)1.0 : (parent->dpi / (FABS)dpi);
  fgTransform transform = { coverage->left*scale, 0, coverage->top*scale, 0, coverage->right*scale, 0, coverage->bottom*scale, 0, 0, 0, 0 };
  self->coverage = *coverage;
  self->dpi = dpi;
  self->mnext = !prev ? parent->monitors : 0;
  if(self->mnext) self->mnext->mprev = self;
  self->mprev = prev;
  if(self->mprev) self->mprev->mnext = self;
  else parent->monitors = self;

  fgElement_InternalSetup(&self->element, *parent, &prev->element, 0, flags, &transform, 0, (fgDestroy)&fgMonitor_Destroy, (fgMessage)&fgMonitor_Message);
}

void FG_FASTCALL fgMonitor_Destroy(fgMonitor* self)
{
  assert(self != 0);
  if(self->mprev) self->mprev->mnext = self->mnext;
  if(self->mnext) self->mnext->mprev = self->mprev;
  self->mnext = self->mprev = 0;

  fgElement_Destroy(&self->element);
}

size_t FG_FASTCALL fgMonitor_Message(fgMonitor* self, const FG_Msg* msg)
{
  switch(msg->type)
  {
  case FG_GETDPI:
    return self->dpi;
  case FG_SETDPI:
  {
    self->dpi = (size_t)msg->otherint;
    float scale = fgroot_instance->dpi / (float)self->dpi;
    CRect area = { self->coverage.left*scale, 0, self->coverage.top*scale, 0, self->coverage.right*scale, 0, self->coverage.bottom*scale, 0 };
    size_t ret = self->element.SetArea(area);
    fgElement_Message(&self->element, msg); // Passes the SETDPI message to all children
    return ret;
  }
  case FG_DRAW: // Override draw call so we never clip, and replace the root DPI with our DPI
  {
    fgElement* hold = self->element.last; // we draw backwards through our list.
    AbsRect curarea;

    while(hold)
    {
      if(!(hold->flags&FGELEMENT_HIDDEN) && hold != fgroot_instance->topmost)
      {
        ResolveRectCache(hold, &curarea, (AbsRect*)msg->other, (hold->flags & FGELEMENT_BACKGROUND) ? 0 : &self->element.padding);
        _sendmsg<FG_DRAW, void*, size_t>(hold, &curarea, self->dpi);
      }
      hold = hold->prev;
    }
    return FG_ACCEPT;
  }
  case FG_SETAREA:
  {
    float scale = fgroot_instance->dpi / (float)self->dpi;
    if(!msg->subtype)
    {
      size_t r = fgElement_Message(&self->element, msg);
      self->coverage = AbsRect { self->element.transform.area.left.abs / scale, self->element.transform.area.top.abs / scale, self->element.transform.area.right.abs / scale, self->element.transform.area.bottom.abs / scale };
      return r;
    }
    if(msg->other != 0)
    {
      self->coverage = *(AbsRect*)msg->other;
      CRect area = { self->coverage.left*scale, 0, self->coverage.top*scale, 0, self->coverage.right*scale, 0, self->coverage.bottom*scale, 0 };
      FG_Msg m = { 0 };
      m.type = FG_SETAREA;
      m.other = &area;
      return fgElement_Message(&self->element, &m);
    }
    return 0;
  }
  case FG_GETCLASSNAME:
    return (size_t)"Monitor";
  }

  return fgElement_Message(&self->element, msg);
}