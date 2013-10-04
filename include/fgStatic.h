// Copyright �2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feathergui.h"

#ifndef __FG_RENDERABLE_H__
#define __FG_RENDERABLE_H__

#include "feathergui.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define STATIC_SET_PIECE(n,y,x) if(self->##x) (*self->##x##->message)(self->##x,FG_RSHOW,(void*)(n==y),0)
#define STATIC_SET_ENABLE1(n,a) STATIC_SET_PIECE(n,0,a)
#define STATIC_SET_ENABLE2(n,a,b) STATIC_SET_PIECE(n,1,b); STATIC_SET_ENABLE1(n,a)
#define STATIC_SET_ENABLE3(n,a,b,c) STATIC_SET_PIECE(n,2,c); STATIC_SET_ENABLE2(n,a,b)
#define STATIC_SET_ENABLE4(n,a,b,c,d) STATIC_SET_PIECE(n,3,d); STATIC_SET_ENABLE3(n,a,b,c)
#define STATIC_SET_ENABLE5(n,a,b,c,d,e) STATIC_SET_PIECE(n,4,e); STATIC_SET_ENABLE4(n,a,b,c,d)

// Message types for statics only
enum FG_RENDERMSG
{
  FG_RDRAW,
  FG_RMOVE,
  FG_RSHOW,
  FG_RADDCHILD,
  FG_RREMOVECHILD,
  FG_RSETAREA,
  FG_RSETELEMENT,
  FG_RSETTEXT,
  FG_RSETUV,
  FG_RSETCOLOR,
  FG_RSETFLAGS,
  FG_RSETORDER,
  FG_RSETFONT, //arg should point to a string that contains the name of the font, other should be the size of the font. Optionally, the top 16-bits of other can specify the line-height.
  FG_RCLONE, // Initializes an empty fgStatic passed in as arg as a clone of this static.
  FG_RCUSTOM
};

enum FG_STATICFLAGS
{
  FGSTATIC_CLIP=1, // Causes any type of static to clip its children.
  FGSTATIC_HIDDEN=2, // Hides a static and all its children.
  FGSTATIC_MARKER=(1<<((sizeof(fgFlag)<<3)-1)), // Tells us this is a static and not a window
};

enum FG_TEXTFLAGS
{
  FGTEXT_CHARWRAP=4, // Wraps lines that go past the edge of the container by character
  FGTEXT_WORDWRAP=8, // Wraps lines that go past the edge of the container by word (the definition of a "word" is implementation specific)
  FGTEXT_ELLIPSES=16, // Lines that go past the bounderies of the text object are cut off with an ellipses (...)
  FGTEXT_RTL=32, // Forces right-to-left text rendering.
  FGTEXT_RIGHTALIGN=64,
  FGTEXT_CENTER=128, // Text horizontal centering behaves differently, because it centers each individual line.
  FGTEXT_STRETCH=256, // Stretches the text to fill the area.
};

enum FG_IMAGEFLAGS
{
  FGIMAGE_STRETCHX=4, // Stretches the image instead of tiling it
  FGIMAGE_STRETCHY=8,
};

struct __WINDOW;

// Representation of a static, which is implemented by the GUI implementation
typedef struct __RENDERABLE {
  fgChild element;
  void (FG_FASTCALL *message)(struct __RENDERABLE* self, unsigned char type, void* arg, int other);
  struct __RENDERABLE* (FG_FASTCALL *clone)(struct __RENDERABLE* self);
  struct __WINDOW* parent;
  void* userdata;
} fgStatic;

typedef struct __RENDERABLE_DEF {
  fgElement element;
  int order; // order relative to other windows or statics
  fgFlag flags;
} fgStaticDef;

typedef struct __IMAGE_DEF {
  fgStaticDef def;
  unsigned int color;
  const char* data;
  size_t length; // If length is 0, data is treated as a filepath
  CRect uv;
} fgImageDef;

typedef struct __TEXT_DEF {
  fgStaticDef def;
  unsigned int color;
  const char* text;
  const char* font;
  unsigned short fontsize;
  unsigned short lineheight;
} fgTextDef;

FG_EXTERN void FG_FASTCALL fgStatic_Init(fgStatic* self);
FG_EXTERN void FG_FASTCALL fgStatic_Destroy(fgStatic* self);
FG_EXTERN void FG_FASTCALL fgStatic_Message(fgStatic* self, unsigned char type, void* arg, int other);
FG_EXTERN void FG_FASTCALL fgStatic_SetWindow(fgStatic* self, struct __WINDOW* window);
FG_EXTERN void FG_FASTCALL fgStatic_RemoveParent(fgStatic* self);
FG_EXTERN void FG_FASTCALL fgStatic_NotifyParent(fgStatic* self);
FG_EXTERN void FG_FASTCALL fgStatic_Clone(fgStatic* self, fgStatic* from); // Clones information and all the children from "from" to "self"
FG_EXTERN void FG_FASTCALL fgStatic_SetParent(fgStatic* BSS_RESTRICT self, fgChild* BSS_RESTRICT parent);

FG_EXTERN fgStatic* FG_FASTCALL fgLoadImage(const char* path);
FG_EXTERN fgStatic* FG_FASTCALL fgLoadImageData(const void* data, size_t length);
FG_EXTERN fgStatic* FG_FASTCALL fgLoadImageDef(const fgImageDef* def);
FG_EXTERN fgStatic* FG_FASTCALL fgLoadVector(const char* path);
FG_EXTERN fgStatic* FG_FASTCALL fgLoadVectorData(const void* data, size_t length);
FG_EXTERN fgStatic* FG_FASTCALL fgLoadText(const char* text, fgFlag flags, const char* font, unsigned short fontsize, unsigned short lineheight);
FG_EXTERN fgStatic* FG_FASTCALL fgLoadTextDef(const fgTextDef* def);
FG_EXTERN fgStatic* FG_FASTCALL fgEmptyStatic(fgFlag flags);

// An item represented by both text and an image is extremely common, so we define this helper struct to make things simpler
typedef struct {
  fgStatic* image; // This is the image displayed alongside the item
  const char* text; // This is the item text itself
  const char* auxtext; // This represents either the shortcut key (for menu items) or a tooltip, or it may be ignored.
} fgTriplet;

#ifdef  __cplusplus
}
#endif

#endif
