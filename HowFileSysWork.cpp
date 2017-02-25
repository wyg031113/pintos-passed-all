struct inode_disk
{
	off_t length;  //文件数据大小单位字节
	uint32_t blocks[15];//三级索引结构
	unsigned magic;
	uint32_t unused[111];
};
struct inode //内存中维护的inode
{
	struct list_elem elem;
	block_sector_t sector;
	int open_cnt;
	bool removed;
	int deny_write_cnt;
	struct inode_disk data;
};
struct file
{
	struct inode *inode;
	off_t pos;
	bool deny_write;
};
struct dir  //用于目录文件相当于struct file
{
	struct inode *inode;
	off_t pos;
};
struct dir_entry//目录文件的内容记载着该目录中一个个文件和目录
{
	block_sector_t inode_sector;
	char name[NAME_MAX+1];
	bool IsDir; //如果这个文件是目录，就为true,否则为false; 
	bool in_use;
}
文件是怎么创建的。
1.调用filesys_create(name,size);
    (1)打开要创建文件的目录dir=dir_open_root();
    (2)给文件的disk_inode分配一个物理块sector
    (3)创建文件的disk_inode:调用inode_create(sector,size); 这里分配了文件
    (4)把文件名，disk_inode块号都登记到目录dir中:调用dir_add(dir,name,sector)
文件是怎么打开的。
1.调用filesys_open
	(1)先打开文件所在的目录，得到struct dir.其实相当于struct file.依据dir,打开其
		inode,据inode就能读取目录文件内容了，内容是一个个struct dir_entry结构
			如果其中是一个目录(可以根据dir_entry结构中的IsDir判断)，则可以把
			他打开为struct dir,继续在里面找文件和目录。
			如果是一个文件，且是要打开的文件，里面有文件disk_inode所在的磁盘块号
			加入struct file:这里调用的是dir_lookup(dir,name,&inode),这个函数不但
			找到了文件的disk_inode号，而且把他变为了内存inode,此时又调用:
						file_open(inode);把inode包装成了struct file;
						如果调用dir_open(inode),则把inode包装成了struct dir

目录是怎么创建的。
1.调用mkdir(dirname)
	 (1)从dirname中分离出父目录和要创建的目录：/a/b/c/ == /a/b/c
	 (2)假如要创建目录c，/a/b已经存在
	 (3)首先打开根目录 / 得到struct dir dirroot;
	 (4)在要目录/中调用 dir_lookup(dirroot,b,&inode)
	 (5)然后调用dir_open(inode)把inode包装成struct dir dirb
	 	如果目录长的话，继续去第(4)步，一直到最后一层目录。
	 (6)在dirb中创建一个新的目录c
	 	到这里可以把目录c当作一个普通文件，如同filesys_create()中一们创建。
		不同的是，要调用dir_add时要把dir_entry结构中的IsDir置为true
		
