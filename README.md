# Final-Project-026
# `Network File System (NFS) Project`

## Overview

Welcome to the Network File System (NFS) project! This project aims to implement a distributed file system that enables seamless access to files over a network. The system is composed of three major components: Clients, Naming Server, and Storage Servers.

## Components

### Clients

- **Role:** Represents systems or users requesting access to files within the network file system.
- **Functionality:** Initiates file-related operations such as reading, writing, and deleting.
- **Importance:** Serves as the primary interface for interacting with the NFS.

### Naming Server

- **Role:** Central hub orchestrating communication between clients and storage servers.
- **Functionality:** Acts as a directory service, providing clients with information about the specific storage server where a requested file or folder is located.
- **Importance:** Ensures efficient and accurate file access by maintaining a directory of file locations.

### Storage Servers

- **Role:** Responsible for the physical storage and retrieval of files and folders.
- **Functionality:** Manages data persistence, distribution, and security across the network.
- **Importance:** Forms the foundation of the NFS by handling the actual storage and retrieval of files, ensuring data is stored securely and efficiently.

## Getting Started

To get started with the NFS project, follow the setup instructions in the respective directories for Clients, Naming Server, and Storage Servers.

# NFS File Operations

Within the NFS ecosystem, clients enjoy a suite of essential file operations, enabling seamless interaction with the network file system:

1. **Writing a File/Folder:**
   - *Description:* Clients can actively create and update the content of files and folders within the NFS. This operation encompasses the storage and modification of data, ensuring that the NFS remains a dynamic repository.

2. **Reading a File:**
   - *Description:* Reading operations empower clients to retrieve the contents of files stored within the NFS. This fundamental operation grants clients access to the information they seek.

3. **Deleting a File/Folder:**
   - *Description:* Clients retain the ability to remove files and folders from the network file system when they are no longer needed, contributing to efficient space management.

4. **Creating a File/Folder:**
   - *Description:* The NFS allows clients to generate new files and folders, facilitating the expansion and organization of the file system. This operation involves the allocation of storage space and the initialization of metadata for the newly created entities.

5. **Listing All Files and Folders in a Folder:**
   - *Description:* Navigating the NFS structure becomes effortless for clients as they can retrieve comprehensive listings of files and subfolders within a specified directory. This feature aids in efficient file system exploration and management.

6. **Getting Additional Information:**
   - *Description:* Clients can access a wealth of supplementary information about specific files. This includes details such as file size, access rights, timestamps, and other metadata, providing clients with comprehensive insights into the files they interact with.

The exact specifics of the operations that we have implemented are elaborated below.


# NFS Specifications

## 1. Naming and Storage Servers

### 1.1 Initialization

The process of initializing the Naming Server (NM) and Storage Servers (SS) establishes the foundation for the entire network file system. Here are the key steps involved in starting the system:

#### Initialize the Naming Server (NM):

- **Description:** The first step is to initialize the Naming Server, which serves as the central coordination point in the NFS. It manages the directory structure and maintains essential information about file locations.

#### Initialize Storage Server 1 (SS_1):

- **Description:** Initialization of SS_1 involves several sub-steps:
  - Upon initialization, SS_1 sends vital details about its existence to the Naming Server. This includes:
    - IP address: To facilitate communication and location tracking.
    - Port for NM Connection: A dedicated port for direct communication with the Naming Server.
    - Port for Client Connection: A separate port for clients to interact with SS_1.
    - List of Accessible Paths: A comprehensive list of file and folder paths that are accessible on SS_1.

#### Initialize SS_2 to SS_n:

- **Description:** Following the same procedure as SS_1, additional Storage Servers (SS_2 to SS_n) are initialized, each providing their details to the Naming Server.

#### NM Starts Accepting Client Requests:

- **Description:** Once all Storage Servers are initialized and registered with the Naming Server, the Naming Server begins accepting client requests. It becomes the central point through which clients access and manage files and directories within the network file system.

### 1.2 On Storage Servers (SS)

The Storage Servers play a crucial role in the network file system and are equipped with the following functionalities:

#### Adding New Storage Servers:

- **Description:** New Storage Servers, which begin running after the initial initialization phase, have the capability to dynamically add their entries to the Naming Server at any point during execution.

#### Commands Issued by NM:

- **Description:** The Naming Server can issue specific commands to the Storage Servers, including:
  - Create an Empty File/Directory: Instructs a Storage Server to create an empty file or directory, initiating the storage of new data.
  - Delete a File/Directory: Storage Servers can receive commands to delete files or directories, freeing up storage space and maintaining data consistency.
  - Copy Files/Directories: Storage Servers can copy files or directories from other Storage Servers, with the NM providing relevant IP addresses for efficient data transfer.

#### Client Interactions:

- **Description:** Storage Servers facilitate client interactions by providing the following functionalities:
  - Read a File: Clients can request Storage Servers to retrieve and send the content of a specific file, allowing seamless data access.
  - Write to a File: Clients can send data to Storage Servers, which is then written to the designated file, ensuring data updates and modifications are correctly stored.
  - Get Size and Permissions: Clients can query Storage Servers to retrieve essential file information such as size and access permissions, aiding in file management.

### 1.3 On Naming Server (NM)

#### Storing Storage Server Data:

- **Description:** One of the fundamental functions of the Naming Server (NM) is to serve as the central repository for critical information provided by Storage Servers (SS) upon connection.

#### Client Task Feedback:

- **Description:** Upon completion of tasks initiated by clients (as described in Specification 2), the NM plays a pivotal role in providing timely and relevant feedback to the requesting clients.



## 2. Concurrent Client Access

Our NFS design is designed to accommodate multiple clients attempting to access the Naming Server (NM) simultaneously. To ensure a smooth experience, the NM responds to client requests with an initial ACK to acknowledge the receipt of the request. The following steps outline the concurrent client access mechanism:

### 2.1 Initial ACK for Client Requests

- **Description:** The Naming Server (NM) responds to client requests with an initial ACK to acknowledge the receipt of the request.
- **Timeout Handling:** If this initial ACK is not received within a reasonable timeframe, the client may display a timeout message.
- **Non-Blocking Processing:** Importantly, the NM should not block while processing operations specified in parts 2 and 3 of Specification 2.
- **ACK Mechanism:** An initial ACK and a final ACK from the relevant Storage Server (SS) can be employed, allowing the NM to handle other requests between these acknowledgments.

## 3. Concurrent File Reading

While multiple clients can read the same file simultaneously, there are specific considerations for file write operations. The following guidelines define the behavior of concurrent file reading and writing:

### 3.1 Concurrent File Reading

- **Description:** Multiple clients can read the same file simultaneously.
- **ACK Mechanism:** The NM ensures that an initial ACK is sent to acknowledge the receipt of the read request.
- **Non-Blocking Processing:** The NM remains non-blocking during concurrent file reading operations.

### 3.2 File Write Operations

- **Description:** Only one client can execute write operations on a file at any given time.
- **Exclusive Write Access:** Write operations are exclusive to a single client at a time to maintain data consistency.
- **ACK Mechanism:** The NM acknowledges the receipt of a write request and ensures that the operation is completed before allowing another client to write to the same file.


## 4. SEARCH IN NAMING SERVERS

Efficient Search: Optimize the search process employed by the Naming Server when serving client requests. Avoid linear searches and explore more efficient data structures such as Tries and Hashmaps to swiftly identify the correct Storage Server (SS) for a given request. This optimization enhances response times, especially in systems with a large number of files and folders.

### 4.1 Optimized Search Process

- **Description:** Improve the efficiency of the search process used by the Naming Server (NM) when handling client requests.
- **Optimized Data Structures:** Avoid linear searches and explore more efficient data structures such as Tries and Hashmaps.
- **Swift Identification of SS:** Ensure the Naming Server can swiftly identify the correct Storage Server (SS) for a given client request.
- **Response Time Enhancement:** This optimization aims to enhance response times, particularly in systems with a large number of files and folders.

LRU Caching: Implement LRU (Least Recently Used) caching for recent searches. By caching recently accessed information, the NM can expedite subsequent requests for the same data, further improving response times and system efficiency.

### 4.2 LRU Caching Implementation

- **Description:** Implemented LRU (Least Recently Used) caching for recently accessed information in the Naming Server.
- **Caching Benefits:** Caching allows the NM to store and quickly retrieve recently accessed data.
- **Expedited Subsequent Requests:** Subsequent client requests for the same data are expedited, leading to improved response times.
- **System Efficiency:** LRU caching contributes to overall system efficiency by reducing redundant search operations.



## 5. FAILURE DETECTION

### 5.1 Storage Server (SS) Failure Detection

- **Description:**
  - The Naming Server (NM) has been equipped to detect Storage Server (SS) failures.
- **Prompt Response:**
  - This capability ensures that the NFS can respond promptly to any disruptions in SS availability.
- **Monitoring Mechanism:**
  - Implemented a monitoring mechanism to detect SS failures and trigger appropriate actions.

## 6. DATA REDUNDANCY AND REPLICATION

### 6.1 Redundancy and Replication Strategy

- **Description:**
  - Implemented a redundancy and replication strategy for data stored within the NFS.
- **Duplicated Storage:**
  - Duplicated every file and folder in an SS in two other SS (once the number of SS exceeds two).
- **Read-Only Operations:**
  - At this stage, only read operations is being allowed on replicated stores.
- **SS Failure Handling:**
  - In the event of an SS failure, the NM would retrieve the requested data from one of the replicated stores.

### 6.2 SS Recovery

- **Description:**
  - When an SS comes back online (reconnects to the NM),we have matched the duplicated stores back to the original SS.
- **No New Entries During Recovery:**
  - We have ensured that no new entries are added to the list of accessible files and folders in the NM during the SS recovery process.

## 7. ASYNCHRONOUS DUPLICATION

### 7.1 Asynchronous Duplication

- **Description:**
  - Every write command is being duplicated asynchronously across all replicated stores.
- **Fault Tolerance:**
  - The NM does not wait for acknowledgment but ensures that data is redundantly stored for fault tolerance.
- **Asynchronous Process:**
  - Implemented an asynchronous process for duplication to avoid delays in the write operations.



# Contributors

A big thank you to all the wonderful people who contributed to this project!

<!-- Add contributors' names and, if applicable, their GitHub profiles or other relevant links -->

- [Tanishq agarwal](https://github.com/johndoe) (2022101060)
- [Gopal Garg](https://github.com/janesmith) (2022101079)
- [Sahil Patel](https://github.com/bobjohnson) (2022101046)

<!-- Feel free to add more contributors as needed -->






