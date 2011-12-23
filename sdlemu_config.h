#ifndef __SDL_CONFIG_H__
#define __SDL_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

int sdlemu_init_config(const char * filename);
const char * sdlemu_getval_string(const char * key_string, const char * default_string);
int sdlemu_getval_int(const char * key_string, int default_int);
int sdlemu_getval_bool(const char * key_string, int default_int);

#ifdef __cplusplus
}
#endif

#endif
