#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x4b3f3d8b, "module_layout" },
	{ 0x323354c2, "kobject_put" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xa28e0a9f, "kmalloc_caches" },
	{ 0xbc8d25a8, "param_ops_int" },
	{ 0x5021bd81, "_raw_write_lock_irqsave" },
	{ 0x263ed23b, "__x86_indirect_thunk_r12" },
	{ 0xdfc1a6f0, "device_destroy" },
	{ 0x144b53a6, "__register_chrdev" },
	{ 0x47c20f8a, "refcount_dec_not_one" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x999e8297, "vfree" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0xf4a05b50, "pv_ops" },
	{ 0x908cba63, "kobject_create_and_add" },
	{ 0xeb078aee, "_raw_write_unlock_irqrestore" },
	{ 0xfb384d37, "kasprintf" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x809b6564, "current_task" },
	{ 0xc5850110, "printk" },
	{ 0xbcab6ee6, "sscanf" },
	{ 0xa1c76e0a, "_cond_resched" },
	{ 0xf3faef38, "device_create" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x6a53b672, "module_put" },
	{ 0x40a9b349, "vzalloc" },
	{ 0xb1342cdb, "_raw_read_lock_irqsave" },
	{ 0x296695f, "refcount_warn_saturate" },
	{ 0xc959d152, "__stack_chk_fail" },
	{ 0x1000e51, "schedule" },
	{ 0x2ea2c95c, "__x86_indirect_thunk_rax" },
	{ 0xdf2ebb87, "_raw_read_unlock_irqrestore" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x6f4bca22, "kmem_cache_alloc_trace" },
	{ 0x3eeb2322, "__wake_up" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0xe01b5476, "kernel_kobj" },
	{ 0x37a0cba, "kfree" },
	{ 0x2385ed0e, "param_array_ops" },
	{ 0x96848186, "scnprintf" },
	{ 0xe08d94ab, "class_destroy" },
	{ 0x92540fbf, "finish_wait" },
	{ 0xa35cf151, "sysfs_create_file_ns" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x8ab6414a, "__class_create" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0xe72a06c3, "try_module_get" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "448FE888CEEAC4BEFA3A4CB");
