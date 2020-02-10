/*
 * NK Loader - Kernel functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Device Manager (Disk) path utilities
 */

#include "loader/utl/string.h"
#include "loader/utl/stdio.h"

#include "loader/krn/dm_path.h"

struct dm_path_t {
	uint32_t s;
	uint32_t p;
	uint32_t n;
} __attribute__((aligned (4)));

/*
 * Liberation d'un path
 */
void dm_path_free(struct dm_path_t* path) {
	free(path);
}

/*
 * Creation d'un dm_path_t
 */
struct dm_path_t* dm_path_create(const char* filename) {
	struct dm_path_t* path;

	uint32_t n = strlen(filename);
	
	path =(struct dm_path_t*) malloc(n+sizeof(struct dm_path_t));
	if (path == 0) return path;

	path->s = 0;
	path->p = 0;
	path->n = n;
	memmove(path+1,filename,n);

	dm_path_next(path);

	return path;
}

/*
 * Prefixer un chemin
 */
struct dm_path_t* dm_path_prepend(struct dm_path_t* path, const char* name) {
	struct dm_path_t* new;
	char* s = (char*)(path+1);
	char* d;
	size_t l = strlen(name);
	size_t sep = 0;

	if (l>0 && name[l-1] != '/') {
		l++;
		sep = 1;
	}

	if (l >= path->s) {
		new = (struct dm_path_t*) malloc(path->n - path->s + l + sizeof(struct dm_path_t));
		if (new == 0) return 0;	

		d = (char*)(new+1);

		new -> s = 0;
		new -> p = 0;
		new -> n = l + path->n - path->s;

		memmove(d,name,l);
		if (sep == 1) {
			d[l-1]='/';
		}
		memmove(d+l,s+path->s,path->n - path->s);

		free(path);

		dm_path_next(new);
		return new;
	}
	
	memmove(s+path->s-l,name,l);
	if (sep == 1) {
		*(s+path->s-1)='/';
	}
	path->s -= l;
	path->p = path->s;

	dm_path_next(path);

	return path;
}

/*
 * Passe a l element suivant
 */
void dm_path_next(struct dm_path_t* path) {
	const char* s = (const char*)(path + 1);
	const char *sep;

	if (path->s == path->n) {
		return;
	}

	path->s = path->p;
	while ( path->s < path->n && s[path->s] == '/') path->s++;

	sep = strchr(s+path->s,'/');

	path->p = ( sep == 0 ? path->n : sep - s); 
}

/*
 * get current path name
 */
const char* dm_path_name(struct dm_path_t* path) {
	const char* s = (const char*)(path + 1);
	return s + path->s;
}

/*
 * get current path size
 */
size_t dm_path_size(struct dm_path_t* path) {
	const char* s = (const char*)(path + 1);
	return path->p - path->s;
}

int32_t dm_path_hasnext(struct dm_path_t* path) {
	return (path->s < path->n);
}

/*
 * print debuging information
 */
void dm_path_debug(struct dm_path_t* path) {
	const char* s = (const char*)(path + 1);

	printf("%.*s", path->p - path->s, s + path->s);	
}
