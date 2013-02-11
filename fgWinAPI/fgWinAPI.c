// Copyright �2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feathergui.h"

#include "fgWinAPI.h"
#include "bss_defines.h"
#include "win32_includes.h"
#include <CommCtrl.h>

#if defined(BSS_DEBUG) && defined(BSS_CPU_x86_64)
#pragma comment(lib, "../bin/feathergui64_d.lib")
#elif defined(BSS_CPU_x86_64)
#pragma comment(lib, "../bin/feathergui64.lib")
#elif defined(BSS_DEBUG)
#pragma comment(lib, "../bin/feathergui_d.lib")
#else
#pragma comment(lib, "../bin/feathergui.lib")
#endif

WinAPIfgRoot* _fgroot=0; // fgRoot singleton variable

fgStatic* FG_FASTCALL fgLoadVector(const char* path)
{
  return 0;
}
fgStatic* FG_FASTCALL fgLoadVectorData(const void* data, size_t length)
{
  return 0;
}
void FG_FASTCALL fgRoot_destroy(fgRoot* self)
{
  (*self->mouse.element.destroy)(&self->mouse);
  fgWindow_Destroy((fgWindow*)self);
}
fgRoot* FG_FASTCALL fgInitialize()
{
  return WinAPIfgInitialize(0);
}
void FG_FASTCALL WinAPIfgRoot_Update(fgRoot* self, double delta)
{
  MSG msg;

  while(PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  fgRoot_Update(self,delta);
}

//char FG_FASTCALL WinAPIfgRoot_Behavior(fgWindow* self, const FG_Msg* msg)
//{
//  if(msg->type==WINAPIFGTOP_MSGFILTER) return 0;
//  return fgRoot_BehaviorDefault(self,msg);
//}

fgRoot* FG_FASTCALL WinAPIfgInitialize(void* instance)
{
  WNDCLASSEXW wcex = { sizeof(WNDCLASSEXW), // cbSize
            CS_HREDRAW | CS_VREDRAW,        // style
            (WNDPROC)fgWindowWndProc,       // lpfnWndProc
            NULL,                           // cbClsExtra
            NULL,                           // cbWndExtra
            instance,                       // hInstance
            NULL,                           // hIcon
            NULL,                           // hCursor
            GetSysColorBrush(COLOR_WINDOW), // hbrBackground
            NULL,                           // lpszMenuName
            L"FeatherWindow",               // lpszClassName
            NULL};                          // hIconSm
  INITCOMMONCONTROLSEX initex = { sizeof(INITCOMMONCONTROLSEX), 
    ICC_STANDARD_CLASSES|ICC_BAR_CLASSES|ICC_TAB_CLASSES|ICC_TREEVIEW_CLASSES|ICC_USEREX_CLASSES };

  if(!wcex.hInstance) wcex.hInstance=GetModuleHandle(0);
	if(!RegisterClassExW(&wcex)) {
    assert(0);
    return 0;
  }
  if(!InitCommonControlsEx(&initex)) {
    assert(0);
    return 0;
  }

  _fgroot = (WinAPIfgRoot*)malloc(sizeof(WinAPIfgRoot));
  fgWindow_Init((fgWindow*)_fgroot,0,0,0,0);
  _fgroot->instance=wcex.hInstance;
  _fgroot->root.gui.element.destroy=&fgRoot_destroy;
  _fgroot->root.behaviorhook=&fgRoot_BehaviorDefault;
  _fgroot->root.keymsghook=0;
  _fgroot->root.winrender=&fgRoot_WinRender;
  _fgroot->root.update=&WinAPIfgRoot_Update;
  _fgroot->root.updateroot=0;

  fgWindow_Init(&_fgroot->root.mouse,0,0,0,0);
  

  return (fgRoot*)_fgroot;
}

fgRoot* FG_FASTCALL fgSingleton()
{
  return (fgRoot*)_fgroot;
}

void FG_FASTCALL WinAPIutf8to16(wchar_t** t, const char* src)
{
  size_t len = MultiByteToWideChar(CP_UTF8, 0, src, -1, 0, 0);
  *t=0;
  assert(len!=0);

  if(len)
  {
    *t = malloc(len*sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, src, -1, *t, len);
  }
}

void FG_FASTCALL WinAPIfgWindow_Destroy(fgWindow* self)
{
  HANDLE h = ((WinAPIfgWindow*)self)->handle;
  fgWindow_Destroy(self);
  if(!h) return; // If h is NULL we already blew up the window.
  ((WinAPIfgWindow*)self)->handle=0;
  DestroyWindow(h); // Do winAPI destruction
}

unsigned char getallbtn(WPARAM wParam)
{
  return (wParam&MK_LBUTTON)|(wParam&MK_RBUTTON)|((wParam&MK_MBUTTON)>>2)|((wParam&MK_XBUTTON1)>>2)|((wParam&MK_XBUTTON2)>>2);
}

void makelparampts(FG_Msg* msg, POINTS FAR* pts)
{
  msg->x=pts->x;
  msg->y=pts->y;
}

#define MAKELPPOINTS(l)       ((POINTS FAR *)&(l))

LRESULT CALLBACK fgWindowWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  BYTE allkeys[256];
  //PAINTSTRUCT ps; 
  //HDC hdc; 
  WinAPIfgWindow* wn;
  FG_Msg msg={0};
  WinAPIMessage wmsg = { message, wParam, lParam };

  wn = (WinAPIfgWindow*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
  
  switch (message)
  {
  case WM_MOUSEWHEEL:
  case WM_MOUSEMOVE:
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_LBUTTONDBLCLK:
  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP:
  case WM_RBUTTONDBLCLK:
  case WM_MBUTTONDOWN:
  case WM_MBUTTONUP:
  case WM_MBUTTONDBLCLK:
  case WM_XBUTTONDOWN:
  case WM_XBUTTONUP:
  case WM_XBUTTONDBLCLK:
  case WM_SYSKEYUP:
  case WM_SYSKEYDOWN:
	case WM_KEYUP:
  case WM_KEYDOWN:
  case WM_UNICHAR:
  case WM_CHAR:
    assert(wn!=0);
  }
  
  if(wn!=0)
  {
    msg.type=WINAPIFGTOP_MSGFILTER;
    msg.other=&wmsg;
    if((*_fgroot->root.behaviorhook)((fgWindow*)wn,&msg)!=0) // We invert the meaning of 1 and 0 here so it remains a sane default.
      return 0;
    msg.other=0;
  }

  switch (message)
	{
  case WM_CREATE:
    break;
  //case WM_PAINT: 
  //    hdc = BeginPaint(hWnd, &ps); 
  //    TextOut(hdc, 0, 0, "Hello, Windows!", 15); 
  //    EndPaint(hWnd, &ps); 
  //    return 0;
  case WM_CLOSE:
    msg.type=FG_DESTROY;
    (*_fgroot->root.behaviorhook)((fgWindow*)wn,&msg);
    return 0;
  case WM_DESTROY:
    if(wn->handle!=0) // If handle is NULL, we're already nuking the feather window so don't do it again
    {
      wn->handle=0; // In this case the window is already being destroyed so prevent any further destruction.
      VirtualFreeChild((fgChild*)wn);
    }
    break;
  case WM_MOUSEWHEEL:
    msg.type=FG_MOUSESCROLL;
    msg.scrolldelta=GET_WHEEL_DELTA_WPARAM(wParam);
    (*_fgroot->root.behaviorhook)((fgWindow*)wn,&msg);
    break;
  //case WM_NCCALCSIZE:
  //  return 0;
  case WM_MOUSEMOVE:
    msg.type=FG_MOUSEMOVE;
    msg.allbtn=getallbtn(wParam);
    makelparampts(&msg,MAKELPPOINTS(lParam));
    (*_fgroot->root.behaviorhook)((fgWindow*)wn,&msg);
    break;
  case WM_LBUTTONDOWN:
    msg.type=FG_MOUSEDOWN;
    msg.allbtn=getallbtn(wParam);
    msg.button=FG_MOUSELBUTTON;
    makelparampts(&msg,MAKELPPOINTS(lParam));
    (*_fgroot->root.behaviorhook)((fgWindow*)wn,&msg);
    SetCapture(hWnd);
    break;	
  case WM_LBUTTONUP:
    msg.type=FG_MOUSEUP;
    msg.allbtn=getallbtn(wParam);
    msg.button=FG_MOUSELBUTTON;
    makelparampts(&msg,MAKELPPOINTS(lParam));
    (*_fgroot->root.behaviorhook)((fgWindow*)wn,&msg);
    ReleaseCapture();
    break;
  case WM_LBUTTONDBLCLK:
    msg.type=FG_MOUSEDOWN;
    msg.allbtn=getallbtn(wParam);
    msg.button=FG_MOUSELBUTTON;
    makelparampts(&msg,MAKELPPOINTS(lParam));
    (*_fgroot->root.behaviorhook)((fgWindow*)wn,&msg);
    break;
  case WM_RBUTTONDOWN:
    SetCapture(hWnd);
    msg.type=FG_MOUSEDOWN;
    msg.allbtn=getallbtn(wParam);
    msg.button=FG_MOUSERBUTTON;
    makelparampts(&msg,MAKELPPOINTS(lParam));
    (*_fgroot->root.behaviorhook)((fgWindow*)wn,&msg);
    break;
  case WM_RBUTTONUP:
    msg.type=FG_MOUSEUP;
    msg.allbtn=getallbtn(wParam);
    msg.button=FG_MOUSERBUTTON;
    makelparampts(&msg,MAKELPPOINTS(lParam));
    (*_fgroot->root.behaviorhook)((fgWindow*)wn,&msg);
    ReleaseCapture();
    break;	
  case WM_RBUTTONDBLCLK:
    msg.type=FG_MOUSEDOWN;
    msg.allbtn=getallbtn(wParam);
    msg.button=FG_MOUSERBUTTON;
    makelparampts(&msg,MAKELPPOINTS(lParam));
    (*_fgroot->root.behaviorhook)((fgWindow*)wn,&msg);
    break;
  case WM_MBUTTONDOWN:
    SetCapture(hWnd);
    msg.type=FG_MOUSEDOWN;
    msg.allbtn=getallbtn(wParam);
    msg.button=FG_MOUSEMBUTTON;
    makelparampts(&msg,MAKELPPOINTS(lParam));
    (*_fgroot->root.behaviorhook)((fgWindow*)wn,&msg);
    break;
  case WM_MBUTTONUP:
    msg.type=FG_MOUSEUP;
    msg.allbtn=getallbtn(wParam);
    msg.button=FG_MOUSEMBUTTON;
    makelparampts(&msg,MAKELPPOINTS(lParam));
    (*_fgroot->root.behaviorhook)((fgWindow*)wn,&msg);
    ReleaseCapture();
    break;	
  case WM_MBUTTONDBLCLK:
    msg.type=FG_MOUSEDOWN;
    msg.allbtn=getallbtn(wParam);
    msg.button=FG_MOUSEMBUTTON;
    makelparampts(&msg,MAKELPPOINTS(lParam));
    (*_fgroot->root.behaviorhook)((fgWindow*)wn,&msg);
    break;
  case WM_XBUTTONDOWN:
    msg.type=FG_MOUSEDOWN;
    msg.allbtn=getallbtn(wParam);
    msg.button=GET_XBUTTON_WPARAM(wParam)==XBUTTON1?FG_MOUSEXBUTTON1:FG_MOUSEXBUTTON2;
    makelparampts(&msg,MAKELPPOINTS(lParam));
    (*_fgroot->root.behaviorhook)((fgWindow*)wn,&msg);
    break;
  case WM_XBUTTONUP:
    msg.type=FG_MOUSEUP;
    msg.allbtn=getallbtn(wParam);
    msg.button=GET_XBUTTON_WPARAM(wParam)==XBUTTON1?FG_MOUSEXBUTTON1:FG_MOUSEXBUTTON2;
    makelparampts(&msg,MAKELPPOINTS(lParam));
    (*_fgroot->root.behaviorhook)((fgWindow*)wn,&msg);
    break;
  case WM_XBUTTONDBLCLK:
    msg.type=FG_MOUSEDOWN;
    msg.allbtn=getallbtn(wParam);
    msg.button=GET_XBUTTON_WPARAM(wParam)==XBUTTON1?FG_MOUSEXBUTTON1:FG_MOUSEXBUTTON2;
    makelparampts(&msg,MAKELPPOINTS(lParam));
    (*_fgroot->root.behaviorhook)((fgWindow*)wn,&msg);
    break;
  case WM_SYSKEYUP:
  case WM_SYSKEYDOWN:
	case WM_KEYUP:
  case WM_KEYDOWN:
    GetKeyboardState(allkeys);
    msg.keydown=message==WM_KEYDOWN||message==WM_SYSKEYDOWN;
    msg.type=msg.keydown?FG_KEYDOWN:FG_KEYUP;
    msg.keycode=(unsigned char)wParam;
    msg.sigkeys=((allkeys[VK_SHIFT] & 0x80)>>7)|((allkeys[VK_CONTROL] & 0x80)>>6)|((allkeys[VK_MENU] & 0x80)>>5)|(((lParam&0x40000000)!=0)<<3);
    (*_fgroot->root.behaviorhook)((fgWindow*)wn,&msg);
    return 0;
  case WM_ACTIVATE:
    //if(_ps_enginemouselock) {
    //  switch(LOWORD(wParam))
    //  {
    //  case WA_ACTIVE:
    //  case WA_CLICKACTIVE:
    //    _lockcursor(true);
    //    break;
    //  case WA_INACTIVE:
    //    _lockcursor(false);
    //    break;
    //  }
    //}
    //break;
  case WM_NCHITTEST:
    //if(_ps_overridehittest) return HTCAPTION;
    break;
  case WM_UNICHAR:
    if(wParam==UNICODE_NOCHAR) return TRUE;
  case WM_CHAR:
    GetKeyboardState(allkeys);
    msg.type=FG_KEYCHAR;
    msg.keychar=(int)wParam;
    msg.sigkeys=((allkeys[VK_SHIFT] & 0x80)>>7)|((allkeys[VK_CONTROL] & 0x80)>>6)|((allkeys[VK_MENU] & 0x80)>>5);
    (*_fgroot->root.behaviorhook)((fgWindow*)wn,&msg);
    return 0;
  //case WM_WINDOWPOSCHANGING:
  //case WM_WINDOWPOSCHANGED: // Make sure we keep track of where the window actually is.
  //  narea.left.abs=((WINDOWPOS*)lParam)->x;
  //  narea.left.rel=0;
  //  narea.top.abs=((WINDOWPOS*)lParam)->y;
  //  narea.top.rel=0;
  //  narea.right.abs=((WINDOWPOS*)lParam)->x+((WINDOWPOS*)lParam)->cx;
  //  narea.right.rel=0;
  //  narea.bottom.abs=((WINDOWPOS*)lParam)->y+((WINDOWPOS*)lParam)->cy;
  //  narea.bottom.rel=0;
  //  fgWindow_SetArea(wn->window,&narea);
  //  return 0;
  //default:
  //  OutputDebugString(cStr("\n%i",message));
  //  break;
  }

	return DefWindowProcW(hWnd, message, wParam, lParam);
}