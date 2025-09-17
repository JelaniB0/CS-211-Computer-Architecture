#include "pa3.h"

/* ======================================================================
 * Provided example functions
 * ====================================================================== */

// Print a given node's data
void pa3_print_node_data(const struct fs_node * const node) {
    printf("%s {props=[type:%c, perms:%c%c], num_children=%d} @ [%p]\n",
        node->name,
        (node->props & P_FILE_BIT) ? 'F' : 'D',
        (node->props & P_READ_BIT) ? 'R' : '-',
        (node->props & P_WRITE_BIT) ? 'W' : '-',
        node->num_children,
        (void *) node);
}

// Implementation of print_tree
void pa3_print_tree_impl(const struct fs_node * const node, int depth) {
    printf("%*s> ", depth * 4, "");
    pa3_print_node_data(node);
    for (int child_idx = 0; child_idx < node->num_children; child_idx++)
        pa3_print_tree_impl(node->child_list[child_idx], depth + 1);
}

// Print a tree's data in a digestible format. You are encouraged to make use of this in your testing!
void pa3_print_tree(const struct fs_node * const root) {
    if (root == NULL) {
        printf("[ERROR] trying to print a NULL tree\n");
        return;
    }
    pa3_print_tree_impl(root, 0);
}

// Given a pointer to the start of a node's data, return its props field as a uint8_t
uint8_t get_props(void *node_data) {
    return *((uint8_t *) node_data);
}

// Given a pointer to the start of a node's data, return its num_children field as a uint8_t
uint8_t get_num_children(void *node_data) {
    return *((uint8_t *) node_data + sizeof(uint8_t));
}

/* ======================================================================
 * Your implementations start here! For detailed explanations of each
 * function, either look in pa3.h or consult the Google Doc.
 * Good luck! :)
 *
 * IMPORTANT NOTE: You may wish to define some static or global data
 * structures to hold your data to make some aspect of this assignment
 * easier. If you do so, the autograder will be unable to properly
 * run your code due to how it is implemented, and you may receive a 0,
 * so please avoid this.
 * ====================================================================== */

#define NODE_SIZE_BYTES 275 //could be 274
enum error_code pa3_extract_node_name(char **name, char *name_data) {
    if (name == NULL || name_data == NULL){
        return ERROR_INVALID_OPERATION;
    }
    size_t size = strlen(name_data);
    *name = (char*)malloc(size + 1);
    //incase malloc fails
    if (*name == NULL){
        return ERROR_MALLOC_FAIL;
    }
    strncpy(*name, name_data, size);
    (*name)[size] = '\0';
    return ERROR_NONE;
}
void *pa3_node_data_by_index(uint16_t idx, void *data) {
    return (char *)data + (idx * NODE_SIZE_BYTES);
}
//hopefully this meets the standards for memory!! Main change is endianness and going from malloc to calloc. 
enum error_code pa3_deserialize_node(struct fs_node **node, uint16_t idx, void *data) {
    //deleted check, don't need to check if inputs are null/assumption that everything inputted is not null. 
    *node = (struct fs_node *)malloc(sizeof(struct fs_node)); //replaced with calloc to see if it saves on memory. 
    if (!*node) {
        return ERROR_MALLOC_FAIL;
    }
    void *nodeData = (char *)data + (idx * NODE_SIZE_BYTES);
    (*node)->props = get_props(nodeData);
    (*node)->num_children = get_num_children(nodeData);
    // Extract name directly
    enum error_code nameResult = pa3_extract_node_name(&((*node)->name), (char *)nodeData + 2);  
    if (nameResult != ERROR_NONE) {
        free(*node);
        *node = NULL;
        return nameResult;
    }
    if ((*node)->num_children > 0) {
        (*node)->child_list = (struct fs_node **)malloc((*node)->num_children * sizeof(struct fs_node *)); //replaced w calloc.
        //if malloc fails then exit code.  
        if (!(*node)->child_list) {
            free(*node);
            *node = NULL;
            return ERROR_MALLOC_FAIL;
        }

        uint16_t *childIndices = (uint16_t *)((char *)nodeData + 258);
        for (uint8_t i = 0; i < (*node)->num_children; i++) {
            uint16_t childIdx = (childIndices[i] << 8) | (childIndices[i] >> 8); // Handle endianness since data is in bid endian/system in little endian. 
            if (childIdx == 0) {
                (*node)->child_list[i] = NULL;
                continue;
            }
            //recursively deserialize children incase. 
            enum error_code child_result = pa3_deserialize_node(&((*node)->child_list[i]), childIdx, data);
            if (child_result != ERROR_NONE) {
                while (i > 0) {
                    free((*node)->child_list[--i]);
                }
                //if child deserializion fails, decrement and free child, return error. 
                free((*node)->child_list);
                free(*node);
                *node = NULL;
                return child_result;
            }
        }
    } else {
        //incase node has no children. 
        (*node)->child_list = NULL;
    }

    return ERROR_NONE;
}
//should be done now hopefully? Test in gradescope/test cases before moving on. 
void pa3_free_tree(struct fs_node *root) {
    if (root == NULL) {
        return;
    }
    if(root->num_children > 0){
        for (uint8_t i = 0; i < root->num_children; i++) {
            pa3_free_tree(root->child_list[i]); //free each child recursively
        }
        free(root->child_list);   //Free the child list itself if children. 
    }
    //Free name
    free(root->name);
    //freeing root node. 
    free(root);
}
//HOPEFULLY WORKS NOW PLZZZZ
//updated navigation code that works for all name cases? 
//updated read permissions spot for code, checks before hand if there are read perms. 
enum error_code pa3_navigate_to_node(struct fs_node *root, struct fs_node **found, const char *path) {
    //check if any of input values are not valid/NULL. 
    if (root == NULL|| found == NULL|| path == NULL|| path[0] != PATH_SEPARATOR) {
        // printf("[DEBUG] Invalid input: root=%p, found=%p, path=%s\n", (void *)root, (void *)found, path ? path : "(null)");
        return ERROR_INVALID_OPERATION;
    }
    // printf("[DEBUG] Root node name: %s\n", root->name);
    // **Check if root has read permissions before proceeding**
    if (!(root->props & P_READ_BIT)) {
        // printf("[DEBUG] ERROR_MISSING_PERMISSIONS: No read permission for root '%s'\n", root->name);
        return ERROR_MISSING_PERMISSIONS;
    }
    // Base case: if navigating to root (`/`), return immediately
    if (strcmp(path, "/") == 0) {
        // printf("[DEBUG] Navigating to root directory\n");
        *found = root;
        return ERROR_NONE;
    }
    *found = root;
    path++; // Skip initial /
    char pathCopy[MAX_NAME_LENGTH];
    strncpy(pathCopy, path, MAX_NAME_LENGTH - 1);
    pathCopy[MAX_NAME_LENGTH - 1] = '\0';
    // printf("[DEBUG] Navigating path: %s\n", path_copy);
    char *token = strtok(pathCopy, "/");
    // Conditionally skip the root name if it's redundant
    if (token && strcmp(token, root->name) == 0) {
        if (root->num_children > 0) {  // Skip only if root has children
            // printf("[DEBUG] Skipping redundant root node name: '%s'\n", token);
            token = strtok(NULL, "/");
        } else {
            // printf("[DEBUG] Root is the only node, not skipping token: '%s'\n", token);
        }
    }
    while (token) {
        // printf("[DEBUG] Searching for token: '%s' in node: '%s' (num_children: %d)\n", token, (*found)->name, (*found)->num_children);
        int foundChild = 0;
        for (uint8_t i = 0; i < (*found)->num_children; i++) {
            struct fs_node *child = (*found)->child_list[i];
            if (child) {
                // printf("[DEBUG] Checking child: '%s' (expected match: '%s')\n", child->name, token);
                if (strcmp(child->name, token) == 0) {  // Ensure exact match
                    if (!(child->props & P_READ_BIT)) {
                        // printf("[DEBUG] ERROR_MISSING_PERMISSIONS: No read permission for '%s'\n", child->name);
                        return ERROR_MISSING_PERMISSIONS;
                    }
                    // printf("[DEBUG] Match found: '%s' -> Updating current node\n", child->name);
                    *found = child;
                    foundChild = 1;
                    break;
                }
            }
        }
        if (!foundChild) { //equivalent to == 0 since 0 false in C. 
            // printf("[DEBUG] ERROR_NOT_FOUND: '%s' not found in '%s'\n", token, (*found)->name);
            return ERROR_NOT_FOUND;
        }
        token = strtok(NULL, "/");  // Move to the next token in the path
    }
    // printf("[DEBUG] Successfully navigated to node: %s\n", (*found)->name);
    return ERROR_NONE;
}

//hope this works alongside rm. 
//bruh this method is to remove a child from a list only, rm is responsible for actually freeing. 
//should be good but i'll see.  Last change of the night. If it fails go again tomorrow or next day. 
//need 2 change childlist size/reallocate it. 
enum error_code pa3_remove_child_from_child_list(struct fs_node *parent, int child_idx) {
    if (!parent || child_idx < 0 || child_idx >= parent->num_children) { //since we cannot assume inputs are valid. 
        return ERROR_INVALID_OPERATION;
    }

    //printf("[DEBUG] Removing child at index %d from parent %s\n", child_idx, parent->name);

    //Shift the remaining children left due to child being gone. 
    for (int i = child_idx; i < parent->num_children - 1; i++) {
        parent->child_list[i] = parent->child_list[i + 1];
    }

    parent->num_children--;
    parent->child_list[parent->num_children] = NULL; // Clear last reference

    //parent has no more children, free child_list completely/no dangling ptrs. 
    if (parent->num_children == 0) {
        free(parent->child_list); //Free memory to prevent leaks.
        parent->child_list = NULL; //set to NULL for safety.
    } else {
        //shrink child_list to avoid wasted memory.
        parent->child_list = realloc(parent->child_list, parent->num_children * sizeof(struct fs_node *));
        if (parent->num_children > 0 && parent->child_list == NULL) { // realloc failure check
            return ERROR_MALLOC_FAIL;
        }
    }

    //printf("[DEBUG] Successfully removed child at index %d from parent %s\n", child_idx, parent->name);
    return ERROR_NONE;
}

//debugged and worked for fs9, i think its good. 
//should be good but i'll see.  Last change of the night. If it fails go again tomorrow or next day. 
enum error_code pa3_rm(struct fs_node *root, const char *path, const char *fname) {
    if (strchr(fname, PATH_SEPARATOR)) {
        // printf("[DEBUG] Invalid filename: '%s' contains '%c'. Filenames cannot contain '/'.\n", fname, PATH_SEPARATOR);
        return ERROR_INVALID_OPERATION;
    }

    struct fs_node *parent2 = NULL;
    enum error_code err;

    //Base case: Check if the path is root. 
    if (strcmp(path, "/") == 0) {
        // printf("[DEBUG] pa3_rm: Path is root, no parent needed.\n");
        parent2 = root;
    } else {
        // Navigate to the parent directory of the target file/directory
        // printf("[DEBUG] pa3_rm: Navigating to path: %s\n", path);
        err = pa3_navigate_to_node(root, &parent2, path);
        if (err != ERROR_NONE || parent2 == NULL) {
            // printf("[DEBUG] pa3_rm: Navigation failed to path: %s\n", path);
            return ERROR_NOT_FOUND;
        }
    }

    // Check if trying to delete the root node itself
    if (parent2 == root && strcmp(fname, root->name) == 0) {
        // printf("[DEBUG] pa3_rm: Cannot delete the root directory.\n");
        return ERROR_INVALID_OPERATION;
    }

    // Check if the parent directory has write permissions
    // printf("[DEBUG] pa3_rm: Checking permissions for parent directory: %s\n", parent2->name);
    if (!(parent2->props & P_WRITE_BIT)) {
        // printf("[DEBUG] pa3_rm: Missing write permissions for parent directory.\n");
        return ERROR_MISSING_PERMISSIONS;
    }

    // Search for the target file or directory in the parent's children list
    for (int i = 0; i < parent2->num_children; i++) {
        struct fs_node *child = parent2->child_list[i];
        // printf("[DEBUG] pa3_rm: Checking child: %s\n", child->name);

        if (child && strcmp(child->name, fname) == 0) {
            // printf("[DEBUG] pa3_rm: Found matching child: %s\n", child->name);

            // Check if the file/directory has write permissions
            if (!(child->props & P_WRITE_BIT)) {
                // printf("[DEBUG] pa3_rm: Child %s does not have write permissions.\n", child->name);
                return ERROR_MISSING_PERMISSIONS;
            }

            // If it's a directory with children, recursively delete them first
            if (child->num_children > 0) {
                // printf("[DEBUG] pa3_rm: Child %s is a directory with children, recursively deleting.\n", child->name);
                // Recursively delete each child without navigating to their path (just use child node)
                for (int j = 0; j < child->num_children; j++) {
                    struct fs_node *grandchild = child->child_list[j];
                    if (grandchild) {
                        // printf("[DEBUG] pa3_rm: Recursively deleting child: %s\n", grandchild->name);
                        pa3_rm(root, child->name, grandchild->name);  // Directly use child node, no navigation
                    }
                }
            }

            // Now that we've handled all children, remove the child from the parent's child list
            err = pa3_remove_child_from_child_list(parent2, i);
            if (err != ERROR_NONE) {
                // printf("[DEBUG] pa3_rm: Failed to remove child from parent list, error: %d\n", err);
                return err;
            }

            // Free the memory associated with the child (file or empty directory)
            // printf("[DEBUG] pa3_rm: Freeing child node: %s\n", child->name);
            pa3_free_tree(child);
            return ERROR_NONE;
        }
    }

    // If no matching file/directory is found
    // printf("[DEBUG] pa3_rm: No matching file/directory found in target directory.\n");
    return ERROR_NOT_FOUND;
}


enum error_code pa3_create_node(struct fs_node *root, const char *path, const char *name, int is_file) {
    if (root == NULL || path == NULL || name == NULL || strlen(name) >= MAX_NAME_LENGTH) {
        return ERROR_INVALID_OPERATION;
    } 
    //find parent directory
    struct fs_node *parent = NULL;

    //Handle root-level creation for fs0 as an example. 
    if (strcmp(path, "/") == 0 || strcmp(path, "") == 0) {
        parent = root;  //directly assigning root as parent if creating under root
    } else {
        enum error_code result = pa3_navigate_to_node(root, &parent, path);
        if (result != ERROR_NONE) {
            return result; //returns the kind of error that occurs with pa3_navigate_to_node. 
        }
    }

    if (!(parent->props & P_WRITE_BIT)) {
        return ERROR_MISSING_PERMISSIONS;
    } 
    if (parent->props & P_FILE_BIT) {
        return ERROR_INVALID_OPERATION;
    }
    for (uint8_t i = 0; i < parent->num_children; i++) {
        if (strcmp(parent->child_list[i]->name, name) == 0) { //make sure to use parent->child_list[i]->name instead of parent->child_list[i]
            return ERROR_ALREADY_EXISTS; //checks for if node already exists before creating to avoid duplicates.
        }
    }
    if (parent->num_children >= MAX_NUM_CHILDREN_PER_NODE) {
        return ERROR_INVALID_OPERATION;
    }
    struct fs_node *newNode = malloc(sizeof(struct fs_node));
    if (newNode == NULL) {
        return ERROR_MALLOC_FAIL;
    }
    newNode->name = malloc(strlen(name) + 1);
    if (newNode->name == NULL) {
        free(newNode);
        return ERROR_MALLOC_FAIL;
    }
    strcpy(newNode->name, name);
    // Useless code. 
    // if (newNode->name == NULL) {
    //     free(newNode);
    //     return ERROR_MALLOC_FAIL;
    // }
    newNode->props = is_file ? P_FILE_BIT : 0;
    //Default permissions for file/set them automatically. 
    newNode->props |= P_READ_BIT | P_WRITE_BIT; 
    newNode->num_children = 0;
    newNode->child_list = NULL;

    //add new node to parent's child list. 
    parent->child_list = realloc(parent->child_list, (parent->num_children + 1) * sizeof(struct fs_node *)); // need to reallocate more memory to child_list. 
    if (parent->child_list == NULL) {
        free(newNode->name); //free memory for name and node just in case memory is not allocated correctly. 
        free(newNode);
        return ERROR_MALLOC_FAIL;
    }
    parent->child_list[parent->num_children++] = newNode;
    return ERROR_NONE;
}

enum error_code pa3_touch(struct fs_node *root, const char *path, const char *fname) {
    return pa3_create_node(root, path, fname, 1); //so File if 1.
}

enum error_code pa3_mkdir(struct fs_node *root, const char *path, const char *dname) {
//check if the root node is a directory
if (root->props & P_FILE_BIT) {
    return ERROR_INVALID_OPERATION;  //cannot create a directory under a file
}

//check if the parent directory has write permission
if (!(root->props & P_WRITE_BIT)) {
    return ERROR_MISSING_PERMISSIONS;  //no write permission
}

//call create_node with isFile = 0, so directory
return pa3_create_node(root, path, dname, 0);  
}

//updated code. Hopefully works now. 
void pa3_chmod_impl(struct fs_node *node, int read, int write) {
    if (node == NULL) { //can't assume node won't be null. 
        return; 
    }

    // Update only the permission bits
    //reverted back to old code format to be a little safer. 
    if (read) {
        node->props |= P_READ_BIT;  // Enable read permission
    } else {
        node->props &= ~P_READ_BIT; // Remove read permission
    }

    if (write) {
        node->props |= P_WRITE_BIT;  // Enable write permission
    } else {
        node->props &= ~P_WRITE_BIT; // Remove write permission
    }

    //does changing directory permissions change all child permissions as well? 
    //may have to delete code below if not. 
    // If it's a directory, update children as well
    if ((node->props & P_FILE_BIT) == 0) { // Means it's a directory instead of file. 
        for (int i = 0; i < node->num_children; i++) {
            if (node->child_list[i]) {
                pa3_chmod_impl(node->child_list[i], read, write);
            }
        }
    }
}
//this part should be done? Might have to debug it more later. 
enum error_code pa3_chmod(struct fs_node *root, const char *fpath, int read, int write) {
    struct fs_node *node;
    enum error_code nav_result = pa3_navigate_to_node(root, &node, fpath);
    if (nav_result != ERROR_NONE) return nav_result;
	else if (!(node->props & P_WRITE_BIT)) return ERROR_MISSING_PERMISSIONS;

    pa3_chmod_impl(node, read, write);
    return ERROR_NONE;
}