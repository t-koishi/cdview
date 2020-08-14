
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "ks_std.h"
#include "ks_pdb.h"

KS_PDB_ATOM *ks_allocate_pdb_atom(void)
{
  int i;
  KS_PDB_ATOM *p;
  if((p = (KS_PDB_ATOM*)ks_malloc(sizeof(KS_PDB_ATOM),"ks_allocate_pdb_atom")) == NULL){
    ks_error_memory();
    return NULL;
  }
  p->flags = 0;
  p->label = 0;
  p->name[0] = '\0';
  p->altloc = ' ';
  p->residue[0] = '\0';
  p->chain = ' ';
  p->residue_label = 0;
  p->icode = ' ';
  for(i = 0; i < 3; i++)
    p->cd[i] = 0;
  p->occupancy = 0;
  p->temp_factor = 0;
  p->other[0] = '\0';
  return p;
}
void ks_free_pdb_atom(KS_PDB_ATOM *pa)
{
  ks_free(pa);
}
static void encode_pdb_ext(unsigned int flags, char ext[KS_PDB_EXTENSION_CHAR_NUM])
{
  int i;
  unsigned int ext_flg[KS_PDB_EXTENSION_CHAR_NUM];

  /*
  printf("%x MAIN %c PRIME %c NO_TRANS %c NO_ROT %c SOLVENT %c\n",flags,
	 flags&KS_PDB_ATOM_EXT_MAIN ? 'o':'x',
	 flags&KS_PDB_ATOM_EXT_PRIME ? 'o':'x',
	 flags&KS_PDB_ATOM_EXT_NO_TRANS ? 'o':'x',
	 flags&KS_PDB_ATOM_EXT_NO_ROT ? 'o':'x',
	 flags&KS_PDB_ATOM_EXT_SOLVENT ? 'o':'x');
  */
  for(i = 0; i < KS_PDB_EXTENSION_CHAR_NUM; i++){
    ext_flg[i] = 0;
  }
  flags = flags>>KS_PDB_EXTENSION_FLG_SHIFT;

  for(i = 0; i < KS_PDB_EXTENSION_CHAR_NUM; i++){
    ext_flg[i] = flags&7;
    /*    printf("%d %d\n",i,ext_flg[i]);*/
    flags = flags>>4;
  }
  for(i = 0; i < KS_PDB_EXTENSION_CHAR_NUM; i++){
    if((i == 3 || i == 7) && ext_flg[i] == 0){
      ext[i] = ' ';
    } else {
      sprintf(&ext[i],"%d",ext_flg[i]);
    }
  }
}
static void decode_pdb_ext(unsigned int *flags, const char *line)
{
  int i,j;
  int range[2][KS_RANGE] = {{4,7},{0,3}};
  const char *cp;
  unsigned int ext_flg[KS_PDB_EXTENSION_CHAR_NUM];

  cp = line;
  for(i = 0; i < 2; i++){
    cp = strstr(cp,".")-2;
    ks_assert(cp != NULL);
    /*    printf("'%s'\n",cp);*/
    for(j = range[i][KS_RANGE_MAX]; j >= range[i][KS_RANGE_MIN]; cp++){
      /*      printf("%d %c %p\n",j,*cp,cp);*/
      if(*cp != '.'){
	if(isdigit(*(unsigned char*)cp)){
	  ext_flg[j] = (*cp)-'0';
	} else {
	  ext_flg[j] = 0;
	}
	j--;
      }
    }
  }
  /*
  printf("%d %d %d %d\n",ext_flg[7],ext_flg[6],ext_flg[5],ext_flg[4]);
  printf("%d %d %d %d\n",ext_flg[3],ext_flg[2],ext_flg[1],ext_flg[0]);
  */
  for(i = KS_PDB_EXTENSION_CHAR_NUM-1; i >= 0 ; i--){
    /*    (*flags) |= ext_flg[i]<<(i*3+KS_PDB_EXTENSION_FLG_SHIFT);*/
    (*flags) |= ext_flg[i]<<(i*4+KS_PDB_EXTENSION_FLG_SHIFT);
  }
  /*
  printf("MAIN %c PRIME %c NO_TRANS %c NO_ROT %c SOLVENT %c %x\n",
	 (*flags)&KS_PDB_ATOM_EXT_MAIN ? 'o':'x',
	 (*flags)&KS_PDB_ATOM_EXT_PRIME ? 'o':'x',
	 (*flags)&KS_PDB_ATOM_EXT_NO_TRANS ? 'o':'x',
	 (*flags)&KS_PDB_ATOM_EXT_NO_ROT ? 'o':'x',
	 (*flags)&KS_PDB_ATOM_EXT_SOLVENT ? 'o':'x',
	 *flags
	 );
  */
  /*  ks_exit(EXIT_FAILURE);
  */
}
BOOL ks_sscan_pdb_atom(const char *line, BOOL decnode_pdb_extension, KS_PDB_ATOM *pa)
{
  int i,j,c;
  char c0[10];
  char cb;

  /*          1         2         3         4         5         6         7         8 */
  /* 12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
  /* ATOM    145  N   VAL A  25      32.433  16.336  57.540  1.00 11.92      A1   N   */
  /* ATOM      1  N   MET A   1      28.443  21.126 101.631 1.000100.00 */
  /* ATOM      1  N   GLY     1      41.863  28.070  11.605  1.00  0.00 */
  /* H */
  /* 1-5,5   : recode name */
#if 1
  for(i = 0,c = 0; !isdigit(((unsigned char*)line)[i]); i++,c++);
  /* ?-11,6   : Atom serial number */
  for(i = 0; c < 11; i++)
    c0[i] = line[c++];
  c0[i] = '\0';
  pa->label = atoi(c0);
  /*
  printf("%d %s\n",c,c0);
  ks_exit(EXIT_FAILURE);
  */
#else
  for(i = 0,c = 0; i < 5; i++)
    c0[i] = line[c++];
  c0[i] = '\0';
  if(strncmp("ATOM",c0,4) != 0){
    ks_error("ks_read_pdb_atom: this line is not ATOM. %s\n",c0);
    return KS_FALSE;
  }
  /* 6-11,6   : Atom serial number */
  for(i = 0; i < 6; i++)
    c0[i] = line[c++];
  c0[i] = '\0';
  pa->label = atoi(c0);
#endif

  /* 12, 1   : blank */
  c++;

  /* 13-16,4   : Atom name */
  for(i = 0; i < 4; i++)
    c0[i] = line[c++];
  c0[i] = '\0';
  strcpy( pa->name,c0);
  if(!isalpha(((unsigned char*)pa->name)[0])){
    cb = pa->name[0];
    for(i = 1; pa->name[i] != ' ' && pa->name[i] != '\0'; i++){
      pa->name[i-1] = pa->name[i];
    }
    pa->name[--i] = cb;
  }

  /* 17, 1 : Alternate location indicator */
  pa->altloc = line[c++];

  /* 18-20,3   : Residue name */
  for(i = 0; i < 3; i++)
    pa->residue[i] = line[c++];
  pa->residue[i] = '\0';

  /* 21, 1   : blank */
  c++;

  if(isalpha(((unsigned char*)line)[c])){
    /* 22, 1 : Chain identifier */
    pa->chain = line[c++];

    /* 23-26,4   :  Residue sequence number */
    for(j = 0; j < 4; j++)
      c0[j] = line[c++];
    c0[j] = '\0';
    pa->residue_label = atoi(c0);
  } else {
    /* 22-26,5   :  Residue sequence number */
    for(j = 0; j < 5; j++)
      c0[j] = line[c++];
    c0[j] = '\0';
    pa->residue_label = atoi(c0);
  }

  /* 27, 1 : Code for insertion of residues */
  pa->icode = line[c++];
  
  /* 28-30,3   : blank */
  c += 3;

  /* 31-54,8,8,8 : x y z */
  for(i = 0; i < 3; i++){
    for(j = 0; j < 8; j++){
      c0[j] = line[c++];
    }
    c0[j] = '\0';
    pa->cd[i] = (float)atof(c0);
  }

  pa->flags = 0;
  if(decnode_pdb_extension == KS_TRUE){
    decode_pdb_ext(&pa->flags,&line[c]);
  }

  /* 55-60,6   : Occupancy */
  for(i = 0; i < 6; i++){
    c0[i] = line[c++];
  }
  c0[i] = '\0';
  pa->occupancy = (float)atof(c0);

  /* 61-66,6   : Temperature factor */
  for(i = 0; i < 6; i++){
    c0[i] = line[c++];
  }
  c0[i] = '\0';
  pa->temp_factor = (float)atof(c0);

  /* 67-72,6   : blank */
  for(i = 0; i < 6; i++)
    if(line[c++] == '\0'){
      pa->other[0] = '\0';
      goto KS_READ_PDB_ATOM_SKIP;
    }
  /* 73-80,8   : other */
  for(i = 0; i < 8; i++){
    pa->other[i] = line[c++];
  }
  pa->other[i] = '\0';

 KS_READ_PDB_ATOM_SKIP:
  ;
  /*
  printf("ATOM  %5d %4s%c%3s %c%4d%c   %8.3f%8.3f%8.3f%6.2f%6.2f%8s --\n"
	 ,pa->label,pa->name,pa->altloc,pa->residue,pa->chain,pa->residue_label,pa->icode
	 ,pa->cd[0],pa->cd[1],pa->cd[2],pa->occupancy,pa->temp_factor,pa->other);
  ks_exit(EXIT_FAILURE);
  */
  return KS_TRUE;
}
void ks_copy_pdb_atom(KS_PDB_ATOM *p0, const KS_PDB_ATOM *p1)
{
  int i;
  p0->flags = p1->flags;
  p0->label = p1->label;
  strcpy(p0->name,p1->name);
  p0->altloc = p1->altloc;
  strcpy(p0->residue,p1->residue);
  p0->chain =  p1->chain;
  p0->residue_label =  p1->residue_label;
  p0->icode =  p1->icode;
  for(i = 0; i < 3; i++)
    p0->cd[i] = p1->cd[i];
  p0->occupancy = p1->occupancy;
  p0->temp_factor = p1->temp_factor;
  strcpy(p0->other,p1->other);
}
void ks_fprint_pdb_atom(FILE *fp, const KS_PDB_ATOM pa)
{
  int i, len;
  char name[5],c;

  if(pa.name[0] == ' ' || isdigit(((unsigned char*)pa.name)[0])){
    strcpy(name,pa.name);
  } else {
    len = (int)strlen(pa.name);
    for(i = 0; i < 4; i++){
      if(i < len){
	name[i] = pa.name[i];
      } else {
	name[i] = ' ';
      }
    }
    name[4] = '\0';

    if(!isalpha(((unsigned char*)name)[3])){
      c = name[3];
      name[3] = name[2];
      name[2] = name[1];
      name[1] = name[0];
      name[0] = c;
    }
  }

  if(pa.flags&KS_PDB_ATOM_HETATM){
    fprintf(fp,"HETATM %4d %-4s%c%-3s",pa.label,name,pa.altloc,pa.residue);
  } else {
    fprintf(fp,"ATOM %6d %-4s%c%-3s",pa.label,name,pa.altloc,pa.residue);
  }
  if(pa.chain == ' '){
    fprintf(fp,"%6d",pa.residue_label);
  } else {
    fprintf(fp," %c%4d",pa.chain,pa.residue_label);
  }
  fprintf(fp,"%c   %8.3f%8.3f%8.3f"
	  ,pa.icode
	  ,pa.cd[0],pa.cd[1],pa.cd[2]);
  if(pa.flags&KS_PDB_ATOM_EXT){
    char ext[KS_PDB_EXTENSION_CHAR_NUM];
    encode_pdb_ext(pa.flags,ext);
    fprintf(fp," %c%c.%c%c %c%c.%c%c\n",ext[7],ext[6],ext[5],ext[4],ext[3],ext[2],ext[1],ext[0]);
  } else {
    fprintf(fp,"%6.2f%6.2f\n"
	    ,pa.occupancy,pa.temp_factor);
  }
}
void ks_sprint_pdb_atom(char *s, const KS_PDB_ATOM pa)
{
  char s1[80],s2[80],s3[80];
  snprintf(s1,sizeof(s1),"ATOM %6d %-4s%c%-3s",pa.label,pa.name,pa.altloc,pa.residue);
  if(pa.chain == ' '){
    sprintf(s2,"%6d",pa.residue_label);
  } else {
    sprintf(s2," %c%4d",pa.chain,pa.residue_label);
  }
  sprintf(s3,"%c   %8.3f%8.3f%8.3f%6.2f%6.2f"
	  ,pa.icode
	  ,pa.cd[0],pa.cd[1],pa.cd[2],pa.occupancy,pa.temp_factor);
  strcpy(s,s1);
  strcat(s,s2);
  strcat(s,s3);
  /*
  printf("%s\n",s1);
  printf("%s\n",s2);
  printf("%s\n",s3);
  printf("%s\n",s);
  exit(0);
  */
}
void ks_fprint_pdb_ter(FILE *fp, const KS_PDB_ATOM pa)
{
  fprintf(fp,"TER  %6d      %-3s",pa.label,pa.residue);
  if(pa.chain == ' '){
    fprintf(fp,"%6d\n",pa.residue_label);
  } else {
    fprintf(fp," %c%4d\n",pa.chain,pa.residue_label);
  }
}
static KS_AMBER_PARM_ATOM *new_amber_parm_atom(char *name, const double p)
{
  KS_AMBER_PARM_ATOM *newp;
  if((newp = (KS_AMBER_PARM_ATOM*)ks_malloc(sizeof(KS_AMBER_PARM_ATOM),"new_amber_parm_atom"))
     == NULL){
    ks_error("new_amber_parm_atom");
    return NULL;
  }
  if((newp->atom = ks_malloc_char_copy(name,"new_amber_parm_atom")) == NULL)
    return NULL;
  newp->mass = p;
  newp->next = NULL;
  return newp;
}
static KS_AMBER_PARM_ATOM *add_amber_parm_atom(KS_AMBER_PARM_ATOM *listp, KS_AMBER_PARM_ATOM *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
static KS_AMBER_PARM_ATOM *__KS_USED__ addend_amber_parm_atom(KS_AMBER_PARM_ATOM *listp, 
							      KS_AMBER_PARM_ATOM *newp)
{
  KS_AMBER_PARM_ATOM *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
static void free_amber_parm_atom(KS_AMBER_PARM_ATOM *listp)
{
  KS_AMBER_PARM_ATOM *next;
  for(; listp != NULL; listp = next){
    ks_free(listp->atom);
    next = listp->next;
    ks_free(listp);
  }
}
static KS_AMBER_PARM_BOND *new_amber_parm_bond(char *atom0, char *atom1, double erg, double bond)
{
  KS_AMBER_PARM_BOND *newp;

  if((newp = (KS_AMBER_PARM_BOND*)ks_malloc(sizeof(KS_AMBER_PARM_BOND),
					    "new_amber_parm_bond")) == NULL){
    return NULL;
  }
  if((newp->atom[0] = ks_malloc_char_copy(atom0,"new_amber_parm_bond")) == NULL)
    return NULL;
  if((newp->atom[1] = ks_malloc_char_copy(atom1,"new_amber_parm_bond")) == NULL)
    return NULL;
  newp->erg = erg;
  newp->bond = bond;
  newp->next = NULL;
  return newp;
}
static KS_AMBER_PARM_BOND *add_amber_parm_bond(KS_AMBER_PARM_BOND *listp, KS_AMBER_PARM_BOND *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
static KS_AMBER_PARM_BOND *__KS_USED__ addend_amber_parm_bond(KS_AMBER_PARM_BOND *listp, 
						  KS_AMBER_PARM_BOND *newp)
{
  KS_AMBER_PARM_BOND *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
static void free_amber_parm_bond(KS_AMBER_PARM_BOND *listp)
{
  KS_AMBER_PARM_BOND *next;
  for(; listp != NULL; listp = next){
    ks_free(listp->atom[0]);
    ks_free(listp->atom[1]);
    next = listp->next;
    ks_free(listp);
  }
}
static KS_AMBER_PARM_ANGLE *new_amber_parm_angle(char *atom0, char *atom1, char *atom2,
						 double erg, double angle)
{
  KS_AMBER_PARM_ANGLE *newp;

  if((newp = (KS_AMBER_PARM_ANGLE*)ks_malloc(sizeof(KS_AMBER_PARM_ANGLE),
					     "new_amber_parm_angle")) == NULL){
    return NULL;
  }

  if((newp->atom[0] = ks_malloc_char_copy(atom0,"new_amber_parm_angle")) == NULL)
    return NULL;
  if((newp->atom[1] = ks_malloc_char_copy(atom1,"new_amber_parm_angle")) == NULL)
    return NULL;
  if((newp->atom[2] = ks_malloc_char_copy(atom2,"new_amber_parm_angle")) == NULL)
    return NULL;
  newp->erg = erg;
  newp->angle = angle;
  newp->next = NULL;

  return newp;
}
static KS_AMBER_PARM_ANGLE *add_amber_parm_angle(KS_AMBER_PARM_ANGLE *listp, 
						 KS_AMBER_PARM_ANGLE *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
static KS_AMBER_PARM_ANGLE *__KS_USED__ addend_amber_parm_angle(KS_AMBER_PARM_ANGLE *listp, 
								KS_AMBER_PARM_ANGLE *newp)
{
  KS_AMBER_PARM_ANGLE *p;

  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;

  return listp;
}
static void free_amber_parm_angle(KS_AMBER_PARM_ANGLE *listp)
{
  int i;
  KS_AMBER_PARM_ANGLE *next;
  for(; listp != NULL; listp = next){
    for(i = 0; i < 3; i++)
      ks_free(listp->atom[i]);
    next = listp->next;
    ks_free(listp);
  }
}
static KS_AMBER_PARM_TORSION *new_amber_parm_torsion(char *atom0, char *atom1, char *atom2, 
						     char *atom3, int path, double erg, 
						     double angle, int n)
{
  KS_AMBER_PARM_TORSION *newp;

  if((newp = (KS_AMBER_PARM_TORSION*)ks_malloc(sizeof(KS_AMBER_PARM_TORSION),
					       "new_amber_parm_torsion")) == NULL){
    return NULL;
  }

  if((newp->atom[0] = ks_malloc_char_copy(atom0,"new_amber_parm_torsion")) == NULL)
    return NULL;
  if((newp->atom[1] = ks_malloc_char_copy(atom1,"new_amber_parm_torsion")) == NULL)
    return NULL;
  if((newp->atom[2] = ks_malloc_char_copy(atom2,"new_amber_parm_torsion")) == NULL)
    return NULL;
  if((newp->atom[3] = ks_malloc_char_copy(atom3,"new_amber_parm_torsion")) == NULL)
    return NULL;
  newp->erg = erg;
  newp->angle = angle;
  newp->path = path;
  newp->n = n;
  newp->next = NULL;

  return newp;
}
static KS_AMBER_PARM_TORSION *add_amber_parm_torsion(KS_AMBER_PARM_TORSION *listp, 
						     KS_AMBER_PARM_TORSION *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
static KS_AMBER_PARM_TORSION *__KS_USED__ addend_amber_parm_torsion(KS_AMBER_PARM_TORSION *listp, 
								    KS_AMBER_PARM_TORSION *newp)
{
  KS_AMBER_PARM_TORSION *p;

  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;

  return listp;
}
static void free_amber_parm_torsion(KS_AMBER_PARM_TORSION *listp)
{
  int i;
  KS_AMBER_PARM_TORSION *next;
  for(; listp != NULL; listp = next){
    for(i = 0; i < 4; i++)
      ks_free(listp->atom[i]);
    next = listp->next;
    ks_free(listp);
  }
}
static KS_AMBER_PARM_LJ *new_amber_parm_lj(char *name, double sigma, double epsilon)
{
  KS_AMBER_PARM_LJ *newp;

  if((newp = (KS_AMBER_PARM_LJ*)ks_malloc(sizeof(KS_AMBER_PARM_LJ),
					  "new_amber_parm_lj")) == NULL){
    return NULL;
  }

  if((newp->atom = ks_malloc_char_copy(name,"new_amber_parm_lj")) == NULL)
    return NULL;
  newp->sigma = sigma;
  newp->epsilon = epsilon;

  newp->next = NULL;

  return newp;
}
static KS_AMBER_PARM_LJ *add_amber_parm_lj(KS_AMBER_PARM_LJ *listp, KS_AMBER_PARM_LJ *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
static KS_AMBER_PARM_LJ *__KS_USED__ addend_amber_parm_lj(KS_AMBER_PARM_LJ *listp, 
							  KS_AMBER_PARM_LJ *newp)
{
  KS_AMBER_PARM_LJ *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
static void free_amber_parm_lj(KS_AMBER_PARM_LJ *listp)
{
  KS_AMBER_PARM_LJ *next;
  for(; listp != NULL; listp = next){
    ks_free(listp->atom);
    next = listp->next;
    ks_free(listp);
  }
}
static int count_hyphen_1st_column(char *s)
{
  int c = 0;
  for(; *s && *s != '.'; s++)
    if(*s == '-') c++;
  return c;
}
BOOL check_amber_parm(KS_AMBER_PARM *ap)
{
  BOOL ret = KS_TRUE;
  {
    KS_AMBER_PARM_ATOM *p,*q;
    for(p = ap->atom; p->next != NULL; p = p->next){
      for(q = p->next; q != NULL; q = q->next){
	if(strcmp(p->atom,q->atom) == 0){
	  printf("%s ATOM: %s %f\n",ap->file_name,p->atom,p->mass);
	  ret = KS_FALSE;
	}
      }
    }
  }
  {
    KS_AMBER_PARM_BOND *p,*q;
    for(p = ap->bond; p != NULL; p = p->next){
      for(q = p->next; q != NULL; q = q->next){
	if((strcmp(p->atom[0],q->atom[0]) == 0 && strcmp(p->atom[1],q->atom[1]) == 0) ||
	   (strcmp(p->atom[0],q->atom[1]) == 0 && strcmp(p->atom[1],q->atom[0]) == 0)){
	  printf("%s\nBOND: %s %s %f %f\nBOND: %s %s %f %f\n",
		 ap->file_name,
		 p->atom[0],p->atom[1],p->erg,p->bond,
		 q->atom[0],q->atom[1],q->erg,q->bond
		 );
	  ret = KS_FALSE;
	}
      }
    }
  }
  {
    KS_AMBER_PARM_ANGLE *p,*q;
    for(p = ap->angle; p != NULL; p = p->next){
      for(q = p->next; q != NULL; q = q->next){
	if((strcmp(p->atom[0],q->atom[0]) == 0 && 
	    strcmp(p->atom[1],q->atom[1]) == 0 && 
	    strcmp(p->atom[2],q->atom[2]) == 0) ||
	   (strcmp(p->atom[0],q->atom[2]) == 0 && 
	    strcmp(p->atom[1],q->atom[1]) == 0 && 
	    strcmp(p->atom[2],q->atom[0]) == 0)){
	  printf("%s\nANGLE: %s %s %s %f %f\nANGLE: %s %s %s %f %f\n"
		 ,ap->file_name
		 ,p->atom[0],p->atom[1],p->atom[2],p->erg,p->angle
		 ,q->atom[0],q->atom[1],q->atom[2],q->erg,q->angle
		 );
	  ret = KS_FALSE;
	}
      }
    }
  }
  {
    KS_AMBER_PARM_TORSION *p,*q;
    for(p = ap->torsion; p != NULL; p = p->next){
      for(q = p->next; q != NULL; q = q->next){
	if(((strcmp(p->atom[0],q->atom[0]) == 0 && 
	     strcmp(p->atom[1],q->atom[1]) == 0 && 
	     strcmp(p->atom[2],q->atom[2]) == 0 && 
	     strcmp(p->atom[3],q->atom[3]) == 0) ||
	    (strcmp(p->atom[0],q->atom[3]) == 0 && 
	     strcmp(p->atom[1],q->atom[2]) == 0 && 
	     strcmp(p->atom[2],q->atom[1]) == 0 && 
	     strcmp(p->atom[3],q->atom[0]) == 0)) && p->n == q->n){
	  printf("%s\nTORSION: %s %s %s %s %d %f %f % d\n"
		 "TORSION: %s %s %s %s %d %f %f % d\n",ap->file_name
		 ,p->atom[0],p->atom[1],p->atom[2],p->atom[3]
		 ,p->path,p->erg,p->angle,p->n
		 ,q->atom[0],q->atom[1],q->atom[2],q->atom[3]
		 ,q->path,q->erg,q->angle,q->n);
	  ret = KS_FALSE;
	}
      }
    }
  }
  {
    KS_AMBER_PARM_TORSION *p,*q;
    for(p = ap->improper; p != NULL; p = p->next){
      for(q = p->next; q != NULL; q = q->next){
	if((strcmp(p->atom[0],q->atom[0]) == 0 && 
	    strcmp(p->atom[1],q->atom[1]) == 0 && 
	    strcmp(p->atom[2],q->atom[2]) == 0 && 
	    strcmp(p->atom[3],q->atom[3]) == 0) ||

	   (strcmp(p->atom[0],q->atom[0]) == 0 && 
	    strcmp(p->atom[1],q->atom[3]) == 0 && 
	    strcmp(p->atom[2],q->atom[2]) == 0 && 
	    strcmp(p->atom[3],q->atom[1]) == 0) ||

	   (strcmp(p->atom[0],q->atom[1]) == 0 && 
	    strcmp(p->atom[1],q->atom[0]) == 0 && 
	    strcmp(p->atom[2],q->atom[2]) == 0 && 
	    strcmp(p->atom[2],q->atom[3]) == 0) ||

	   (strcmp(p->atom[0],q->atom[1]) == 0 &&
	    strcmp(p->atom[1],q->atom[3]) == 0 && 
	    strcmp(p->atom[2],q->atom[2]) == 0 && 
	    strcmp(p->atom[3],q->atom[0]) == 0) ||

	   (strcmp(p->atom[0],q->atom[3]) == 0 && 
	    strcmp(p->atom[1],q->atom[0]) == 0 && 
	    strcmp(p->atom[2],q->atom[2]) == 0 && 
	    strcmp(p->atom[3],q->atom[1]) == 0) ||

	   (strcmp(p->atom[0],q->atom[3]) == 0 && 
	    strcmp(p->atom[1],q->atom[1]) == 0 && 
	    strcmp(p->atom[2],q->atom[2]) == 0 && 
	    strcmp(p->atom[3],q->atom[0]) == 0)){
	  printf("%s\nIMPROPER TORSON: %s %s %s %s %d %f %f % d\n"
		 "IMPROPER TORSON: %s %s %s %s %d %f %f % d\n",ap->file_name
		 ,p->atom[0],p->atom[1],p->atom[2],p->atom[3]
		 ,p->path,p->erg,p->angle,p->n
		 ,q->atom[0],q->atom[1],q->atom[2],q->atom[3]
		 ,q->path,q->erg,q->angle,q->n);
	  ret = KS_FALSE;
	}
      }
    }
  }
  return ret;
}
static unsigned int _amber_parm_label = 0;
KS_AMBER_PARM *ks_new_amber_parm(char *file_name, KS_CHAR_LIST *search_path)
{
  int i;
  KS_AMBER_PARM *ap;
  FILE *fp;
  char read_buf[256];
  char cc[8][256];
  char *cp;
  int get_mode = 0; /* 0:atom 1:bond 2:angle 3:torsion 4:improper torsion 5:eqivalent atom 6:LJ */

  if((ap = (KS_AMBER_PARM*)ks_malloc(sizeof(KS_AMBER_PARM),"ap")) == NULL){
    return NULL;
  }
  if((ap->file_name = ks_malloc_char_copy(file_name,"ap->file_name")) == NULL){
    return NULL;
  }
  /*  printf("file_name '%s' '%s'\n",file_name,ap->file_name);*/
  ap->label = _amber_parm_label++;
  ap->atom = NULL;
  ap->bond = NULL;
  ap->angle = NULL;
  ap->torsion = NULL;
  ap->improper = NULL;
  ap->equiv_atom_n = NULL;
  ap->equiv_atom_c = NULL;
  ap->lj = NULL;
  ap->next = NULL;

  if((fp = ks_open_file(file_name,"rt",search_path)) == NULL)
    return NULL;

  while(fgets(read_buf,sizeof(read_buf),fp) != NULL){
    if(read_buf[0] == '#') continue;
    /*
    printf("%d %d %s",get_mode,count_hyphen_1st_column(read_buf),read_buf);
    */
    if(get_mode == 0){ /* ATOM */
      if(ks_isblank_all(read_buf) == KS_TRUE){
	/*
	{
	  KS_AMBER_PARM_ATOM *apa;
	  for(apa = ap->atom; apa != NULL; apa = apa->next){
	    printf("%s %f\n",apa->atom,apa->mass);
	  }
	}
	*/
	if(ap->atom != NULL)
	  get_mode++;
      } else {
	sscanf(read_buf,"%s %s",cc[0],cc[1]);
	if(ks_isfloat_all(cc[1]) == KS_TRUE){
	  /*
	  ap->atom = addend_amber_parm_atom(ap->atom,new_amber_parm_atom(cc[0],atof(cc[1])));
	  */
	  ap->atom = add_amber_parm_atom(ap->atom,new_amber_parm_atom(cc[0],atof(cc[1])));
	}
      }
    } else if(get_mode == 1){ /* bond */
      if(ks_isblank_all(read_buf) == KS_TRUE){
	/*
	{
	  KS_AMBER_PARM_BOND *p;
	  for(p = ap->bond; p != NULL; p = p->next){
	    printf("%s %s %f %f\n",p->atom[0],p->atom[1],p->erg,p->bond);
	  }
	}
	*/
	if(ap->bond != NULL)
	  get_mode++;
      } else {
	/*	printf("bond? %s %d\n",read_buf,count_hyphen_1st_column(read_buf));*/
	if(/*read_buf[2] == '-'*/count_hyphen_1st_column(read_buf) >= 1){
	  /*	  printf("%s",read_buf);*/
	  cp = read_buf;
	  for(i = 0; i < 4; i++)
	    cp = ks_get_str("- ",cp,cc[i],sizeof(cc[i]));
	  /*
	  cp = ks_get_chars(" -",cp,cc[0],sizeof(cc[0]));
	  cp = ks_skip_chars(" -",cp);
	  cp = ks_get_chars(" ",cp,cc[1],sizeof(cc[1]));
	  for(i = 0; i < 2; i++){
	    cp = ks_skip_chars(" ",cp);
	    cp = ks_get_chars(" ",cp,cc[i+2],sizeof(cc[i+2]));
	  }
	  */
	  /*	  printf("'%s' '%s' '%s' '%s'\n",cc[0],cc[1],cc[2],cc[3]);*/
	  /*
	  ap->bond = addend_amber_parm_bond(ap->bond,
					    new_amber_parm_bond(cc[0],cc[1],
								atof(cc[2]),atof(cc[3])));
	  */
	  ap->bond = add_amber_parm_bond(ap->bond,
					 new_amber_parm_bond(cc[0],cc[1],
							     atof(cc[2]),atof(cc[3])));
	}
      }
    } else if(get_mode == 2){ /* angle */
      if(ks_isblank_all(read_buf) == KS_TRUE){
	/*
	{
	  KS_AMBER_PARM_ANGLE *p;
	  for(p = ap->angle; p != NULL; p = p->next){
	    printf("%s %s %s %f %f\n",p->atom[0],p->atom[1],p->atom[2],p->erg,p->angle);
	  }
	}
	*/
	if(ap->angle != NULL)
	  get_mode++;
      } else {
	if(/*read_buf[2] == '-' && read_buf[5] == '-'*/count_hyphen_1st_column(read_buf) >= 2){
	  for(cp = read_buf, i = 0; i < 5; i++)
	    cp = ks_get_str("- ",cp,cc[i],sizeof(cc[i]));
	  /*
	  printf("'%s' '%s' '%s' '%s' '%s'\n",cc[0],cc[1],cc[2],cc[3],cc[4]);
	  */
	  /*
	  ap->angle = 
	    addend_amber_parm_angle(ap->angle,
				    new_amber_parm_angle(cc[0],cc[1],cc[2],
							 atof(cc[3]),atof(cc[4])));
	  */
	  ap->angle = 
	    add_amber_parm_angle(ap->angle,
				 new_amber_parm_angle(cc[0],cc[1],cc[2],
						      atof(cc[3]),atof(cc[4])));
	}
      }
    } else if(get_mode == 3){ /* torsion */
      if(ks_isblank_all(read_buf) == KS_TRUE){
	/*
	{
	  KS_AMBER_PARM_TORSION *p;
	  for(p = ap->torsion; p != NULL; p = p->next){
	    printf("%s %s %s %s %d %f %f % d\n",p->atom[0],p->atom[1],p->atom[2],p->atom[3]
		   ,p->path,p->erg,p->angle,p->n);
	  }
	}
	*/
	if(ap->torsion != NULL)
	  get_mode++;
      } else {
	if(/*read_buf[2] == '-' && read_buf[5] == '-' && read_buf[8] == '-'*/
	   count_hyphen_1st_column(read_buf) >= 3){
	  for(cp = read_buf, i = 0; i < 4; i++)
	    cp = ks_get_str("- ",cp,cc[i],sizeof(cc[i]));
	  for(i = 4; i < 8; i++)
	    cp = ks_get_str(" ",cp,cc[i],sizeof(cc[i]));
	  /*
	  printf("'%s' '%s' '%s' '%s' '%s' '%s' '%s' '%s'\n"
		 ,cc[0],cc[1],cc[2],cc[3],cc[4],cc[5],cc[6],cc[7]);
	  */
	  /*
	  ap->torsion = 
	    addend_amber_parm_torsion(ap->torsion,
				      new_amber_parm_torsion(cc[0],cc[1],cc[2],cc[3], atoi(cc[4]),
							     atof(cc[5]),atof(cc[6]),atoi(cc[7])));
	  */
	  ap->torsion = 
	    add_amber_parm_torsion(ap->torsion,
				   new_amber_parm_torsion(cc[0],cc[1],cc[2],cc[3], atoi(cc[4]),
							  atof(cc[5]),atof(cc[6]),atoi(cc[7])));
	}
      }
    } else if(get_mode == 4){ /* improper torsion */
      if(ks_isblank_all(read_buf) == KS_TRUE){
	/*
	{
	  KS_AMBER_PARM_TORSION *p;
	  for(p = ap->improper; p != NULL; p = p->next){
	    printf("%s %s %s %s %d %f %f % d\n",p->atom[0],p->atom[1],p->atom[2],p->atom[3]
		   ,p->path,p->erg,p->angle,p->n);
	  }
	}
	*/
	if(ap->improper != NULL)
	  get_mode++;
      } else {
	if(/*read_buf[2] == '-' && read_buf[5] == '-' && read_buf[8] == '-'*/
	   count_hyphen_1st_column(read_buf) >= 3){
	  for(cp = read_buf, i = 0; i < 7; i++)
	    cp = ks_get_str("- ",cp,cc[i],sizeof(cc[i]));
	  /*
	  printf("'%s' '%s' '%s' '%s' '%s' '%s' '%s'\n"
		 ,cc[0],cc[1],cc[2],cc[3],cc[4],cc[5],cc[6]);
	  */
	  /*
	  ap->improper = 
	    addend_amber_parm_torsion(ap->improper,
				      new_amber_parm_torsion(cc[0],cc[1],cc[2],cc[3], 2,
							     atof(cc[4]),atof(cc[5]),atoi(cc[6])));
	  */
	  if(strcmp(cc[0],"ZZ") == 0 && strcmp(cc[1],"ZZ") == 0 && 
	     strcmp(cc[2],"ZZ") == 0 && strcmp(cc[3],"ZZ") == 0){ // dummy to skip improper torsion
	    get_mode++;
	  } else {
	    ap->improper = 
	      add_amber_parm_torsion(ap->improper,
				     new_amber_parm_torsion(cc[0],cc[1],cc[2],cc[3], 2,
							    atof(cc[4]),atof(cc[5]),atoi(cc[6])));
	  }
	}
      }
    } else if(get_mode == 5){
      if(ks_isblank_all(read_buf) == KS_TRUE){
	if(ap->equiv_atom_n != NULL/* && ap->equiv_atom_c != NULL*/){
	  /*
	  {
	    KS_CHAR_LIST *p;
	    for(p = ap->equiv_atom_n; p != NULL; p = p->next){
	      printf("nlist %s\n",p->name);
	    }
	    for(p = ap->equiv_atom_c; p != NULL; p = p->next){
	      printf("clist %s\n",p->name);
	    }
	  }
	  */
	  get_mode++;
	}
      } else {
	if(read_buf[0] == 'N'){
	  for(cp = read_buf; cp != NULL;){
	    cp = ks_get_str(" ",cp,cc[0],sizeof(cc[0]));
	    /*
	    cp = ks_get_chars(" ",cp,cc[0],sizeof(cc[0]));
	    cp = ks_skip_chars(" ",cp);
	    */
	    /*	    printf("n_get '%s' '%s'\n",cc[0],cp);*/
	    ap->equiv_atom_n = 
	      ks_addend_char_list(ap->equiv_atom_n,ks_new_char_list(cc[0],0));
	    if(ks_isblank_all(cp) == KS_TRUE) break;
	  }
	} else if(read_buf[0] == 'C'){
	  for(cp = read_buf; cp != NULL;){
	    cp = ks_get_str(" ",cp,cc[0],sizeof(cc[0]));
	    /*
	    cp = ks_get_chars(" ",cp,cc[0],sizeof(cc[0]));
	    cp = ks_skip_chars(" ",cp);
	    */
	    /*	    printf("c_get '%s'\n",cc[0]);*/
	    ap->equiv_atom_c = 
	      ks_addend_char_list(ap->equiv_atom_c,ks_new_char_list(cc[0],0));
	    if(ks_isblank_all(cp) == KS_TRUE) break;
	  }
	}
      }
    } else {
      if(/*read_buf[2] == '-' && read_buf[5] == '-' && read_buf[8] == '-'*/
	 count_hyphen_1st_column(read_buf) >= 3){ /* additional torsion */
	get_mode = 3;
      } else {
	cc[0][0] = '\0'; cc[1][0] = '\0'; cc[2][0] = '\0';
	sscanf(read_buf,"%s %s %s",cc[0],cc[1],cc[2]);
	/*
	printf("'%s' '%s' '%s' %d %d\n",cc[0],cc[1],cc[2]
	       ,ks_isfloat_all(cc[1]),ks_isfloat_all(cc[2]));
	*/
	if(ks_isfloat_all(cc[1])== KS_TRUE && ks_isfloat_all(cc[2]) == KS_TRUE){
	  /*
	  printf("'%s' '%s' '%s' %d %d %s\n",cc[0],cc[1],cc[2]
		 ,ks_isfloat_all(cc[1]),ks_isfloat_all(cc[2]),file_name);
	  */
	  /*
	  ap->lj = addend_amber_parm_lj(ap->lj, new_amber_parm_lj(cc[0],
								  atof(cc[1])*2/pow(2,1./6.),
								  atof(cc[2])));
	  */
	  ap->lj = add_amber_parm_lj(ap->lj, new_amber_parm_lj(cc[0],
							       atof(cc[1])*2/pow(2,1./6.),
							       atof(cc[2])));
	}
      }
    }
  }

  fclose(fp);
  /*
  if(check_amber_parm(ap) == KS_FALSE){
  }
  */
  /*
  if(ap->lj == NULL){
    ks_error("%d atom %p bond %p angle %p torsion %p improper %p lj %p\n",get_mode
	     ,ap->atom,ap->bond,ap->angle,ap->torsion,ap->improper,ap->lj);
    ks_error("lj is null %s\n",file_name);
    return NULL;
  }
  */
  ks_assert(ap->lj != NULL);

  return ap;
}
KS_AMBER_PARM *ks_addend_amber_parm(KS_AMBER_PARM *listp, KS_AMBER_PARM *newp)
{
  KS_AMBER_PARM *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
int ks_count_amber_parm(KS_AMBER_PARM *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
void ks_free_amber_parm(KS_AMBER_PARM *listp)
{
  KS_AMBER_PARM *next;
  _amber_parm_label = 0;
  for(; listp != NULL; listp = next){
    ks_free(listp->file_name);
    free_amber_parm_atom(listp->atom);
    free_amber_parm_bond(listp->bond);
    free_amber_parm_angle(listp->angle);
    free_amber_parm_torsion(listp->torsion);
    free_amber_parm_torsion(listp->improper);
    free_amber_parm_lj(listp->lj);
    ks_free_char_list(listp->equiv_atom_n);
    ks_free_char_list(listp->equiv_atom_c);
    next = listp->next;
    ks_free(listp);
  }
}
KS_AMBER_PARM *ks_read_amber_parm(char *parm_list_file_name, KS_CHAR_LIST *parm_search_path)
{
  FILE *fp;
  char read_buf[256];
  KS_AMBER_PARM *new_parm = NULL;
  KS_AMBER_PARM *parm = NULL;

  if((fp = ks_open_file(parm_list_file_name,"rt",parm_search_path)) == NULL){
    return NULL;
  }
  while(fgets(read_buf,sizeof(read_buf),fp) != NULL){
    if(ks_isblank_all(read_buf) == KS_FALSE && read_buf[0] != '#'){
      /*	printf("%s\n",read_buf);*/
      ks_remove_return(read_buf);
      if((new_parm = ks_new_amber_parm(read_buf,NULL)) == NULL){
	ks_error("ks_read_amber_parm: file open error %s\n",read_buf);
	return NULL;
      }
      if(new_parm->lj == NULL){
	ks_error("ks_read_amber_parm: lj is null %s\n",read_buf);
	return NULL;
      }
      parm = ks_addend_amber_parm(parm,new_parm);
    }
  }
  fclose(fp);

  /*
  {
    KS_AMBER_PARM *pp;
    for(pp = parm; pp != NULL; pp = pp->next){
      printf("%s\n",pp->file_name);
    }
    ks_exit(EXIT_FAILURE);
  }
  */
  /*
  {
    KS_AMBER_PARM *pp;
    for(pp = parm; pp != NULL; pp = pp->next){
      ks_print_amber_parm(pp);
    }
    ks_exit(EXIT_FAILURE);
  }
  */
  return parm;
}
void ks_print_amber_parm(KS_AMBER_PARM *ap)
{
  printf("%d\n",ap->label);
  {
    KS_AMBER_PARM_ATOM *p;
    for(p = ap->atom; p != NULL; p = p->next){
      printf("%s ATOM: %s %f\n",ap->file_name,p->atom,p->mass);
    }
  }
  {
    KS_AMBER_PARM_BOND *p;
    for(p = ap->bond; p != NULL; p = p->next){
      printf("%s BOND: %s %s %f %f\n",ap->file_name,p->atom[0],p->atom[1],p->erg,p->bond);
    }
  }
  {
    KS_AMBER_PARM_ANGLE *p;
    for(p = ap->angle; p != NULL; p = p->next){
      printf("%s ANGLE: %s %s %s %f %f\n"
	     ,ap->file_name,p->atom[0],p->atom[1],p->atom[2],p->erg,p->angle);
    }
  }
  {
    KS_AMBER_PARM_TORSION *p;
    for(p = ap->torsion; p != NULL; p = p->next){
      printf("%s TORSION: %s %s %s %s %d %f %f % d\n",ap->file_name
	     ,p->atom[0],p->atom[1],p->atom[2],p->atom[3]
	     ,p->path,p->erg,p->angle,p->n);
    }
  }
  {
    KS_AMBER_PARM_TORSION *p;
    for(p = ap->improper; p != NULL; p = p->next){
      printf("%s IMPROPER TORSON: %s %s %s %s %d %f %f % d\n",ap->file_name
	     ,p->atom[0],p->atom[1],p->atom[2],p->atom[3]
	     ,p->path,p->erg,p->angle,p->n);
    }
  }
  {
    KS_CHAR_LIST *p;
    for(p = ap->equiv_atom_n; p != NULL; p = p->next){
      printf("%s N-LIST %s\n",ap->file_name,p->name);
    }
    for(p = ap->equiv_atom_c; p != NULL; p = p->next){
      printf("%s C-LIST %s\n",ap->file_name,p->name);
    }
  }
  {
    KS_AMBER_PARM_LJ *p;
    for(p = ap->lj; p != NULL; p = p->next){
      printf("%s LJ: %s %f %f\n",ap->file_name,p->atom,p->sigma,p->epsilon);
    }
  }
  /*
  {
    KS_CHAR_LIST *p;
    printf("%s N_list:",ap->file_name);
    for(p = ap->equiv_atom_n; p != NULL; p = p->next){
      printf(" %s",p->name);
    }
    printf("\n");
  }
  {
    KS_CHAR_LIST *p;
    printf("%s C_list:",ap->file_name);
    for(p = ap->equiv_atom_c; p != NULL; p = p->next){
      printf(" %s",p->name);
    }
    printf("\n");
  }
  */
}
static KS_AMBER_PREP_BLOCK *new_amber_prep_block(char **line, int num)
{
  int i,j,k;
  int column_num;
  char *cp;
  char cc[4][20];
  KS_INT_LIST *intl;
  KS_AMBER_PREP_BLOCK *newp = NULL;

  if((newp = (KS_AMBER_PREP_BLOCK*)ks_malloc(sizeof(KS_AMBER_PREP_BLOCK),
					     "KS_AMBER_PREP_BLOCK")) == NULL){
    return NULL;
  }
  newp->flags = 0;
  newp->improper = NULL;
  newp->loop = NULL;

  for(i = 0; i < num; i++){
    ks_remove_end_blank(line[i]);
    /*    printf("%d '%s'\n",i,line[i]);*/
  }
  if(strlen(line[0]) < 128){
    strcpy(newp->name,line[0]);
  } else {
    for(i = 0; i < 127; i++)
      newp->name[i] = line[0][i];
    newp->name[i] = '\0';
  }

  cp = line[2];
  /*  printf("%s",cp);*/
  /*
  cp = ks_skip_chars(" ",cp);
  cp = ks_get_chars(" ",cp,newp->type,sizeof(newp->type));
  cp = ks_skip_chars(" ",cp);
  */
  cp = ks_get_str(" ",cp,newp->type,sizeof(newp->type));
  if(strlen(newp->type) < 3)
    for(i = 0; i < 3-(int)strlen(newp->type); i++)
      strcat(newp->type," ");
  /*  printf("'%s'\n",newp->type);*/
  cp = ks_get_str(" ",cp,cc[0],sizeof(cc[0]));
  if(strncmp(cc[0],"INT",3) == 0){
    newp->flags |= KS_AMBER_PREP_BLOCK_INT;
  } else if(strncmp(cc[0],"XYZ",3) == 0){
    newp->flags |= KS_AMBER_PREP_BLOCK_XYZ;
  } else {
    ks_error("unknown coordination type '%s'",line[2]);
    return NULL;
  }
  /*
  printf("name: '%s'\n",newp->name);
  printf("type: '%s'\n",newp->type);
  printf("flags: %d\n",newp->flags);
  */
  newp->atom_num = 0;
  for(i = 5; ks_isblank_all(line[i]) == KS_FALSE; i++)
    newp->atom_num++;

  if((newp->atom=(KS_AMBER_PREP_ATOM*)ks_malloc(newp->atom_num*sizeof(KS_AMBER_PREP_ATOM),
						"newp"))
     ==NULL){
    return NULL;
  }
  /*  printf("%s\n",line[5]);*/
  for(cp = line[5], i = 0; (cp = ks_get_str(" ",cp,cc[0],sizeof(cc[0]))) != NULL; i++);
  column_num = i;
  if(column_num == 10){
    if(newp->flags&KS_AMBER_PREP_BLOCK_INT){
      j = 0;
      for(i = 0; i < 3; i++)
	newp->atom[i].charge = 0;
      for(i = 0; i < newp->atom_num; i++){
	/*      printf("%s\n",line[i+5]);*/
	sscanf(line[i+5],"%d %s %s %s %d %d %d %lf %lf %lf"
	       ,&newp->atom[i].label
	       ,newp->atom[i].name,newp->atom[i].type,&newp->atom[i].topological_type
	       ,&newp->atom[i].link_label[0]
	       ,&newp->atom[i].link_label[1]
	       ,&newp->atom[i].link_label[2]
	       ,&newp->atom[i].bond,&newp->atom[i].angle
	       ,&newp->atom[i].torsion);
	/*
	  printf("%d %d %-4s %-2s %c % d % d % d %f %f %f % f\n",j++
	  ,newp->atom[i].label
	  ,newp->atom[i].name,newp->atom[i].type,newp->atom[i].topological_type
	  ,newp->atom[i].link_label[0]
	  ,newp->atom[i].link_label[1]
	  ,newp->atom[i].link_label[2]
	  ,newp->atom[i].bond,newp->atom[i].angle
	  ,newp->atom[i].torsion,newp->atom[i].charge);
	*/
      }
    } else {
      ks_error("unknown prep format\n");
      ks_free(newp);
      return NULL;
    }
  } else if(column_num == 11){
    if(newp->flags&KS_AMBER_PREP_BLOCK_INT){
      for(i = 0; i < newp->atom_num; i++){
	/*      printf("%s\n",line[i+5]);*/
	sscanf(line[i+5],"%d %s %s %s %d %d %d %lf %lf %lf %lf"
	       ,&newp->atom[i].label
	       ,newp->atom[i].name,newp->atom[i].type,&newp->atom[i].topological_type
	       ,&newp->atom[i].link_label[0]
	       ,&newp->atom[i].link_label[1]
	       ,&newp->atom[i].link_label[2]
	       ,&newp->atom[i].bond,&newp->atom[i].angle
	       ,&newp->atom[i].torsion,&newp->atom[i].charge);
	/*
	  printf("%d %-4s %-2s %c % d % d % d %f %f %f % f\n"
	  ,newp->atom[i].label
	  ,newp->atom[i].name,newp->atom[i].type,newp->atom[i].topological_type
	  ,newp->atom[i].link_label[0]
	  ,newp->atom[i].link_label[1]
	  ,newp->atom[i].link_label[2]
	  ,newp->atom[i].bond,newp->atom[i].angle
	  ,newp->atom[i].torsion,newp->atom[i].charge);
	*/
      }
    } else {
      ks_error("unknown prep format\n");
      ks_free(newp);
      return NULL;
    }
  } else if(column_num == 8){
    if(newp->flags&KS_AMBER_PREP_BLOCK_XYZ){
      for(i = 0; i < newp->atom_num; i++){
	/*	printf("%s\n",line[i+5]);*/
	sscanf(line[i+5],"%d %s %s %s %lf %lf %lf %lf"
	       ,&newp->atom[i].label
	       ,newp->atom[i].name,newp->atom[i].type,&newp->atom[i].topological_type
	       ,&newp->atom[i].cd[0]
	       ,&newp->atom[i].cd[1]
	       ,&newp->atom[i].cd[2]
	       ,&newp->atom[i].charge);
	newp->atom[i].link_label[0] = -1;
	newp->atom[i].link_label[1] = -1;
	newp->atom[i].link_label[2] = -1;
	newp->atom[i].bond = 0;
	newp->atom[i].angle = 0;
	newp->atom[i].torsion = 0;
	/*
	printf("%d %-4s %-2s %c %f %f %f % f\n"
	       ,newp->atom[i].label
	       ,newp->atom[i].name,newp->atom[i].type,newp->atom[i].topological_type
	       ,newp->atom[i].cd[0]
	       ,newp->atom[i].cd[1]
	       ,newp->atom[i].cd[2]
	       ,newp->atom[i].charge);
	*/
      }
      /*      ks_exit(EXIT_FAILURE);*/
    } else {
      ks_error("unknown prep format\n");
      ks_free(newp);
      return NULL;
    }
  } else {
    ks_error("column number '%d' is not 10 nor 11\n",column_num);
    ks_free(newp);
    return NULL;
  }
  /*
  i = newp->atom_num;
  newp->atom[i].label = newp->atom[i-1].label+1;
  strcpy(newp->name,"DUMM");
  strcpy(newp->type,"DU");
  newp->atom[i].topological_type = 'M';
  newp->atom[i].link_label[0] = -1;
  newp->atom[i].link_label[1] = -1;
  newp->atom[i].link_label[2] = -1;
  newp->atom[i].bond = 0;
  newp->atom[i].angle = 0;
  newp->atom[i].torsion = 0;
  newp->atom[i].charge = 0;
  */
  for(i = newp->atom_num+5; i < num; i++){
    /*    printf("%d '%s'\n",i,line[i]);*/
    if(strncmp(line[i],"IMPROPER",8) == 0){
      for(i++;ks_isblank_all(line[i]) == KS_FALSE && i <num; i++){
	intl = NULL;
	if(line[i][0] != '#'){
	  /*	  printf("IMP %d '%s'\n",i,line[i]);*/
	  cp = line[i];
	  for(j = 0; j < 4; j++){
	    cp = ks_get_str(" ",cp,cc[j],sizeof(cc[j]));
	    /*	    printf("%d '%s'\n",j,cc[j]);*/
	    if(strcmp(cc[j],"-M") == 0){
	      intl = ks_addend_int_list(intl,ks_new_int_list(2));
	      /*	    printf("set -2\n");*/
	    } else if(strcmp(cc[j],"+M") == 0){
	      intl = ks_addend_int_list(intl,ks_new_int_list(newp->atom_num));
	      /*	    printf("set -1\n");*/
	    } else {
	      BOOL hit = KS_FALSE;
	      for(k = 0; k < newp->atom_num; k++){
		if(strcmp(newp->atom[k].name,cc[j]) == 0){
		  hit = KS_TRUE;
		  intl = ks_addend_int_list(intl,ks_new_int_list(k));
		  /*		printf("set %d\n",k);*/
		}		
	      }
	      if(hit == KS_FALSE){
		ks_error("improper: %s is not found",cc[j]);
		ks_error("%s\n",line[i]);
		return NULL;
	      }
	    }
	  }
	  newp->improper = ks_add_int_list_list(newp->improper,
						ks_new_int_list_list(intl));
	}
      }
      /*
      {
	KS_INT_LIST_LIST *p;
	for(p = newp->improper; p != NULL; p = p->next){
	  for(intl = p->p; intl != NULL; intl = intl->next){
	    printf("%d ",intl->val);
	  }
	  printf("\n");
	}
      }
      */
    } else if(strncmp(line[i],"LOOP",4) == 0){
      for(i++;ks_isblank_all(line[i]) == KS_FALSE && i <num; i++){
	intl = NULL;
	/*	printf("LOOP %d '%s'\n",i,line[i]);*/
	cp = line[i];
	for(j = 0; j < 2; j++){
	  cp = ks_get_str(" ",cp,cc[j],sizeof(cc[j]));
	  /*	  printf("%d '%s'\n",j,cc[j]);*/
	  for(k = 0; k < newp->atom_num; k++){
	    if(strcmp(newp->atom[k].name,cc[j]) == 0){
	      intl = ks_addend_int_list(intl,ks_new_int_list(k));
	      /*	      printf("set %d\n",k);*/
	    }
	  }
	}
	/*
	printf("intl %d\n",intl);
	{
	  KS_INT_LIST *pi;
	  for(pi = intl; pi != NULL; pi = pi->next)
	    printf("val %d\n",pi->val);
	}
	*/
	newp->loop = ks_add_int_list_list(newp->loop,ks_new_int_list_list(intl));
      }
      /*
      {
	KS_INT_LIST_LIST *p;
	for(p = newp->loop; p != NULL; p = p->next){
	  printf("%d ",p->p);
	  for(intl = p->p; intl != NULL; intl = intl->next){
	    printf("%d ",intl->val);
	  }
	  printf("\n");
	}
      }
      exit(0);
      */
    } else if(strncmp(line[i],"CHARGE",6) == 0){
      j = 0;
      for(i++;ks_isblank_all(line[i]) == KS_FALSE && i < num; i++){
	/*	printf("%s\n",line[i]);*/
	for(cp = line[i]; (cp = ks_get_str(" ",cp,cc[0],sizeof(cc[0]))) != NULL;){
	  /*	  printf("%d %s\n",j,cc[0]);*/
	  newp->atom[j+3].charge = atof(cc[0]);
	  j++;
	}
      }
    }
  }

  newp->next = NULL;
  return newp;
}
static KS_AMBER_PREP_BLOCK *add_amber_prep_block(KS_AMBER_PREP_BLOCK *listp,
						 KS_AMBER_PREP_BLOCK *newp)
{
  if(listp == NULL && newp == NULL)
    return NULL;
  newp->next = listp;
  return newp;
}
KS_AMBER_PREP_BLOCK *ks_lookup_amber_prep_block(KS_AMBER_PREP_BLOCK* listp, char *type)
{
  for(; listp != NULL; listp = listp->next){
    /*    printf("'%s' '%s'\n",listp->type,type);*/
    if(strcmp(listp->type,type) == 0)
      return listp;
  }
  return NULL;
}
void ks_free_amber_prep_block(KS_AMBER_PREP_BLOCK *listp)
{
  KS_AMBER_PREP_BLOCK *next;
  for(; listp != NULL; listp = next){
    ks_free(listp->atom);
    ks_free_int_list_list(listp->loop);
    ks_free_int_list_list(listp->improper);
    next = listp->next;
    ks_free(listp);
  }
}
KS_AMBER_PREP *ks_new_amber_prep(char *file_name, KS_CHAR_LIST *search_path)
{
  int i,j,k;
  KS_AMBER_PREP *p;
  FILE *fp;
  char read_buf[256];
  KS_BUF_CHAR_PP *block;
  KS_AMBER_PREP_BLOCK *new_prep_block;

  if((p = (KS_AMBER_PREP*)ks_malloc(sizeof(KS_AMBER_PREP),"KS_AMBER_PREP"))
     == NULL){
    return NULL;
  }
  if((p->file_name = ks_malloc_char_copy(file_name,"KS_AMBER_PREP")) == NULL){
    return NULL;
  }
  p->flags = 0;
  p->block = NULL;
  p->next = NULL;

  if((fp = ks_open_file(file_name,"rt",search_path)) == NULL)
    return p;
  /*  printf("file_name %s\n",file_name);*/

  if(fgets(read_buf,sizeof(read_buf),fp) == NULL){
    ks_error("prep file format error\n");
    return NULL;
  }
  sscanf(read_buf,"%d %d %d",&i,&j,&k);
  if(k == 2)
    p->flags |= KS_AMBER_PREP_NORMAL;
  else if(k == 200)
    p->flags |= KS_AMBER_PREP_N_TERM;
  else if(k == 201)
    p->flags |= KS_AMBER_PREP_C_TERM;
  else {
    ks_error("unknown prep type %d (2: All atom model  200: N-terminal  201:C-terminal\n",k);
    return NULL;
  }
  if(fgets(read_buf,sizeof(read_buf),fp) == NULL){
    ks_error("prep file format error\n");
    return NULL;
  }

  block = ks_allocate_buf_char_pp(1,2,256,"block");

  while(fgets(read_buf,sizeof(read_buf),fp) != NULL){
    /*    fprintf(stderr,"%s",read_buf);*/
    ks_remove_return(read_buf);
    ks_set_buf_char_pp(block,block->num++,read_buf);
    /*    printf("block %d '%s'\n",block->num,block->buf[block->num-1]);*/
    if(strncmp(read_buf,"DONE",4) == 0){
      if((new_prep_block = new_amber_prep_block(block->buf,block->num)) != NULL){
	if((p->block = add_amber_prep_block(p->block,new_prep_block))
	   == NULL){
	  ks_error("error in %s\n",file_name);
	}
      }
      block->num = 0;
    }
  }  
  fclose(fp);
  ks_free_buf_char_pp(block);
  return p;
}
KS_AMBER_PREP *ks_addend_amber_prep(KS_AMBER_PREP *listp, KS_AMBER_PREP *newp)
{
  KS_AMBER_PREP *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
int ks_count_amber_prep(KS_AMBER_PREP *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
void ks_free_amber_prep(KS_AMBER_PREP *listp)
{
  KS_AMBER_PREP *next;
  for(; listp != NULL; listp = next){
    ks_free(listp->file_name);
    ks_free_amber_prep_block(listp->block);
    next = listp->next;
    ks_free(listp);
  }
}
KS_AMBER_PREP *ks_read_amber_prep(char *prep_list_file_name, KS_CHAR_LIST *prep_search_path)
{
  FILE *fp;
  char read_buf[256];
  KS_AMBER_PREP *prep = NULL;

  if(prep == NULL){ /* read prep files */
    if((fp = ks_open_file(prep_list_file_name,"rt",prep_search_path)) == NULL){
      return NULL;
    }
    while(fgets(read_buf,sizeof(read_buf),fp) != NULL){
      if(ks_isblank_all(read_buf) == KS_FALSE && read_buf[0] != '#'){
	/*	printf("%s",read_buf);*/
	ks_remove_return(read_buf);
	prep = ks_addend_amber_prep(prep,ks_new_amber_prep(read_buf,NULL));
      }
    }
    fclose(fp);
    /*
    {
      KS_AMBER_PREP *pp;
      for(pp = prep; pp != NULL; pp = pp->next){
	ks_print_amber_prep(pp);
      }
      ks_exit(EXIT_FAILURE);
    }
    */
  }
  return prep;
}
void ks_print_amber_prep(KS_AMBER_PREP *prep)
{
  int i;
  KS_AMBER_PREP_BLOCK *pb;
  KS_INT_LIST_LIST *pii;
  KS_INT_LIST *pi;

  printf("file_name: %s\n",prep->file_name);
  if(prep->flags&KS_AMBER_PREP_NORMAL)      printf("type: normal\n");
  else if(prep->flags&KS_AMBER_PREP_N_TERM) printf("type: N-terminal\n");
  else if(prep->flags&KS_AMBER_PREP_C_TERM) printf("type: C-terminal\n");
  else  printf("type: UNKNOWN\n");
  for(pb = prep->block; pb != NULL; pb = pb->next){
    printf("\n");
    printf("  name: '%s'\n",pb->name);
    printf("  type: '%s'\n",pb->type);
    if(pb->flags&KS_AMBER_PREP_BLOCK_XYZ)      printf("  coordination tyep : XYZ\n");
    else if(pb->flags&KS_AMBER_PREP_BLOCK_INT) printf("  coordination tyep : INT\n");
    else printf("  coordination tyep : UNKNOWN\n");
    for(i = 0; i < pb->atom_num; i++){
      printf("    %3d %-4s %-2s %c % 4d % 4d % 4d  %5.3f  %6.2f  % 7.2f  % 7.4f\n"
	     ,pb->atom[i].label
	     ,pb->atom[i].name
	     ,pb->atom[i].type
	     ,pb->atom[i].topological_type
	     ,pb->atom[i].link_label[0],pb->atom[i].link_label[1],pb->atom[i].link_label[2]
	     ,pb->atom[i].bond
	     ,pb->atom[i].angle
	     ,pb->atom[i].torsion
	     ,pb->atom[i].charge);
    }
    for(pii = pb->improper; pii != NULL; pii = pii->next){
      printf("  improper: ");
      for(pi = pii->p; pi != NULL; pi = pi->next){
	if(pi->value == -2)
	  printf("-M   ");
	else if(pi->value == -1)
	  printf("+M   ");
	else
	  printf("%-4s ",pb->atom[pi->value].name);
      }
      for(pi = pii->p; pi != NULL; pi = pi->next){
	printf("% 3d ",pi->value);
      }
      printf("\n");
    }
    for(pii = pb->loop; pii != NULL; pii = pii->next){
      printf("  loop: ");
      for(pi = pii->p; pi != NULL; pi = pi->next){
	printf("%-4s ",pb->atom[pi->value].name);
       }
      for(pi = pii->p; pi != NULL; pi = pi->next){
	printf("% 3d ",pi->value);
      }
      printf("\n");
    }
  }
  printf("\n");
}
BOOL ks_get_amber_element_symbol(char *type, char element[3])
{
  int i,j;
  static char symbol[][20][3]={{"H" ,"HO","HS","HC","H1","H2","H3",
				"HP","HA","H4","H5","HW","HZ",""},
			       {"O" ,"O2","OW","OH","OS","O3",""},
			       {"C" ,"CA","CB","CC","CD","CK","CM",
				"CN","CQ","CR","CT","CV","CW","C*","CY","CZ",""},
			       {"N" ,"NY","N*","N2","N3","NA","NB","NC",""},
			       {"S" ,"SH",""},
			       {"P" ,""},
			       {"Cl","IM",""},
			       {"Li",""},
			       {"Na","IP",""},
			       {"K" ,""},
			       {"Rb",""},
			       {"Cs",""},
			       {"Mg","MG",""},
			       {"Ca","C0",""},
			       {"Zn",""},
			       {"F" ,""},
			       {"Br",""},
			       {"I" ,"IB",""},
			       {"LP",""},
			       {"Fe","FE",""},
			       {"Cu","CU",""},
			       {"DU",""},
			       {"L1",""},
			       {""}};

  for(i = 0; symbol[i][0][0] != '\0'; i++){
    for(j = 0; symbol[i][j][0] != '\0'; j++){
      /*      printf(" '%s'",symbol[i][j]);*/
      if(strcmp(symbol[i][j],type) == 0){
	strcpy(element,symbol[i][0]);
	return KS_TRUE;
	/*	return symbol[i][0];*/
      }
    }
    /*    printf("\n");*/
  }
  for(i = 0; symbol[i][0][0] != '\0'; i++){
    for(j = 0; symbol[i][j][0] != '\0'; j++){
      if(strncmp(symbol[i][j],type,1) == 0){
	strcpy(element,symbol[i][0]);
	return KS_TRUE;
	/*	return symbol[i][0];*/
      }
    }
  }
  return KS_FALSE;
}
BOOL ks_is_pdb_water(char *residue)
{
  if(strncmp(residue,"WAT",3) == 0 ||
     strncmp(residue,"H2O",3) == 0 ||
     strncmp(residue,"HOH",3) == 0 ||
     strncmp(residue,"TIP",3) == 0)
    return KS_TRUE;
  else 
    return KS_FALSE;
}
BOOL ks_is_rigid_mol(char *residue)
{
  if(strncmp(residue,"PF6",3) == 0 ||
     strncmp(residue,"NO3",3) == 0 ||
     strncmp(residue,"URE",3) == 0)
    return KS_TRUE;
  else 
    return KS_FALSE;
}
KS_PDB_BUF *ks_allocate_pdb_buf(int size, int grow)
{
  int i;
  KS_PDB_BUF *pb;

  if((pb = (KS_PDB_BUF*)ks_malloc(sizeof(KS_PDB_BUF),"KS_PDB_BUF")) == NULL){
    ks_error_memory();
    return NULL;
  }

  pb->num = 0;
  pb->size = size;
  pb->grow = grow;
  pb->add_bond = NULL;
  pb->crystal_vector = NULL;
  pb->crystal_group[0] = '\0';

  if((pb->atom = (KS_PDB_ATOM**)ks_malloc(pb->size*sizeof(KS_PDB_ATOM*),"pb->atom")) == NULL){
    ks_error_memory();
    return NULL;
  }
  for(i = 0; i < pb->size; i++){
    if((pb->atom[i] = ks_allocate_pdb_atom()) == NULL){
      ks_error("malloc_pdb_buf");
      return KS_FALSE;
    }
  }
  return pb;
}
BOOL ks_grow_pdb_buf(KS_PDB_BUF *pdb)
{
  int i;
  KS_PDB_ATOM **p;

  if((p = (KS_PDB_ATOM**)ks_malloc(pdb->size*pdb->grow*sizeof(KS_PDB_ATOM*),
				   "malloc_pdb_buf")) == NULL){
    return KS_FALSE;
  }
  for(i = 0; i < pdb->size*pdb->grow; i++){
    if((p[i] = ks_allocate_pdb_atom()) == NULL){
      ks_error("malloc_pdb_buf");
      return KS_FALSE;
    }
  }
  for(i = 0; i < pdb->size; i++){
    ks_copy_pdb_atom(p[i],pdb->atom[i]);
  }
  for(i = 0; i < pdb->size; i++){
    ks_free(pdb->atom[i]);
  }
  ks_free(pdb->atom);
  pdb->atom = p;
  pdb->size *= pdb->grow;
  return KS_TRUE;
}
static void set_pdb_buf(char *read_buf, KS_PDB_BUF *pdb, double *offset, double *side, 
			BOOL pdb_ext)
{
  int i;

  if(pdb->num >= pdb->size){
    /*
    {
      int i;
      printf("realloc_pdb_buf %d %d\n",pdb->num,pdb->size);
      for(i = 0; i < pdb->size; i++)
	printf("%d %p %p\n",i,pdb,pdb->atom[i]);
    }
    */
    ks_grow_pdb_buf(pdb);
  }
  if((ks_sscan_pdb_atom(read_buf,pdb_ext,pdb->atom[pdb->num])) == KS_TRUE){
    if(offset != NULL){
      for(i = 0; i < 3; i++)
	pdb->atom[pdb->num]->cd[i] += (float)offset[i];
    }
    if(side != NULL){
      for(i = 0; i < 3; i++){
	if(pdb->atom[pdb->num]->cd[i] < 0){
	  /*
	  printf("- %d %d %f %f\n",pdb->num,i,
		 pdb->atom[pdb->num]->cd[i],
		 pdb->atom[pdb->num]->cd[i]+side[i]);
	  */
	  pdb->atom[pdb->num]->cd[i] += side[i];
	}
	if(pdb->atom[pdb->num]->cd[i] >= side[i]){
	  /*
	  printf("+ %d %d %f %f\n",pdb->num,i,
		 pdb->atom[pdb->num]->cd[i],
		 pdb->atom[pdb->num]->cd[i]-side[i]);
	  */
	  pdb->atom[pdb->num]->cd[i] -= side[i];
	}
      }
    }
    pdb->num++;
  }
}
void ks_free_pdb_buf(KS_PDB_BUF *pb)
{
  int i;
  for(i = 0; i < pb->size; i++)
    ks_free(pb->atom[i]);
  ks_free(pb->atom);
  if(pb->add_bond != NULL){
    ks_free_int_list(pb->add_bond);
  }
  if(pb->crystal_vector != NULL){
    ks_free(pb->crystal_vector);
  }
  ks_free(pb);
}
static void set_quaternion(double phi, double theta, double psi, double q[4])
{
  q[0] = cos(theta/2)*cos((phi+psi)/2);
  q[1] = sin(theta/2)*cos((phi-psi)/2);
  q[2] = sin(theta/2)*sin((phi-psi)/2);
  q[3] = cos(theta/2)*sin((phi+psi)/2);
}
static void quaternion_rot(double *q, double *cd, double *tcd)
{
  tcd[0] = (cd[0]     *( q[0]*q[0]+q[1]*q[1]-q[2]*q[2]-q[3]*q[3])+
	    cd[1]*( 2)*( q[1]*q[2]-q[0]*q[3])+
	    cd[2]*( 2)*( q[1]*q[3]+q[0]*q[2]));
  tcd[1] = (cd[0]*( 2)*( q[1]*q[2]+q[0]*q[3])+
	    cd[1]     *( q[0]*q[0]-q[1]*q[1]+q[2]*q[2]-q[3]*q[3])+
	    cd[2]*( 2)*( q[2]*q[3]-q[0]*q[1]));
  tcd[2] = (cd[0]*( 2)*( q[1]*q[3]-q[0]*q[2])+
	    cd[1]*( 2)*( q[2]*q[3]+q[0]*q[1])+
	    cd[2]     *( q[0]*q[0]-q[1]*q[1]-q[2]*q[2]+q[3]*q[3]));
}
BOOL ks_set_pdb_buf(char *file_name, double *offset, double *rotation, double *side, 
		    KS_PDB_BUF **pdb, BOOL pick_ATOM, BOOL pick_HETATM, BOOL no_rigid_mol_flg)
{
  int i;
  FILE *fp;
  char read_buf[256];
  int num = 0;
  int bond[2];
  BOOL pdb_ext = KS_FALSE;
  /*
  size_t pick_key_len;
  */
  /*
  pick_key_len = strlen(pick_key);
  */
  if((fp = fopen(file_name,"rt")) == NULL){
    ks_error("file open error %s\n",file_name);
    return KS_FALSE;
  }

  /*
  while(fgets(read_buf,sizeof(read_buf),fp) != NULL){
    if(strncmp(read_buf,pick_key,pick_key_len) == 0){
      num++;
    }
  }
  */
  num = 1;

  if(*pdb == NULL){
    if((*pdb = ks_allocate_pdb_buf(num,2)) == NULL){
      return KS_FALSE;
    }
  }
  /*
  (*pdb)->file_name = 
    ks_add_char_list((*pdb)->file_name,ks_new_char_list(file_name,(*pdb)->num));
  (*pdb)->parm_label = 
    ks_add_char_list((*pdb)->parm_label,ks_new_char_list(file_name,parm_label));
  */
  rewind(fp);

  while(fgets(read_buf,sizeof(read_buf),fp) != NULL){
    if((strncmp(read_buf,"ATOM",4) == 0 && pick_ATOM) ||
       (strncmp(read_buf,"HETATM",6) == 0 && pick_HETATM)){
      /*      printf("%s",read_buf);*/
      set_pdb_buf(read_buf,(*pdb),offset,side,pdb_ext);
      if(ks_is_pdb_water((*pdb)->atom[(*pdb)->num-1]->residue))
	(*pdb)->atom[(*pdb)->num-1]->flags |= KS_PDB_ATOM_WATER;
      if(ks_is_rigid_mol((*pdb)->atom[(*pdb)->num-1]->residue) && no_rigid_mol_flg == KS_FALSE)
	(*pdb)->atom[(*pdb)->num-1]->flags |= KS_PDB_ATOM_RIGID_MOL;
      if(strncmp((*pdb)->atom[(*pdb)->num-1]->name,"OXT",3) == 0)
	(*pdb)->atom[(*pdb)->num-1]->flags |= KS_PDB_ATOM_CHAIN_END;
      if(strncmp(read_buf,"HETATM",6) == 0){
	(*pdb)->atom[(*pdb)->num-1]->flags |= KS_PDB_ATOM_HETATM;
      }
    } else if((*pdb)->num != 0 && 
	      (strncmp(read_buf,"TER",3) == 0 || strncmp(read_buf,"END",3) == 0)){
      (*pdb)->atom[(*pdb)->num-1]->flags |= KS_PDB_ATOM_CHAIN_END;
    } else if(strncmp(read_buf,"REMARK ADD_BOND",15) == 0){
      /*      printf("%s",read_buf);*/
      sscanf(read_buf,"REMARK ADD_BOND %d %d",&bond[0],&bond[1]);
      /*      printf("%d %d\n",bond[0],bond[1]);*/
      (*pdb)->add_bond = ks_add_int_list((*pdb)->add_bond,ks_new_int_list(bond[0]));
      (*pdb)->add_bond = ks_add_int_list((*pdb)->add_bond,ks_new_int_list(bond[1]));
    } else if(strncmp(read_buf,"REMARK",6) == 0){
      if(strstr(read_buf,KS_PDB_EXTENSION_IDENTIFIER) != NULL){
	pdb_ext = KS_TRUE;
      }
    } else if(strncmp(read_buf,"CRYST1",6) == 0 && (*pdb)->crystal_vector == NULL){
      int j,k;
      char c0[256];
      double d0,d1;
      double uv[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
      double len[3],ang[3];

      /*      printf("%s",read_buf);*/
      if(((*pdb)->crystal_vector = ks_malloc_double_p(9,"crystal_vector")) == NULL){
	ks_error_memory();
	return KS_FALSE;
      }
      /*
      sscanf(read_buf,"%s %lf %lf %lf %lf %lf %lf %10s",c0
	     ,&len[0],&len[1],&len[2]
	     ,&ang[0],&ang[1],&ang[2]
	     ,(*pdb)->crystal_group);
      */
      /*
Cols.  1-6    Record name "CRYST1"
      7-15    a (A*)
     16-24    b (A*)
     25-33    c (A*)
     34-40    alpha (deg)
     41-47    beta  (deg)
     48-54    gamma (deg)
     56-66    Space group symbol, left justified (not used)
     67-70    Z    (not used)
      */
      /*
	1234567890123456789012345678901234567890123456789012345678901234567890
	CRYST1    4.863   61.926   15.413  90.00  98.11  90.00 P 1 21 1      4          
      */
      /*      printf("%s",read_buf);*/
      i = 6;
      for(k = 0; k < 3; k++){
	for(j = 0; j < 9; i++,j++){
	  c0[j] = read_buf[i];
	  /*	  printf("%d %d %d %c\n",i,j,k,read_buf[i]);*/
	}
	c0[j] = '\0';
	len[k] = atof(c0);
	/*	printf("%d %f '%s'\n",k,len[k],c0);*/
      }
      for(k = 0; k < 3; k++){
	for(j = 0; j < 7; i++,j++){
	  c0[j] = read_buf[i];
	  /*	  printf("%d %d %d %c\n",i,j,k,read_buf[i]);*/
	}
	c0[j] = '\0';
	ang[k] = atof(c0);
	/*	printf("%d %f '%s'\n",k,ang[k],c0);*/
      }
      for(j = 0; j < 11; i++,j++){
	c0[j] = read_buf[i];
	/*	printf("%d %d %d %c\n",i,j,k,read_buf[i]);*/
      }
      c0[j] = '\0';
      /*      printf("%s\n",c0);*/
      for(i = 0, j = 0; c0[i]; i++){
	if(c0[i] != ' ' && c0[i] != '\n' && j < sizeof((*pdb)->crystal_group)){
	  (*pdb)->crystal_group[j++] = c0[i];
	}
      }
      /*
      printf("'%s'\n",(*pdb)->crystal_group);
      */
      for(i = 0; i < 3; i++){
	uv[i][0] *= len[0];
	uv[i][1] *= len[1];
	uv[i][2] *= len[2];
	ang[i] = ang[i]/180.*M_PI;
      }
      d0 = (cos(ang[0])-cos(ang[1])*cos(ang[2]))/sin(ang[2]);
      d1 = sqrt(pow(sin(ang[1]),2)-d0*d0);
      for(i = 0; i < 3; i++){
	(*pdb)->crystal_vector[i*3]   = uv[i][0]+uv[i][1]*cos(ang[2])+uv[i][2]*cos(ang[1]);
	(*pdb)->crystal_vector[i*3+1] = uv[i][1]*sin(ang[2]) + uv[i][2]*d0;
	(*pdb)->crystal_vector[i*3+2] = uv[i][2]*d1;
      }
    }
  }
  if((*pdb)->num != 0){
    (*pdb)->atom[(*pdb)->num-1]->flags |= KS_PDB_ATOM_RESIDUE_END;
    (*pdb)->atom[(*pdb)->num-1]->flags |= KS_PDB_ATOM_CHAIN_END;
    (*pdb)->atom[(*pdb)->num-1]->flags |= KS_PDB_ATOM_AGGREGATE_END;
  }
  /*  printf("%d\n",(*pdb)->num-1);*/
  fclose(fp);

  /*  printf("%d %d\n",(*pdb)->num, (*pdb)->size);*/

  if(rotation != NULL){
    int j;
    double center[3] = {0,0,0}, cd[3],tcd[3];
    double q[4];
    for(i = 0; i < (*pdb)->num; i++){
      /*      ks_fprint_pdb_atom(stdout,*(*pdb)->atom[i]);*/
      for(j = 0; j < 3; j++){
	center[j] += (*pdb)->atom[i]->cd[j];
      }
    }
    for(j = 0; j < 3; j++){
      center[j] /= (*pdb)->num;
    }
    /*    printf("%f %f %f\n",center[0],center[1],center[2]);*/
    set_quaternion(rotation[0]/180*M_PI,rotation[1]/180*M_PI,rotation[2]/180*M_PI,q);
    /*    printf("%f %f %f %f\n",q[0],q[1],q[2],q[3]);*/
    for(i = 0; i < (*pdb)->num; i++){
      for(j = 0; j < 3; j++){
	cd[j] = (*pdb)->atom[i]->cd[j] - center[j];
      }
      quaternion_rot(q,cd,tcd);
      for(j = 0; j < 3; j++){
	(*pdb)->atom[i]->cd[j] = tcd[j] + center[j];
      }
    }
  }

  return KS_TRUE;
}
void ks_analysis_pdb_buf(KS_PDB_BUF *pdb, int aggregate_requirement, BOOL no_rigid_mol_flg)
{
  int i;
  int residue_label0;
  int atom_num = 0;
  int residue_num = 0;
  int chain_num = 0;
  KS_CHAR_LIST *mol_list = NULL;
  KS_CHAR_LIST *agg_list = NULL;
  KS_CHAR_LIST *cp;

  if(pdb->num == 0) return;

  residue_label0 = pdb->atom[0]->residue_label;
  for(i = 1; i < pdb->num; i++){
    /*    ks_fprint_pdb_atom(stdout,*pdb->atom[i]);*/
    if(residue_label0 != pdb->atom[i]->residue_label || strncmp(pdb->atom[i-1]->name,"OXT",3)==0){
      /*      printf("residue\n");*/
      pdb->atom[i-1]->flags |= KS_PDB_ATOM_RESIDUE_END;
    }
    residue_label0 = pdb->atom[i]->residue_label;
    if(pdb->atom[i-1]->residue_label > pdb->atom[i]->residue_label || 
       strncmp(pdb->atom[i-1]->name,"OXT",3) == 0){
      /*      printf("chain\n");*/
      pdb->atom[i-1]->flags |= KS_PDB_ATOM_CHAIN_END;
      pdb->atom[i-1]->flags |= KS_PDB_ATOM_RESIDUE_END;
    }
    if(pdb->atom[i-1]->flags& KS_PDB_ATOM_CHAIN_END){
    }
    if(ks_is_pdb_water(pdb->atom[i-1]->residue) == KS_FALSE &&
       ks_is_pdb_water(pdb->atom[i]->residue) == KS_TRUE){
      pdb->atom[i-1]->flags |= KS_PDB_ATOM_AGGREGATE_END;
      pdb->atom[i-1]->flags |= KS_PDB_ATOM_CHAIN_END;
      pdb->atom[i-1]->flags |= KS_PDB_ATOM_RESIDUE_END;
    }
    if(ks_is_pdb_water(pdb->atom[i]->residue) == KS_TRUE){
      if(i == pdb->num-1 || pdb->atom[i]->residue_label != pdb->atom[i+1]->residue_label){
	pdb->atom[i]->flags |= KS_PDB_ATOM_CHAIN_END;
      }
    }
    if(ks_is_rigid_mol(pdb->atom[i-1]->residue) == KS_FALSE &&
       ks_is_rigid_mol(pdb->atom[i]->residue) == KS_TRUE && no_rigid_mol_flg == KS_FALSE){
      pdb->atom[i-1]->flags |= KS_PDB_ATOM_AGGREGATE_END;
      pdb->atom[i-1]->flags |= KS_PDB_ATOM_CHAIN_END;
      pdb->atom[i-1]->flags |= KS_PDB_ATOM_RESIDUE_END;
    }
  }

  residue_label0 = pdb->atom[0]->residue_label;
  for(i = 0; i < pdb->num; i++){
    /*    ks_fprint_pdb_atom(stdout,*pdb->atom[i]);*/
    atom_num++;
    if(pdb->atom[i]->flags&KS_PDB_ATOM_RESIDUE_END){
      /*      printf("%d %s ",i,pdb->atom[i]->residue);*/
      /*
      printf("    %d RESIDUE_END %d\n",residue_num,atom_num);
      */
      atom_num = 0;
      residue_num++;
    }
    if(pdb->atom[i]->flags&KS_PDB_ATOM_RESIDUE_END &&
       pdb->atom[i]->flags&KS_PDB_ATOM_CHAIN_END &&
       residue_num == 1){
      /*
      printf("--------------- %4d %-4s %-2s %d\n"
	     ,i,pdb->atom[i]->name,pdb->atom[i]->residue,residue_num);
      */
      cp = ks_lookup_char_list(mol_list,pdb->atom[i]->residue);
      if(cp == NULL){
	mol_list = ks_add_char_list(mol_list,ks_new_char_list(pdb->atom[i]->residue,1));
      } else {
	cp->value++;
      }
    }
    if(pdb->atom[i]->flags&KS_PDB_ATOM_CHAIN_END){
      /*
      printf("  %d CHAIN_END %d\n",chain_num,residue_num);
      */
      residue_num = 0;
      chain_num++;
    }
    if(pdb->atom[i]->flags&KS_PDB_ATOM_AGGREGATE_END){
      /*      printf("AGGREGATE_END %d\n",chain_num);*/
      chain_num  = 0;
    }
  }
  for(cp = mol_list; cp != NULL; cp = cp->next){
    if(cp->value > aggregate_requirement){
      agg_list = ks_add_char_list(agg_list,ks_new_char_list(cp->name,0));
    }
    /*    printf("%s %d\n",cp->name,cp->value);*/
  }
  /*
  for(cp = agg_list; cp != NULL; cp = cp->next){
    printf("%s %d\n",cp->name,cp->value);
  }
  */
  for(i = 0; i < pdb->num; i++){
    for(cp = agg_list; cp != NULL; cp = cp->next){
      if(i != 0){
	if(strcmp(pdb->atom[i-1]->residue,cp->name) != 0 && 
	   strcmp(pdb->atom[i]->residue,cp->name) == 0){
	  pdb->atom[i-1]->flags |= KS_PDB_ATOM_AGGREGATE_END;
	}
      }
      if(i != pdb->num-1){
	if(strcmp(pdb->atom[i]->residue,cp->name) == 0 && 
	   strcmp(pdb->atom[i+1]->residue,cp->name) != 0){
	  pdb->atom[i]->flags |= KS_PDB_ATOM_AGGREGATE_END;
	}
      }
    }
  }
  /*
  for(i = 0; i < pdb->num; i++){
    ks_fprint_pdb_atom(stdout,*pdb->atom[i]);
    atom_num++;
    if(pdb->atom[i]->flags&KS_PDB_ATOM_RESIDUE_END){
      printf("    %d RESIDUE_END %d\n",residue_num,atom_num);
      atom_num = 0;
      residue_num++;
    }
    if(pdb->atom[i]->flags&KS_PDB_ATOM_RESIDUE_END &&
       pdb->atom[i]->flags&KS_PDB_ATOM_CHAIN_END &&
       residue_num == 1){
      printf("---------------------- %4d %-4s %-2s %d\n"
	     ,i,pdb->atom[i]->name,pdb->atom[i]->residue,residue_num);
    }
    if(pdb->atom[i]->flags&KS_PDB_ATOM_CHAIN_END){
      printf("  %d CHAIN_END %d\n",chain_num,residue_num);
      residue_num = 0;
      chain_num++;
    }
    if(pdb->atom[i]->flags&KS_PDB_ATOM_AGGREGATE_END){
      printf("AGGREGATE_END %d\n",chain_num);
      chain_num  = 0;
    }
  }
  ks_exit(EXIT_FAILURE);
  */

  ks_free_char_list(mol_list);
  ks_free_char_list(agg_list);
}
void ks_clear_pdb_buf(KS_PDB_BUF *pdb)
{
  int i;
  for(i = 0; i < pdb->num; i++)
    pdb->atom[i]->flags = 0;
  pdb->num = 0;
  ks_free_int_list(pdb->add_bond);
  pdb->add_bond = NULL;
}
static void __KS_USED__ set_ext_flg(unsigned int *flags, unsigned int mask, BOOL set)
{
  *flags |= KS_PDB_ATOM_EXT;
  if(set == KS_TRUE){
    *flags |= mask;
  } else {
    *flags &= ~(mask);
  }
}
/*
void ks_set_pdb_extension_bool(KS_PDB_ATOM *pa, int ext_type, BOOL flg)
{
  set_ext_flg(&pa->flags,ks_pdb_ext_flags[ext_type].pdb_atom,flg);
}
BOOL ks_get_pdb_extension_bool(KS_PDB_ATOM *pa, int ext_type)
{
  if(pa->flags&ks_pdb_ext_flags[ext_type].pdb_atom){
    return KS_TRUE;
  } else {
    return KS_FALSE;
  }
}
*/
BOOL ks_fprint_pdb_header(FILE *fpout, char *file_name, char *key)
{
  char read_buf[256];
  FILE *fp;
  if((fp = fopen(file_name,"rt")) == NULL){
    ks_error("file open error %s\n",file_name);
    return KS_FALSE;
  }
  while(fgets(read_buf,sizeof(read_buf),fp) != NULL){
    if(strncmp(read_buf,key,strlen(key)) == 0){
      fprintf(fpout,"%s",read_buf);
    }
  }
  fclose(fp);
  return KS_TRUE;
}
