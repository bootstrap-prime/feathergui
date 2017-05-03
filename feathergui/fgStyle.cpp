// Copyright �2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feathergui.h"

#include "bss-util/khash.h"
#include "bss-util/bss_util.h"
#include "fgStyle.h"
#include "feathercpp.h"

KHASH_INIT(fgStyles, const char*, FG_UINT, 1, kh_str_hash_funcins, kh_str_hash_insequal);
fgStyleStatic fgStyleStatic::Instance;

fgStyleStatic::fgStyleStatic() { h = kh_init_fgStyles(); }
fgStyleStatic::~fgStyleStatic() { Clear(); }
void fgStyleStatic::Clear()
{
  if(h)
  {
    for(khiter_t i = 0; i < h->n_buckets; ++i)
    {
      if(kh_exist(h, i))
        fgFreeText(kh_key(h, i), __FILE__, __LINE__);
    }
    kh_destroy_fgStyles(h);
    h = 0;
  }
}

void fgStyle_Init(fgStyle* self)
{
  memset(self, 0, sizeof(fgStyle));
}

void fgStyle_Destroy(fgStyle* self)
{
  while(self->styles)
    fgStyle_RemoveStyleMsg(self, self->styles);
}

fgStyleMsg* fgStyle_AddStyleMsg(fgStyle* self, const FG_Msg* msg, unsigned int arg1size, unsigned int arg2size)
{
  unsigned int sz = sizeof(fgStyleMsg) + arg1size + arg2size;
  assert(!(sz & 0xC0000000));
  fgStyleMsg* r = (fgStyleMsg*)fgmalloc<char>(sz, __FILE__, __LINE__);
  MEMCPY(&r->msg, sizeof(FG_Msg), msg, sizeof(FG_Msg));

  if(arg1size > 0)
  {
    void* arg1 = r->msg.p;
    r->msg.p = r + 1;
    memcpy(r->msg.p, arg1, arg1size);
    sz |= 0x40000000;
  }

  if(arg2size > 0)
  {
    void* arg2 = r->msg.p2;
    r->msg.p2 = ((char*)(r + 1)) + arg1size;
    memcpy(r->msg.p2, arg2, arg2size);
    sz |= 0x80000000;
  }

  r->sz = sz;
  r->next = self->styles;
  self->styles = r;
  return r;
}

fgStyleMsg* fgStyle_CloneStyleMsg(const fgStyleMsg* self)
{
  unsigned int sz = self->sz&(~0xC0000000);
  fgStyleMsg* r = (fgStyleMsg*)fgmalloc<char>(sz, __FILE__, __LINE__);
  MEMCPY(r, sz, self, sz);

  if(r->sz & 0x40000000)
    r->msg.p = (char*)r + ((char*)self->msg.p - (char*)self);

  if(r->sz & 0x80000000)
    r->msg.p2 = (char*)r + ((char*)self->msg.p2 - (char*)self);

  return r;
}

void fgStyle_RemoveStyleMsg(fgStyle* self, fgStyleMsg* msg)
{
  if(self->styles == msg)
    self->styles = msg->next;
  else
  {
    fgStyleMsg* cur = self->styles;
    while(cur && cur->next != msg) cur = cur->next;
    if(cur) cur->next = msg->next;
  }
  fgfree(msg, __FILE__, __LINE__);
}


fgStyleMsg* fgStyle::AddStyleMsg(const FG_Msg* msg) { return fgStyle_AddStyleMsg(this, msg, 0, 0); }
void fgStyle::RemoveStyleMsg(fgStyleMsg* msg) { fgStyle_RemoveStyleMsg(this, msg); }

FG_UINT fgStyle_GetName(const char* name)
{
  static FG_UINT count = 0;
  assert(count < (sizeof(FG_UINT)<<3));
  
  int r;
  khiter_t iter = kh_put_fgStyles(fgStyleStatic::Instance.h, name, &r);
  if(r) // if it wasn't in there before, we need to initialize the index
  {
    kh_key(fgStyleStatic::Instance.h, iter) = fgCopyText(name, __FILE__, __LINE__);
    kh_val(fgStyleStatic::Instance.h, iter) = (1 << count++);
  }
  return kh_val(fgStyleStatic::Instance.h, iter);
}

const char* fgStyle_GetMapIndex(FG_UINT index)
{
  for(khiter_t i = 0; i < kh_end(fgStyleStatic::Instance.h); ++i)
    if(kh_exist(fgStyleStatic::Instance.h, i) && kh_val(fgStyleStatic::Instance.h, i) == index)
      return kh_key(fgStyleStatic::Instance.h, i);
  return 0;
}

FG_UINT fgStyle_GetAllNames(const char* names)
{
  size_t len = strlen(names) + 1;
  DYNARRAY(char, tokenize, len);
  MEMCPY(tokenize, len, names, len);
  char* context;
  char* token = STRTOK(tokenize, "+", &context);
  FG_UINT style = 0;
  while(token)
  {
    style |= fgStyle_GetName(token);
    token = STRTOK(0, "+", &context);
  }

  return style;
}