#include <stdlib.h>
#include <stdio.h>

#include "pa3_test.h"

void report_test_result(enum test_result tr) {
    switch(tr) {
        case TEST_RESULT_PASS:                           
            printf("[INFO] TEST RESULT = PASS\n"); 
            break;

        case TEST_RESULT_FAIL_TO_DESERIALIZE_FILESYSTEM: 
            printf("[INFO] TEST RESULT = ERROR: unable to deserialize\n"); 
            break;

        case TEST_RESULT_UNABLE_TO_OPEN_TEST_FILE:       
            printf("[INFO] TEST RESULT = ERROR: unable to open test file\n"); 
            break;

        case TEST_RESULT_FAIL:                           
            printf("[INFO] TEST RESULT = FAIL: failed\n"); 
            break;

        default:                           
            printf("[INFO] TEST RESULT = ERROR: unknown test result: %d\n", (int)tr); 
            break;        
    }
}

void report_error_code(enum error_code ec) {
    switch(ec) {
        case ERROR_NONE:                           
            break;

        case ERROR_MALLOC_FAIL: 
            printf("[INFO] ERROR CODE: malloc failed\n"); 
            break;

        case ERROR_NOT_FOUND:       
            printf("[INFO] ERROR CODE: couldn't find file/dir\n"); 
            break;

        case ERROR_MISSING_PERMISSIONS:                           
            printf("[INFO] ERROR CODE: insufficient permissions for operation\n"); 
            break;

		case ERROR_INVALID_OPERATION:
			printf("[INFO] ERROR CODE: operation is invalid\n");
			break;

		case ERROR_ALREADY_EXISTS:
			printf("[INFO] ERROR CODE: operation failed because result already exists\n");
			break;

        default:                           
            printf("[INFO] ERROR CODE: unknown error code %d\n", (int)ec); 
            break;        
    }
}

// 

/**
 * fs0_test |
 * 
 * Functionality:
 *     - run a series of test operations on a given filesystem, testing your code's functionality
 * 
 * Accepts:
 * 	   - a pointer to an fs_node, the root of the filesystem
 * 
 * Returns:
 *     - a test_result, TEST_RESULT_PASS if it was successful, else some error
 * 
 * Assumptions:
 *     - the passed in filesystem will not be NULL
 * 
 * Important notes:
 *     - passing each test case requires:
 *         > allocating no more memory than precisely needed for every allocation operation
 *         > freeing that memory properly on cleanup
 *         > correctly executing the given operation
 *     - use this function as a template for filling in the others, or for making more!
 *     - we also encourage you to modify this function to test more functionality
 */
enum test_result fsB_test(struct fs_node *root) {
    printf("============= deserialized filesystem =================\n");
    pa3_print_tree(root);

    // /* add a directory to the root */
    //  enum error_code ec = pa3_mkdir(root, "/Root Directory", "test directory name");
    // if(ec != ERROR_NONE)
    // {
    //     printf("[INFO] failed to create new directory\n");
	// 	report_error_code(ec);
    //     return TEST_RESULT_FAIL;
    // }
    // printf("past test 1\n");
    // pa3_print_tree(root);


    // /* add a file to the new directory */
    // enum error_code ec = pa3_touch(root, "/root/Directory 0/Directory 1/test directory name", "new file name");
    // if(ec != ERROR_NONE)
    // {
    //     printf("[INFO] failed to create new file\n");
	// 	report_error_code(ec);
    //     return TEST_RESULT_FAIL;
    // }
    // printf("past test 2\n");
    // // pa3_print_tree(root);


    /* set the file to read only */
    // enum error_code ec = pa3_chmod(root, "/Root Directory/Directory 1", 1, 0);
    // if(ec != ERROR_NONE)
    // {
    //     printf("[INFO] failed to chmod new file\n");
	// 	report_error_code(ec);
    //     return TEST_RESULT_FAIL;
    // }
    // printf("past test 3\n");
    // pa3_print_tree(root);
    /* try removing the new root: not allowed */
//    enum error_code ec = pa3_rm(root, "/", "Root Directory");
//     if(ec != ERROR_INVALID_OPERATION)
//     {
//         printf("[INFO] rm did not return the A right error code when trying to delete the root\n");
// 		report_error_code(ec);
//         return TEST_RESULT_FAIL;
//     }
//     printf("past test 4\n");
//     pa3_print_tree(root);

    // /* try removing the new file */
    enum error_code ec = pa3_rm(root, "/Root Directory","Directory 1");
    if(ec != ERROR_NONE)
    {
        printf("[INFO] rm did not return the B right error code when trying to delete the read only file\n");
		report_error_code(ec);
        return TEST_RESULT_FAIL;
     }
    pa3_print_tree(root);
	pa3_free_tree(root);

    return TEST_RESULT_PASS;
}

// enum test_result fs1_test(struct fs_node *root) {
//     printf("============= deserialized filesystem =================\n");
//     pa3_print_tree(root);

//     /* add a directory to the root */
//     enum error_code ec = pa3_mkdir(root, "/root", "test directory name");
//     if(ec != ERROR_NONE)
//     {
//         printf("[INFO] failed to create new directory\n");
// 		report_error_code(ec);
//         return TEST_RESULT_FAIL;
//     }

//     /* add a file to the new directory */
//     ec = pa3_touch(root, "/root/test directory name", "new file name");
//     if(ec != ERROR_NONE)
//     {
//         printf("[INFO] failed to create new file\n");
// 		report_error_code(ec);
//         return TEST_RESULT_FAIL;
//     }

//     /* set the file to read only */
//     ec = pa3_chmod(root, "/root/test directory name/new file name", 1, 0);
//     if(ec != ERROR_NONE)
//     {
//         printf("[INFO] failed to chmod new file\n");
// 		report_error_code(ec);
//         return TEST_RESULT_FAIL;
//     }

//     /* try removing the new root: not allowed */
//     ec = pa3_rm(root, "/", "root");
//     if(ec != ERROR_INVALID_OPERATION)
//     {
//         printf("[INFO] rm did not return the right error code when trying to delete the root\n");
// 		report_error_code(ec);
//         return TEST_RESULT_FAIL;
//     }

//     /* try removing the new file */
//     ec = pa3_rm(root, "/root/test directory name", "new file name");
//     if(ec != ERROR_MISSING_PERMISSIONS)
//     {
//         printf("[INFO] rm did not return the right error code when trying to delete the read only file\n");
// 		report_error_code(ec);
//         return TEST_RESULT_FAIL;
//     }

// 	pa3_free_tree(root);

//     return TEST_RESULT_PASS;
// }

// enum test_result fs2_test(struct fs_node *root) {
//     printf("============= deserialized filesystem =================\n");
//     pa3_print_tree(root);

//     /* add a directory to the root */
//     enum error_code ec = pa3_mkdir(root, "/root", "test directory name");
//     if(ec != ERROR_NONE)
//     {
//         printf("[INFO] failed to create new directory\n");
// 		report_error_code(ec);
//         return TEST_RESULT_FAIL;
//     }

//     /* add a file to the new directory */
//     ec = pa3_touch(root, "/root/test directory name", "new file name");
//     if(ec != ERROR_NONE)
//     {
//         printf("[INFO] failed to create new file\n");
// 		report_error_code(ec);
//         return TEST_RESULT_FAIL;
//     }

//     /* set the file to read only */
//     ec = pa3_chmod(root, "/root/test directory name/new file name", 1, 0);
//     if(ec != ERROR_NONE)
//     {
//         printf("[INFO] failed to chmod new file\n");
// 		report_error_code(ec);
//         return TEST_RESULT_FAIL;
//     }

//     /* try removing the new root: not allowed */
//     ec = pa3_rm(root, "/", "root");
//     if(ec != ERROR_INVALID_OPERATION)
//     {
//         printf("[INFO] rm did not return the right error code when trying to delete the root\n");
// 		report_error_code(ec);
//         return TEST_RESULT_FAIL;
//     }

//     /* try removing the new file */
//     ec = pa3_rm(root, "/root/test directory name", "new file name");
//     if(ec != ERROR_MISSING_PERMISSIONS)
//     {
//         printf("[INFO] rm did not return the right error code when trying to delete the read only file\n");
// 		report_error_code(ec);
//         return TEST_RESULT_FAIL;
//     }

// 	pa3_free_tree(root);

//     return TEST_RESULT_PASS;
// }

// enum test_result fs3_test(struct fs_node *root) {
//     printf("============= deserialized filesystem =================\n");
//     pa3_print_tree(root);

//     /* add a directory to the root */
//     enum error_code ec = pa3_mkdir(root, "/root", "test directory name");
//     if(ec != ERROR_NONE)
//     {
//         printf("[INFO] failed to create new directory\n");
// 		report_error_code(ec);
//         return TEST_RESULT_FAIL;
//     }

//     /* add a file to the new directory */
//     ec = pa3_touch(root, "/root/test directory name", "new file name");
//     if(ec != ERROR_NONE)
//     {
//         printf("[INFO] failed to create new file\n");
// 		report_error_code(ec);
//         return TEST_RESULT_FAIL;
//     }

//     /* set the file to read only */
//     ec = pa3_chmod(root, "/root/test directory name/new file name", 1, 0);
//     if(ec != ERROR_NONE)
//     {
//         printf("[INFO] failed to chmod new file\n");
// 		report_error_code(ec);
//         return TEST_RESULT_FAIL;
//     }

//     /* try removing the new root: not allowed */
//     ec = pa3_rm(root, "/", "root");
//     if(ec != ERROR_INVALID_OPERATION)
//     {
//         printf("[INFO] rm did not return the right error code when trying to delete the root\n");
// 		report_error_code(ec);
//         return TEST_RESULT_FAIL;
//     }

//     /* try removing the new file */
//     ec = pa3_rm(root, "/root/test directory name", "new file name");
//     if(ec != ERROR_MISSING_PERMISSIONS)
//     {
//         printf("[INFO] rm did not return the right error code when trying to delete the read only file\n");
// 		report_error_code(ec);
//         return TEST_RESULT_FAIL;
//     }

// 	pa3_free_tree(root);

//     return TEST_RESULT_PASS;
// }

// read the serialized filesystem file from disk into an array of bytes
void *read_serialized_filesystem_file(const char *serialized_filename)
{
    FILE *f_in = fopen(serialized_filename, "rb");
    if(f_in == NULL)
    {
        printf("[ERROR] unable to open output filename for read: \"%s\"\n", serialized_filename);
        return NULL;
    }
    fseek(f_in, 0, SEEK_END);
    size_t f_in_sz = ftell(f_in);
    fseek(f_in, 0, SEEK_SET);
    void *file_contents = (void *)malloc(f_in_sz);
    if(file_contents == NULL)
    {
        printf("[ERROR] unable allocate memory for input file");
        fclose(f_in);
        return NULL;
    }
    size_t n_read = fread(file_contents, 1, f_in_sz, f_in);
    if(n_read != f_in_sz)
    {
        printf("[ERROR] unable to read entire input file! expected %lu bytes; got %lu bytes\n", f_in_sz, n_read);
        fclose(f_in);
        free(file_contents);
        return NULL;
    }
    fclose(f_in);
    f_in = NULL;

    return file_contents;
}