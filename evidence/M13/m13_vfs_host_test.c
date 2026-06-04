#include "../include/mcs_vfs.h"
#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(label, expr) \
    do { \
        if (expr) { \
            printf("[PASS] %s\n", label); \
            g_pass++; \
        } else { \
            printf("[FAIL] %s  (line %d)\n", label, __LINE__); \
            g_fail++; \
        } \
    } while (0)

static mcs_ramfs_t fs;
static mcs_fd_table_t fdt;

static void test_ramfs_init(void) {
    mcs_ramfs_init(&fs);
    CHECK("ramfs_init: root node used",    fs.nodes[0].used == 1u);
    CHECK("ramfs_init: root is dir",       fs.nodes[0].type == MCS_VNODE_DIR);
    CHECK("ramfs_init: node_count == 1",   fs.node_count == 1u);
    CHECK("ramfs_init: data_used == 0",    fs.data_used == 0u);
}

static void test_seed_and_lookup(void) {
    mcs_vnode_t *node;
    const char *hello = "Hello, RAMFS!";
    int rc;


    rc = mcs_ramfs_seed_file(&fs, "/hello.txt",
                              (const uint8_t *)hello, strlen(hello));
    CHECK("seed: rc == OK",   rc == MCS_OK);

    rc = mcs_ramfs_lookup(&fs, "/hello.txt", &node);
    CHECK("lookup: rc == OK",      rc == MCS_OK);
    CHECK("lookup: node != NULL",  node != (mcs_vnode_t *)0);
    if (node) {
        CHECK("lookup: size correct",  node->size == strlen(hello));
        CHECK("lookup: type FILE",     node->type == MCS_VNODE_FILE);
    }

    rc = mcs_ramfs_lookup(&fs, "/noexist.txt", &node);
    CHECK("lookup missing: ENOENT",  rc == MCS_ENOENT);
}

static void test_open_read(void) {
    char buf[64];
    mcs_ssize_t n;
    int fd;

    mcs_fd_table_init(&fdt);
    fd = mcs_vfs_open(&fdt, &fs, "/hello.txt", MCS_O_RDONLY);
    CHECK("open rdonly: fd >= 0",  fd >= 0);

    memset(buf, 0, sizeof(buf));
    n = mcs_vfs_read(&fdt, fd, buf, sizeof(buf) - 1u);
    CHECK("read: n > 0",               n > 0);
    CHECK("read: content match",       strcmp(buf, "Hello, RAMFS!") == 0);

    n = mcs_vfs_read(&fdt, fd, buf, sizeof(buf));
    CHECK("read at EOF: n == 0",       n == 0);

    CHECK("close: OK",  mcs_vfs_close(&fdt, fd) == MCS_OK);
}

static void test_open_write(void) {
    int fd;
    mcs_ssize_t n;
    char rbuf[64];


    fd = mcs_vfs_open(&fdt, &fs, "/write.txt",
                      MCS_O_WRONLY | MCS_O_CREAT);
    CHECK("open wronly creat: fd >= 0",  fd >= 0);

    n = mcs_vfs_write(&fdt, fd, "MCSOS\n", 6u);
    CHECK("write: n == 6",  n == 6);
    CHECK("close wr: OK",   mcs_vfs_close(&fdt, fd) == MCS_OK);

    fd = mcs_vfs_open(&fdt, &fs, "/write.txt", MCS_O_RDONLY);
    CHECK("open after write: fd >= 0",  fd >= 0);
    memset(rbuf, 0, sizeof(rbuf));
    n = mcs_vfs_read(&fdt, fd, rbuf, sizeof(rbuf) - 1u);
    CHECK("read back: n == 6",          n == 6);
    CHECK("read back: content match",   strcmp(rbuf, "MCSOS\n") == 0);
    CHECK("close rd: OK",  mcs_vfs_close(&fdt, fd) == MCS_OK);
}

static void test_lseek(void) {
    int fd;
    char buf[8];
    mcs_ssize_t n;
    int pos;

    fd = mcs_vfs_open(&fdt, &fs, "/hello.txt", MCS_O_RDONLY);
    CHECK("lseek open: fd >= 0",  fd >= 0);

    pos = mcs_vfs_lseek(&fdt, fd, 7, MCS_SEEK_SET);
    CHECK("lseek SET 7: pos == 7",  pos == 7);

    memset(buf, 0, sizeof(buf));
    n = mcs_vfs_read(&fdt, fd, buf, 5u);
    CHECK("read after seek: n == 5",         n == 5);
    CHECK("read after seek: content match",  strcmp(buf, "RAMFS") == 0);

    CHECK("close lseek: OK",  mcs_vfs_close(&fdt, fd) == MCS_OK);
}

static void test_dup(void) {
    int fd, fd2;
    char buf[32];
    mcs_ssize_t n;

    fd = mcs_vfs_open(&fdt, &fs, "/hello.txt", MCS_O_RDONLY);
    CHECK("dup open: fd >= 0",  fd >= 0);

    fd2 = mcs_vfs_dup(&fdt, fd);
    CHECK("dup: fd2 >= 0",        fd2 >= 0);
    CHECK("dup: fd2 != fd",       fd2 != fd);

    memset(buf, 0, sizeof(buf));
    n = mcs_vfs_read(&fdt, fd2, buf, 5u);
    CHECK("read via dup: n == 5",          n == 5);
    CHECK("read via dup: content match",   strcmp(buf, "Hello") == 0);

    CHECK("close orig: OK",  mcs_vfs_close(&fdt, fd) == MCS_OK);
    CHECK("close dup:  OK",  mcs_vfs_close(&fdt, fd2) == MCS_OK);
}

static void test_error_cases(void) {
    int fd;
    char buf[8];

    CHECK("read bad fd: EBADF",
          mcs_vfs_read(&fdt, 99, buf, 4u) == MCS_EBADF);

    fd = mcs_vfs_open(&fdt, &fs, "/hello.txt", MCS_O_RDONLY);
    CHECK("write to rdonly: EACCES",
          mcs_vfs_write(&fdt, fd, "x", 1u) == MCS_EACCES);
    mcs_vfs_close(&fdt, fd);

    CHECK("open noexist no creat: ENOENT",
          mcs_vfs_open(&fdt, &fs, "/ghost.txt", MCS_O_RDONLY) == MCS_ENOENT);

    CHECK("close after close: EBADF",
          mcs_vfs_close(&fdt, fd) == MCS_EBADF);
}

static void test_sys_wrappers(void) {
    mcs_process_t proc;
    int fd;
    char buf[32];
    mcs_ssize_t n;

    proc.pid = 42u;
    mcs_fd_table_init(&proc.fd_table);

    fd = mcs_sys_open(&proc, &fs, "/hello.txt", MCS_O_RDONLY);
    CHECK("sys_open: fd >= 0",  fd >= 0);

    memset(buf, 0, sizeof(buf));
    n = mcs_sys_read(&proc, fd, buf, sizeof(buf) - 1u);
    CHECK("sys_read: n > 0",            n > 0);
    CHECK("sys_read: content match",    strcmp(buf, "Hello, RAMFS!") == 0);

    CHECK("sys_lseek SET 0: OK",
          mcs_sys_lseek(&proc, fd, 0, MCS_SEEK_SET) == 0);

    CHECK("sys_close: OK",  mcs_sys_close(&proc, fd) == MCS_OK);
}

int main(void) {
    printf("=== M13 VFS Host Tests ===\n");
    test_ramfs_init();
    test_seed_and_lookup();
    test_open_read();
    test_open_write();
    test_lseek();
    test_dup();
    test_error_cases();
    test_sys_wrappers();
    printf("\nResults: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
