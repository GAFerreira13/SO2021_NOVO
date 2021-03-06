#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0x8d0ae4f, "struct_module" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0xcc5005fe, "msleep_interruptible" },
	{ 0x2323fd40, "nonseekable_open" },
	{ 0x37a0cba, "kfree" },
	{ 0xf2a644fb, "copy_from_user" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x4292364c, "schedule" },
	{ 0x852abecf, "__request_region" },
	{ 0xb407b205, "ioport_resource" },
	{ 0xd85a396a, "cdev_add" },
	{ 0x2be5747e, "cdev_alloc" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0x4bd6e07e, "cdev_del" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x1b7d4074, "printk" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";

