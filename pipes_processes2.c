#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/**
 * Executes the command "cat scores | grep Lakers".  In this quick-and-dirty
 * implementation the parent doesn't wait for the child to finish and
 * so the command prompt may reappear before the child terminates.
 *
 */

int main(int argc, char **argv)
{
  int pipefd[2];
  pid_t cat_pid, grep_pid;

  // Determine the search term: use argv[1] if provided, otherwise default
  char *grep_term = "Lakers"; // Default search term
  if (argc > 1) {
    grep_term = argv[1];
  }

  // Arguments for the two commands
  char *cat_args[] = {"cat", "scores", NULL};
  char *grep_args[] = {"grep", grep_term, NULL};

  // Create the pipe
  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  // --- Fork for the 'grep' process (Child 1) ---
  grep_pid = fork();
  if (grep_pid == -1) {
    perror("fork (grep)");
    exit(EXIT_FAILURE);
  }

  if (grep_pid == 0) {
    // --- Child 1 (grep) ---
    // This process reads from the pipe

    // Replace standard input (0) with the read-end of the pipe (pipefd[0])
    if (dup2(pipefd[0], STDIN_FILENO) == -1) {
      perror("dup2 (grep)");
      exit(EXIT_FAILURE);
    }

    // Close both ends of the pipe in the child
    // We don't need them after dup2
    close(pipefd[0]);
    close(pipefd[1]);

    // Execute grep
    execvp("grep", grep_args);
    
    // execvp only returns if an error occurred
    perror("execvp (grep)");
    exit(EXIT_FAILURE);
  }

  // --- Fork for the 'cat' process (Child 2) ---
  cat_pid = fork();
  if (cat_pid == -1) {
    perror("fork (cat)");
    exit(EXIT_FAILURE);
  }

  if (cat_pid == 0) {
    // --- Child 2 (cat) ---
    // This process writes to the pipe

    // Replace standard output (1) with the write-end of the pipe (pipefd[1])
    if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
      perror("dup2 (cat)");
      exit(EXIT_FAILURE);
    }

    // Close both ends of the pipe in this child
    close(pipefd[0]);
    close(pipefd[1]);

    // Execute cat
    execvp("cat", cat_args);

    // execvp only returns if an error occurred
    perror("execvp (cat)");
    exit(EXIT_FAILURE);
  }

  // --- Parent Process ---
  // This process created both children
  
  // The parent must close *both* ends of the pipe, otherwise
  // the 'grep' child will never get EOF and will hang.
  close(pipefd[0]);
  close(pipefd[1]);

  // Wait for both children to finish
  waitpid(grep_pid, &status, 0);
  waitpid(cat_pid, &status, 0);
  
  return 0;
}