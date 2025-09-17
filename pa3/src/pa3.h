#ifndef PA3_H
#define PA3_H

// You are welcome to use any and all methods within these headers, but you should not add any others.
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * You will notice that most of the below functions return an error_code as opposed to a normal value like
 * an int or a pointer. This is quite standard practice in C. Many standard library functions return
 * a number corresponding to a macro-defined error code, or return a non-zero value to indicate an error.
 * 
 * Expect your functions to use all of these, likely multiple times each.
 */
enum error_code {
    ERROR_NONE, // Successful completion
    ERROR_MALLOC_FAIL, // Some call to malloc() failed
    ERROR_NOT_FOUND, // The desired node could not be found at the specified location
    ERROR_MISSING_PERMISSIONS, // An operation was attempted on a file for which there were not sufficient permissions
    ERROR_INVALID_OPERATION, // Something about the attempted operation was invalid
    ERROR_ALREADY_EXISTS // Tried to create something that already exists
};

#define PATH_SEPARATOR '/'
#define MAX_NAME_LENGTH 256u
#define MAX_NUM_CHILDREN_PER_NODE 8u

#define P_FILE_BIT  (1u << 1) // 1 means the given node is a file, 0 means directory
#define P_READ_BIT  (1u << 2) // 1 means the given node is readable, 0 means it is not
#define P_WRITE_BIT (1u << 3) // 1 means the given node is writable, 0 means it is not

/* Layout of the provided binary files:
 *
 * node 0: +-------------------------+
 * (root)  | uint8_t props	     |
 *	   +-------------------------+
 *	   | uint8_t num_children    |
 *	   +-------------------------+
 *	   | char name[]             |	This region contains a C string of 
 *	   |		   	     |	length MAX_NAME_LENGTH in every node.
 *	   |        [padding]        |
 *	   +-------------------------+
 *	   | uint16_t child_index[]  |	This region contains an array of uint16_ts
 *	   |			     |	of length MAX_NUM_CHILDREN_PER_NODE in every node.
 *     |        [padding]        |  Ignore indices past num_children.
 * node 1: +-------------------------+
 *	   |           ...           |
 *	   +-------------------------+
 */

// The definition of a filesystem node.
struct fs_node {
    uint8_t props;
    uint8_t num_children;
    char *name;
    struct fs_node **child_list; // An array of pointers
};

// Print a given node's data
void pa3_print_node_data(const struct fs_node * const node);

// Print a tree's data in a digestible format. You are encouraged to make use of this in your testing!
void pa3_print_tree(const struct fs_node * const root);

/**
 * pa3_extract_node_name |
 * 
 * Functionality:
 *     - extract a series of bytes into a string
 * 
 * Accepts:
 *     - a double pointer to a char, meant to be filled in with the string
 *     - a pointer to the beginning of the region containing the bytes making up the node's name
 * 
 * Returns:
 *     - an error code, ERROR_NONE if successful else some other error
 * 
 * Assumptions:
 *     - the bytes in name_data are properly formatted as described above
 * 
 * Important notes:
 *     - remember: add 1 to a C-string's length to get its size in bytes (to account for the null-terminator)
 */
enum error_code pa3_extract_node_name(char **name, char *name_data);

/**
 * pa3_node_data_by_index |
 * 
 * Functionality:
 *     - return a pointer to a specific node's data in the bytes, by index
 * 
 * Accepts:
 *     - the node's index
 *     - a pointer to the beginning of the region containing the bytes read from the file
 * 
 * Returns:
 *     - a void pointer to the beginning of the region containing the node's data
 */
void *pa3_node_data_by_index(uint16_t idx, void *data);

/**
 * pa3_deserialize_node |
 * 
 * Functionality:
 *     - transform the binary data read from the file into a tree of fs_node structs
 * 
 * Accepts:
 *     - a double pointer to an fs_node, meant to be filled in with the deserialized node
 *     - an index, which corresponds to the node's index in the list in the binary file, as shown above
 *     - a void pointer to data, which are the bytes of the binary file
 * 
 * Returns:
 *     - an error code, ERROR_NONE if successful else some other error
 *         > the set of error codes you may need to return is defined in the enum, but
 * 	     particularly you may wish to look at ERROR_MALLOC_FAIL here
 * 
 * Assumptions:
 *     - the bytes in data are properly formatted as described above
 *     - 0 <= idx < (the total number of nodes)
 *     - none of the passed in arguments will be NULL
 * 
 * Important notes:
 *     - consider using calloc, since it may be desirable that some pointers are NULL
 */
enum error_code pa3_deserialize_node(struct fs_node **node, uint16_t idx, void *data);

/**
 * pa3_free_tree |
 * 
 * Functionality:
 *     - free the trees you create with deserialize_node
 * 
 * Accepts:
 *     - a pointer to an fs_node, the root of the tree you want to free
 * 
 * Important notes:
 *     - remember that the order in which you free things may be important
 */
void pa3_free_tree(struct fs_node *root);

/**
 * pa3_navigate_to_node |
 * 
 * Functionality:
 *     - navigate the filesystem tree to a specific node, and insert it into found
 * 
 * Accepts:
 *     - a pointer to an fs_node, the root of the tree you wish to navigate
 *     - a double pointer to an fs_node, where the result should go
 *     - an absolute path to the node ( /path/to/node )
 * 
 * Returns:
 *     - an error code, ERROR_NONE if successful else some other error
 * 
 * Important notes:
 *     - a string's length and its size (or width) are not necessarily the same thing
 */
enum error_code pa3_navigate_to_node(struct fs_node *root, struct fs_node **found, const char *path);

/**
 * pa3_remove_child_from_child_list |
 * 
 * Functionality:
 *     - properly remove a child from a node's child_list
 * 
 * Accepts:
 *     - a pointer to an fs_node, the parent whose child you wish to delete
 *     - the index of the child you wish to delete
 * 
 * Returns:
 *     - an error code, ERROR_NONE if successful else some other error
 * 
 * Important notes:
 *     - careful with this one!
 */
enum error_code pa3_remove_child_from_child_list(struct fs_node *parent, int child_idx);

/**
 * pa3_rm |
 * 
 * Functionality:
 *     - equivalent to rm -r, delete a file or directory recursively (careful!)
 * 
 * Accepts:
 *     - a pointer to an fs_node, the root of the tree containing the node you wish to delete
 *     - an absolute path to the parent directory of the node you wish to delete ( /path/to/parent )
 *     - the name of the node you wish to delete
 * 
 * Returns:
 *     - an error code, ERROR_NONE if successful else some other error
 * 
 * Assumptions:
 *     - none of the passed in arguments will be NULL
 * 
 * Important notes:
 *     - remember to be careful with your recursion
 */
enum error_code pa3_rm(struct fs_node *root, const char *path, const char *fname);

/**
 * pa3_create_node |
 * 
 * Functionality:
 *     - create a new child node under a parent, either a file or directory
 * 
 * Accepts:
 *     - a pointer to an fs_node, the root of the tree containing the directory you wish to add a child to
 *     - an absolute path to that directory ( /path/to/directory )
 *     - the name of the node you wish to create
 *     - an int value; 1 means this should be a file, 0 means it should be a directory
 * 
 * Returns:
 *     - an error code, ERROR_NONE if successful else some other error
 * 
 * Important notes:
 *     - remember to match permissions!
 */
enum error_code pa3_create_node(struct fs_node *root, const char *path, const char *name, int is_file);

/**
 * pa3_touch |
 * 
 * Functionality:
 *     - equivalent to touch, create a file
 * 
 * Accepts:
 *     - a pointer to an fs_node, the root of the tree containing the directory you wish to add a file to
 *     - an absolute path to that directory ( /path/to/directory )
 *     - the name of the file you wish to create
 * 
 * Returns:
 *     - an error code, ERROR_NONE if successful else some other error
 * 
 * Assumptions:
 *     - none of the passed in arguments will be NULL
 * 
 * Important notes:
 *     - permissions!
 */
enum error_code pa3_touch(struct fs_node *root, const char *path, const char *fname);

/**
 * pa3_mkdir |
 * 
 * Functionality:
 *     - equivalent to mkdir, create a directory
 * 
 * Accepts:
 *     - a pointer to an fs_node, the root of the tree containing the parent directory you wish to add a subdirectory to
 *     - an absolute path to that parent directory ( /path/to/parentdir )
 *     - the name of the directory you wish to create
 * 
 * Returns:
 *     - an error code, ERROR_NONE if successful else some other error
 * 
 * Assumptions:
 *     - none of the passed in arguments will be NULL
 */
enum error_code pa3_mkdir(struct fs_node *root, const char *path, const char *dname);

/**
 * pa3_chmod_impl |
 * 
 * Functionality:
 *     - the implementation of actually changing a node's permissions
 * 
 * Accepts:
 *     - a pointer to an fs_node, the node whose permissions you wish to modify
 *     - an int value; 1 means to assign that node read-access, 0 means to remove it
 *     - an int value; 1 means to assign that node write-access, 0 means to remove it
 * 
 * Assumptions:
 *     - read and write will only have values of 1 or 0
 */
void pa3_chmod_impl(struct fs_node *node, int read, int write);

/**
 * pa3_chmod |
 * 
 * Functionality:
 *     - equivalent to chmod, modify a node's permissions
 * 
 * Accepts:
 *     - a pointer to an fs_node, the root of the tree containing the node whose permissions you wish to modify
 *     - an absolute path to that node ( /path/to/node )
 *     - an int value; 1 means to assign that node read-access, 0 means to remove it
 *     - an int value; 1 means to assign that node write-access, 0 means to remove it
 * 
 * Returns:
 *     - an error code, ERROR_NONE if successful else some other error
 * 
 * Assumptions:
 *     - none of the passed in arguments will be NULL
 *     - read and write will only have values of 1 or 0
 * 
 * Important notes:
 *     - feel free to use this to inform your implementations of the other commands!
 */
enum error_code pa3_chmod(struct fs_node *root, const char *fpath, int read, int write);

#endif /* PA3_H */
