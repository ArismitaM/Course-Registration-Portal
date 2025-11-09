OS Mini Project - Design and Development of Course Registration Portal (Academia)
Name - Arismita Mukherjee
Roll number - IMT2023585
Batch - IMT2023
Date of Submission - 15th May, 2025

server.c

This is a server-side application written in C for design and development of course registration portial (academia). 
It allows clients to connect (using socket programming), login as admin, faulty or student. 
As a faculty, you can add and delete courses. As a student, you can select which course you want to opt under which faculty.
As an admin, you can add or deactivate students and faculties.

To achieve the above functionalities, I have a linked list of nodes with each node containing a mutex lock 
corresponding to each file used in the system to maintain information related to students, faculties, courses and course registration.
Another mutex lock is present, it adds/updates values within a file.

How the lock functions:
This first creates a lock on LL
Then searches for the node in LL, if not present creates a node
Then locks the file mutex and unlocks the lock in the LL
Then makes changes in the file
Then unlocks the file 

The file structure is as follows:

students.txt -> student name and password
members.txt -> active students names
inactMembers.txt -> inactive students
faculties.txt -> name and password
facMembers.txt -> faculty names
faculty/<facName>/course.txt -> 
    <course name>
    <max registrations allowed>
    <current number of registrations>
    <student 1>
    <student 2>
    ...
    <student curr no. of regs>
student/<student name>/registration ->
    faculty/<fac name 1>/<course name 1>
    faculty/<fac name 2>/<course name 2>

Functions:

Utility functions:

lockFile_t *createALock(char *name) : creates a node in Linked List
lockFile_t *findTheLock(char *name) : searching a node in Linked List
lockFile_t *lockAFile(char *path) : This 1st creates a lock on LL, then searches for a node in LL, if not present creates a node.
                                    Then locks the file mutex and unlocks the lock in the LL, then makes changes in the file and unlocks the file.
void unLockAFile(lockFile_t *lock) : unlocks the file
FILE *openFile4Reading(char *fileName) : not always guaranteed that a file exists, so creates a file in append mode
int fileExists(const char *filename) : checks if file exists or not
int createDirectories(const char *path) : creates Directories 
void readStrFromBuffer(int *newsock, char *buffer, int ret, char *dest) : reads from socket and stores in the destination variable
int verifyLogin(int *newsock,char *buffer,int ret, char *fileName, char *userType) : verifies login from faculty and student by matching username and password
int findNumReg(char *courseFileName, int linenum) : line 2 - max reg allowed, line 3 - current number of registrations         
int checkNRegister(char *courseFileName, char *studentname) : checks if the new registration will exceed the max allowed registration, if not it goes ahead and registers
int checkNDeRegister(char *courseFileName, char *studentname, char *stuFileName) : Deregisters
For both the above functions, a temp file is created where the changes are first made and then the original file is overwritten and the temp file is deleted.
void getCourseNFacultyNames(char *str, char *course, char *faculty) : finds faculty and course name from the registrations file

Student Functions:

int studentVerifyLogin(int *newsock,char *buffer,int ret) : verifies student login with utility function
int register4ACourse(int *newsock, char *buffer, int ret) : student can register for a course using This
int deregisterACourse(int *newsock, char *buffer, int ret) : deregisters from a course
int listAllRegisteredCourses(int *newsock, char *buffer, int ret) : lists the courses a student has taken

Faculty Functions:

int facultyVerifyLogin(int *newsock,char *buffer,int ret) : verifies faculty login with utility function
int addACourse(int *newsock, char *buffer, int ret) : adds a course for a particular faculty
int remACourse(int *newsock, char *buffer, int ret) : removes a course ONLY if total students registered in that course is 0
int listRegisteredStudents(int *newsock, char *buffer, int ret) : lists students registered for a course

Admin Functions:

int modifyUser(int *newsock,char *buffer,int ret, char *fileName, char *userType) : You are allowed to change the passoword and not the username
int modifyStudent(int *newsock,char *buffer,int ret) : uses modify user
int modifyFaculty(int *newsock,char *buffer,int ret) : uses modify user
int registerUser(int *newsock,char *buffer,int ret, char *passwdFileName, char *listFileName, int userType) : creates a user (takes in username and password)
int registerStudent(int *newsock,char *buffer,int ret) : uses register user
int registerFaculty(int *newsock,char *buffer,int ret) : uses regiser user
int switchUserActivation(int *newsock, char *buffer, int ret, char *fromFileName, char *toFileName, char *activationType) : uses a temporary file to make changes
int deActivateStudent(int *newsock,char *buffer,int ret) : fromFile is members.txt toFile is inactMembers.txt
int activateStudent(int *newsock,char *buffer,int ret) : fromFile is inactMembers.txt toFile is members.txt
int admin(int *newsock,char *buffer,int ret) : only 1 admin is allowed and the username for admin is admin and password for admin is also admin.

Usage:

Compile the server.c file using a C compiler:
gcc server.c -o server

Run the server:
./server

The server will listen for incoming connections. Clients can connect to the server.

Note
This server uses file locking to prevent race conditions when multiple clients are trying to make changes in course registrations or logging in as student or faculty at the same time. 
It also uses socket programming to communicate with clients.

client.c

This is a client-side application written in C for design and development of course registration portial (academia).
It allows clients to connect (using socket programming), login as admin, faulty or student. 
As a faculty, you can add and delete courses. As a student, you can select which course you want to opt under which faculty.
As an admin, you can add or deactivate students and faculties.

Functions:

ssize_t writeAll(int sockfd, const void *buf, size_t len) : error checks while writing the content into socket
int str2Int(const char *str, int *result) : removes \n and put \0 (it ensures the input is an integer for max registrations)
void studentMenu(int sock, char *name) : Prints the student menu from which options can be selected
void facultyMenu(int sock, char *name) : Prints the faculty menu from which options can be selected
void adminMenu(int sock) : Prints the admin menu from which options can be selected

Usage: 

Compile the client.c file using a C compiler:
gcc client.c -o client

Run the client:
./client

The client will connect to the server. Server will listen for incoming connections.

Note
This client uses socket programming to communicate with the server. 
It sends requests to the server and receives responses. The server handles all the operations related to registering faculties and students and registering courses.
