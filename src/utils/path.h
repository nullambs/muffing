#ifndef MUFFING_PATH_H
#define MUFFING_PATH_H

typedef char pathc_t;
typedef pathc_t *path_t;

path_t pathjoin(unsigned count, ...);
path_t filename(path_t);

#endif // !MUFFING_PATH_H
