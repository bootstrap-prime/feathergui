// Copyright �2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feathergui.h"

#include "fgWindow.h"
#include "fgRoot.h"

fgChild* fgFocusedWindow = 0;
fgChild* fgLastHover = 0;
fgChild* fgCaptureWindow = 0;

void FG_FASTCALL fgWindow_Init(fgWindow* BSS_RESTRICT self, fgFlag flags, fgChild* BSS_RESTRICT parent, const fgElement* element)
{ 
  assert(self!=0);
  memset(self,0,sizeof(fgWindow));
  fgChild_Init(&self->element, flags, parent, element);
  self->element.destroy=&fgWindow_Destroy;
  self->element.message=&fgWindow_Message;
  self->element.flags=flags;
  self->tabnext = self->tabprev = self; // This creates an infinite loop of tabbing
  if(element) self->element.element=*element;
}
void FG_FASTCALL fgWindow_Destroy(fgWindow* self)
{
  assert(self!=0);
  if(self->tabprev) self->tabprev->tabnext = self->tabnext;
  if(self->tabnext) self->tabnext->tabprev = self->tabprev;
  self->tabnext = self->tabprev = 0;

  if(self->name) free(self->name);
  fgChild_Destroy(&self->element);
}

void FG_FASTCALL fgWindow_DoHoverCalc(fgWindow* self) // This is a seperate function so some controls can force a focus event to instead activate hover instead.
{
  if(fgLastHover != (fgChild*)self)
  {
    if(fgLastHover != 0)
      fgChild_VoidMessage((fgChild*)fgLastHover, FG_MOUSEOFF, 0);
    fgLastHover = (fgChild*)self;
    fgChild_VoidMessage((fgChild*)self, FG_MOUSEON, 0);
  }
}

size_t FG_FASTCALL fgWindow_Message(fgWindow* self, const FG_Msg* msg)
{
  int otherint = msg->otherint;
  assert(self!=0);
  assert(msg!=0);
  
  switch(msg->type)
  {
  case FG_KEYDOWN:
    if(msg->keycode == FG_KEY_TAB && !(msg->sigkeys&(2|4)))
    {
      fgWindow* target = (msg->sigkeys & 1) ? self->tabprev : self->tabnext;
      if(target != self)
        fgChild_VoidMessage((fgChild*)self, FG_GOTFOCUS, 0);
      return 0;
    }
    break;
  case FG_MOUSEDOWN:
    fgWindow_DoHoverCalc(self);
    if(fgFocusedWindow != (fgChild*)self)
      fgChild_VoidMessage((fgChild*)self, FG_GOTFOCUS, 0);
    //if(msg->button == FG_MOUSERBUTTON && self->contextmenu != 0)
    //  fgChild_VoidMessage((fgChild*)self->contextmenu, FG_GOTFOCUS, 0);
  case FG_MOUSESCROLL:
  case FG_MOUSEMOVE:
    if(msg->type == FG_MOUSEMOVE && fgSingleton()->drag) // Send a dragging message if necessary. Does not initiate a drag for you (this is because some drags are initiated via click and drag, and some are just clicking).
      fgChild_VoidMessage((fgChild*)self, FG_DRAGGING, fgSingleton()->drag);
    return 0;
  case FG_MOUSEUP: 
  { // Any control that gets a MOUSEUP event immediately fires a MOUSEMOVE event at that location, which will force the focus to shift to a different control if the mouseup occured elsewhere.
    FG_Msg m = *msg;
    m.type = FG_MOUSEMOVE;
    fgRoot_Inject(fgSingleton(), &m);
  }
    if(fgSingleton()->drag != 0) // If necessary, send a drop message to the current control we're hovering over.
      fgChild_VoidMessage(fgLastHover, FG_DROP, fgSingleton()->drag);
    return 0;
  case FG_GOTFOCUS:
    if(!fgChild_Message((fgChild*)self, msg)) // checks if we resolved via lastfocus.
      return 0;
    if(fgFocusedWindow) // We do this here so you can disable getting focus by blocking this message without messing things up
      fgChild_VoidMessage((fgChild*)fgFocusedWindow, FG_LOSTFOCUS, self);
    fgFocusedWindow = (fgChild*)self;
    return 0;
  case FG_LOSTFOCUS:
    assert(fgFocusedWindow == (fgChild*)self);
    if(fgFocusedWindow == (fgChild*)self)
    {
      fgFocusedWindow = 0;
      if(self->element.parent)
      {
        if(!msg->other)
          fgChild_VoidMessage(self->element.parent, FG_GOTFOCUS, 0);
        else
          self->element.parent->lastfocus = (fgChild*)self;
      }
    }
    return 0;
  case FG_CLONE:
  {
    fgWindow* hold = msg->other;
    if(!hold)
      hold = malloc(sizeof(fgWindow));
    hold->contextmenu = hold->contextmenu;
    fgChild_VoidMessage((fgChild*)hold, FG_SETNAME, self->name);

    FG_Msg m = *msg;
    m.other = hold;
    return fgChild_Message((fgChild*)self, msg);
  }
    return 0;
  case FG_GETCLASSNAME:
    return (size_t)"fgWindow";
  case FG_SETNAME:
    if(self->name) free(self->name);
    self->name = fgCopyText(msg->other);
    return 0;
  case FG_GETNAME:
    return (size_t)self->name;
  }
  return fgChild_Message((fgChild*)self, msg);
}

size_t FG_FASTCALL fgWindow_HoverProcess(fgWindow* self, const FG_Msg* msg)
{
  assert(self!=0 && msg!=0);
  switch(msg->type)
  {
  case FG_MOUSEON:
    fgChild_VoidMessage((fgChild*)self,FG_HOVER,0);
    break;
  case FG_MOUSEUP:
    if(MsgHitCRect(msg, &self->element)) // We can get a MOUSEUP when the mouse is outside of the control but we DO NOT fire it unless it's actually in the control.
      fgChild_VoidMessage((fgChild*)self, FG_ACTION, 0);
    fgChild_VoidMessage((fgChild*)self, FG_HOVER, 0); // Revert to hover no matter what. The other handler will fire off a mousemove for us that will handle the hover change event.
    if(fgCaptureWindow == (fgChild*)self) // Remove our control hold on mouse messages.
      fgCaptureWindow = 0;
    break;
  case FG_MOUSEOFF: // FG_MOUSEUP falls through
    fgChild_VoidMessage((fgChild*)self,FG_NUETRAL, 0);
    break;
  case FG_MOUSEDOWN:
    if(msg->button == FG_MOUSELBUTTON)
    {
      fgCaptureWindow = (fgChild*)self;
      fgChild_VoidMessage((fgChild*)self, FG_ACTIVE, 0);
    }
    break;
  }
  return fgWindow_Message(self,msg);
}

FG_EXTERN void FG_FASTCALL fgWindow_TabAfter(fgWindow* self, fgWindow* prev)
{
  self->tabnext = prev->tabnext;
  self->tabprev = prev;
  prev->tabnext = self;
}

FG_EXTERN void FG_FASTCALL fgWindow_TabBefore(fgWindow* self, fgWindow* next)
{
  self->tabprev = next->tabprev;
  self->tabnext = next;
  next->tabprev = self;
}