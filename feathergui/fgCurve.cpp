// Copyright �2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feathergui.h"

#include "fgCurve.h"
#include "feathercpp.h"

using namespace bss_util;

fgElement* FG_FASTCALL fgCurve_Create(const AbsVec* points, size_t npoints, unsigned int color, fgElement* BSS_RESTRICT parent, fgElement* BSS_RESTRICT next, const char* name, fgFlag flags, const fgTransform* transform)
{
  fgElement* r = fgCreate("Curve", parent, next, name, flags, transform);
  if(color) fgIntMessage(r, FG_SETCOLOR, color, 0);
  if(points && npoints > 0) _sendmsg<FG_SETITEM, const void*, size_t>(r, points, npoints);
  return r;
}

void FG_FASTCALL fgCurve_Init(fgCurve* self, fgElement* BSS_RESTRICT parent, fgElement* BSS_RESTRICT next, const char* name, fgFlag flags, const fgTransform* transform)
{
  fgElement_InternalSetup(&self->element, parent, next, name, flags, transform, (fgDestroy)&fgCurve_Destroy, (fgMessage)&fgCurve_Message);
}

void FG_FASTCALL fgCurve_Destroy(fgCurve* self)
{
  reinterpret_cast<cDynArray<AbsVec>&>(self->points).~cDynArray();
  reinterpret_cast<cDynArray<AbsVec>&>(self->cache).~cDynArray();
  fgElement_Destroy(&self->element);
}

// p must point to an array of at least 4 points. We do not check that in the function as it is internal only.
void FG_FASTCALL fgCurve_GenCubic(fgCurve* self, AbsVec* p)
{
  AbsVec s1[4];
  s1[0].x = p[0].x;
  s1[0].y = p[0].y;
  s1[1].x = (p[0].x + p[1].x) / 2.0f;
  s1[1].y = (p[0].y + p[1].y) / 2.0f;

  AbsVec s2[4];
  s2[3].x = p[3].x;
  s2[3].y = p[3].y;
  s2[2].x = (p[2].x + p[3].x) / 2.0f;
  s2[2].y = (p[2].y + p[3].y) / 2.0f;

  AbsVec p12 = { (p[1].x + p[2].x) / 2.0f, (p[1].y + p[2].y) / 2.0f };

  s2[1].x = (p12.x + s2[2].x) / 2.0f;
  s2[1].y = (p12.y + s2[2].y) / 2.0f;
  s1[2].x = (s1[0].x + p12.x) / 2.0f;
  s1[2].y = (s1[0].y + p12.y) / 2.0f;
  s1[3].x = s2[0].x = (s1[2].x + s2[1].x) / 2.0f;
  s1[3].y = s2[0].x = (s1[2].y + s2[1].y) / 2.0f;

  double dx = p[3].x - p[0].x;
  double dy = p[3].y - p[0].y;

  double d2 = fabs(((p[1].x - p[3].x) * dy - (p[1].y - p[3].y) * dx));
  double d3 = fabs(((p[2].x - p[3].x) * dy - (p[2].y - p[3].y) * dx));

  if((d2 + d3)*(d2 + d3) < self->factor * (dx*dx + dy*dy))
    reinterpret_cast<cDynArray<AbsVec>&>(self->cache).Add(s1[3]);
  else
  {
    fgCurve_GenCubic(self, s1);
    fgCurve_GenCubic(self, s2);
  }
}

size_t FG_FASTCALL fgCurve_Message(fgCurve* self, const FG_Msg* msg)
{
  assert(self != 0 && msg != 0);
  switch(msg->type)
  {
  case FG_CONSTRUCT:
    fgElement_Message(&self->element, msg);
    memset(&self->points, 0, sizeof(fgVectorPoint));
    memset(&self->cache, 0, sizeof(fgVectorPoint));
    self->color.color = 0;
    self->factor = 0.1f;
    return FG_ACCEPT;
  case FG_SETCOLOR:
    self->color.color = msg->otherint;
    fgDirtyElement(&self->element);
    break;
  case FG_GETCOLOR:
    return self->color.color;
  case FG_SETSTATE:
    self->factor = msg->otherf;
    break;
  case FG_GETSTATE:
    return *(size_t*)&self->factor;
  case FG_ADDITEM:
    if(msg->otheraux >= self->points.l)
      reinterpret_cast<cDynArray<AbsVec>&>(self->points).Add(*(AbsVec*)msg->other);
    else
      reinterpret_cast<cDynArray<AbsVec>&>(self->points).Insert(*(AbsVec*)msg->other, msg->otheraux);
    self->cache.l = 0;
    break;
  case FG_GETITEM:
    if(msg->otherint < 0 || msg->otherint >= self->points.l)
      return 0;
    return (size_t)(self->points.p + msg->otherint);
  case FG_REMOVEITEM:
    reinterpret_cast<cDynArray<AbsVec>&>(self->points).Remove(msg->otherint);
    self->cache.l = 0;
    break;
  case FG_SETITEM:
    reinterpret_cast<cDynArray<AbsVec>&>(self->points).Set((AbsVec*)msg->other, msg->otheraux);
    self->cache.l = 0;
    break;
  case FG_DRAW:
  {
    if(msg->subtype & 1) break;
    AbsRect area = *(AbsRect*)msg->other;
    float scale = (!msg->otheraux || !fgroot_instance->dpi) ? 1.0 : (fgroot_instance->dpi / (float)msg->otheraux);
    area.left *= scale;
    area.top *= scale;
    area.right *= scale;
    area.bottom *= scale;
    AbsVec center = ResolveVec(&self->element.transform.center, &area);
    AbsVec scalevec = AbsVec { 1.0f, 1.0f };

    if(!(self->element.flags&FGCURVE_CURVEMASK))
    {
      if(self->points.l > 0)
        fgDrawLines(self->points.p, self->points.l, self->color.color, &area.topleft, &scalevec, self->element.transform.rotation, &center);
      else
      {
        AbsVec p[2] = { {0,0}, { area.right - area.left - 1, area.bottom - area.top - 1} };
        fgDrawLines(p, 2, self->color.color, &area.topleft, &scalevec, self->element.transform.rotation, &center);
      }
    } 
    else
    {
      if(!self->cache.l)
      {
        switch(self->element.flags&FGCURVE_CURVEMASK)
        {
        case FGCURVE_QUADRATIC:
          break;
        case FGCURVE_CUBIC:
          for(size_t i = 4; i <= self->cache.l; i += 4)
          {
            reinterpret_cast<cDynArray<AbsVec>&>(self->cache).Add(self->cache.p[i - 4]);
            fgCurve_GenCubic(self, self->cache.p + i - 4);
            reinterpret_cast<cDynArray<AbsVec>&>(self->cache).Add(self->cache.p[i - 1]);
          }
          break;
        case FGCURVE_BSPLINE:
          break;
        }
      }

      fgDrawLines(self->cache.p, self->cache.l, self->color.color, &area.topleft, &scalevec, self->element.transform.rotation, &center);
    }
  }
  break;
  case FG_GETCLASSNAME:
    return (size_t)"Curve";
  }
  return fgElement_Message(&self->element, msg);
}