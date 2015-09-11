// owner Surote Wongpaiboon , Kasetsart University
// at Kyushu Institute of Technology
// 2015
#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

struct fnode {

								char name[256];
								int size;
								char *data;
								struct stat stat; // stat for stamp time etc.
								struct fnode *next;

};
struct fnode *head = NULL;

struct fnode *find_fnode(const char *filename);

struct fnode *create_fnode(const char *filename)
{
								struct fnode *new_node = (struct fnode*) malloc (sizeof (struct fnode)); // reserve memory for new_node
								long n_size = strlen(filename); // length filename

								strncpy(new_node->name,filename,n_size); // copy filename to new_node->name

								new_node->size = 0; // init new_node->size = 0

								new_node->next = head; // new_node->next point head
								head = new_node; // head point new_node

								return new_node;

}
// path is the file that you create fuse will show you
int mfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
								struct fnode *create; // pointer for create node
								struct fnode *dup; // pointer for check duplicate node
								dup = find_fnode(path+1);
								if(dup!=NULL) // check if filename in list
																return -ENOENT;
								create = create_fnode(path+1); // create node

								if (create != NULL) // if create success!
																return 0;
								return -ENOENT;
}

struct fnode *find_fnode(const char *filename)
{
								struct fnode *point; // pointer for find node
								point = head; // start at head
								if(point) // if pointer != NULL
								{
																while(strcmp(point->name,filename)!=0) // compare name in list with file่่่name
																{
																								point = point->next; // next
																								if(point) // if pointer != NULL
																								{
																																printf("%s %d\n",point->name,point->size);
																																continue;
																								}
																								else // end of list
																																break;
																}
								}

								return point; // can return NULL
}

int mfs_getattr(const char *path, struct stat *stbuf)
{
								int res = 0;

								struct fnode *point = find_fnode(path+1); //+1 cut '/' from path and find node in list
								if(strcmp("/",path)==0) // if path == root
								{
																stbuf->st_mode = S_IFDIR | 0755;
																stbuf->st_nlink = 2;

								}
								else if(point) // if point != NULL
								{
																if(strcmp(point->name,path+1)==0) // compare name in list with path(deleted '/')
																{
																								printf("C getattr %s %d\n",point->name,point->size);
																								stbuf->st_mode = S_IFREG | 0777;
																								stbuf->st_size = point->size;
																								stbuf->st_nlink = 1;
																								printf("C after getattr %s %d\n",point->name,point->size);
																}
								}
								else
								{
																return -ENOENT;
								}
								return res;
}
// for touch
int mfs_utimens(const char *path, const struct timespec tv[2])
{
								const struct timespec access_time = tv[0];
								const struct timespec mod_time = tv[1];

								struct fnode *point=find_fnode(path+1);
								if(point)
								{
																point->stat.st_atime = access_time.tv_sec;
																point->stat.st_mtime = mod_time.tv_sec;
																return 0;
								}
								else
																return -ENOENT;
}

int mfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
								(void) offset;
								(void) fi;

								filler(buf,".",NULL,0); // regist .
								filler(buf,"..",NULL,0); // regist ..
								struct fnode *node;
								node = head;
								while(node!=NULL)
								{
																printf("C readdir %s\n",node->name);
																filler(buf,node->name,NULL,0);
																node=node->next;
								}
								return 0;
}
//fuse open auto
/*
   int mfs_open(const char *path,struct fuse_file_info *fi)
   {
   struct fnode *point;
   point = find_fnode(path+1);
   if(point!=NULL) //find node name in path
   return 0;
   //	if(strcmp(path,str_p)==0) //find node name in path
   //		return 0;
   return -ENOENT;
   }
 */
int mfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
								struct fnode *node;
								if(strlen(path) == 0) return -ENOENT;
								node = find_fnode(path+1);
								if(node)
								{
																if(node->size < (size + offset))
																{
																								int new_size = size+offset;
																								void * new_mem = realloc(node->data,new_size);
																								if(new_mem == 0) return -ENOENT;
																								memset(new_mem+offset,0,size);
																								node->data = new_mem;
																								node->size = new_size;
																}
																memcpy((node->data + offset),buf,size);
																return size;
								}
								else
																return -ENOENT;
}


int mfs_read(const char *path, char *buf, size_t size, off_t offset,
													struct fuse_file_into *fi)
{
								(void) fi;
								size_t len;
								struct fnode *node;
								node = find_fnode(path+1);
								if(node)
								{
																if(strcmp(path+1,node->name)!=0)
																								return -ENOENT;
																int read_size = ((offset+size)>node->size) ? (node->size - offset) : size;
																if(read_size<0) read_size = 0;

																memcpy(buf,(node->data + offset),read_size);
																return read_size;
								}
}

struct  fuse_operations mfs_oper = {

								.create = mfs_create, // for create a file
								.getattr = mfs_getattr, // for all cmd
								.readdir = mfs_readdir, // for cmd like ls
								.utimens = mfs_utimens, // for touch stamp
//	.open = mfs_open,
								.read = mfs_read, // for open and read file
								.write = mfs_write, // for write file

};

int main(int argc, char *argv[])
{
								return fuse_main(argc, argv, &mfs_oper, NULL);
}
