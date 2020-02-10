/*
 * NK Loader - Kernel functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * IO Abstraction - relay to term
 */

#include "loader/types.h"
#include "loader/krn/tm.h"
#include "loader/krn/ios.h"

/*
 * Methods
 */
static size_t ios_tm_write(struct ios_t*, const void* buf, size_t count);

static struct iosm_t ios_tm_method = {
        .write = ios_tm_write
};

/*
 * Private data
 */
static struct ios_t ios_tm_desc[] = {
        {
                .d = (unsigned int)0,
                .m = &ios_tm_method
        }
};

struct ios_t* ios_tm_init(int fd) {
        if (fd == 0) {
                return &ios_tm_desc[0];
        }

        return 0;
}

size_t ios_tm_write(struct ios_t* ios,const void* buf, size_t count) {
        int i;
        unsigned int td = (unsigned int)ios->d;

        for (i = 0; i < count; i++) {
                tm_putchar(td, ((char*)buf)[i]);
        }
}

