void* mir_alloc(size_t sz) {
   __coverity_alloc__(sz);
}

void mir_free(void* x) {
   __coverity_free__(x);
}
