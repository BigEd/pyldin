enum Declare_Type {DT_DCB, DT_DCW, DT_DCD,
                   DT_DBB, DT_DBW, DT_DBD,
                   DT_DWD, DT_DFL, DT_DWL};

extern void a_swi(char *);
extern void a_mul(char *, int);				/* int is mul=0 mla=1 */
extern void a_datapro(char *, int);			/* int is datapro type */
extern void a_declare(char *, enum Declare_Type);	/* enum is declare type */
extern void a_branch(char *);
extern void a_mdtrans(char *, int);			/* int is 0 = stm, 1 = ldm */
extern void a_dtrans(char *, int);			/* int is 0 = store, 1 = load */
extern void a_adr(char *);
extern void a_bin(char *);
extern void a_dcf(char *);
extern void a_fpunary(char *, int);			/* int is type */
extern void a_fpbinry(char *, int);			/* int is type */
extern void a_fpcmp(char *, int);			/* int is i type */
extern void a_fpregtr(char *, int);			/* int is i type */
extern void a_fpdtran(char *, int);			/* int is 0 = store, 1 = load */
extern void a_cpswp(char *);
extern void a_cpdtran(char *, int);
extern void a_cpoper(char *, int);			/* 0=cdp, 1=mrc, 2=mcr */

extern char *labeloffset(char *, int *);
extern int getreg(void);
extern int getregfp(void);
extern int getregcp(void);
extern int getregshift(int);
extern int getfpconst(char *);
extern int getnum(void);
