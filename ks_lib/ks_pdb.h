#ifndef KS_PDB_H
#define KS_PDB_H

#define KS_PDB_EXTENSION_IDENTIFIER "KS_PDB_EXTENSION"

enum {
  KS_PDB_EXTENSION_CHAR_NUM = 8,
  KS_PDB_EXTENSION_FLG_SHIFT = 8,
  KS_PDB_EXTENSION_BIT_WIDTH = 24
};

#define KS_PDB_ATOM_RESIDUE_END   (unsigned int)0x0001
#define KS_PDB_ATOM_CHAIN_END     (unsigned int)0x0002
#define KS_PDB_ATOM_AGGREGATE_END (unsigned int)0x0004
#define KS_PDB_ATOM_WATER         (unsigned int)0x0008
#define KS_PDB_ATOM_HETATM        (unsigned int)0x0010
#define KS_PDB_ATOM_RIGID_MOL     (unsigned int)0x0020
#define KS_PDB_ATOM_EXT           (unsigned int)0x0080
#define KS_PDB_ATOM_EXT_MAIN      (unsigned int)0x0100
#define KS_PDB_ATOM_EXT_PRIME     (unsigned int)0x0200
#define KS_PDB_ATOM_EXT_NO_TRANS  (unsigned int)0x1000
#define KS_PDB_ATOM_EXT_NO_ROT    (unsigned int)0x2000
#define KS_PDB_ATOM_EXT_SOLVENT   (unsigned int)0x4000

typedef struct KS_PDB_ATOM KS_PDB_ATOM;
struct KS_PDB_ATOM{
  unsigned int flags;
  int label;
  /*  char name[5];*/
  char name[64];
  char altloc;
  /*  char residue[4];*/
  char residue[64];
  char chain;
  int residue_label;
  char icode;
  float cd[3];
  float occupancy;
  float temp_factor;
  /*  char other[9];*/
  char other[64];
};

typedef struct KS_PDB_BUF KS_PDB_BUF;
struct KS_PDB_BUF{
  KS_PDB_ATOM **atom;
  int size;
  int num;
  int grow;
  KS_INT_LIST *add_bond;
  double *crystal_vector;
  char crystal_group[16];
};

typedef struct KS_AMBER_PARM_ATOM KS_AMBER_PARM_ATOM;
struct KS_AMBER_PARM_ATOM{
  char *atom;
  double mass;
  KS_AMBER_PARM_ATOM *next;
};
typedef struct KS_AMBER_PARM_BOND KS_AMBER_PARM_BOND;
struct KS_AMBER_PARM_BOND{
  char *atom[2];
  double erg;
  double bond;
  KS_AMBER_PARM_BOND *next;
};
typedef struct KS_AMBER_PARM_ANGLE KS_AMBER_PARM_ANGLE;
struct KS_AMBER_PARM_ANGLE{
  char *atom[3];
  double erg;
  double angle;
  KS_AMBER_PARM_ANGLE *next;
};
typedef struct KS_AMBER_PARM_TORSION KS_AMBER_PARM_TORSION;
struct KS_AMBER_PARM_TORSION{
  char *atom[4];
  double erg;
  double angle;
  int path,n;
  KS_AMBER_PARM_TORSION *next;
};
typedef struct KS_AMBER_PARM_LJ KS_AMBER_PARM_LJ;
struct KS_AMBER_PARM_LJ{
  char *atom;
  double sigma,epsilon;
  KS_AMBER_PARM_LJ *next;
};
typedef struct KS_AMBER_PARM KS_AMBER_PARM;
struct KS_AMBER_PARM{
  unsigned int label;
  char *file_name;
  KS_AMBER_PARM_ATOM *atom;
  KS_AMBER_PARM_BOND *bond;
  KS_AMBER_PARM_ANGLE *angle;
  KS_AMBER_PARM_TORSION *torsion;
  KS_AMBER_PARM_TORSION *improper;
  KS_CHAR_LIST *equiv_atom_n;
  KS_CHAR_LIST *equiv_atom_c;
  KS_AMBER_PARM_LJ *lj;
  KS_AMBER_PARM *next;
};

/*
0000 0
0001 1
0010 2
0011 3
0100 4
0101 5
0110 6
0111 7
1000 8
1001 9
1010 A
1011 B
1100 C
1101 D
1110 E
1111 F
*/

#define KS_AMBER_PREP_NORMAL 0x01
#define KS_AMBER_PREP_N_TERM 0x02
#define KS_AMBER_PREP_C_TERM 0x04

#define KS_AMBER_PREP_BLOCK_XYZ 0x01
#define KS_AMBER_PREP_BLOCK_INT 0x02

typedef struct KS_AMBER_PREP_ATOM KS_AMBER_PREP_ATOM;
struct KS_AMBER_PREP_ATOM{
  int label;
  char name[8];
  char type[8];
  char topological_type;
  int link_label[3];
  double bond;
  double angle;
  double torsion;
  double cd[3];
  double charge;
};
typedef struct KS_AMBER_PREP_BLOCK KS_AMBER_PREP_BLOCK;
struct KS_AMBER_PREP_BLOCK{
  char name[128];
  char type[8];
  unsigned int flags;
  KS_AMBER_PREP_ATOM *atom;
  int atom_num;
  KS_INT_LIST_LIST *loop;
  KS_INT_LIST_LIST *improper;
  KS_AMBER_PREP_BLOCK *next;
};
typedef struct KS_AMBER_PREP KS_AMBER_PREP;
struct KS_AMBER_PREP{
  int label;
  char *file_name;
  unsigned int flags;
  KS_AMBER_PREP_BLOCK *block;
  KS_AMBER_PREP *next;
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  KS_PDB_ATOM *ks_allocate_pdb_atom(void);
  void ks_free_pdb_atom(KS_PDB_ATOM *pa);
  BOOL ks_sscan_pdb_atom(const char *line, BOOL decnode_pdb_extension, KS_PDB_ATOM *pa);
  void ks_fprint_pdb_atom(FILE *fp, const KS_PDB_ATOM pa);
  void ks_sprint_pdb_atom(char *s, const KS_PDB_ATOM pa);
  void ks_fprint_pdb_ter(FILE *fp, const KS_PDB_ATOM pa);
  KS_AMBER_PARM *ks_new_amber_parm(char *file_name, KS_CHAR_LIST *search_path);
  KS_AMBER_PARM *ks_addend_amber_parm(KS_AMBER_PARM *listp, KS_AMBER_PARM *newp);
  int ks_count_amber_parm(KS_AMBER_PARM *listp);
  KS_AMBER_PARM *ks_read_amber_parm(char *parm_list_file_name, KS_CHAR_LIST *parm_search_path);
  void ks_free_amber_parm(KS_AMBER_PARM *listp);
  KS_AMBER_PREP *ks_new_amber_prep(char *file_name, KS_CHAR_LIST *search_path);
  KS_AMBER_PREP *ks_addend_amber_prep(KS_AMBER_PREP *listp, KS_AMBER_PREP *newp);
  int ks_count_amber_prep(KS_AMBER_PREP *listp);
  KS_AMBER_PREP *ks_read_amber_prep(char *prep_list_file_name, KS_CHAR_LIST *prep_search_path);
  void ks_free_amber_prep(KS_AMBER_PREP *listp);
  void ks_print_amber_parm(KS_AMBER_PARM *ap);
  void ks_print_amber_prep(KS_AMBER_PREP *prep);
  void ks_copy_pdb_atom(KS_PDB_ATOM *p0, const KS_PDB_ATOM *p1);
  KS_AMBER_PREP_BLOCK *ks_lookup_amber_prep_block(KS_AMBER_PREP_BLOCK* listp, char *type);
  void ks_free_amber_prep_block(KS_AMBER_PREP_BLOCK *listp);
  BOOL ks_get_amber_element_symbol(char *type, char element[3]);
  BOOL ks_is_pdb_water(char *residue);

  KS_PDB_BUF *ks_allocate_pdb_buf(int size, int grow);
  BOOL ks_grow_pdb_buf(KS_PDB_BUF *pdb);
  void ks_free_pdb_buf(KS_PDB_BUF *pb);
  BOOL ks_set_pdb_buf(char *file_name, double *offset, double *rotation, double *side, 
		      KS_PDB_BUF **pdb, BOOL pick_ATOM, BOOL pick_HETATM, BOOL no_rigid_mol_flg);
  /*
  BOOL ks_set_pdb_buf(char *file_name, double *offset, double *side, 
		      KS_PDB_BUF **pdb, char *pick_key);
  */
  void ks_analysis_pdb_buf(KS_PDB_BUF *pdb, int aggregate_requirement, BOOL no_rigid_mol_flg);
  void ks_clear_pdb_buf(KS_PDB_BUF *pdb);
  void ks_set_pdb_extension_bool(KS_PDB_ATOM *pa, int ext_type, BOOL flg);
  BOOL ks_fprint_pdb_header(FILE *fp, char *file_name, char *key);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
