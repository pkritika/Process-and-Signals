// C program to demonstrate use of fork() and pipe() 

#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 
  
int main() 
{ 
    // We use two pipes 
    // fd1: Parent (P1) -> Child (P2)
    // fd2: Child (P2) -> Parent (P1)
  
    int fd1[2];  // Used to store two ends of first pipe 
    int fd2[2];  // Used to store two ends of second pipe 
  
    char fixed_str1[] = "howard.edu"; 
    char fixed_str2[] = "gobison.org";
    char input_str[100]; 
    pid_t p; 
  
    if (pipe(fd1)==-1) 
    { 
        fprintf(stderr, "Pipe Failed" ); 
        return 1; 
    } 
    if (pipe(fd2)==-1) 
    { 
        fprintf(stderr, "Pipe Failed" ); 
        return 1; 
    } 
  
    printf("Enter a string to concatenate: ");
    scanf("%s", input_str); 
    
    p = fork(); 
  
    if (p < 0) 
    { 
        fprintf(stderr, "fork Failed" ); 
        return 1; 
    } 
  
    // Parent process (P1)
    else if (p > 0) 
    { 
        // P1 writes to fd1[1] and reads from fd2[0]
        // Close unused ends
        close(fd1[0]); // Close reading end of pipe 1
        close(fd2[1]); // Close writing end of pipe 2
  
        // Write input string to P2 via pipe 1
        write(fd1[1], input_str, strlen(input_str)+1); 
        // Close write end of pipe 1, signaling P2
        close(fd1[1]);
  
        // Wait for child process to finish
        wait(NULL); 

        // Read the result from P2 via pipe 2
        char received_str[200]; // Make buffer larger
        read(fd2[0], received_str, 200);

        // Concatenate "gobison.org"
        int k = strlen(received_str); 
        int i; 
        for (i=0; i<strlen(fixed_str2); i++) 
            received_str[k++] = fixed_str2[i]; 
  
        // **FIXED BUG**: Null-terminate the correct string
        received_str[k] = '\0';   // string ends with '\0' 
  
        printf("Final concatenated string: %s\n", received_str);
  
        // Close remaining pipe end
        close(fd2[0]);
    } 
  
    // Child process (P2)
    else
    { 
        // P2 reads from fd1[0] and writes to fd2[1]
        // Close unused ends
        close(fd1[1]); // Close writing end of pipe 1
        close(fd2[0]); // Close reading end of pipe 2
      
        // Read a string from P1 via pipe 1
        char concat_str[200]; // Make buffer larger
        read(fd1[0], concat_str, 200); 
  
        // Concatenate "howard.edu"
        int k = strlen(concat_str); 
        int i; 
        for (i=0; i<strlen(fixed_str1); i++) 
            concat_str[k++] = fixed_str1[i]; 
  
        concat_str[k] = '\0';   // string ends with '\0' 
  
        // Print the first concatenation
        printf("P2 concatenated string: %s\n", concat_str);

        // **FIXED LOGIC**: Prompt for second input as required
        char second_input[100];
        printf("Enter another string to concatenate: ");
        scanf("%s", second_input);
        
        // Append the second input to concat_str
        k = strlen(concat_str); // Get new length
        for (i=0; i < strlen(second_input); i++)
            concat_str[k++] = second_input[i];
        
        concat_str[k] = '\0'; // Re-terminate the string

        // Send the new combined string back to P1 via pipe 2
        write(fd2[1], concat_str, strlen(concat_str)+1);

        // Close remaining pipe ends
        close(fd1[0]); 
        close(fd2[1]);
  
        exit(0); 
    } 
}