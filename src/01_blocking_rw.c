/*
* Synchronous blocking IO using raw POSIX syscalls open, 
* read, write and close.
*
* Model everything else in the package will be compared 
* against. 
* Usage:
*   ./01_blocking_rw <source-file> <dest-file>
*
* Build:
*   make 01
*/

#include <fcntl.h>      /* open, O_* flags        */
#include <unistd.h>     /* read, write, close      */
#include <stdio.h>      /* fprintf                 */
#include <stdlib.h>     /* exit                    */
#include <string.h>     /* strerror                */
#include <errno.h>      /* errno                   */

#define CHUNK_SIZE 4096

static void die(const char* what) {
    fprintf(stderr, "error: %s: %s\n", what, strerror(errno));
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    if (argc != 3){
        fprintf(stderr, "usage: %s <source_file> <dest_file>\n", argv[0]);
        return EXIT_FAILURE;
    }


    const char* src_path = argv[1];
    const char* dst_path = argv[2];

    int src_fd = open(src_path, O_RDONLY);
    if (src_fd < 0) die("open(source)");

    int dst_fd = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd < 0) die("open(dest)");

    char buf[CHUNK_SIZE];
    ssize_t bytes_read;
    long total_bytes = 0;
    int calls = 0;

    while((bytes_read = read(src_fd, buf, CHUNK_SIZE)) > 0) {
        calls++;
        ssize_t written_total = 0;
        while(written_total < bytes_read) {
           ssize_t written = write(dst_fd, buf + written_total, bytes_read - written_total);
           if (written < 0) {
                if (errno == EINTR) continue; /* interrupted, retry */
                die("write");
            }
            written_total += written;
        }
        total_bytes += bytes_read;
    }
    if (bytes_read < 0) die("read");
    if (close(src_fd) < 0) die("close(source)");
    if (close(dst_fd) < 0) die("close(dest)");

    fprintf(stderr,
            "copied %ld bytes in %d read()/write() round trips "
            "(chunk size = %d bytes)\n",
            total_bytes, calls, CHUNK_SIZE);

    return EXIT_SUCCESS;
}
