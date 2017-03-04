#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>


#define LICENCE "GPL";
#define AUTEUR "Thomas Laroche thomas.laroche1@univ-tlse3.fr";
#define DESCRIPTION "Lecture destructrice / ecriture à la suite";
#define DEVICE "tp2-2";

static const int B_SIZE = 32;

static int release(struct inode *i,struct file *f);
static int open(struct inode *i, struct file *f);
static ssize_t write(struct file *f, const char *buf, size_t size, loff_t *offset);
static ssize_t read(struct file *f, char *buf, size_t size, loff_t *offset);

dev_t dev;
struct cdev *my_cdev;

struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = read,
    .write = write,
    .open = open,
 	.release = release
};

typedef struct Element Element;
struct Element {
    struct Element *next;
    char *text;
	int bufSize;
};

enum acces {LECTURE , ECRITURE} last_acces;

Element *_liste;
bool _empty;
bool _writing;


static int init(void)
{
    /* allocation dynamique pour les paires (major,mineur) */
    if (alloc_chrdev_region(&dev,0,1,"sample") == -1)
    {
        printk(KERN_ALERT ">>> ERROR alloc_chrdev_region\n");
        return -EINVAL;
    }
    
    /* recuperation et affichage */
    printk(KERN_ALERT "Init allocated (major, minor)=(%d,%d)\n",MAJOR(dev),MINOR(dev));
        
    /* allocation des structures pour les operations */
    my_cdev = cdev_alloc();
    my_cdev->ops = &fops;
    my_cdev->owner = THIS_MODULE;
    
	_liste = NULL;
	_empty = true;

    /* lien entre operations et periph */
    cdev_add(my_cdev,dev,1);
    
    return(0);

}


static int MIN(int a, int b){
    return a < b ? a : b;
}

Element* flush(Element* l){
	Element* tmp = _liste;
	Element* next;
	while(tmp != NULL){
		next = tmp->next;
		kfree(tmp->text);
		kfree(tmp);
		tmp = next;
	}
	return NULL;
}

static int open(struct inode *i, struct file *f){
	//init list
	printk(KERN_ALERT "Open called\n");
	return 0;
}


/**
 * Lecture
 **/
static ssize_t read(struct file *f, char *buf, size_t size, loff_t *offset)
{
	int sizeToCopy;
	Element* tmp;
	last_acces = LECTURE;
	printk(KERN_ALERT "Read called!\n");

	if(_empty)
		return 0;
	
	if(_liste != NULL){
		sizeToCopy = MIN(_liste->bufSize,size);
		tmp = _liste->next;
		if(copy_to_user(buf,_liste->text,sizeToCopy)==0){
			kfree(_liste->text);
			kfree(_liste);
			_liste = tmp;
		}
		else
			return -EFAULT;

		return sizeToCopy;
	}else{
		_empty = true;
		return 0;
	}
}


/**
 *  Ecriture
 **/
static ssize_t write(struct file *f, const char *buf, size_t size, loff_t *offset)
{
	Element* newE = (Element*)kmalloc(sizeof(Element),GFP_KERNEL);
	Element* tmp;
	int sizeToCopy = MIN(size,B_SIZE);
	last_acces = ECRITURE;
	printk(KERN_ALERT "Write called!\n");
/*	if(!_empty && !_writing){
		_liste = flush(_liste);
		_empty = true;
		_writing = true;
	}*/
	newE->text = (char*)kmalloc(sizeToCopy*sizeof(char),GFP_KERNEL);
	newE->bufSize = sizeToCopy - copy_from_user(newE->text,buf,sizeToCopy);
	newE->next = NULL;
	if(_empty){
		_empty = false;
		_liste = newE;
	}else{
		tmp = _liste;
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = newE;
	}
	return newE->bufSize;	
}

static int release(struct inode *i,struct file *f){
	_writing = false;
	printk(KERN_ALERT "Release called\n");
	return 0;
}

static void cleanup(void)
{
    /* liberation */
    printk(KERN_ALERT "Clean up\n");
    unregister_chrdev_region(dev,1);
    cdev_del(my_cdev);
}

module_init(init);    // Appelé au lancement du module
module_exit(cleanup); // Appelé à la suppression du module



MODULE_LICENSE(LICENCE);
/* Classiquement : Nom Email*/
MODULE_AUTHOR(AUTEUR);
/* Ce que fait votre module*/
MODULE_DESCRIPTION(DESCRIPTION);
/* Périphériques supportés*/
MODULE_SUPPORTED_DEVICE(DEVICE);
