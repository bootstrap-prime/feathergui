/* Feather - Lightweight GUI Abstraction Layer
   Copyright �2015 Black Sphere Studios

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/


#ifndef __FEATHER_GUI_H__
#define __FEATHER_GUI_H__

#include <assert.h>
#include <string.h> // memcpy,memset
#include <malloc.h> 
#include "bss_compiler.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef float FREL; // Change this to double for double precision (why on earth you would need that for a GUI, however, is beyond me)
typedef float FABS; // We seperate both of these types because then you don't have to guess if a float is relative or absolute
typedef unsigned int FG_UINT; 
typedef unsigned int fgFlag;

#define FGUI_VERSION_MAJOR 0
#define FGUI_VERSION_MINOR 1
#define FGUI_VERSION_REVISION 0
#define FG_FASTCALL BSS_COMPILER_FASTCALL
#define FG_EXTERN extern BSS_COMPILER_DLLEXPORT

// A unified coordinate specifies things in terms of absolute and relative positions.
typedef struct {
  FABS abs; // Absolute coordinates are added to relative coordinates, which specify a point from a linear interpolation of the parent's dimensions
  FREL rel;
} Coord;

#define T_SETBIT(w,b,f) (((w) & (~(b))) | ((-(char)f) & (b)))
#define MAKE_VEC(_T,_N) typedef struct { _T x; _T y; } _N

MAKE_VEC(FREL,RelVec);
MAKE_VEC(FABS,AbsVec);
// A coordinate vector specifies a point by unified coordinates
MAKE_VEC(Coord,CVec);

#define MAKE_RECT(_T,_T2,_N) typedef struct { \
  union { \
    struct { \
      _T left; \
      _T top; \
      _T right; \
      _T bottom; \
    }; \
    struct { \
      _T2 topleft; \
      _T2 bottomright; \
    }; \
  }; \
} _N

MAKE_RECT(FREL,RelVec,RelRect);
MAKE_RECT(FABS,AbsVec,AbsRect);
// A coordinate rect specifies its topleft and bottomright corners in terms of coordinate vectors.
MAKE_RECT(Coord,CVec,CRect);

static BSS_FORCEINLINE FG_UINT FG_FASTCALL fbnext(FG_UINT in)
{
  return in + 1 + (in>>1) + (in>>3) - (in>>7);
}
static BSS_FORCEINLINE FABS FG_FASTCALL lerp(FABS a, FABS b, FREL amt)
{
	return a+((FABS)((b-a)*amt));
}

typedef struct __VECTOR {
  void* p;
  FG_UINT s; // This is the total size of the array in BYTES.
  FG_UINT l; // This is how much of the array is being used in ELEMENTS.
} fgVector;

FG_EXTERN void FG_FASTCALL fgVector_Init(fgVector* self); // Zeros everything
FG_EXTERN void FG_FASTCALL fgVector_Destroy(fgVector* self);
FG_EXTERN void FG_FASTCALL fgVector_SetSize(fgVector* self, FG_UINT length, FG_UINT size); // Grows or shrinks the array. self.l is shrunk if necessary.
FG_EXTERN void FG_FASTCALL fgVector_CheckSize(fgVector* self, FG_UINT size); // Ensures the vector is large enough to hold at least one more item.
FG_EXTERN void FG_FASTCALL fgVector_Remove(fgVector* self, FG_UINT index, FG_UINT size); // Removes element at the given index.

#define fgVector_Add(self, item, TYPE) fgVector_CheckSize(&self,sizeof(TYPE)); ((TYPE*)self.p)[self.l++]=item;
#define fgVector_Insert(self, item, index, TYPE) fgVector_CheckSize(&self,sizeof(TYPE)); if(self.l-index>0) { memmove(((TYPE*)self.p)+(index+1),((TYPE*)self.p)+index,((self.l++)-index)*sizeof(TYPE)); } ((TYPE*)self.p)[index]=item;
#define fgVector_Get(self, index, TYPE) ((TYPE*)(self).p)[index]
#define fgVector_GetP(self, index, TYPE) (((TYPE*)(self).p)+(index))

typedef struct {
  CRect area;
  FABS rotation;
  CVec center;
} fgElement;

extern const fgElement fgElement_DEFAULT;

enum FG_MSGTYPE
{
  // fgChild
  FG_MOVE, // Passed when any change is made to an element. 1: propagating up, 2: x-axis resize, 4: y-axis resize, 8: x-axis move, 16: y-axis move, 32: zorder change
  FG_SETALPHA, // Used so an entire widget can be made to fade in or out. (Support is not guaranteed)
  FG_SETAREA,
  FG_SETELEMENT,
  FG_SETFLAG, // Send in the flag in the first int arg, then set it to on or off (1 or 0) in the second argument
  FG_SETFLAGS, // Sets all the flags to the first int arg.
  FG_SETMARGIN,
  FG_SETPADDING,
  FG_SETORDER, // Sets the order of a window in the otherint argument
  FG_SETPARENT,
  FG_ADDCHILD, // Pass an FG_Msg with this type and set the other pointer to the child that should be added.
  FG_REMOVECHILD, // Verifies child's parent is this, then sets the child's parent to NULL.
  FG_LAYOUTRESIZE, // Called when the element is resized
  FG_LAYOUTADD, // Called when any child is added that needs to have the layout applied to it.
  FG_LAYOUTREMOVE, // Called when any child is removed that needs to have the layout applied to it.
  FG_LAYOUTMOVE, // Called when any child is moved so the layout can adjust as necessary.
  FG_LAYOUTREORDER, // Called when any child is reordered
  FG_LAYOUTLOAD, // Loads a layout passed in the first pointer with an optional custom class name resolution function passed into the second pointer of type fgChild* (*)(const char*, fgElement*, fgFlag)
  FG_DRAW,
  FG_CLONE, // Clones the fgChild
  FG_SETSKIN,
  FG_GETSKIN,
  FG_SETSTYLE,
  FG_GETCLASSNAME, // Returns a unique string identifier for the class
  // fgWindow
  FG_MOUSEDOWN,
  //FG_MOUSEDBLCLICK,
  FG_MOUSEUP, // Sent to focused window
  FG_MOUSEON,
  FG_MOUSEOFF,
  FG_MOUSEMOVE, // Sent to focused window
  FG_MOUSESCROLL, // Sent to focused window
  FG_KEYUP,
  FG_KEYDOWN,
  FG_KEYCHAR, //Passed in conjunction with keydown/up to differentiate a typed character from other keys.
  FG_JOYBUTTONDOWN,
  FG_JOYBUTTONUP,
  FG_JOYAXIS,
  FG_GOTFOCUS,
  FG_LOSTFOCUS,
  FG_SETNAME, // Sets the unique name for this object for skin collection mapping. Can be null.
  FG_GETNAME, // May return a unique string for this object, or will return NULL.
  // fgButton and others
  FG_NUETRAL, // Sent when a button or other hover-enabled control switches to it's nuetral state
  FG_HOVER, // Sent when a hover-enabled control switches to its hover state
  FG_ACTIVE, // Sent when a hover-enabled control switches to its active state
  FG_ACTION, // Sent when a hover-enabled control recieves a valid click event (a MOUSEUP inside the control while it has focus)
  // fgList, fgMenu, etc.
  FG_SETCOLUMNS, // If the second pointer is 0, the number of columns is set to the first int. Otherwise, the first int is treated as a column index number, which has it's width set to the Coord pointer to by the second pointer
  FG_GETITEM, // If the control has columns, the column number is specified in the high-order word
  FG_GETROW,
  FG_ADDITEM, // Used for anything involving items (menus, lists, etc)
  FG_REMOVEITEM,
  // fgResource or fgText
  FG_SETRESOURCE,
  FG_SETUV,
  FG_SETCOLOR,
  FG_SETFONT,
  FG_SETFONTCOLOR, // split from SETCOLOR so it can be propagated down seperately from setting image colors
  FG_SETTEXT,
  FG_GETRESOURCE,
  FG_GETUV,
  FG_GETCOLOR,
  FG_GETFONT,
  FG_GETFONTCOLOR,
  FG_GETTEXT,
  FG_CUSTOMEVENT
};

enum FG_MOUSEBUTTON // Used in FG_Msg.button and FG_Msg.allbtn
{
  FG_MOUSELBUTTON=1,
  FG_MOUSERBUTTON=2,
  FG_MOUSEMBUTTON=4,
  FG_MOUSEXBUTTON1=8,
  FG_MOUSEXBUTTON2=16,
};

// General message structure which contains the message type and then various kinds of information depending on the type.
typedef struct __FG_MSG {
  union {
    struct { int x; int y; unsigned char button; unsigned char allbtn; }; // Mouse events
    struct { short scrolldelta; }; // Mouse scroll
    struct {  // Keys
        int keychar; //Only used by KEYCHAR, represents a utf32 character
        unsigned char keycode; //only used by KEYDOWN/KEYUP, represents an actual keycode, not a character
        char keydown;
        char sigkeys;
    };
    struct { float joyvalue; short joyaxis; }; // JOYAXIS
    struct { char joydown; short joybutton; }; // JOYBUTTON
    struct { void* other; size_t otheraux; }; // Used by any generic messages (FG_SETPARENT, etc.)
    struct { void* other1; void* other2; }; // Used by anything requiring 2 pointers, possibly for a return value.
    struct { ptrdiff_t otherint; ptrdiff_t otherintaux; }; // Used by any generic message that want an int (FG_SETORDER, etc.)
  };
  unsigned char type;
} FG_Msg;


FG_EXTERN AbsVec FG_FASTCALL ResolveVec(const CVec* v, const AbsRect* last);
FG_EXTERN char FG_FASTCALL CompareAbsRects(const AbsRect* l, const AbsRect* r); // Returns 0 if both are the same or a difference bitset otherwise.
FG_EXTERN char FG_FASTCALL CompareCRects(const CRect* l, const CRect* r); // Returns 0 if both are the same or a difference bitset otherwise.
FG_EXTERN char FG_FASTCALL CompareElements(const fgElement* l, const fgElement* r);
FG_EXTERN void FG_FASTCALL MoveCRect(AbsVec v, CRect* r);
FG_EXTERN char FG_FASTCALL HitAbsRect(const AbsRect* r, FABS x, FABS y);
//FG_EXTERN void FG_FASTCALL ToIntAbsRect(const AbsRect* r, int target[static 4]);
FG_EXTERN void FG_FASTCALL ToIntAbsRect(const AbsRect* r, int target[4]);
FG_EXTERN void FG_FASTCALL ToLongAbsRect(const AbsRect* r, long target[4]);
FG_EXTERN char FG_FASTCALL MsgHitAbsRect(const FG_Msg* msg, const AbsRect* r);

#ifdef  __cplusplus
}
#endif

#endif
