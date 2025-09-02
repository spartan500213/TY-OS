#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

// --- Test Configuration ---
#define TEST_FILE "lseek_test_file.txt"
#define TEST_CONTENT "Hello World"
#define TEST_CONTENT_LEN 11

// --- Helper Functions ---

// A simple assertion function to make tests cleaner.
// If the condition is false, it prints an error and exits.
void
check(int condition, const char *test_name)
{
  if(!condition){
    printf(2, "TEST FAILED: %s\n", test_name);
    exit();
  }
}

// A self-contained string comparison function since user-space lacks strncmp.
int
my_strncmp(const char *s1, const char *s2, uint n)
{
  while(n > 0 && *s1 && (*s1 == *s2)){
    s1++;
    s2++;
    n--;
  }
  if(n == 0){
    return 0;
  }
  return (uchar)*s1 - (uchar)*s2;
}

// Creates the file used for testing.
void
setup_test_file()
{
  int fd;
  
  unlink(TEST_FILE); // Delete file if it exists from a previous run
  fd = open(TEST_FILE, O_CREATE | O_RDWR);
  check(fd >= 0, "Setup: Could not create test file");
  
  int n = write(fd, TEST_CONTENT, TEST_CONTENT_LEN);
  check(n == TEST_CONTENT_LEN, "Setup: Failed to write full content");
  
  close(fd);
}


// --- Test Cases ---

void
test_seek_set()
{
  printf(1, "Running Test: SEEK_SET...\n");
  int fd = open(TEST_FILE, O_RDONLY);
  char buf[10];

  // 1. Seek to the middle of the file
  check(lseek(fd, 6, SEEK_SET) == 6, "SEEK_SET to middle");
  read(fd, buf, 5);
  check(my_strncmp(buf, "World", 5) == 0, "SEEK_SET read from middle");

  // 2. Seek to the beginning of the file
  check(lseek(fd, 0, SEEK_SET) == 0, "SEEK_SET to beginning");
  read(fd, buf, 5);
  check(my_strncmp(buf, "Hello", 5) == 0, "SEEK_SET read from beginning");

  // 3. Seek to the exact end of the file (a valid operation)
  check(lseek(fd, TEST_CONTENT_LEN, SEEK_SET) == TEST_CONTENT_LEN, "SEEK_SET to end");
  int n = read(fd, buf, 1);
  check(n == 0, "SEEK_SET read at end should be EOF");

  close(fd);
  printf(1, "PASSED: SEEK_SET\n\n");
}

void
test_seek_cur()
{
  printf(1, "Running Test: SEEK_CUR...\n");
  int fd = open(TEST_FILE, O_RDONLY);
  char buf[10];

  // 1. Move forward from a known position
  read(fd, buf, 5); // Reads "Hello", offset is now 5
  check(lseek(fd, 1, SEEK_CUR) == 6, "SEEK_CUR to move forward");
  read(fd, buf, 5);
  check(my_strncmp(buf, "World", 5) == 0, "SEEK_CUR read after moving forward");

  // 2. Move backward from a known position
  lseek(fd, 0, SEEK_SET); // rewind
  read(fd, buf, 11); // Read whole string, offset is now 11
  check(lseek(fd, -5, SEEK_CUR) == 6, "SEEK_CUR to move backward");
  read(fd, buf, 5);
  check(my_strncmp(buf, "World", 5) == 0, "SEEK_CUR read after moving backward");
  
  // 3. A zero-offset seek should not change position and return current offset
  lseek(fd, 4, SEEK_SET); // Go to offset 4
  check(lseek(fd, 0, SEEK_CUR) == 4, "SEEK_CUR with zero offset");

  close(fd);
  printf(1, "PASSED: SEEK_CUR\n\n");
}

void
test_seek_end()
{
  printf(1, "Running Test: SEEK_END...\n");
  int fd = open(TEST_FILE, O_RDONLY);
  char buf[10];

  // 1. Seek relative to the end
  check(lseek(fd, -5, SEEK_END) == 6, "SEEK_END relative to end");
  read(fd, buf, 5);
  check(my_strncmp(buf, "World", 5) == 0, "SEEK_END read from relative position");

  // 2. Seek to the absolute end
  check(lseek(fd, 0, SEEK_END) == TEST_CONTENT_LEN, "SEEK_END to absolute end");
  int n = read(fd, buf, 1);
  check(n == 0, "SEEK_END read at end should be EOF");
  
  close(fd);
  printf(1, "PASSED: SEEK_END\n\n");
}

void
test_error_conditions()
{
  printf(1, "Running Test: Error Conditions...\n");
  int fd = open(TEST_FILE, O_RDONLY);

  // 1. Seek to a negative offset
  check(lseek(fd, -1, SEEK_SET) == -1, "Error: Negative absolute offset");

  // 2. Seek beyond the end of the file
  check(lseek(fd, 100, SEEK_SET) == -1, "Error: Offset beyond file size");

  // 3. Seek from current position to a negative final offset
  lseek(fd, 2, SEEK_SET); // offset is 2
  check(lseek(fd, -5, SEEK_CUR) == -1, "Error: Relative seek to negative offset");

  // 4. Seek on an invalid file descriptor
  check(lseek(99, 0, SEEK_SET) == -1, "Error: Invalid file descriptor");

  // 5. Seek on a non-seekable file (like stdout)
  check(lseek(1, 0, SEEK_SET) == -1, "Error: Seek on non-seekable file (stdout)");
  
  // 6. Seek with an invalid 'whence' value
  check(lseek(fd, 0, 99) == -1, "Error: Invalid whence value");

  close(fd);
  printf(1, "PASSED: Error Conditions\n\n");
}


// --- Main Test Runner ---
int
main(void)
{
  printf(1, "\n--- Running lseek() Test Suite ---\n\n");
  
  setup_test_file();
  
  test_seek_set();
  test_seek_cur();
  test_seek_end();
  test_error_conditions();

  printf(1, "--- All lseek() tests passed! ---\n");
  
  exit();
}
