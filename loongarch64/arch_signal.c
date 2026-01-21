/**
 * @file    arch_signal.c
 * @brief   LoongArch64信号处理
 * @author  Intewell Team
 * @date    2025-01-21
 * @version 1.0
 *
 * @details 本文件实现LoongArch64信号处理功能
 *          - 信号上下文保存和恢复
 *          - 信号帧设置
 *          - 用户态信号处理
 *
 * @note MISRA-C:2012 合规
 *
 * @copyright Copyright (c) 2025 Intewell Team
 */
/************************头 文 件******************************/
#include <assert.h>
#include <cache.h>
#include <context.h>
#include <errno.h>
#include <process_signal.h>
#include <sigcontext.h>
#include <signal.h>
#include <syscall.h>
#include <ttos.h>
#include <ttosProcess.h>
#include <uaccess.h>
#include <util.h>
#include <arch-sigcontext.h>
#include <arch-uaccess.h>
#include <cpu-info.h>
#include <cpu-features.h>
#include <fpu.h>
#undef KLOG_TAG
#define KLOG_TAG "arch_signal"
#include <klog.h>
#include <ttosBase.h>
// 应该定义到 libk
#define SA_RESTORER  0x04000000
extern void restore_context(void *context);
extern void sigreturn_code(void);
#define IS_ALIGNED(x, a) (((x) & ((typeof(x))(a)-1)) == 0)
struct rt_sigframe {
	struct siginfo rs_info;
	struct __ucontext rs_uctx;
    // 下面2个是临时实现
    struct arch_context ctxt;
	struct loongarch_fpu fpu __attribute__((aligned(32)));
    unsigned int sig_return_code[2];
};
struct _ctx_layout {
	struct sctx_info *addr;
	unsigned int size;
};
struct extctx_layout {
	unsigned long size;
	unsigned int flags;
	struct _ctx_layout fpu;
	struct _ctx_layout lsx;
	struct _ctx_layout lasx;
	struct _ctx_layout lbt;
	struct _ctx_layout end;
};
void signal_setup_done(struct ksignal *ksig, int stepping)
{
    signal_delivered(ksig, stepping);
}
static void __user *get_ctx_through_ctxinfo(struct sctx_info *info)
{
	return (void __user *)((char *)info + sizeof(struct sctx_info));
}
static int copy_lsx_to_sigcontext(struct lsx_context __user *ctx)
{
	int i;
	int err = 0;
	uint64_t __user *regs	= (uint64_t *)&ctx->regs;
	uint64_t __user *fcc	= &ctx->fcc;
	uint32_t __user *fcsr	= &ctx->fcsr;
    pcb_t pcb = ttosProcessSelf();
	for (i = 0; i < 32; i++) {
		err |= __put_user(pcb->taskControlId->switchContext.fpu.fpr[i].val64[0],
				  &regs[2*i]);
		err |= __put_user(pcb->taskControlId->switchContext.fpu.fpr[i].val64[1],
				  &regs[2*i+1]);
	}
	err |= __put_user(pcb->taskControlId->switchContext.fpu.fcc, fcc);
	err |= __put_user(pcb->taskControlId->switchContext.fpu.fcsr, fcsr);
	return err;
}
static int copy_lsx_from_sigcontext(struct lsx_context __user *ctx)
{
	int i;
	int err = 0;
	u64 fpr_val;
	uint64_t __user *regs	= (uint64_t *)&ctx->regs;
	uint64_t __user *fcc	= &ctx->fcc;
	uint32_t __user *fcsr	= &ctx->fcsr;
    pcb_t pcb = ttosProcessSelf();
	for (i = 0; i < 32; i++) {
		err |= __get_user(fpr_val, &regs[2*i]);
        pcb->taskControlId->switchContext.fpu.fpr[i].val64[0] = fpr_val;
		err |= __get_user(fpr_val, &regs[2*i+1]);
		pcb->taskControlId->switchContext.fpu.fpr[i].val64[1] = fpr_val;
	}
	err |= __get_user(pcb->taskControlId->switchContext.fpu.fcc, fcc);
	err |= __get_user(pcb->taskControlId->switchContext.fpu.fcsr, fcsr);
	return err;
}
static int copy_lasx_to_sigcontext(struct lasx_context __user *ctx)
{
	int i;
	int err = 0;
	uint64_t __user *regs	= (uint64_t *)&ctx->regs;
	uint64_t __user *fcc	= &ctx->fcc;
	uint32_t __user *fcsr	= &ctx->fcsr;
    pcb_t pcb = ttosProcessSelf();
	for (i = 0; i < 32; i++) {
		err |= __put_user(pcb->taskControlId->switchContext.fpu.fpr[i].val64[0],
				  &regs[4*i]);
		err |= __put_user(pcb->taskControlId->switchContext.fpu.fpr[i].val64[1],
				  &regs[4*i+1]);
		err |= __put_user(pcb->taskControlId->switchContext.fpu.fpr[i].val64[2],
				  &regs[4*i+2]);
		err |= __put_user(pcb->taskControlId->switchContext.fpu.fpr[i].val64[3],
				  &regs[4*i+3]);
	}
	err |= __put_user(pcb->taskControlId->switchContext.fpu.fcc, fcc);
	err |= __put_user(pcb->taskControlId->switchContext.fpu.fcsr, fcsr);
	return err;
}
static int copy_lasx_from_sigcontext(struct lasx_context __user *ctx)
{
	int i;
	int err = 0;
	u64 fpr_val;
	uint64_t __user *regs	= (uint64_t *)&ctx->regs;
	uint64_t __user *fcc	= &ctx->fcc;
	uint32_t __user *fcsr	= &ctx->fcsr;
    pcb_t pcb = ttosProcessSelf();
	for (i = 0; i < 32; i++) {
		err |= __get_user(fpr_val, &regs[4*i]);
		pcb->taskControlId->switchContext.fpu.fpr[i].val64[0] = fpr_val;
		err |= __get_user(fpr_val, &regs[4*i+1]);
		pcb->taskControlId->switchContext.fpu.fpr[i].val64[1] = fpr_val;
		err |= __get_user(fpr_val, &regs[4*i+2]);
		pcb->taskControlId->switchContext.fpu.fpr[i].val64[2] = fpr_val;
		err |= __get_user(fpr_val, &regs[4*i+3]);
		pcb->taskControlId->switchContext.fpu.fpr[i].val64[3] = fpr_val;
	}
	err |= __get_user(pcb->taskControlId->switchContext.fpu.fcc, fcc);
	err |= __get_user(pcb->taskControlId->switchContext.fpu.fcsr, fcsr);
	return err;
}
#ifdef CONFIG_CPU_HAS_LBT
static int copy_lbt_to_sigcontext(struct lbt_context __user *ctx)
{
	int err = 0;
	uint64_t __user *regs	= (uint64_t *)&ctx->regs;
	uint32_t __user *eflags	= (uint32_t *)&ctx->eflags;
    pcb_t pcb = ttosProcessSelf();
	err |= __put_user(current->thread.lbt.scr0, &regs[0]);
	err |= __put_user(current->thread.lbt.scr1, &regs[1]);
	err |= __put_user(current->thread.lbt.scr2, &regs[2]);
	err |= __put_user(current->thread.lbt.scr3, &regs[3]);
	err |= __put_user(current->thread.lbt.eflags, eflags);
	return err;
}
static int copy_lbt_from_sigcontext(struct lbt_context __user *ctx)
{
	int err = 0;
	uint64_t __user *regs	= (uint64_t *)&ctx->regs;
	uint32_t __user *eflags	= (uint32_t *)&ctx->eflags;
	err |= __get_user(current->thread.lbt.scr0, &regs[0]);
	err |= __get_user(current->thread.lbt.scr1, &regs[1]);
	err |= __get_user(current->thread.lbt.scr2, &regs[2]);
	err |= __get_user(current->thread.lbt.scr3, &regs[3]);
	err |= __get_user(current->thread.lbt.eflags, eflags);
	return err;
}
static int copy_ftop_to_sigcontext(struct lbt_context __user *ctx)
{
	uint32_t  __user *ftop	= &ctx->ftop;
	return __put_user(current->thread.fpu.ftop, ftop);
}
static int copy_ftop_from_sigcontext(struct lbt_context __user *ctx)
{
	uint32_t  __user *ftop	= &ctx->ftop;
	return __get_user(current->thread.fpu.ftop, ftop);
}
#endif
/*
 * Wrappers for the assembly _{save,restore}_fp_context functions.
 */
int _save_fp_context(uint64_t* regs, uint64_t*fcc, uint32_t*fcsr);
static int save_hw_fpu_context(struct fpu_context __user *ctx)
{
	uint64_t __user *regs	= (uint64_t *)&ctx->regs;
	uint64_t __user *fcc	= &ctx->fcc;
	uint32_t __user *fcsr	= &ctx->fcsr;
	return _save_fp_context(regs, fcc, fcsr);
}
int _restore_fp_context(uint64_t* regs, uint64_t*fcc, uint32_t*fcsr);
static int restore_hw_fpu_context(struct fpu_context __user *ctx)
{
	uint64_t __user *regs	= (uint64_t *)&ctx->regs;
	uint64_t __user *fcc	= &ctx->fcc;
	uint32_t __user *fcsr	= &ctx->fcsr;
	return _restore_fp_context(regs, fcc, fcsr);
}
int _save_lsx_context(uint64_t* regs, uint64_t*fcc, uint32_t*fcsr);
static int save_hw_lsx_context(struct lsx_context __user *ctx)
{
	uint64_t __user *regs	= (uint64_t *)&ctx->regs;
	uint64_t __user *fcc	= &ctx->fcc;
	uint32_t __user *fcsr	= &ctx->fcsr;
	return _save_lsx_context(regs, fcc, fcsr);
}
int _restore_lsx_context(uint64_t* regs, uint64_t*fcc, uint32_t*fcsr);
static int restore_hw_lsx_context(struct lsx_context __user *ctx)
{
	uint64_t __user *regs	= (uint64_t *)&ctx->regs;
	uint64_t __user *fcc	= &ctx->fcc;
	uint32_t __user *fcsr	= &ctx->fcsr;
	return _restore_lsx_context(regs, fcc, fcsr);
}
int _save_lasx_context(uint64_t* regs, uint64_t*fcc, uint32_t*fcsr);
static int save_hw_lasx_context(struct lasx_context __user *ctx)
{
	uint64_t __user *regs	= (uint64_t *)&ctx->regs;
	uint64_t __user *fcc	= &ctx->fcc;
	uint32_t __user *fcsr	= &ctx->fcsr;
	return _save_lasx_context(regs, fcc, fcsr);
}
int _restore_lasx_context(uint64_t* regs, uint64_t*fcc, uint32_t*fcsr);
static int restore_hw_lasx_context(struct lasx_context __user *ctx)
{
	uint64_t __user *regs	= (uint64_t *)&ctx->regs;
	uint64_t __user *fcc	= &ctx->fcc;
	uint32_t __user *fcsr	= &ctx->fcsr;
	return _restore_lasx_context(regs, fcc, fcsr);
}
/*
 * Wrappers for the assembly _{save,restore}_lbt_context functions.
 */
#ifdef CONFIG_CPU_HAS_LBT
static int save_hw_lbt_context(struct lbt_context __user *ctx)
{
	uint64_t __user *regs	= (uint64_t *)&ctx->regs;
	uint32_t __user *eflags	= (uint32_t *)&ctx->eflags;
	return _save_lbt_context(regs, eflags);
}
static int restore_hw_lbt_context(struct lbt_context __user *ctx)
{
	uint64_t __user *regs	= (uint64_t *)&ctx->regs;
	uint32_t __user *eflags	= (uint32_t *)&ctx->eflags;
	return _restore_lbt_context(regs, eflags);
}
static int save_hw_ftop_context(struct lbt_context __user *ctx)
{
	uint32_t __user *ftop	= &ctx->ftop;
	return _save_ftop_context(ftop);
}
static int restore_hw_ftop_context(struct lbt_context __user *ctx)
{
	uint32_t __user *ftop	= &ctx->ftop;
	return _restore_ftop_context(ftop);
}
#endif
/*
 * Thread saved context copy to/from a signal context presumed to be on the
 * user stack, and therefore accessed with appropriate macros from uaccess.h.
 */
static int copy_fpu_to_sigcontext(struct fpu_context __user *ctx)
{
	int i;
	int err = 0;
	uint64_t __user *regs	= (uint64_t *)&ctx->regs;
	uint64_t __user *fcc	= &ctx->fcc;
	uint32_t __user *fcsr	= &ctx->fcsr;
    pcb_t pcb = ttosProcessSelf();
	for (i = 0; i < 32; i++) {
		err |= 
		    __put_user(pcb->taskControlId->switchContext.fpu.fpr[i].val64[0],
			       &regs[i]);
	}
	err |= __put_user(pcb->taskControlId->switchContext.fpu.fcc, fcc);
	err |= __put_user(pcb->taskControlId->switchContext.fpu.fcsr, fcsr);
	return err;
}
static int copy_fpu_from_sigcontext(struct fpu_context __user *ctx)
{
	int i;
	int err = 0;
	u64 fpr_val;
	uint64_t __user *regs	= (uint64_t *)&ctx->regs;
	uint64_t __user *fcc	= &ctx->fcc;
	uint32_t __user *fcsr	= &ctx->fcsr;
    pcb_t pcb = ttosProcessSelf();
	for (i = 0; i < 32; i++) {
		err |= __get_user(fpr_val, &regs[i]);
		pcb->taskControlId->switchContext.fpu.fpr[i].val64[0] = fpr_val;
	}
	err |= __get_user(pcb->taskControlId->switchContext.fpu.fcc, fcc);
	err |= __get_user(pcb->taskControlId->switchContext.fpu.fcsr, fcsr);
	return err;
}
static int fcsr_pending(unsigned int __user *fcsr)
{
	int err, sig = 0;
	unsigned int csr, enabled;
	err = __get_user(csr, fcsr);
	enabled = ((csr & FPU_CSR_ALL_E) << 24);
	/*
	 * If the signal handler set some FPU exceptions, clear it and
	 * send SIGFPE.
	 */
	if (csr & enabled) {
		csr &= ~enabled;
		err |= __put_user(csr, fcsr);
		sig = SIGFPE;
	}
	return err ?: sig;
}
/*
 * Helper routines
 */
static int protected_save_fpu_context(struct extctx_layout *extctx)
{
	int err = 0;
	struct sctx_info __user *info = extctx->fpu.addr;
	struct fpu_context __user *fpu_ctx = (struct fpu_context *)get_ctx_through_ctxinfo(info);
	uint64_t __user *regs	= (uint64_t *)&fpu_ctx->regs;
	uint64_t __user *fcc	= &fpu_ctx->fcc;
	uint32_t __user *fcsr	= &fpu_ctx->fcsr;
	while (1) {
		err = copy_fpu_to_sigcontext(fpu_ctx);
		err |= __put_user(FPU_CTX_MAGIC, &info->magic);
		err |= __put_user(extctx->fpu.size, &info->size);
		if (likely(!err))
			break;
		/* Touch the FPU context and try again */
		err = __put_user(0, &regs[0]) |
			__put_user(0, &regs[31]) |
			__put_user(0, fcc) |
			__put_user(0, fcsr);
		if (err)
			return err;	/* really bad sigcontext */
	}
	return err;
}
static int protected_restore_fpu_context(struct extctx_layout *extctx)
{
	int err = 0, sig = 0, tmp __attribute__((__unused__));
	struct sctx_info __user *info = extctx->fpu.addr;
	struct fpu_context __user *fpu_ctx = (struct fpu_context *)get_ctx_through_ctxinfo(info);
	uint64_t __user *regs	= (uint64_t *)&fpu_ctx->regs;
	uint64_t __user *fcc	= &fpu_ctx->fcc;
	uint32_t __user *fcsr	= &fpu_ctx->fcsr;
	err = sig = fcsr_pending(fcsr);
	if (err < 0)
		return err;
	while (1) {
			err = copy_fpu_from_sigcontext(fpu_ctx);
		if (likely(!err))
			break;
		/* Touch the FPU context and try again */
		err = __get_user(tmp, &regs[0]) |
			__get_user(tmp, &regs[31]) |
			__get_user(tmp, fcc) |
			__get_user(tmp, fcsr);
		if (err)
			break;	/* really bad sigcontext */
	}
	return err ?: sig;
}
static int protected_save_lsx_context(struct extctx_layout *extctx)
{
	int err = 0;
	struct sctx_info __user *info = extctx->lsx.addr;
	struct lsx_context __user *lsx_ctx = (struct lsx_context *)get_ctx_through_ctxinfo(info);
	uint64_t __user *regs	= (uint64_t *)&lsx_ctx->regs;
	uint64_t __user *fcc	= &lsx_ctx->fcc;
	uint32_t __user *fcsr	= &lsx_ctx->fcsr;
    pcb_t pcb = ttosProcessSelf();
	while (1) {
		if (is_lsx_enabled())
			err = save_hw_lsx_context(lsx_ctx);
		else {
            if (is_lsx_enabled())
				save_lsx(pcb->taskControlId);
			else
				save_fp(pcb->taskControlId);
			err = copy_lsx_to_sigcontext(lsx_ctx);
		}
		err |= __put_user(LSX_CTX_MAGIC, &info->magic);
		err |= __put_user(extctx->lsx.size, &info->size);
		if (likely(!err))
			break;
		/* Touch the LSX context and try again */
		err = __put_user(0, &regs[0]) |
			__put_user(0, &regs[32*2-1]) |
			__put_user(0, fcc) |
			__put_user(0, fcsr);
		if (err)
			return err;	/* really bad sigcontext */
	}
	return err;
}
static int protected_restore_lsx_context(struct extctx_layout *extctx)
{
	int err = 0, sig = 0, tmp __attribute__((__unused__));
	struct sctx_info __user *info = extctx->lsx.addr;
	struct lsx_context __user *lsx_ctx = (struct lsx_context *)get_ctx_through_ctxinfo(info);
	uint64_t __user *regs	= (uint64_t *)&lsx_ctx->regs;
	uint64_t __user *fcc	= &lsx_ctx->fcc;
	uint32_t __user *fcsr	= &lsx_ctx->fcsr;
    pcb_t pcb = ttosProcessSelf();
	err = sig = fcsr_pending(fcsr);
	if (err < 0)
		return err;
	while (1) {
		if (is_lsx_enabled())
			err = restore_hw_lsx_context(lsx_ctx);
		else {
			err = copy_lsx_from_sigcontext(lsx_ctx);
            if (is_lsx_enabled())
				restore_lsx(pcb->taskControlId);
			else
				restore_fp(pcb->taskControlId);
		}
		if (likely(!err))
			break;
		/* Touch the LSX context and try again */
		err = __get_user(tmp, &regs[0]) |
			__get_user(tmp, &regs[32*2-1]) |
			__get_user(tmp, fcc) |
			__get_user(tmp, fcsr);
		if (err)
			break;	/* really bad sigcontext */
	}
	return err ?: sig;
}
static int protected_save_lasx_context(struct extctx_layout *extctx)
{
	int err = 0;
	struct sctx_info __user *info = extctx->lasx.addr;
	struct lasx_context __user *lasx_ctx =
		(struct lasx_context *)get_ctx_through_ctxinfo(info);
	uint64_t __user *regs	= (uint64_t *)&lasx_ctx->regs;
	uint64_t __user *fcc	= &lasx_ctx->fcc;
	uint32_t __user *fcsr	= &lasx_ctx->fcsr;
    pcb_t pcb = ttosProcessSelf();
	while (1) {
		if (is_lasx_enabled())
			err = save_hw_lasx_context(lasx_ctx);
		else {
			if (is_lsx_enabled())
				save_lsx(pcb->taskControlId);
			else
				save_fp(pcb->taskControlId);
			err = copy_lasx_to_sigcontext(lasx_ctx);
		}
		err |= __put_user(LASX_CTX_MAGIC, &info->magic);
		err |= __put_user(extctx->lasx.size, &info->size);
		if (likely(!err))
			break;
		/* Touch the LASX context and try again */
		err = __put_user(0, &regs[0]) |
			__put_user(0, &regs[32*4-1]) |
			__put_user(0, fcc) |
			__put_user(0, fcsr);
		if (err)
			return err;	/* really bad sigcontext */
	}
	return err;
}
static int protected_restore_lasx_context(struct extctx_layout *extctx)
{
	int err = 0, sig = 0, tmp __attribute__((__unused__));
	struct sctx_info __user *info = extctx->lasx.addr;
	struct lasx_context __user *lasx_ctx =
		(struct lasx_context *)get_ctx_through_ctxinfo(info);
	uint64_t __user *regs	= (uint64_t *)&lasx_ctx->regs;
	uint64_t __user *fcc	= &lasx_ctx->fcc;
	uint32_t __user *fcsr	= &lasx_ctx->fcsr;
    pcb_t pcb = ttosProcessSelf();
	err = sig = fcsr_pending(fcsr);
	if (err < 0)
		return err;
	while (1) {
		if (is_lasx_enabled())
			err = restore_hw_lasx_context(lasx_ctx);
		else {
			err = copy_lasx_from_sigcontext(lasx_ctx);
			if (is_lsx_enabled())
				restore_lsx(pcb->taskControlId);
			else
				restore_fp(pcb->taskControlId);
		}
		if (likely(!err))
			break;
		/* Touch the LASX context and try again */
		err = __get_user(tmp, &regs[0]) |
			__get_user(tmp, &regs[32*4-1]) |
			__get_user(tmp, fcc) |
			__get_user(tmp, fcsr);
		if (err)
			break;	/* really bad sigcontext */
	}
	return err ?: sig;
}
static unsigned long extframe_alloc(struct extctx_layout *extctx,
				    struct _ctx_layout *layout,
				    size_t size, unsigned int align, unsigned long base)
{
	unsigned long new_base = base - size;
	new_base = round_down(new_base, (align < 16 ? 16 : align));
	new_base -= sizeof(struct sctx_info);
	layout->addr = (void *)new_base;
	layout->size = (unsigned int)(base - new_base);
	extctx->size += layout->size;
	return new_base;
}
static int setup_sigcontext(struct arch_context *regs, struct sigcontext __user *sc,
			    struct extctx_layout *extctx)
{
	int i, err = 0;
	struct sctx_info __user *info;
	err |= __put_user(regs->csr_era, &sc->sc_pc);
	err |= __put_user(extctx->flags, &sc->sc_flags);
	err |= __put_user(0, &sc->sc_regs[0]);
	for (i = 1; i < 32; i++)
		err |= __put_user(regs->regs[i], &sc->sc_regs[i]);
	if (extctx->lasx.addr)
		err |= protected_save_lasx_context(extctx);
	else if (extctx->lsx.addr)
		err |= protected_save_lsx_context(extctx);
	else if (extctx->fpu.addr)
		err |= protected_save_fpu_context(extctx);
#ifdef CONFIG_CPU_HAS_LBT
	if (extctx->lbt.addr)
		err |= protected_save_lbt_context(extctx);
#endif
	/* Set the "end" magic */
	info = (struct sctx_info *)extctx->end.addr;
	err |= __put_user(0, &info->magic);
	err |= __put_user(0, &info->size);
	return err;
}
static int parse_extcontext(struct sigcontext __user *sc, struct extctx_layout *extctx)
{
	int err = 0;
	unsigned int magic, size;
	struct sctx_info __user *info = (struct sctx_info __user *)&sc->sc_extcontext;
	while(1) {
		err |= __get_user(magic, &info->magic);
		err |= __get_user(size, &info->size);
		if (err)
			return err;
		switch (magic) {
		case 0: /* END */
			goto done;
		case FPU_CTX_MAGIC:
			if (size < (sizeof(struct sctx_info) +
				    sizeof(struct fpu_context)))
				goto invalid;
			extctx->fpu.addr = info;
			break;
		case LSX_CTX_MAGIC:
			if (size < (sizeof(struct sctx_info) +
				    sizeof(struct lsx_context)))
				goto invalid;
			extctx->lsx.addr = info;
			break;
		case LASX_CTX_MAGIC:
			if (size < (sizeof(struct sctx_info) +
				    sizeof(struct lasx_context)))
				goto invalid;
			extctx->lasx.addr = info;
			break;
		case LBT_CTX_MAGIC:
			if (size < (sizeof(struct sctx_info) +
				    sizeof(struct lbt_context)))
				goto invalid;
			extctx->lbt.addr = info;
			break;
		default:
			goto invalid;
		}
		info = (struct sctx_info *)((char *)info + size);
	}
done:
	return 0;
invalid:
	return -EINVAL;
}
static int restore_sigcontext(struct arch_context *regs, struct sigcontext __user *sc)
{
	int i, err = 0;
	struct extctx_layout extctx;
	memset(&extctx, 0, sizeof(struct extctx_layout));
	err = __get_user(extctx.flags, &sc->sc_flags);
	if (err)
		goto bad;
#if 0
 	err = parse_extcontext(sc, &extctx);
	if (err)
		goto bad;
#endif
	err |= __get_user(regs->csr_era, &sc->sc_pc);
	for (i = 1; i < 32; i++)
		err |= __get_user(regs->regs[i], &sc->sc_regs[i]);
#if 0
	if (extctx.lasx.addr)
		err |= protected_restore_lasx_context(&extctx);
	else if (extctx.lsx.addr)
		err |= protected_restore_lsx_context(&extctx);
	else if (extctx.fpu.addr)
		err |= protected_restore_fpu_context(&extctx);
#ifdef CONFIG_CPU_HAS_LBT
	if (extctx.lbt.addr)
		err |= protected_restore_lbt_context(&extctx);
#endif
#endif
bad:
	return err;              
}
static unsigned int handle_flags(void)
{
	unsigned int flags = 0;
	return flags;
}
static unsigned long setup_extcontext(struct extctx_layout *extctx, unsigned long sp)
{
	unsigned long new_sp = sp;
	memset(extctx, 0, sizeof(struct extctx_layout));
	extctx->flags = handle_flags();
	/* Grow down, alloc "end" context info first. */
	new_sp -= sizeof(struct sctx_info);
	extctx->end.addr = (void *)new_sp;
	extctx->end.size = (unsigned int)sizeof(struct sctx_info);
	extctx->size += extctx->end.size;
	if (extctx->flags & SC_USED_FP) 
	{
		if (cpu_has_lasx)
			new_sp = extframe_alloc(extctx, &extctx->lasx,
			  sizeof(struct lasx_context), LASX_CTX_ALIGN, new_sp);
		else if (cpu_has_lsx)
			new_sp = extframe_alloc(extctx, &extctx->lsx,
			  sizeof(struct lsx_context), LSX_CTX_ALIGN, new_sp);
		else if (cpu_has_fpu)
			new_sp = extframe_alloc(extctx, &extctx->fpu,
			  sizeof(struct fpu_context), FPU_CTX_ALIGN, new_sp);
	}
#ifdef CONFIG_CPU_HAS_LBT
	if (cpu_has_lbt) {
		new_sp = extframe_alloc(extctx, &extctx->lbt,
			  sizeof(struct lbt_context), LBT_CTX_ALIGN, new_sp);
	}
#endif
	return new_sp;
}
static void __user *get_sigframe(struct ksignal *ksig, struct arch_context *context,
				 struct extctx_layout *extctx)
{
	unsigned long sp;
	/* Default to using normal stack */
	sp = context->regs[3];
// printk("%s,%d sp = 0x%llx\n", __FUNCTION__, __LINE__, sp);
	/*
	 * If we are on the alternate signal stack and would overflow it, don't.
	 * Return an always-bogus address instead so we will die with SIGSEGV.
	 */
	if (on_sig_stack(sp) &&
	    !likely(on_sig_stack(sp - sizeof(struct rt_sigframe))))
		return (void __user __force *)(-1UL);
	sp = sigsp(sp, ksig);
// printk("%s,%d sp = 0x%llx\n", __FUNCTION__, __LINE__, sp);
	sp = round_down(sp, 16);
// printk("%s,%d sp = 0x%llx\n", __FUNCTION__, __LINE__, sp);
	sp = setup_extcontext(extctx, sp);
// printk("%s,%d sp = 0x%llx\n", __FUNCTION__, __LINE__, sp);
	sp -= sizeof(struct rt_sigframe);
// printk("%s,%d sp = 0x%llx\n", __FUNCTION__, __LINE__, sp);
	if (!IS_ALIGNED(sp, 16))
		KLOG_E("not aligned 16");
	return (void __user *)sp;
}
static void save_fpu(struct rt_sigframe *frame)
{
	if (cpu_has_fpu)
        _save_fp(&frame->fpu);
}
static void restore_fpu(struct rt_sigframe *frame)
{
	if (cpu_has_fpu)
        _restore_fp(&frame->fpu);
}
int rt_sigreturn(struct arch_context *context)
{
    int sig;
	process_sigset_t set;
	struct rt_sigframe __user *frame;
    frame = (struct rt_sigframe __user *)context->regs[3];
	if (!user_access_check(frame, sizeof(*frame), UACCESS_R))
    {
        goto badframe;
    }
	if (copy_from_user(&set, &frame->rs_uctx.uc_sigmask, sizeof(set)))
		goto badframe;
    set_current_blocked(&set);
	sig = restore_sigcontext(context, (struct sigcontext*)&frame->rs_uctx.uc_mcontext);
	//if (sig < 0)
	//	goto badframe; 
    //else if (sig)
    //    kernel_signal_kill(ttosProcessSelf()->taskControlId->tid, TO_THREAD, sig, SI_KERNEL, 0);
    /* TODO:jcai 下面将 zero(r0) 寄存器设置为0是何用意？是否应该是设置 a7(r11) 为0？*/
	context->regs[0] = 0; /* No syscall restarting */
	if (restore_altstack(&frame->rs_uctx.uc_stack,context->regs[3]))
		goto badframe;
memcpy(context, &frame->ctxt, sizeof(*context));
restore_fpu(frame);
	return context->regs[4];
badframe:
    KLOG_E("sigframe err, so stop the self");
    kernel_signal_kill(ttosProcessSelf()->taskControlId->tid, TO_THREAD, SIGSEGV, SI_KERNEL, 0);
    return 0;
}
static int setup_rt_frame(struct ksignal *ksig,
			  struct arch_context *context, process_sigset_t *set)
{
	int err = 0;
	struct extctx_layout extctx;
	struct rt_sigframe __user *frame;
	frame = get_sigframe(ksig, context, &extctx);
	if (!user_access_check(frame, sizeof(*frame) + extctx.size,UACCESS_R))
		return -EFAULT;
	/* Create siginfo.  */
	err |= copy_siginfo_to_user(&frame->rs_info, &ksig->info);
	/* Create the ucontext.	 */
	err |= __put_user(0, &frame->rs_uctx.uc_flags);
	err |= __put_user(NULL, &frame->rs_uctx.uc_link);
	__save_altstack(&frame->rs_uctx.uc_stack, context->regs[3]);
	err |= setup_sigcontext(context, (struct sigcontext*)&frame->rs_uctx.uc_mcontext, &extctx);
memcpy(&frame->ctxt, context, sizeof(*context));
save_fpu(frame);
	err |= copy_to_user(&frame->rs_uctx.uc_sigmask, set, sizeof(*set));
	if (err)
		return -EFAULT;
    void *sig_return = 0;
    if (ksig->ka.sa_flags & SA_RESTORER)
    {
        sig_return = ksig->ka.sa_restorer;
printk("[%s:%u] return addr set to %p\n", __func__, __LINE__, sig_return);
    }
    else
    {
        memcpy(frame->sig_return_code, sigreturn_code, sizeof(frame->sig_return_code));
        sig_return = frame->sig_return_code;
    }
	/*
	 * Arguments to signal handler:
	 *
	 *   a0 = signal number
	 *   a1 = pointer to siginfo
	 *   a2 = pointer to ucontext
	 *
	 * c0_era point to the signal handler, $r3 (sp) points to
	 * the struct rt_sigframe.
	 */
	context->regs[4] = ksig->sig;
	context->regs[5] = (unsigned long) &frame->rs_info;
	context->regs[6] = (unsigned long) &frame->rs_uctx;
	context->regs[3] = (unsigned long) frame;
	context->regs[1] = (unsigned long) sig_return;
	context->csr_era = (unsigned long) ksig->ka.__sa_handler.sa_handler;
	return 0;
}
static void handle_signal(struct ksignal *ksig, struct arch_context *context,bool in_syscall)
{
	int ret;
	process_sigset_t *oldset = sigmask_to_save();
#if 0
	/* Are we from a system call? */
	if (in_syscall) 
	{
		switch (context->regs[4]) 
		{
			case -ERESTART_RESTARTBLOCK:
			case -ERESTARTNOHAND:
				context->regs[4] = -EINTR;
				break;
			case -ERESTARTSYS:
				if (!(ksig->ka.sa_flags & SA_RESTART)) 
				{
					context->regs[4] = -EINTR;
					break;
				}
				__attribute__((__fallthrough__));
			case -ERESTARTNOINTR:
				context->regs[4] = context->orig_a0;
				context->csr_era -= 4;
		}
	}
#endif
// printk("%p(%s) [%s:%u] UN:%d signal %d, pc:%p\n",
//     ttosGetRunningTask(), ttosGetRunningTaskName(), __FUNCTION__, __LINE__,
//     context->regs[11], ksig->sig, ksig->ka.__sa_handler.sa_handler);
	setup_rt_frame(ksig, context, oldset);
	signal_delivered(ksig, 0);
	TTOS_TaskEnterUserHook(ttosProcessSelf()->taskControlId);
// printk("%s,%d\n", __FUNCTION__, __LINE__);
	//signal_setup_done(ksig, 0);
	restore_context(context);
}
int arch_do_signal(struct arch_context *context)
{
	struct ksignal ksig;
	bool exist_signal = false;
	bool is_in_syscall = in_syscall(context);
	//printk("%s,%d is_in_syscall = %d\n", __FUNCTION__, __LINE__, is_in_syscall);
	if(is_in_syscall)
	{
        /*
         * Avoid additional syscall restarting via ret_to_user.
         */
        signal_forget_syscall(context);
	}
    /*
     * Get the signal to deliver. When running under ptrace, at this point
     * the debugger may change all of our registers.
     */
    exist_signal = get_signal(&ksig);
	//printk("%s,%d exist_signal = %d\n", __FUNCTION__, __LINE__, exist_signal);
    if (exist_signal)
    {
        handle_signal(&ksig, context, is_in_syscall);
        assert(0);
    }
#if 1
	/* Are we from a system call? */
	if (in_syscall) 
	{
		switch (context->regs[4]) 
		{
			case -ERESTART_RESTARTBLOCK:
			case -ERESTARTNOHAND:
// printk("%p(%s) [%s:%u]: UN: %d, a0:%p, %p, a1: %p\n",
//     ttosGetRunningTask(), ttosGetRunningTaskName(), __func__, __LINE__,
//     context->regs[11], context->regs[4], context->orig_a0, context->regs[5]);
				context->regs[4] = -EINTR;
				break;
			case -ERESTARTSYS:
				if (!(ksig.ka.sa_flags & SA_RESTART)) 
				{
// printk("%p(%s) [%s:%u]: UN: %d, a0:%p, %p, a1: %p\n",
//     ttosGetRunningTask(), ttosGetRunningTaskName(), __func__, __LINE__,
//     context->regs[11], context->regs[4], context->orig_a0, context->regs[5]);
					context->regs[4] = -EINTR;
					break;
				}
// printk("%p(%s) [%s:%u]: UN: %d, a0:%p, %p, a1: %p\n",
//     ttosGetRunningTask(), ttosGetRunningTaskName(), __func__, __LINE__,
//     context->regs[11], context->regs[4], context->orig_a0, context->regs[5]);
                context->regs[4] = context->orig_a0;
                /* 用户空间是通过系统调用指令进入到系统调用异常的。
                csr_era 记录的是用户空间系统调用指令的地址。但是在调用本函数前，已经在 do_syscall() 中
                对 csr_era +4 了。
                为了让系统调用异常返回后再次执行系统调用 context->regs[11] ，所以要对 csr_era -4 ，
                这样当前系统调用异常返回后，就会再次执行系统调用指令。
                */
                context->csr_era -= 4;
                break;
			case -ERESTARTNOINTR:
// printk("%p(%s) [%s:%u]: UN: %d, a0:%p, %p, a1: %p\n",
//     ttosGetRunningTask(), ttosGetRunningTaskName(), __func__, __LINE__,
//     context->regs[11], context->regs[4], context->orig_a0, context->regs[5]);
				context->regs[4] = context->orig_a0;
				context->regs[11] = __NR_restart_syscall;
                /* 用户空间是通过系统调用指令进入到系统调用异常的。
                csr_era 记录的是用户空间系统调用指令的地址。但是在调用本函数前，已经在 do_syscall() 中
                对 csr_era +4 了。
                为了让系统调用异常返回后再次执行系统调用 context->regs[11] ，所以要对 csr_era -4 ，
                这样当前系统调用异常返回后，就会再次执行系统调用指令。
                */
				context->csr_era -= 4;
                break;
		}
	}
#else
	/* Are we from a system call? */
	if (is_in_syscall) 
	{
		switch (context->regs[4]) 
		{
			case -ERESTARTNOHAND:
			case -ERESTARTSYS:
			case -ERESTARTNOINTR:
// printk("%p(%s) [%s:%u]: UN: %d, a0:%p, %p, a1: %p\n",
//     ttosGetRunningTask(), ttosGetRunningTaskName(), __func__, __LINE__,
//     context->regs[11], context->regs[4], context->orig_a0, context->regs[5]);
				context->regs[4] = context->orig_a0;
                /* 用户空间是通过系统调用指令进入到系统调用异常的。
                csr_era 记录的是用户空间系统调用指令的地址。但是在调用本函数前，已经在 do_syscall() 中
                对 csr_era +4 了。
                为了让系统调用异常返回后再次执行系统调用 context->regs[11] ，所以要对 csr_era -4 ，
                这样当前系统调用异常返回后，就会再次执行系统调用指令。
                */
				context->csr_era -= 4;
				break;
			case -ERESTART_RESTARTBLOCK:
// printk("%p(%s) [%s:%u]: UN: %d, a0:%p, %p, a1: %p\n",
//     ttosGetRunningTask(), ttosGetRunningTaskName(), __func__, __LINE__,
//     context->regs[11], context->regs[4], context->orig_a0, context->regs[5]);
				context->regs[4] = context->orig_a0;
				context->regs[11] = __NR_restart_syscall;
                /* 用户空间是通过系统调用指令进入到系统调用异常的。
                csr_era 记录的是用户空间系统调用指令的地址。但是在调用本函数前，已经在 do_syscall() 中
                对 csr_era +4 了。
                为了让系统调用异常返回后再次执行系统调用 __NR_restart_syscall ，所以要对 csr_era -4 ，
                这样当前系统调用异常返回后，就会再次执行系统调用指令。
                */
				context->csr_era -= 4;
				break;
		}
	}
#endif
	/*
	 * If there's no signal to deliver, we just put the saved sigmask
	 * back
	 */
	restore_saved_sigmask();
	//printk("%s,%d end \n", __FUNCTION__, __LINE__);
}
