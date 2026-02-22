/*
 * Stub implementations for SceSharedFb functions.
 * These are referenced by libvita2d but only used in shared framebuffer
 * mode (plugins/overlays). Normal homebrew applications never call them,
 * so empty stubs are safe.
 */

int sceSharedFbClose(int handle) {
    (void)handle;
    return 0;
}

int _sceSharedFbOpen(const void *param, int type) {
    (void)param;
    (void)type;
    return 0;
}

int sceSharedFbGetInfo(int handle, void *info) {
    (void)handle;
    (void)info;
    return 0;
}

int sceSharedFbEnd(int handle) {
    (void)handle;
    return 0;
}

int sceSharedFbBegin(int handle, void *info) {
    (void)handle;
    (void)info;
    return 0;
}
