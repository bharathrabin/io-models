/*
 * 02_buffered_stdio.c
 * Same task as 01_block_rw.c (copy a file), but using libc's buffered stdio API - fopen()/fread()/fwrite()/fclose() - instead of raw syscalls
 * 
 * The key difference: fread()/fwrite() do not map 1:1 to read()/write() syscalls. libc maintains its own userspace buffer (typically 4 - 8 KiB, 
 * see BUFSIZE) per FILE*. Small or oddly-sized reads/writes get coalesced in userspace, and the actual syscall only fires when that internal
 * buffer fills up (on write) or is exhausted (on read).
 * 
 * This still counts as *synchronous, blocking* I/O - fread() can still block waiting on the kernel - but it changes how OFTEN we block,
 * because most calls just touch a userspace buffer and never enter the kernel at all.
 * 
 * We deliberately use a copy chunk SMALLER than the libc buffer (64 bytes vs the default ~4 - 8 KiB BUFSIZE) so the coalescing effect is visible: 
 * our loop calls fread()/fwrite() far more often than the program in 01, but performs far fewer actual syscalls. We verify this with strace/dtrace in
 * the README notes, and formally in 03_timing_harness.c.
 * 
 * Usage:
 *   ./02_buffered_stdio <source_file> <dest_file>
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <errno.h>

 #define COPY_CHUNK 64

 static void die(const char *what) {
    fprintf(stderr, "error:%s: %s\n", what, strerror(errno));
    exit(EXIT_FAILURE);
 }

 int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <source_file> <dest_file>\n", argv[0]);
        return EXIT_FAILURE;
    } 

    const char *src_path = argv[1];
    const char *dest_path = argv[2];

    FILE *src = fopen(src_path, "rb");
    if (!src) die("fopen(source)");

    FILE *dst = fopen(dest_path, "wb");
    if (!dst) die("fopen(dest)");


    fprintf(stderr, "libc default buffer size (BUFSIZE) = %d bytes\n", BUFSIZ);

    char buf[COPY_CHUNK];
    size_t n;
    long total_bytes = 0;
    long fread_calls = 0;
    long fwrite_calls = 0;


    while((n = fread(buf, 1, COPY_CHUNK, src)) > 0) {
        fread_calls++;

        size_t written = fwrite(buf, 1, n, dst);
        fwrite_calls++;
        if (written != n) die("fwrite");

        total_bytes += (long)n;
    }
    if (ferror(src)) die("fread");
    if (fclose(src) != 0) die("fclose(source)");
    if (fclose(dst) != 0) die("fclose(dest)");

    fprintf(stderr,
            "copied %ld bytes using %ld fread() calls and %ld fwrite() "
            "calls (copy chunk = %d bytes, libc buffer = %d bytes)\n",
            total_bytes, fread_calls, fwrite_calls, COPY_CHUNK, BUFSIZ);
    
    return EXIT_SUCCESS;
 }


