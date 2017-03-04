

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <asm/uaccess.h>


#define LICENCE "GPL";
#define AUTEUR "Thomas Laroche thomas.laroche1@univ-tlse3.fr";
#define DESCRIPTION "Lecture destructrice ou non, écriture à la suite, 3 périphériques";
#define DEVICE "tp2-4";

static const int B_SIZE = 32;

static int release(struct inode *i,struct file *f);
static int open(struct inode *i, struct file *f);
static ssize_t write(struct file *f, const char *buf, size_t size, loff_t *offset);
static ssize_t readD(struct file *f, char *buf, size_t size, loff_t *offset);
static ssize_t readND(struct file *f, char *buf, size_t size, loff_t *offset);

dev_t dev;
struct cdev *my_cdev;

struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = NULL,
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

typedef struct SuperElement SuperElement;
struct SuperElement {
    Element *_first;
    struct SuperElement *_next;
    int pid;
};

enum acces {LECTURE , ECRITURE} last_acces;

SuperElement *_superListe;
Element *_indice,*_liste;
bool _empty;
bool _writing;


static int init(void)
{
    /* allocation dynamique pour les paires (major,mineur) */
    if (alloc_chrdev_region(&dev,0,3,"sample") == -1)
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

    _superListe = NULL;
	_liste = NULL;
    _indice = NULL;
	_empty = true;

    /* lien entre operations et periph */
    cdev_add(my_cdev,dev,3);

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
    int minor;
    SuperElement *tmp,*prev,*newSE;
    int tpid = current->tgid;
	printk(KERN_ALERT "Open called\n");
	printk(KERN_ALERT "pid : %d\n",tpid);

    minor = iminor(i);

    prev = NULL;
    tmp = _superListe;
    while(tmp != NULL && _liste == NULL){
        prev = tmp;
        if(tmp->pid == tpid)
            _liste = tmp->_first;
        tmp = tmp->_next;
    }
    if(tmp == NULL){
        newSE = (SuperElement*)kmalloc(sizeof(SuperElement),GFP_KERNEL);
        newSE->_first = NULL;
        newSE->_next = NULL;
        if(prev == NULL){ //liste est vide
            _superListe = newSE;
            _liste = _superListe->_first;
        }else{
          prev->_next = newSE;
        }
    }
    if(minor == 0){
        //Ecriture
        fops.read = NULL;
        fops.write = write;
    }else if(minor == 1){
        //Lecture non-destructrice
        fops.read = readND;
        fops.write = NULL;
        _indice = _liste;
    }else{
        //Lecture destructrice
        fops.read = readD;
        fops.write = NULL;
    }

	return 0;
}


/**
 * Lecture destructrice
 **/
static ssize_t readD(struct file *f, char *buf, size_t size, loff_t *offset)
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
 * Lecture Non-destructrice
 **/
static ssize_t readND(struct file *f, char *buf, size_t size, loff_t *offset)
{
	int sizeToCopy;
	last_acces = LECTURE;
	printk(KERN_ALERT "Read called!\n");

	if(_indice != NULL){
		sizeToCopy = MIN(_indice->bufSize,size);
		if(copy_to_user(buf,_indice->text,sizeToCopy)==0){
			_indice = _indice->next;
		}
		else
			return -EFAULT;

		return sizeToCopy;
	}else{
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
    unregister_chrdev_region(dev,3);
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
