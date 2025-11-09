# Course Registration Portal (Academia)
### Operating Systems Mini Project  
**Name:** Arismita Mukherjee  
**Roll Number:** IMT2023585  
**Batch:** IMT2023  
**Date of Submission:** 15th May 2025  

---

## ✅ Overview

This project implements a **client–server based Course Registration Portal** using **C**, **socket programming**, **file handling**, and **mutex-based locking** to ensure safe concurrent access.

It supports **three user roles**:

- **Admin** – Add/Deactivate/Activate students & faculty  
- **Faculty** – Add/Delete courses, List registered students  
- **Student** – Register/Deregister courses, View registered courses  

The server uses file-level locking and temporary-file rewrite strategies to maintain consistency during concurrent operations.

---

## ✅ Features

### 🔐 Concurrency & Safety
- Custom **linked list of mutex locks**, one per file  
- Prevents race conditions when multiple clients modify course or user data  

### 👥 User Management (Admin)
- Add students/faculty  
- Activate/Deactivate students  
- Modify passwords (usernames fixed)  

### 🎓 Student Powers
- Login  
- Register for a course  
- Deregister from a course  
- List registered courses  

### 👨‍🏫 Faculty Powers
- Login  
- Add a new course  
- Remove a course (only if 0 students registered)  
- List registered students  

---

## ✅ File Structure

students.txt -> student username + password
members.txt -> active students
inactMembers.txt -> inactive students

faculties.txt -> faculty username + password
facMembers.txt -> active faculty list

faculty/<facultyName>/course.txt ->
<course name>
<max registrations>
<current registrations>
<student1>
<student2>
...

student/<studentName>/registration ->
faculty/<facName>/<course1>
faculty/<facName>/<course2>


---

## ✅ Locking Mechanism (Important)

Each file has its own **mutex**, stored inside a Linked List.  
This prevents two clients from editing the same file at once.

### 🔄 Lock Workflow
1. Lock the **Linked List**  
2. Search for file lock node  
3. If missing → create the node  
4. Lock the **file mutex**  
5. Unlock the Linked List  
6. Perform file operations  
7. Unlock the file mutex  

---

## ✅ Server (`server.c`)

The server manages:
- User authentication  
- Student, faculty, admin actions  
- Course registration logic  
- File locking  
- Safe temp-file updates  
- Directory creation  
- Socket communication  

### 🔧 Utility Functions
- `createALock()` – Create lock node  
- `findTheLock()` – Search in lock list  
- `lockAFile()` – Complete lock acquisition  
- `unLockAFile()` – Unlock file  
- `openFile4Reading()` – Safe file open  
- `fileExists()` – File existence check  
- `createDirectories()` – Ensure directory structure  
- `readStrFromBuffer()` – Read from socket  
- `verifyLogin()` – Validate login credentials  
- `findNumReg()` – Get max / current registrations  
- `checkNRegister()` – Safe registration using temp file  
- `checkNDeRegister()` – Safe deregistration  
- `getCourseNFacultyNames()` – Parse registration entry  

### 👩‍🎓 Student Functions
- `studentVerifyLogin()`  
- `register4ACourse()`  
- `deregisterACourse()`  
- `listAllRegisteredCourses()`  

### 👨‍🏫 Faculty Functions
- `facultyVerifyLogin()`  
- `addACourse()`  
- `remACourse()`  
- `listRegisteredStudents()`  

### 👨‍💼 Admin Functions
- `modifyUser()`  
- `modifyStudent()`  
- `modifyFaculty()`  
- `registerUser()`  
- `registerStudent()`  
- `registerFaculty()`  
- `switchUserActivation()`  
- `deActivateStudent()`  
- `activateStudent()`  
- `admin()` — Admin login (username: `admin`, password: `admin`)  

---

## ✅ Client (`client.c`)

Handles user input, menus, and communication with the server.

### 🔧 Client Functions
- `writeAll()` – Safe write to socket  
- `str2Int()` – Validate integer input  
- `studentMenu()` – Student options  
- `facultyMenu()` – Faculty options  
- `adminMenu()` – Admin options  

---


Socket programming (client–server communication)
Modular system design for reliability and safety
