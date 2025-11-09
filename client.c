//OS Mini Project
//Name : Arismita Mukherjee
//Roll : IMT2023585
//Date : 15/05/2024

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/types.h>

#define PORT_NUM 10045

#define MAX_USER_TYPE_LENGTH 20
#define MAX_USERNAME_LENGTH 20
#define MAX_PASSWORD_LENGTH 20
#define MAX_MEMBER_USERNAME_LENGTH 20
#define MAX_BOOK_TITLE_LENGTH 50
#define MAX_NUMBER_OF_CHOICES 10

ssize_t writeAll(int sockfd, const void *buf, size_t len) {
    const char *ptr = (const char *)buf; // Pointer to current position in buffer
    size_t remaining = len;              // Bytes left to send
    ssize_t sent;

    while (remaining > 0) {
        sent = write(sockfd, ptr, remaining);
        if (sent < 0) {
            if (errno == EINTR) {
                continue; // Interrupted, try again
            }
            perror("send failed");
            return -1; // Error
        } else if (sent == 0) {
            fprintf(stderr, "Connection closed by peer\n");
            return -1; // Connection closed
        }

        ptr += sent;        // Advance buffer pointer
        remaining -= sent;   // Reduce remaining bytes
    }

    return len; // Return total bytes sent
}

int str2Int(const char *str, int *result) {
    int num;
    int items = sscanf(str, "%d", &num);

    if (items != 1) {
        fprintf(stderr, "Invalid number: '%s'\n", str);
        return -1;
    }

    *result = num;
    return 0;
}

void studentMenu(int sock, char *name)
{
    char entered_password[MAX_PASSWORD_LENGTH];
    char choice[MAX_NUMBER_OF_CHOICES];
	char user_choice[60];
	char coursename[20];
	char facultyname[20];
	char maxReg[4];
	int maxRegistrations, maxRegIsInt = -1;
	int ret = 0;
	printf("........ Welcome to Student Menu ...........\n");
	while(1) {
    	char server_reply[2000] = "";
		printf("1. Register for a new course\n");
		printf("2. De-Register from a course\n");
		printf("3. List the registered courses\n");
		printf("4. Change password\n");
		printf("5. Logout and Exit\n");
		scanf("%9s", choice);
		printf("Selected Choice: %s\n",choice);
		//snprintf(user_choice, sizeof(user_choice), "%s%s\n", "Stu", choice);
        if(strcmp(choice, "1") == 0) {
        	//writeAll(sock, user_choice, strlen(user_choice));
        	//writeAll(sock, name, strlen(name));
            printf("Enter coursename: \n");
            scanf("%19s", coursename);
            //writeAll(sock, coursename, strlen(coursename));
            printf("Enter facultyname: \n");
            scanf("%19s", facultyname);
            //writeAll(sock, facultyname, strlen(facultyname));
			snprintf(user_choice, sizeof(user_choice), "%s%s:%s:%s:%s", "Stu", choice, name, coursename, facultyname);
        	writeAll(sock, user_choice, strlen(user_choice));
            ret = read(sock, server_reply, 2000);
            printf("%s\n", server_reply);
            if (ret == -1) {
                perror("read error");
                exit(EXIT_FAILURE);
            }
		} else if(strcmp(choice, "2") == 0) {
        	//writeAll(sock, user_choice, strlen(user_choice));
        	//writeAll(sock, name, strlen(name));
            printf("Enter coursename: \n");
            scanf("%19s", coursename);
            //writeAll(sock, coursename, strlen(coursename));
            printf("Enter facultyname: \n");
            scanf("%19s", facultyname);
            //writeAll(sock, facultyname, strlen(facultyname));
			snprintf(user_choice, sizeof(user_choice), "%s%s:%s:%s:%s", "Stu", choice, name, coursename, facultyname);
        	writeAll(sock, user_choice, strlen(user_choice));
            ret = read(sock, server_reply, 2000);
            printf("%s\n", server_reply);
            if (ret == -1) {
                perror("read error");
                exit(EXIT_FAILURE);
            }
		} else if(strcmp(choice, "3") == 0) {
        	//writeAll(sock, user_choice, strlen(user_choice));
        	//writeAll(sock, name, strlen(name));
			snprintf(user_choice, sizeof(user_choice), "%s%s:%s", "Stu", choice, name);
        	writeAll(sock, user_choice, strlen(user_choice));
            ret = read(sock, server_reply, 2000);
            printf("%s\n", server_reply);
            if (ret == -1) {
                perror("read error");
                exit(EXIT_FAILURE);
            }
		} else if(strcmp(choice, "4") == 0) {
			//snprintf(user_choice, sizeof(user_choice), "Adm4"); // Same as modify student
        	//writeAll(sock, user_choice, strlen(user_choice));
            //writeAll(sock, name, strlen(name));
            printf("Enter new password: \n");
            scanf("%19s", entered_password);
            //writeAll(sock, entered_password, sizeof(entered_password));
			snprintf(user_choice, sizeof(user_choice), "%s:%s:%s", "Adm4", name, entered_password);
        	writeAll(sock, user_choice, strlen(user_choice));
            ret = read(sock, server_reply, 2000);
            printf("%s\n", server_reply);
            if (ret == -1) {
                perror("read error");
                exit(EXIT_FAILURE);
            }
		} else if (strcmp(choice, "5") == 0) {
            printf("Exiting Admin Menu\n");
			return;
		}
	}
}

void facultyMenu(int sock, char *name)
{
    char entered_password[MAX_PASSWORD_LENGTH];
    char choice[MAX_NUMBER_OF_CHOICES];
	char user_choice[50];
	char coursename[20];
	char maxReg[4];
	int maxRegistrations, maxRegIsInt = -1;
	int ret = 0;
	printf("........ Welcome to Faculty Menu ...........\n");
	while(1) {
		char server_reply[2000] = " ";
		printf("1. Add a new course\n");
		printf("2. Remove an existing course\n");
		printf("3. List registered students for a course\n");
		printf("4. Change password\n");
		printf("5. Logout and Exit\n");
		scanf("%9s", choice);
		printf("Selected Choice: %s\n",choice);
		//snprintf(user_choice, sizeof(user_choice), "%s%s\n", "Fac", choice);
        if(strcmp(choice, "1") == 0) {
			maxRegIsInt = -1;
        	//writeAll(sock, user_choice, strlen(user_choice));
			//printf("Sent userchoice\n");
        	//writeAll(sock, name, strlen(name));
			//printf("Sent name of faculty\n");
            printf("Enter coursename: \n");
            scanf("%19s", coursename);
            //writeAll(sock, coursename, strlen(coursename));
			while (maxRegIsInt == -1) {
            	printf("Enter max registrations allowed: \n");
            	scanf("%3s", maxReg);
				maxRegIsInt = str2Int(maxReg, &maxRegistrations);
			}
            //writeAll(sock, maxReg, strlen(maxReg));
			snprintf(user_choice, sizeof(user_choice), "%s%s:%s:%s:%s", "Fac", choice, name, coursename, maxReg);
        	writeAll(sock, user_choice, strlen(user_choice));
            ret = read(sock, server_reply, 2000);
            printf("%s\n", server_reply);
            if (ret == -1) {
                perror("read error");
                exit(EXIT_FAILURE);
            }
		} else if(strcmp(choice, "2") == 0) {
        	//writeAll(sock, user_choice, strlen(user_choice));
        	//writeAll(sock, name, strlen(name));
            printf("Enter coursename: \n");
            scanf("%19s", coursename);
            //writeAll(sock, coursename, strlen(coursename));
			snprintf(user_choice, sizeof(user_choice), "%s%s:%s:%s", "Fac", choice, name, coursename);
			printf("Sending to server: %s\n", user_choice);
        	writeAll(sock, user_choice, strlen(user_choice));
            ret = read(sock, server_reply, 2000);
            printf("%s\n", server_reply);
            if (ret == -1) {
                perror("read error");
                exit(EXIT_FAILURE);
            }
		} else if(strcmp(choice, "3") == 0) {
        	//writeAll(sock, user_choice, strlen(user_choice));
        	//writeAll(sock, name, strlen(name));
            printf("Enter coursename: \n");
            scanf("%19s", coursename);
            //writeAll(sock, coursename, strlen(coursename));
			snprintf(user_choice, sizeof(user_choice), "%s%s:%s:%s", "Fac", choice, name, coursename);
        	writeAll(sock, user_choice, strlen(user_choice));
            ret = read(sock, server_reply, 2000);
            printf("%s\n", server_reply);
            if (ret == -1) {
                perror("read error");
                exit(EXIT_FAILURE);
            }
		} else if(strcmp(choice, "4") == 0) {
			//snprintf(user_choice, sizeof(user_choice), "Adm6"); // Same as modify Faculty
        	//writeAll(sock, user_choice, strlen(user_choice));
            //writeAll(sock, name, strlen(name));
            printf("Enter new password: \n");
            scanf("%19s", entered_password);
            //writeAll(sock, entered_password, sizeof(entered_password));
			snprintf(user_choice, sizeof(user_choice), "%s:%s:%s", "Adm6", name, entered_password);
        	writeAll(sock, user_choice, strlen(user_choice));
            ret = read(sock, server_reply, 2000);
            printf("%s\n", server_reply);
            if (ret == -1) {
                perror("read error");
                exit(EXIT_FAILURE);
            }
		} else if (strcmp(choice, "5") == 0) {
            printf("Exiting Admin Menu\n");
			return;
		}
	}
}

void adminMenu(int sock)
{
    char entered_username[MAX_USERNAME_LENGTH];
    char entered_password[MAX_PASSWORD_LENGTH];
    char choice[MAX_NUMBER_OF_CHOICES];
	char user_choice[15];
	long sizeTx = 0;
	int sizeRx = 0;
	int ret = 0;
	printf("........ Welcome to Admin Menu ...........\n");
	while(1) {
    	char server_reply[2000] = "";
		printf("1. Add Student\n");
		printf("2. Activate Student\n");
		printf("3. Deactivate Student\n");
		printf("4. Modify Student\n");
		printf("5. Add Faculty\n");
		printf("6. Modify Faculty\n");
		printf("7. Logout and Exit\n");
		printf("Enter Your Choice: \n");
		scanf("%9s", choice);
		printf("Selected Choice: %s\n",choice);
		snprintf(user_choice, sizeof(user_choice), "%s%s\n", "Adm", choice);

        if(strcmp(choice, "1") == 0) {
        	writeAll(sock, user_choice, strlen(user_choice));
            printf("Enter username: \n");
            scanf("%19s", entered_username);
			//printf("STUDENT: %s\n", entered_username);
            writeAll(sock, entered_username, strlen(entered_username));
            printf("Enter a password: \n");
            scanf("%19s", entered_password);
			//printf("STUDENT Pass: %s\n", entered_password);
			//printf("STUDENT PassLen: %ld\n", sizeof(entered_password));
            sizeTx = writeAll(sock, entered_password, sizeof(entered_password));
			//printf("STUDENT Pass size transmitted: %ld\n", sizeTx);
            ret = read(sock, server_reply, 2000);
            if(strcmp(server_reply, "User already exists") == 0) {
                printf("%s\n", server_reply);
                continue;
            } else if(strcmp(server_reply, "Registered") == 0) {
                printf("%s\n", server_reply);
            }
            if (ret == -1) {
                perror("read error");
                exit(EXIT_FAILURE);
            }
		} else if(strcmp(choice, "2") == 0) {
        	writeAll(sock, user_choice, strlen(user_choice));
            printf("Enter studentname: \n");
            scanf("%19s", entered_username);
			// printf("STUDENT: %s\n", entered_username);
            writeAll(sock, entered_username, strlen(entered_username));
            ret = read(sock, server_reply, 2000);
            printf("%s\n", server_reply);
            if(strcmp(server_reply, "Student is not inactive") == 0) {
                continue;
            } 
            if (ret == -1) {
                perror("read error");
                exit(EXIT_FAILURE);
            }
        } else if(strcmp(choice, "3") == 0) {
        	writeAll(sock, user_choice, strlen(user_choice));
            printf("Enter studentname: \n");
            scanf("%19s", entered_username);
			// printf("STUDENT: %s\n", entered_username);
            writeAll(sock, entered_username, strlen(entered_username));
            ret = read(sock, server_reply, 2000);
            printf("%s\n", server_reply);
            if(strcmp(server_reply, "Student is not active") == 0) {
                continue;
            } 
            if (ret == -1) {
                perror("read error");
                exit(EXIT_FAILURE);
            }
       } else if(strcmp(choice, "4") == 0) {
        	writeAll(sock, user_choice, strlen(user_choice));
            printf("Enter username: \n");
            scanf("%19s", entered_username);
			// printf("STUDENT: %s\n", entered_username);
            writeAll(sock, entered_username, strlen(entered_username));
            printf("Enter modified password: \n");
            scanf("%19s", entered_password);
            sizeTx = writeAll(sock, entered_password, sizeof(entered_password));
            ret = read(sock, server_reply, 2000);
            printf("%s\n", server_reply);
            if(strcmp(server_reply, "Student does not exist") == 0) {
                continue;
            }
            if (ret == -1) {
                perror("read error");
                exit(EXIT_FAILURE);
            }
        } else if(strcmp(choice, "5") == 0) {
        	writeAll(sock, user_choice, strlen(user_choice));
            printf("Enter username: \n");
            scanf("%19s", entered_username);
			// printf("Faculty: %s\n", entered_username);
            writeAll(sock, entered_username, strlen(entered_username));
            printf("Enter a password: \n");
            scanf("%19s", entered_password);
			// printf("Faculty Pass: %s\n", entered_password);
			// printf("Faculty PassLen: %ld\n", sizeof(entered_password));
            sizeTx = writeAll(sock, entered_password, sizeof(entered_password));
			// printf("Faculty Pass size transmitted: %ld\n", sizeTx);
            ret = read(sock, server_reply, 2000);
            // sizeRx = printf("%s\n", server_reply);
            printf("%s\n", server_reply);
			if (ret == -1) {
				perror("Error reading from server");
			}
			server_reply[sizeRx] = '\0';
			printf("%s\n", server_reply);
            if(strcmp(server_reply, "Faculty already exists") == 0) {
                continue;
            }
            if (ret == -1) {
                perror("read error");
                exit(EXIT_FAILURE);
            }
        } else if(strcmp(choice, "6") == 0) {
        	writeAll(sock, user_choice, strlen(user_choice));
            printf("Enter facultyName: \n");
            scanf("%19s", entered_username);
			// printf("Faculty: %s\n", entered_username);
            writeAll(sock, entered_username, strlen(entered_username));
            printf("Enter modified password: \n");
            scanf("%19s", entered_password);
            sizeTx = writeAll(sock, entered_password, sizeof(entered_password));
            ret = read(sock, server_reply, 2000);
            printf("%s\n", server_reply);
            if(strcmp(server_reply, "Faculty does not exist") == 0) {
                continue;
            }
            if (ret == -1) {
                perror("read error");
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(choice, "7") == 0) {
            printf("Exiting Admin Menu\n");
			return;
		}
	}
}

int main()
{
    struct sockaddr_in server;
    int sock;
    char entered_username[MAX_USERNAME_LENGTH];
    char entered_password[MAX_PASSWORD_LENGTH];
    char message[1000], server_reply[2000];
    char buf[100];
    int ret;
    char choice[MAX_NUMBER_OF_CHOICES];
	char user_choice[15];
    int auth_status=0;
    int loginType = 0; // 1 - admin, 2 - faculty, 3 - student
	long sizeTx = 0;
	int sizeRx = 0;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    printf("Socket created\n");
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT_NUM);
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
    printf("...............Welcome Back to Academia :: Course Registration..............\n");

    while(auth_status==0)
    {
		printf("Login Type\n");
        printf("Enter Your Choice {1.Admin, 2.Faculty, 3.Student}\n");
        printf("Enter 'end' to exit: \n");
        scanf("%9s", user_choice);
        writeAll(sock, user_choice, strlen(user_choice));
        if (strcmp(user_choice, "1") == 0) {
	    	printf("User: Admin\n");
            printf("Enter password: ");
            scanf("%19s", entered_password);
            writeAll(sock, entered_password, strlen(entered_password));
            ret = read(sock, server_reply, 2000);
            printf("%s\n", server_reply);
            if(strcmp(server_reply, "Login successful") == 0)
            {
                auth_status = 1;
                loginType = 1;
            }
            else if(strcmp(server_reply, "Login failed") == 0)
            {
                printf("Invalid password\n");
                continue;
            }
            if (ret == -1) 
            {
                perror("read error");
                exit(EXIT_FAILURE);
            }
            // printf("auth_status: %d\n",auth_status);
			if ((auth_status == 1) && (loginType == 1)) {
				adminMenu(sock);
				auth_status = 0;
				loginType = 0;
			}
		} else if (strcmp(user_choice, "2") == 0) {
	    	printf("Enter Faculty Name:\n");
            scanf("%19s", entered_username);
            writeAll(sock, entered_username, strlen(entered_username));
            printf("Enter password:\n");
            scanf("%19s", entered_password);
            writeAll(sock, entered_password, strlen(entered_password));
            ret = read(sock, server_reply, 2000);
            printf("%s\n", server_reply);
            if(strcmp(server_reply, "Login successful") == 0)
            {
                auth_status = 1;
                loginType = 2; 
            }
            else if(strcmp(server_reply, "Login failed") == 0)
            {
                printf("Incorrect credentials\n");
                continue;
            }
            if (ret == -1) 
            {
                perror("read error");
                exit(EXIT_FAILURE);
            }
            // printf("auth_status: %d\n",auth_status);
			if ((auth_status == 1) && (loginType == 2)) {
				facultyMenu(sock, entered_username);
				auth_status = 0;
				loginType = 0;
			}
		} else if (strcmp(user_choice, "3") == 0) {
	    	printf("Enter Student Name:\n");
            scanf("%19s", entered_username);
            writeAll(sock, entered_username, strlen(entered_username));
            printf("Enter password:\n");
            scanf("%19s", entered_password);
            writeAll(sock, entered_password, strlen(entered_password));
            ret = read(sock, server_reply, 2000);
            printf("%s\n", server_reply);
            if(strcmp(server_reply, "Login successful") == 0)
            {
                auth_status = 1;
                loginType = 3; 
            }
            else if(strcmp(server_reply, "Login failed") == 0)
            {
                printf("Incorrect credentials\n");
                continue;
            }
            if (ret == -1) 
            {
                perror("read error");
                exit(EXIT_FAILURE);
            }
            // printf("auth_status: %d\n",auth_status);
			if ((auth_status == 1) && (loginType == 3)) {
				studentMenu(sock, entered_username);
				auth_status = 0;
				loginType = 0;
			}
        } else if(strcmp(user_choice, "end") == 0) {
            printf("Exiting\n");
            write(sock, "end", strlen("end"));
            close(sock);
            exit(0);
        } else {
            printf("Please enter a valid choice only\n");
		}
    }

    write(sock, "Bye", strlen("Bye"));
    close(sock);
    exit(0);
}
