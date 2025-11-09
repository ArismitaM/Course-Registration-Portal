//OS Mini Project
//Name : Arismita Mukherjee
//Roll : IMT2023585
//Date : 15/05/2024

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#define PORT_NUM 10045
#define USER_FACULTY 1
#define USER_STUDENT 2

pthread_mutex_t lockMutex = PTHREAD_MUTEX_INITIALIZER; // lock for the Linked List of locks

typedef struct fileLock { // node of the lock LL
	char *path;
	pthread_mutex_t mutex;
	struct fileLock *next; 
} lockFile_t;

lockFile_t *fileLockHead = NULL; // head

lockFile_t *createALock(char *name) // creating a node in LL
{
	printf("Creating a new lock node for %s\n", name);
	lockFile_t *lock = (lockFile_t *)malloc(sizeof(lockFile_t));	
	if (lock == NULL){ 
		return NULL;
	}
	lock->path = strdup(name);
	if (lock->path == NULL) {
        perror("Failed to duplicate path string");
        free(lock);
        return NULL;
    }
	if (pthread_mutex_init(&lock->mutex, NULL) != 0) {
        perror("Failed to initialize mutex");
        free(lock->path);
        free(lock);
        return NULL;
    }
	lock->next = NULL;
	return lock;
}

lockFile_t *findTheLock(char *name) // searching for a node in LL
{
	if (fileLockHead == NULL) {
		fileLockHead = createALock(name);
		return fileLockHead;
	}
	lockFile_t *traverser = fileLockHead;
	lockFile_t *follower = fileLockHead;
	int found = 0;
	while (!found && traverser!= NULL) {
		if (strcmp(traverser->path, name) == 0) {
			printf("Found an existing lock node for %s\n", name);
			return traverser;
		} else {
			follower = traverser;
			traverser = traverser->next;
		}
	}
	if (!found) {
		// Create a new lock node and add it to the list
		traverser = createALock(name);
		follower->next = traverser;
	}
	return traverser;
}

// This first creates a lock on LL
// Then searches for the node in LL, if not present creates a node
// Then locks the file mutex and unlocks the lock in the LL
// Then makes changes in the file
// Then unlocks the file 
lockFile_t *lockAFile(char *path)
{
	printf("Lock up file: %s\n", path);
	// Guard the guard - make sure threads take this mutex lock
	// before they read/modify the mutex lock list.
	// Else, there is a chance of this very important DS to go
	// to the dogs. Whichever thread has gone in can safely
	// lock the file before coming out...
	pthread_mutex_lock(&lockMutex);
	lockFile_t *lock = findTheLock(path);
	pthread_mutex_lock(&(lock->mutex));
	pthread_mutex_unlock(&lockMutex);
	printf("Locked up file: %s\n", path);
	return lock;
}

void unLockAFile(lockFile_t *lock)
{
	printf("Unlocking file: %s\n", lock->path);
	pthread_mutex_unlock(&(lock->mutex));
}

FILE *openFile4Reading(char *fileName) // not always guaranteed that the file exists
{
    FILE *retPtr = fopen(fileName,"a+"); // ensures the file is created if it does not exist (that's why we use a+ -> append mode)
    if(retPtr==NULL) {
        perror("Error opening file - students.txt for reading!\n");
        exit(EXIT_FAILURE);
    }
	fclose(retPtr);
    retPtr = fopen(fileName,"r");
    if(retPtr==NULL) {
        perror("Error opening file - students.txt for reading!\n");
        exit(EXIT_FAILURE);
    }
	return retPtr;
}

int fileExists(const char *filename) { // checks if file exists or not
	printf("Existance check for Filename: %s\n", filename);
    if (access(filename, 0) == 0) {
		printf("Exists...\n");
        return 1; // File exists
    }
    if (errno == ENOENT) {
        return -1; // File does not exist
    }
    perror("Error checking file");
    return -1; // Other error
}

int createDirectories(const char *path) // creates directories
{
	char *path_copy = strdup(path);
    if (path_copy == NULL) {
        perror("Error allocating memory");
        return -1;
    }

    char *current = path_copy;
    char *slash;
	// Ignore the leading "/"s
	while (*current == '/') current++;
    char separator = '/';

	// Handle each component of the path
	while ((slash = strchr(current, separator)) != NULL || *current != '\0') {
		// make a component of the path ready to be analyzed by marking it
		// end with '\0' where the next slash is. So, from current to slash
		// is the component of the path we will be checking if it exists.
		// If not, we will create it
		// Note at the end of this analysis, we will set the component of the
		// path where slash is pointing to back to separator. So, always what
		// we analyze is the entire path - till the current location of slash
        if (slash != NULL) {
            *slash = '\0';
        }
		struct stat st;
        if (stat(path_copy, &st) != 0) {
            if (errno != ENOENT) { // This error is not of path not existant
                perror("Error checking directory");
                free(path_copy);
                return -1;
            }
            if (mkdir(path_copy, 0755) != 0 && errno != EEXIST) {
                perror("Error creating directory");
                free(path_copy);
                return -1;
            }
        } else if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s' exists but is not a directory.\n", path_copy);
            free(path_copy);
            return -1;
        }
		if (slash == NULL) break; // Last component handled
        *slash = separator;
        current = slash + 1;
    }
	free(path_copy);
    printf("Directory '%s' is ready.\n", path);
    return 0;
}

void readStrFromBuffer(int *newsock, char *buffer, int ret, char *dest)
{
    memset(buffer,0,sizeof(buffer));
    ret=read(*newsock,buffer,sizeof(buffer)-1);
    if(ret<0) {
        perror("Error reading from socket\n");
        exit(1);
    }
    buffer[ret]='\0';
    strcpy(dest,buffer);
}

int verifyLogin(int *newsock,char *buffer,int ret, char *fileName, char *userType)
{
    char username[100];
    char userPattern[200];
    char password[100];

	readStrFromBuffer(newsock, buffer, ret, username);
	readStrFromBuffer(newsock, buffer, ret, password);

	snprintf(userPattern, sizeof(userPattern), "%s %s", username, password); // matches username<space>password 
    char temp[100];
	printf("We will search for '%s' in the lines\n", userPattern);
	lockFile_t *lock =  lockAFile(fileName); // locking the file before reading
	printf("sleeping for 10s>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	sleep(10);
	printf("sleep done>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	FILE *file = openFile4Reading(fileName);
    while(fgets(temp, sizeof(temp), file)!=NULL) {
		//printf("Length of line: %ld, Line being verified: %s\n", strlen(temp), temp);
		//printf("Length of input: %ld, Input verified: %s\n", strlen(userPattern), userPattern);
		if (((strlen(temp) - 1) == strlen(userPattern)) &&
			(strncmp(temp, userPattern, strlen(userPattern)) == 0)) {
			fclose(file);
			unLockAFile(lock);
        	write(*newsock,"Login successful",17);
			if (strcmp(userType, "Student") == 0) {
        		printf("Student successfully logged in\n");
			} else {
        		printf("Faculty successfully logged in\n");
			}
			return(1);
		}
	}
	fclose(file);
	unLockAFile(lock);
    write(*newsock,"Login Failed",13);
	if (strcmp(userType, "Student") == 0) {
        printf("Student login failed\n");
	} else {
        printf("Faculty login failed\n");
	}
	return(1);
}

int findNumReg(char *courseFileName, int linenum) // line 2 - max reg allowed, line 3 - current number of registrations
{
	int retVal = -1, ret = -1;
	lockFile_t *lock = lockAFile(courseFileName);
    FILE *crsFile = fopen(courseFileName,"r");
	if (crsFile == NULL) {
		unLockAFile(lock);
        printf("Error opening file - %s for reading!\n", courseFileName);
		return -1;
	}
	char buffer[80];
    int line_number = 0;
	
    // Read the file line by line
    while (fgets(buffer, 80, crsFile) != NULL) {
        line_number++;

        // Check if we've reached the 3rd line
        if (line_number == linenum) {
            // Remove trailing newline (if any)
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }

			ret = sscanf(buffer, "%d", &retVal);
			if (ret != 1) {
				printf("3rd line does not contain a number\n");
				fclose(crsFile);
				unLockAFile(lock);
				return (-1);
			}
            break; // Exit after reading the 3rd line
        }
    }
	fclose(crsFile);
	unLockAFile(lock);
	return retVal;
}

// checks if the new registration will exceed the max allowed registration, 
// if not it goes ahead and registers
int checkNRegister(char *courseFileName, char *studentname) 
{
	int maxReg = findNumReg(courseFileName, 2);
	int currReg = findNumReg(courseFileName, 3);
	if (currReg >= maxReg) {
		return -1;
	}

	lockFile_t *lock = lockAFile(courseFileName);
	FILE *crsFile = fopen(courseFileName, "r");
	if (crsFile == NULL) {
		unLockAFile(lock);
		return -1;
	}
	char tmpFileName[110];
    char temp[80];
	snprintf(tmpFileName, sizeof(tmpFileName), "%s%s", "tmp", studentname); // make a temporary file to make the update the current number of registrations and then copy the temp file onto the course file 
    FILE *tmpFile = fopen(tmpFileName,"w+");
	if (tmpFile == NULL) {
        printf("Error opening file - %s for writing!\n", tmpFileName);
		fclose(crsFile);
		unLockAFile(lock);
        return -1;
	}
	int lineNo = 0;
    while(fgets(temp, sizeof(temp), crsFile)!=NULL) {
		lineNo++;
		if (lineNo == 3) {
            fprintf(tmpFile, "%d\n", currReg+1);
        } else {
    		fprintf(tmpFile,"%s",temp);
		}
    }
    fclose(tmpFile);
	fclose(crsFile);
	crsFile = fopen(courseFileName, "w");
	if (crsFile == NULL) {
		unLockAFile(lock);
		remove(tmpFileName);
		return -1;
	}
	tmpFile = fopen(tmpFileName, "r");
	if (tmpFile == NULL) {
		fclose(crsFile);
		unLockAFile(lock);
		remove(tmpFileName);
		return -1;
	}
    while(fgets(temp, sizeof(temp), tmpFile)!=NULL) {
    	fprintf(crsFile,"%s",temp);
    }
	fprintf(crsFile, "%s\n", studentname);
	fclose(tmpFile);
	fclose(crsFile);
	unLockAFile(lock);
	remove(tmpFileName);
	return 1;
}

int checkNDeRegister(char *courseFileName, char *studentname, char *stuFileName)
{
	int currReg = findNumReg(courseFileName, 3);
	if (currReg <= 0) {
		return -1;
	}

	lockFile_t *lock = lockAFile(courseFileName);
	FILE *crsFile = fopen(courseFileName, "r");
	if (crsFile == NULL) {
		unLockAFile(lock);
		return -1;
	}
	char tmpFileName[110];
    char temp[80];
	snprintf(tmpFileName, sizeof(tmpFileName), "%s%s", "tmp", studentname);
    FILE *tmpFile = fopen(tmpFileName,"w+");
	if (tmpFile == NULL) {
        printf("Error opening file - %s for writing!\n", tmpFileName);
		fclose(crsFile);
		unLockAFile(lock);
        return -1;
	}
	int lineNo = 0;
	int len = 0;
	int found = 0;
    while(fgets(temp, sizeof(temp), crsFile)!=NULL) {
		lineNo++;
		if (lineNo == 3) {
            fprintf(tmpFile, "%d\n", currReg-1);
        } else {
			len = strlen(temp);
			if (len > 0 && temp[len - 1] == '\n') {
				temp[len - 1] = '\0';
			}
			if (strncmp(temp, studentname, strlen(studentname)) == 0) {
				found = 1; // The student was registered for this course
				continue;
			} else {
				temp[len-1] = '\n';
    			fprintf(tmpFile,"%s",temp);
			}
		}
    }
    fclose(tmpFile);
	fclose(crsFile);
	if (found == 0) {
		// The username is not present in the list of registered students for this course
		// Nothing to be done.
		remove(tmpFileName);
		unLockAFile(lock);
		return -1;
	}
	crsFile = fopen(courseFileName, "w");
	if (crsFile == NULL) {
		remove(tmpFileName);
		unLockAFile(lock);
		return -1;
	}
	tmpFile = fopen(tmpFileName, "r");
	if (tmpFile == NULL) {
		fclose(crsFile);
		unLockAFile(lock);
		remove(tmpFileName);
		return -1;
	}
    while(fgets(temp, sizeof(temp), tmpFile)!=NULL) {
    	fprintf(crsFile,"%s",temp);
    }
	fclose(tmpFile);
	fclose(crsFile);
	unLockAFile(lock);
	remove(tmpFileName);
	// Now need to remove the course from the list of courses
	// registered by the user in the user space
	lock =  lockAFile(stuFileName);
	crsFile = fopen(stuFileName, "r");
	if (crsFile == NULL) {
		// As the user has been removed from the list of registered
		// students, the de-registration is successful
		unLockAFile(lock);
		return 1;
	}
    tmpFile = fopen(tmpFileName,"w+");
	if (tmpFile == NULL) {
        printf("Error opening file - %s for writing!\n", tmpFileName);
		fclose(crsFile);
		unLockAFile(lock);
        return 1;
	}
	found = 0;
    while(fgets(temp, sizeof(temp), crsFile)!=NULL) {
		len = strlen(temp);
		if (len > 0 && temp[len - 1] == '\n') {
			temp[len - 1] = '\0';
		}
		if (strncmp(temp, courseFileName, strlen(courseFileName)) == 0) {
			found = 1; // The student was registered for this course
			continue;
		} else {
			temp[len-1] = '\n';
    		fprintf(tmpFile,"%s",temp);
		}
    }
    fclose(tmpFile);
	fclose(crsFile);
	if (found == 0) {
		// The course wasnt in the list of courses registered by the user
		// in his registration file.
		return 1;
	}
	crsFile = fopen(stuFileName, "w");
	if (crsFile == NULL) {
		remove(tmpFileName);
		unLockAFile(lock);
		return -1;
	}
	tmpFile = fopen(tmpFileName, "r");
	if (tmpFile == NULL) {
		fclose(crsFile);
		unLockAFile(lock);
		remove(tmpFileName);
		return -1;
	}
    while(fgets(temp, sizeof(temp), tmpFile)!=NULL) {
    	fprintf(crsFile,"%s",temp);
    }
	fclose(tmpFile);
	fclose(crsFile);
	unLockAFile(lock);
	remove(tmpFileName);
	return 1;
}

void getCourseNFacultyNames(char *str, char *course, char *faculty)
{
	int count = 0;
	char *token = strtok(str, "/");
	while (token != NULL) {
		count++;
		printf("Token: %s\n", token);
		if (count == 2) {
			strcpy(faculty, token);
		} else if (count == 3) {
			strcpy(course, token);
		}
		token = strtok(NULL, "/");
	}
}

void tokenize(char *str, char **tokenized)
{
	int count = 0;
	char *token = strtok(str, ":");
	while (token != NULL) {
		printf("Token: %s\n", token);
		tokenized[count] = strdup(token);
		count++;
		token = strtok(NULL, ":");
	}
}

/******************************** Student Functions ******************************/
int studentVerifyLogin(int *newsock,char *buffer,int ret)
{
	return verifyLogin(newsock, buffer, ret, "students.txt", "Student");
}

int register4ACourse(int *newsock, char *buffer, int ret)
{
	char *tokenized[4];
	tokenize(buffer, tokenized);
    //char username[100];
    //char coursename[100];
	//char facultyname[100];
	char courseFileName[300];
	char stuRegFileName[300];
	//readStrFromBuffer(newsock, buffer, ret, username);
	//readStrFromBuffer(newsock, buffer, ret, coursename);
	//readStrFromBuffer(newsock, buffer, ret, facultyname);
	snprintf(courseFileName, sizeof(courseFileName), "faculty/%s/%s", tokenized[3], tokenized[2]);
	snprintf(stuRegFileName, sizeof(stuRegFileName), "student/%s/registration", tokenized[1]);
	lockFile_t *lock1 =  lockAFile(courseFileName);
	printf("sleeping>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	sleep(10);
	printf("done sleeping>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	if (fileExists(courseFileName) == -1) {
		unLockAFile(lock1);
    	write(*newsock,"Course does not exist",22);
    	printf("Course %s by faculty %s does not exist\n", tokenized[2], tokenized[3]);
		return (1);
	}
	unLockAFile(lock1);
	lockFile_t *lock =  lockAFile(stuRegFileName);
	FILE *stuFile = openFile4Reading(stuRegFileName);
	int registered = 0;
	char lineRead[80];
	size_t len = 0;
    while (fgets(lineRead, 80, stuFile) != NULL) {
		len = strlen(buffer);
		if (len > 0 && lineRead[len - 1] == '\n') {
			lineRead[len - 1] = '\0';
		}
		if (strncmp(lineRead, courseFileName, strlen(courseFileName)) == 0) {
			registered = 1;
			break;
		}
	}
	fclose(stuFile);
	unLockAFile(lock);
	if (registered == 1) {
    	write(*newsock,"You are already registered for this course",43);
    	printf("Student %s is already registered for this course\n", tokenized[1]);
		return (1);
	}
	int allowed2Register = checkNRegister(courseFileName, tokenized[1]);
	if (allowed2Register < 1) {
    	write(*newsock,"Max registrations for course reached - not allowed",51);
    	printf("Student %s not allowed to register - max registrations for course reached\n", tokenized[1]);
		return (1);
	}
	lock =  lockAFile(stuRegFileName);
	stuFile = fopen(stuRegFileName, "a");
	if (stuFile == NULL) {
        printf("Error opening file - %s for writing!\n", courseFileName);
		unLockAFile(lock);
        exit(EXIT_FAILURE);
	}
    fprintf(stuFile,"%s\n",courseFileName); // Contains the name of the course
	fclose(stuFile);
	unLockAFile(lock);
    write(*newsock,"Course Registration Success",28);
    printf("Student %s successfully registered for course: %s by faculty %s\n", tokenized[1], tokenized[2], tokenized[3]);
	for (int i = 0; i < 4; i++) {
		free(tokenized[i]);
	}
	return(1);
}


int deregisterACourse(int *newsock, char *buffer, int ret)
{
	char *tokenized[4];
	tokenize(buffer, tokenized);
	printf("User: %s, Course: %s, Faculty: %s\n", tokenized[1], tokenized[2], tokenized[3]);
    //char username[100];
    //char coursename[100];
	//char facultyname[100];
	char courseFileName[300];
	char stuRegFileName[300];
	//readStrFromBuffer(newsock, buffer, ret, username);
	//readStrFromBuffer(newsock, buffer, ret, coursename);
	//readStrFromBuffer(newsock, buffer, ret, facultyname);
	snprintf(courseFileName, sizeof(courseFileName), "faculty/%s/%s", tokenized[3], tokenized[2]);
	printf("CourseFileName: %s\n", courseFileName);
	snprintf(stuRegFileName, sizeof(stuRegFileName), "student/%s/registration", tokenized[1]);
	printf("CourseFileName: %s\n", stuRegFileName);
	lockFile_t *lock1 =  lockAFile(courseFileName);
	if (fileExists(courseFileName) == -1) {
		unLockAFile(lock1);
    	write(*newsock,"Course does not exist",22);
    	printf("Course %s by faculty %s does not exist\n", tokenized[2], tokenized[3]);
		return (1);
	}
	unLockAFile(lock1);
	lockFile_t *lock =  lockAFile(stuRegFileName);
	FILE *stuFile = openFile4Reading(stuRegFileName);
	int registered = 0;
	char lineRead[80];
	size_t len = 0;
    while (fgets(lineRead, 80, stuFile) != NULL) {
		len = strlen(buffer);
		if (len > 0 && lineRead[len - 1] == '\n') {
			lineRead[len - 1] = '\0';
		}
		if (strncmp(lineRead, courseFileName, strlen(courseFileName)) == 0) {
			registered = 1;
			break;
		}
	}
	fclose(stuFile);
	unLockAFile(lock);
	if (registered == 0) {
    	write(*newsock,"You are not registered for this course",39);
    	printf("Student %s is not registered for this course\n", tokenized[1]);
		return (1);
	}
	int deregisterStatus = checkNDeRegister(courseFileName, tokenized[1], stuRegFileName);
	if (deregisterStatus < 1) {
    	write(*newsock,"You are not registered for the course",38);
    	printf("Student %s is not registered for this course, cannot deregister\n", tokenized[1]);
		return (1);
	}
    write(*newsock,"Successfully de-registered from the course",43);
    printf("Student %s is not registered for this course, cannot deregister\n", tokenized[1]);
	for (int i = 0; i < 4; i++) {
		free(tokenized[i]);
	}
	return 1;
}

int listAllRegisteredCourses(int *newsock, char *buffer, int ret)
{
	char *tokenized[2];
	tokenize(buffer, tokenized);
    //char username[100];
	char stuRegFileName[300];
	char courseList[2000] = "";
	//readStrFromBuffer(newsock, buffer, ret, username);
	snprintf(stuRegFileName, sizeof(stuRegFileName), "student/%s/registration", tokenized[1]);
	printf("Registered Course File for %s is %s\n", tokenized[1], stuRegFileName);

	lockFile_t *lock =  lockAFile(stuRegFileName);
	FILE *stuFile = openFile4Reading(stuRegFileName);
	char lineRead[80];
	char courseName[20];
	char facultyName[20];
	char courseNFac[42];
	size_t len = 0;
    while (fgets(lineRead, 80, stuFile) != NULL) {
		len = strlen(lineRead);
		if (len > 0 && lineRead[len - 1] == '\n') {
			lineRead[len - 1] = '\0';
		}
		printf("Line Read len: %ld Line: %s\n", len, lineRead);
		getCourseNFacultyNames(lineRead, courseName, facultyName);
		snprintf(courseNFac, sizeof(courseNFac), "%s::%s ", facultyName, courseName);
		strcat(courseList, courseNFac);
		printf("Courses So Far-> %s\n", courseList);
	}

	fclose(stuFile);
	unLockAFile(lock);
	if (strlen(courseList) == 0) {
    	write(*newsock, "Not registered for any course yet", 34);
	} else {
    	write(*newsock,courseList,strlen(courseList));
	}
    printf("Student %s is registered for these courses: %s\n", tokenized[1], courseList);
	for (int i = 0; i < 2; i++) {
		free(tokenized[i]);
	}
	return 1;
}

/********************************* Faculty Functions ******************************/
int facultyVerifyLogin(int *newsock,char *buffer,int ret)
{
	return verifyLogin(newsock, buffer, ret, "faculties.txt", "Faculty");
}

int addACourse(int *newsock, char *buffer, int ret)
{
	printf("Adding a Course: %s\n", buffer);
	char *tokenized[4];
	tokenize(buffer, tokenized);
    //char username[100];
    //char coursename[100];
	//char maxstudents[4]; // we can keep this as str. Client has ensured its a number
	char courseFileName[300];
	/*
	readStrFromBuffer(newsock, buffer, ret, username);
	printf("Received username: %s\n", username);
	readStrFromBuffer(newsock, buffer, ret, coursename);
	printf("Received coursename: %s\n", coursename);
	readStrFromBuffer(newsock, buffer, ret, maxstudents);
	printf("Received maxStudents: %s\n", maxstudents);
	*/
	snprintf(courseFileName, sizeof(courseFileName), "faculty/%s/%s", tokenized[1], tokenized[2]);
	printf("The course File Directory: %s\n", courseFileName);
	lockFile_t *lock =  lockAFile(courseFileName);
	printf("sleeping>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	sleep(10);
	printf("done sleeping>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	if (fileExists(courseFileName) == 1) {
		unLockAFile(lock);
        write(*newsock,"Course already exists",22);
        printf("Course %s already exists\n", tokenized[2]);
		return (1);
	}
    FILE *crsFile = fopen(courseFileName,"w+");
	if (crsFile == NULL) {
		unLockAFile(lock);
        printf("Error opening file - %s for writing!\n", courseFileName);
        exit(EXIT_FAILURE);
	}
    fprintf(crsFile,"%s\n",tokenized[2]); // Contains the name of the course
    fprintf(crsFile,"%s\n",tokenized[3]); // Contains the max allowed registrations
    fprintf(crsFile,"0\n"); // Contains the number of registrations so far
	// At the end contains the list of students registered. When creating the
	// course - no students are registered
	fclose(crsFile);
	unLockAFile(lock);
    write(*newsock,"New Course Registered",22);
    printf("New Course Registered\n");
	for (int i = 0; i < 4; i++) {
		free(tokenized[i]);
	}
	return (1);
}

int remACourse(int *newsock, char *buffer, int ret)
{
	printf("Removing a course\n");
	char *tokenized[3];
	tokenize(buffer, tokenized);
    //char username[100]="";
    //char coursename[100]="";
	char courseFileName[300];
	//readStrFromBuffer(newsock, buffer, ret, username);
	//readStrFromBuffer(newsock, buffer, ret, coursename);
	snprintf(courseFileName, sizeof(courseFileName), "faculty/%s/%s", tokenized[1], tokenized[2]);
	lockFile_t *lock =  lockAFile(courseFileName);
	printf("sleeping>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	sleep(10);
	printf("done sleeping>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	if (fileExists(courseFileName) == -1) {
		unLockAFile(lock);
        write(*newsock,"Course does not exist",22);
        printf("Course %s does not exist\n", tokenized[2]);
		return (1);
	}
	unLockAFile(lock);
	int currReg = findNumReg(courseFileName, 3);
	if (currReg > 0) {
    	write(*newsock,"Course has registrations - cannot be removed",45);
    	printf("Course %s has %d registrations. Cannot be removed\n", tokenized[2], currReg);
		return (1);
	}
	lock =  lockAFile(courseFileName);
	remove(courseFileName);
	unLockAFile(lock);
    write(*newsock,"Course Removed",15);
    printf("Course %s Removed\n", tokenized[2]);
	for (int i = 0; i < 3; i++) {
		free(tokenized[i]);
	}
	return (1);
}

int listRegisteredStudents(int *newsock, char *buffer, int ret)
{
	char *tokenized[3];
	tokenize(buffer, tokenized);
    //char username[100];
    //char coursename[100];
	char courseFileName[300];
	char registeredStudents[2000] = "";
	//readStrFromBuffer(newsock, buffer, ret, username);
	//readStrFromBuffer(newsock, buffer, ret, coursename);
	snprintf(courseFileName, sizeof(courseFileName), "faculty/%s/%s", tokenized[1], tokenized[2]);
	if (fileExists(courseFileName) == -1) {
        write(*newsock,"Course does not exist",22);
        printf("Course %s does not exist\n", tokenized[2]);
		return (1);
	}
	int currReg = findNumReg(courseFileName, 3);
	if (currReg == 0) {
    	write(*newsock,"No Registration for course",27);
    	printf("Course %s has %d registrations\n", tokenized[2], currReg);
		return (1);
	}

	lockFile_t *lock =  lockAFile(courseFileName);
	FILE *crsFile = openFile4Reading(courseFileName);
	char lineRead[80];
	char studentName[81];
	size_t len = 0;
	int line = 0;
    while (fgets(lineRead, 80, crsFile) != NULL) {
		line++;
		if (line < 4) {
			continue;
		}
		len = strlen(lineRead);
		if (len > 0 && lineRead[len - 1] == '\n') {
			lineRead[len - 1] = '\0';
		}
		snprintf(studentName, sizeof(studentName), "%s ", lineRead);
		strcat(registeredStudents, studentName);
		printf("Students So Far-> %s\n", registeredStudents);
	}

	fclose(crsFile);
	unLockAFile(lock);
    write(*newsock, registeredStudents, strlen(registeredStudents));
    printf("Registered Students: %s\n", registeredStudents);
	for (int i = 0; i < 3; i++) {
		free(tokenized[i]);
	}
	return (1);
}

/********************************** Admin Functions *******************************/

int modifyUser(int *newsock,char *buffer,int ret, char *fileName, char *userType)
{
	char *tokenized[3];
	tokenize(buffer, tokenized);
	char tmpFileName[110];
    //char username[100];
    char userPattern[101];
    //char password[100];
	//readStrFromBuffer(newsock, buffer, ret, username);
	snprintf(userPattern, sizeof(userPattern), "%s%s", tokenized[1], " ");
	printf("We will search for '%s' in the lines\n", userPattern);
	//readStrFromBuffer(newsock, buffer, ret, password);

	lockFile_t *lock =  lockAFile(fileName);
    FILE *file = openFile4Reading(fileName);

    char temp[100];
    int found=0;
	snprintf(tmpFileName, sizeof(tmpFileName), "%s%s", "tmp", tokenized[1]);
    FILE *tmpFile = fopen(tmpFileName,"w+");
	if (tmpFile == NULL) {
		unLockAFile(lock);
        printf("Error opening file - %s for writing!\n", tmpFileName);
        exit(EXIT_FAILURE);
	}
    while(fgets(temp, sizeof(temp), file)!=NULL) {
        if(strncmp(temp,userPattern,strlen(userPattern))==0) {
            found=1;
        } else {
    		fprintf(tmpFile,"%s",temp);
		}
    }
    fclose(tmpFile);
	fclose(file);
    if(found==0) {
		if (strcmp(userType, "Student") == 0) {
        	write(*newsock,"Student does not exist",22);
        	printf("Student %s does not exist\n", tokenized[1]);
		} else {
        	write(*newsock,"Faculty does not exist",22);
        	printf("Faculty %s does not exist\n", tokenized[1]);
		}
		remove(tmpFileName);
		unLockAFile(lock);
        return 1;
    }

	// The student is present. So, now copy the contents of the tmpfile
	// to the students.txt after locking it. And then write the username and password
	// to it at the end.
    tmpFile = fopen(tmpFileName,"r");
	if (tmpFile == NULL) {
        printf("Error opening file - %s for reading!\n", tmpFileName);
		unLockAFile(lock);
        exit(EXIT_FAILURE);
	}
    file = fopen(fileName,"w+");
    if(file==NULL)
    {
        perror("Error opening file - students.txt!\n");
        exit(EXIT_FAILURE);
    }
    while(fgets(temp, sizeof(temp), tmpFile)!=NULL) {
    	fprintf(file,"%s",temp);
    }
    fclose(tmpFile);
	unLockAFile(lock);
	remove(tmpFileName);
    fprintf(file,"%s %s\n",tokenized[1],tokenized[2]);
	fclose(file);
	
    write(*newsock,"Updated",8);
    printf("Updated\n");
	for (int i = 0; i < 3; i++) {
		free(tokenized[i]);
	}
    return 1;
}

int modifyStudent(int *newsock,char *buffer,int ret)
{
	return modifyUser(newsock, buffer, ret, "students.txt", "Student");
}

int modifyFaculty(int *newsock,char *buffer,int ret)
{
	return modifyUser(newsock, buffer, ret, "faculties.txt", "Faculty");
}


int registerUser(int *newsock,char *buffer,int ret, char *passwdFileName, char *listFileName, int userType)
{
    char username[100];
    char password[100];
	readStrFromBuffer(newsock, buffer, ret, username);
	readStrFromBuffer(newsock, buffer, ret, password);
	lockFile_t *lock =  lockAFile(listFileName);
	printf("sleeping>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	sleep(10);
	printf("done sleeping>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    FILE *members = openFile4Reading(listFileName);
    char temp[100];
    int found=0;
    while(fscanf(members,"%s",temp)!=EOF) {
        if(strcmp(temp,username)==0) {
            found=1;
            break;
        }
    }
    if(found==1) {
        write(*newsock,"User already exists",20);
    	fclose(members);
		unLockAFile(lock);
        return 1;
    }
	char dirPath[110];
	if (userType == USER_STUDENT) {
		snprintf(dirPath, sizeof(dirPath), "student/%s", username);
	} else {
		snprintf(dirPath, sizeof(dirPath), "faculty/%s", username);
	}
	if (createDirectories(dirPath) == -1) {
        write(*newsock,"User path could not be created",30);
    	fclose(members);
		unLockAFile(lock);
        return 1;
	}
    fclose(members);
    members = fopen(listFileName,"a+");
    if(members==NULL)
    {
        perror("Error opening file!\n");
		unLockAFile(lock);
        exit(EXIT_FAILURE);
    }
    fprintf(members,"%s\n",username);
    fclose(members);
	unLockAFile(lock);

	lock =  lockAFile(passwdFileName);
    FILE *students = fopen(passwdFileName,"a+");
    if(students==NULL) {
        printf("Error opening file - %s for reading!\n", passwdFileName);
		unLockAFile(lock);
        exit(EXIT_FAILURE);
    }
    fprintf(students,"%s %s\n",username,password);
    fclose(students);
	unLockAFile(lock);

    write(*newsock,"Registered",11);
    printf("Registered\n");
    return 1;
}

int registerStudent(int *newsock,char *buffer,int ret)
{
	return registerUser(newsock, buffer, ret, "students.txt", "members.txt", USER_STUDENT);
}

int registerFaculty(int *newsock,char *buffer,int ret)
{
	return registerUser(newsock, buffer, ret, "faculties.txt", "facMembers.txt", USER_FACULTY);
}

int switchUserActivation(int *newsock, char *buffer, int ret, char *fromFileName, char *toFileName, char *activationType)
{
	char tmpFileName[110];
    char username[100];
	readStrFromBuffer(newsock, buffer, ret, username);
	lockFile_t *lock = lockAFile(fromFileName);
	printf("sleeping>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	sleep(10);
	printf("done sleeping>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    FILE *fromMembers = openFile4Reading(fromFileName);
    char temp[100];
    int found=0;
	snprintf(tmpFileName, sizeof(tmpFileName), "%s%s", "tmp", username);
    FILE *tmpFile = fopen(tmpFileName,"w+");
	if (tmpFile == NULL) {
		unLockAFile(lock);
        printf("Error opening file - %s for writing!\n", tmpFileName);
        exit(EXIT_FAILURE);
	}
	// Read the activeMembers list 1 name at a time. If not matching - write
	// to tmpFile. If matches, do not write that name to tmpFile.
	// At the end - tmpFile will contain the new list of inactive students -
	// will not contain the name of this student to be activated.
    while(fgets(temp, sizeof(temp), fromMembers)!=NULL) {
		//printf("Active Member read: %s\n", temp);
        if(strncmp(temp,username,strlen(username))==0) {
            found=1;
        } else {
    		fprintf(tmpFile,"%s",temp);
		}
    }
    fclose(tmpFile);
	fclose(fromMembers);
	unLockAFile(lock);
    if(found==0) {
		if (strcmp(activationType, "Deactivate") == 0) {
        	write(*newsock,"Student is not active",22);
        	printf("Student %s is not active\n", username);
		} else {
        	write(*newsock,"Student is not Inactive",24);
        	printf("Student %s is not Inactive\n", username);
		}
		remove(tmpFileName);
        return 1;
    }

	// Update the fromList after removing the user 
	// from the list - which is now in tmpFile
    tmpFile = fopen(tmpFileName,"r");
	if (tmpFile == NULL) {
        printf("Error opening file - %s for reading!\n", tmpFileName);
        exit(EXIT_FAILURE);
	}
	lock =  lockAFile(fromFileName);
    fromMembers = fopen(fromFileName,"w");
    if(fromMembers==NULL)
    {
        perror("Error opening file - members.txt!\n");
        exit(EXIT_FAILURE);
    }
    while(fgets(temp, sizeof(temp), tmpFile)!=NULL) {
    	fprintf(fromMembers,"%s",temp);
    }
    fclose(tmpFile);
	remove(tmpFileName);
	fclose(fromMembers);
	unLockAFile(lock);
	
	// Add the username to inactive students list file, after locking
	// the list of active students file
	lock =  lockAFile(toFileName);
    FILE *toMembers = fopen(toFileName,"a");
    if(toMembers==NULL)
    {
        perror("Error opening file!\n");
		unLockAFile(lock);
        exit(EXIT_FAILURE);
    }
    fprintf(toMembers,"%s\n",username);
    fclose(toMembers);
	unLockAFile(lock);
	if (strcmp(activationType, "Deactivate") == 0) {
        write(*newsock,"Deactivated",12);
        printf("Deactivated\n");
	} else {
        write(*newsock,"Activated",10);
        printf("Activated\n");
	}
    return 1;
}

int deActivateStudent(int *newsock,char *buffer,int ret)
{
	return switchUserActivation(newsock, buffer, ret, "members.txt", "inactMembers.txt", "Deactivate");
}

int activateStudent(int *newsock,char *buffer,int ret)
{
	return switchUserActivation(newsock, buffer, ret, "inactMembers.txt", "members.txt", "Activate");
}


int admin(int *newsock,char *buffer,int ret)
{
    printf("Inside admin Verification\n");
    char password[100];
    int result = read(*newsock, password, sizeof(password) - 1);  
    if (result < 0) 
    {
        perror("Error reading from socket");
        return -1;
    }
    
    password[result] = '\0';
    printf("Password: %s\n",password);
    if(strcmp(password,"admin")==0)
    {
        printf("Admin login successful\n");
        write(*newsock,"Login successful",17);
    }
    else
        write(*newsock,"Login failed",13);
    return 1;
}

void *operator(void *arg)
{
    int nsd = *(int*)arg;
	char buffer[1024];
    int ret;
    while(1)
    {
        ret=read(nsd,buffer,sizeof(buffer)-1);

        if(ret<=0)
        {
            if (ret < 0)
                perror("Error reading from socket\n");
            else
                printf("Client closed the connection\n");

            close(nsd);
            break;
        }

        buffer[ret]='\0';
        printf("User choice: %s\n",buffer);
        if(strncmp(buffer,"1", strlen("1"))==0)
        {
            admin(&nsd,buffer,ret);
        } else if(strncmp(buffer,"Adm1", strlen("Adm1"))==0) {
            registerStudent(&nsd,buffer,ret);
        } else if(strncmp(buffer,"Adm2", strlen("Adm2"))==0) {
            activateStudent(&nsd,buffer,ret);
        } else if(strncmp(buffer,"Adm3", strlen("Adm3"))==0) {
            deActivateStudent(&nsd,buffer,ret);
        } else if(strncmp(buffer,"Adm4", strlen("Adm4"))==0) {
            modifyStudent(&nsd,buffer,ret);
        } else if(strncmp(buffer,"Adm5", strlen("Adm5"))==0) {
            registerFaculty(&nsd,buffer,ret);
        } else if(strncmp(buffer,"Adm6", strlen("Adm6"))==0) {
            modifyFaculty(&nsd,buffer,ret);
        } else if(strcmp(buffer,"2")==0) {
            facultyVerifyLogin(&nsd,buffer,ret);
        } else if(strncmp(buffer,"Fac1", strlen("Fac1"))==0) {
            addACourse(&nsd,buffer,ret);
        } else if(strncmp(buffer,"Fac2", strlen("Fac2"))==0) {
            remACourse(&nsd,buffer,ret);
        } else if(strncmp(buffer,"Fac3", strlen("Fac3"))==0) {
            listRegisteredStudents(&nsd,buffer,ret);
        } else if(strcmp(buffer,"3")==0) {
            studentVerifyLogin(&nsd,buffer,ret);
        } else if(strncmp(buffer,"Stu1", strlen("Stu1"))==0) {
            register4ACourse(&nsd,buffer,ret);
        } else if(strncmp(buffer,"Stu2", strlen("Stu2"))==0) {
            deregisterACourse(&nsd,buffer,ret);
        } else if(strncmp(buffer,"Stu3", strlen("Stu3"))==0) {
			listAllRegisteredCourses(&nsd,buffer,ret);
        } else if(strcmp(buffer,"end")==0) {
            close(nsd);
            printf("Client closed the connection\n");
            return NULL;
        }

    }
    close(nsd);
    free(arg);
    return NULL;
}

int main()
{
    struct sockaddr_in server, cli,clientconnection; //serv==server, cli==clientconnection
    int sock, client_sock,newsock, c, read_size; //sd==sock and nsd==newsock
    char client_message[2000];
    char buffer[1024];
    int ret;

    printf("Server starting\n");

    sock = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server, sizeof(server));

    if (sock == -1)
    {
        perror("Could not create socket\n");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT_NUM);

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Bind failed\n");
        exit(1);
    }

    if(listen(sock, 50) < 0)
    {
        perror("Listen failed\n");
        exit(1);
    }
    printf("Server started... listening on port %d\n", ntohs(server.sin_port));
    while(1)
    {
        int len = sizeof(cli);
        int *nsd = malloc(sizeof(int));
        *nsd = accept(sock, (struct sockaddr*)&cli, &len);//accept connection ;sock is sd
        pthread_t tid;
	printf("Creating a new thread to handle a client request.......\n");
        pthread_create(&tid, NULL, operator, (void*)nsd);//create a thread for each client
        //Creating a thread for each client so that the server can handle multiple clients at the same time
    }
    close(sock);
    return 0;
}
