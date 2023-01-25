#include <wayland-client.h>
#include <wayland-egl.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <poll.h>
#include <errno.h>
#include <string.h>

#include "eglutint.h"
#include <wayland-webos-shell-client-protocol.h>

struct display {
   struct wl_display *display;
   struct wl_compositor *compositor;
   struct wl_shell *shell;
   struct wl_webos_shell *webos_shell;
   uint32_t mask;
};

struct window {
   struct wl_surface *surface;
   struct wl_shell_surface *shell_surface;
   struct wl_webos_shell_surface *webos_shell_surface;
   struct wl_callback *callback;
};

static struct display display = {0, };
static struct window window = {0, };

static void
registry_handle_global(void *data, struct wl_registry *registry, uint32_t id,
                       const char *interface, uint32_t version)
{
   struct display *d = data;

   if (strcmp(interface, "wl_compositor") == 0) {
      d->compositor =
         wl_registry_bind(registry, id, &wl_compositor_interface, 1);
   } else if (strcmp(interface, "wl_shell") == 0) {
      d->shell = wl_registry_bind(registry, id, &wl_shell_interface, 1);
   } else if (strcmp(interface, "wl_webos_shell") == 0) {
      d->webos_shell = wl_registry_bind(registry, id, &wl_webos_shell_interface, 1);
   }
}

static void
registry_handle_global_remove(void *data, struct wl_registry *registry,
                              uint32_t name)
{
}

static const struct wl_registry_listener registry_listener = {
   registry_handle_global,
   registry_handle_global_remove
};

static void
sync_callback(void *data, struct wl_callback *callback, uint32_t serial)
{
   int *done = data;

   *done = 1;
   wl_callback_destroy(callback);
}

static const struct wl_callback_listener sync_listener = {
   sync_callback
};

static int
wayland_roundtrip(struct wl_display *display)
{
   struct wl_callback *callback;
   int done = 0, ret = 0;

   callback = wl_display_sync(display);
   wl_callback_add_listener(callback, &sync_listener, &done);
   while (ret != -1 && !done)
      ret = wl_display_dispatch(display);

   if (!done)
      wl_callback_destroy(callback);

   return ret;
}

void
_eglutNativeInitDisplay(void)
{
   struct wl_registry *registry;

   _eglut->native_dpy =  display.display = wl_display_connect(NULL);

   if (!_eglut->native_dpy)
      _eglutFatal("failed to initialize native display");

   registry = wl_display_get_registry(_eglut->native_dpy);
   wl_registry_add_listener(registry, &registry_listener, &display);
   wayland_roundtrip(_eglut->native_dpy);
   wl_registry_destroy(registry);

   _eglut->surface_type = EGL_WINDOW_BIT;
}

void
_eglutNativeFiniDisplay(void)
{
   wl_compositor_destroy(display.compositor);
   wl_display_flush(_eglut->native_dpy);
   wl_display_disconnect(_eglut->native_dpy);
}

static void webos_shell_handle_state_changed(void *data, struct wl_webos_shell_surface *wl_webos_shell_surface, uint32_t state)
{
    switch(state)
    {
        case WL_WEBOS_SHELL_SURFACE_STATE_FULLSCREEN:
            break;
        case WL_WEBOS_SHELL_SURFACE_STATE_MINIMIZED:
            break;
    }
}

static void webos_shell_handle_position_changed(void *data, struct wl_webos_shell_surface *wl_webos_shell_surface, int32_t x, int32_t y)
{
}

static void webos_shell_handle_close(void *data, struct wl_webos_shell_surface *wl_webos_shell_surface)
{
   /* XXX: not sure this is right */
   eglutDestroyWindow(_eglut->current->index);
   exit(0);
}

static void webos_shell_handle_expose(void *data, struct wl_webos_shell_surface *wl_webos_shell_surface, struct wl_array *rectangles)
{
}

static void webos_shell_handle_state_about_to_change(void *data, struct wl_webos_shell_surface *wl_webos_shell_surface, uint32_t state)
{
}

static const struct wl_webos_shell_surface_listener webos_shell_listener = {
    webos_shell_handle_state_changed,
    webos_shell_handle_position_changed,
    webos_shell_handle_close,
    webos_shell_handle_expose,
    webos_shell_handle_state_about_to_change
};

void
_eglutNativeInitWindow(struct eglut_window *win, const char *title,
                       int x, int y, int w, int h)
{
   struct wl_egl_window *native;
   struct wl_region *region;

   window.surface = wl_compositor_create_surface(display.compositor);

   region = wl_compositor_create_region(display.compositor);
   wl_region_add(region, 0, 0, w, h);
   wl_surface_set_opaque_region(window.surface, region);
   wl_region_destroy(region);

   window.shell_surface = wl_shell_get_shell_surface(display.shell,
         window.surface);
   native = wl_egl_window_create(window.surface, w, h);

   wl_shell_surface_set_toplevel(window.shell_surface);

   window.webos_shell_surface = wl_webos_shell_get_shell_surface(display.webos_shell, window.surface);
   if (window.webos_shell_surface == NULL) {
      fprintf(stderr, "wl_webos_shell_get_shell_surface: error\n");
   } else {
      fprintf(stderr, "window.webos_shell_surface = %p\n", window.webos_shell_surface);
   }

   const char *app_id = NULL, *display_id = NULL;

   if ((app_id = getenv("APP_ID")) == NULL) {
      app_id = DEFAULT_APP_ID;
   }

   if ((display_id = getenv("DISPLAY_ID")) == NULL) {
      display_id = "0";
   }

   wl_webos_shell_surface_add_listener(window.webos_shell_surface, &webos_shell_listener, display.display);
   wl_webos_shell_surface_set_property(window.webos_shell_surface, "appId", app_id);
   wl_webos_shell_surface_set_property(window.webos_shell_surface, "displayAffinity", display_id);

   win->native.u.window = native;
   win->native.width = w;
   win->native.height = h;
}

void
_eglutNativeFiniWindow(struct eglut_window *win)
{
   wl_egl_window_destroy(win->native.u.window);

   wl_shell_surface_destroy(window.shell_surface);
   wl_surface_destroy(window.surface);

   if (window.callback)
      wl_callback_destroy(window.callback);
}

static void
draw(void *data, struct wl_callback *callback, uint32_t time);

static const struct wl_callback_listener frame_listener = {
   draw
};

static void
draw(void *data, struct wl_callback *callback, uint32_t time)
{	
   struct window *window = (struct window *)data;
   struct eglut_window *win = _eglut->current;

   if (win->display_cb)
      win->display_cb();
   eglSwapBuffers(_eglut->dpy, win->surface);

   if (callback)
      wl_callback_destroy(callback);

   window->callback = wl_surface_frame(window->surface);
   wl_callback_add_listener(window->callback, &frame_listener, window);
}

void
_eglutNativeEventLoop(void)
{
   struct pollfd pollfd;
   int ret;

   draw(&window, NULL, 0);

   pollfd.fd = wl_display_get_fd(display.display);
   pollfd.events = POLLIN;
   pollfd.revents = 0;

   while (1) {
      wl_display_dispatch_pending(display.display);

      if (_eglut->idle_cb)
         _eglut->idle_cb();

      ret = wl_display_flush(display.display);
      if (ret < 0 && errno == EAGAIN)
         pollfd.events |= POLLOUT;
      else if (ret < 0)
         break;

      if (poll(&pollfd, 1, _eglut->redisplay ? 0 : -1) == -1)
         break;

      if (pollfd.revents & (POLLERR | POLLHUP))
         break;

      if (pollfd.revents & POLLIN) {
         ret = wl_display_dispatch(display.display);
         if (ret == -1)
            break;
      }

      if (pollfd.revents & POLLOUT) {
         ret = wl_display_flush(display.display);
         if (ret == 0)
            pollfd.events &= ~POLLOUT;
         else if (ret == -1 && errno != EAGAIN)
            break;
      }

      if (_eglut->redisplay) {
         struct eglut_window *win = _eglut->current;

         _eglut->redisplay = 0;

         if (win->display_cb)
            win->display_cb();

         eglSwapBuffers(_eglut->dpy, win->surface);
      }
   }
}